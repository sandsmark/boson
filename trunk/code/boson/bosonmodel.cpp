/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosonmodel.h"

#include "defines.h"
#include "bosonmodeltextures.h"
#include "bosonprofiling.h"
#include "bo3dtools.h"
#include "bo3dsload.h"
#include "bosonglwidget.h"
#include "bodebug.h"
#include "bomesh.h"

#include <ksimpleconfig.h>
#include <kstaticdeleter.h>

#include <qimage.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qintdict.h>

// use GL_TRIANGLE_STRIP ? (experimental and not working!)
// AB: there are 2 different optimizing approaches. one is to use
// GL_TRIANGLE_STRIP, the other one is using a single glBegin()/glEnd() call (we
// need to place all textures of that model into a single big texture map and
// adjust the coordinates accordingly). unfortunately they exclude each other.
// so we need to be compare both approaches to find out which is better.
// i am expecting the strips to be faster
#define USE_STRIP 0

BosonModelTextures* BosonModel::mModelTextures = 0;
static KStaticDeleter<BosonModelTextures> sd;

BoFrame::BoFrame()
{
 init();
}

BoFrame::BoFrame(const BoFrame& f, int meshCount)
{
 init();
 mDisplayList = f.mDisplayList;
 mDepthMultiplier = f.mDepthMultiplier;
 mRadius = f.mRadius;

 if (meshCount == 0) {
	return;
 }
 if (f.mMeshCount == 0 || !f.mMeshes || !f.mMatrices) {
 }
 allocMeshes(meshCount);
 for (int i = 0; i < meshCount; i++) {
	mMeshes[i] = f.mMeshes[i]; // copy the pointer only
	mMatrices[i].loadMatrix(f.mMatrices[i]);
 }
}

void BoFrame::init()
{
 mDisplayList = 0;
 mDepthMultiplier = 0.0f;
 mRadius = 0.0;
 mMatrices = 0;
 mMeshes = 0;
 mMeshCount = 0;
}

BoFrame::~BoFrame()
{
 if (mDisplayList != 0) {
	glDeleteLists(mDisplayList, 1);
 }
 delete[] mMeshes;
 delete[] mMatrices;
}

void BoFrame::allocMeshes(int meshes)
{
 if (mMeshCount != 0) {
	boError() << k_funcinfo << "meshes already loaded" << endl;
	return;
 }
 if (mMatrices) {
	boError() << k_funcinfo << "matrices already allocated??" << endl;
	delete[] mMatrices;
 }
 if (mMeshes) {
	boError() << k_funcinfo << "meshes already allocated??" << endl;
	delete[] mMeshes;
 }
 mMeshes = new BoMesh*[meshes];
 mMatrices = new BoMatrix[meshes];
 mMeshCount = meshes; // unused?
 for (int i = 0; i < meshes; i++) {
	mMeshes[i] = 0;
 }
}

void BoFrame::setMesh(int index, BoMesh* mesh)
{
 if (index < 0 || index >= mMeshCount) {
	boError() << k_funcinfo << "invalid mesh " << mesh << " , count=" << mMeshCount << endl;
	return;
 }
 BO_CHECK_NULL_RET(mesh);
 BO_CHECK_NULL_RET(mMeshes);
 // we store the *pointer* only!
 mMeshes[index] = mesh;
}

BoMatrix* BoFrame::matrix(int index) const
{
 return &mMatrices[index];
}

class BosonModel::BoHelper
{
public:
	BoHelper()
	{
		mMaxX = 0.0;
		mMinX = 0.0;
		mMaxY = 0.0;
		mMinY = 0.0;
		mMaxZ = 0.0;
		mMinZ = 0.0;
	}
	
	void addPoint(const BoVector3& v)
	{
		addPoint(v[0], v[1], v[2]);
	}
	void addPoint(float x, float y, float z)
	{
		if (x > mMaxX) {
			mMaxX = x;
		} else if (x < mMinX) {
			mMinX = x;
		}
		if (y > mMaxY) {
			mMaxY = y;
		} else if (y < mMinY) {
			mMinY = y;
		}
		if (z > mMaxZ) {
			mMaxZ = z;
		} else if (z < mMinZ) {
			mMinZ = z;
		}
	}

	// radius of the bounding sphere
//	float radius() const
//	{
	//TODO
//	}

	// well in theory mMaxX == -mMinX
	// but usually..
	float diffX() const { return (mMinX - mMaxX); }
	float diffY() const { return (mMinY - mMaxY); }
	float diffZ() const { return (mMinZ - mMaxZ); }

	float lengthX() const { return (mMaxX - mMinX); }
	float lengthY() const { return (mMaxY - mMinY); }
	float lengthZ() const { return (mMaxZ - mMinZ); }

	float scale(float w, float h) const
	{
		float scaleX = w / lengthX();
		float scaleY = h / lengthY();
		// we don't care about z-size here!
		return QMIN(scaleX, scaleY);
	}



	float mMaxX;
	float mMinX; // lowest (i.e. negative) x
	float mMaxY;
	float mMinY;
	float mMaxZ;
	float mMinZ;
};


class BosonModel::Private
{
public:
	Private()
	{
		mLoader = 0;
	}

	QIntDict<BoMesh> mMeshes;
	QIntDict<BoFrame> mFrames;
	QMap<BoMesh*, QString> mTextures;

	QValueList<GLuint> mNodeDisplayLists;
	QIntDict<BoFrame> mConstructionSteps;
	QIntDict<BosonAnimation> mAnimations;
	QMap<QString, QString> mTextureNames;
	QString mDirectory;
	QString mFile;

	Bo3DSLoad* mLoader;
};

BosonModel::BosonModel(const QString& dir, const QString& file, float width, float height)
{
 init();
 d->mDirectory = dir;
 d->mFile = file;
 mWidth = width;
 mHeight = height;
}

void BosonModel::init()
{
 d = new Private;
 mTeamColor = 0;
 mWidth = 0;
 mHeight = 0;
 d->mMeshes.setAutoDelete(true);
 d->mFrames.setAutoDelete(true);
 d->mConstructionSteps.setAutoDelete(true);
 d->mAnimations.setAutoDelete(true);
 if (!mModelTextures) {
	mModelTextures = new BosonModelTextures();
	sd.setObject(mModelTextures);
 }

 // add the default mode 0
 insertAnimationMode(0, 0, 1, 1);
}

BosonModel::~BosonModel()
{
 boDebug(100) << k_funcinfo << endl;
 finishLoading();
 mModelTextures->removeModel(this);
 boDebug(100) << k_funcinfo << "delete " << d->mFrames.count() << " frames" << endl;
 d->mFrames.clear();
 boDebug(100) << k_funcinfo << "delete " << d->mConstructionSteps.count() << " construction frames" << endl;
 d->mConstructionSteps.clear();
 boDebug(100) << k_funcinfo << "delete " << d->mNodeDisplayLists.count() << " child display lists" << endl;
 QValueList<GLuint>::Iterator it = d->mNodeDisplayLists.begin();
 for (; it != d->mNodeDisplayLists.end(); ++it) {
	glDeleteLists((*it), 1);
 }
 d->mAnimations.clear();
 boDebug(100) << k_funcinfo << "delete meshes" << endl;
 d->mMeshes.clear();
 delete d;
 boDebug(100) << k_funcinfo << "done" << endl;
}

const QString& BosonModel::baseDirectory() const
{
 return d->mDirectory;
}

QString BosonModel::file() const
{
 return baseDirectory() + d->mFile;
}

void BosonModel::setLongNames(QMap<QString, QString> names)
{
 d->mTextureNames = names;
}

QString BosonModel::cleanTextureName(const char* name) const
{
 QString s = QString(name).lower();
 if (d->mTextureNames.contains(s)) {
	return d->mTextureNames[s];
 }
 return s;
}

void BosonModel::loadTextures(const QStringList& list)
{
 QStringList::ConstIterator it = list.begin();
 for (; it != list.end(); ++it) {
	mModelTextures->insert(this, cleanTextureName(*it));
 }
}

void BosonModel::loadModel()
{
 if (d->mFile.isEmpty() || d->mDirectory.isEmpty()) {
	boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
	return;
 }
 boProfiling->start(BosonProfiling::LoadModel);
 d->mLoader = new Bo3DSLoad(d->mDirectory, d->mFile, this);

 // TODO: add a profiling entry for this
 d->mLoader->loadModel();

 applyMasterScale();

 boProfiling->start(BosonProfiling::LoadModelTextures);
 loadTextures(d->mLoader->textures());
 boProfiling->stop(BosonProfiling::LoadModelTextures);

 if (!BoContext::currentContext()) {
	boError(100) << k_funcinfo << "NULL current context" << endl;
	return;
 }
 boProfiling->start(BosonProfiling::LoadModelDisplayLists);
 QMap<BoMesh*, QString>::Iterator it = d->mTextures.begin();
 for (; it != d->mTextures.end(); ++it) {
	BoMesh* mesh = it.key();
	QString tex = cleanTextureName(it.data());
	GLuint myTex;
	if (tex.isEmpty()) {
		myTex = 0;
	} else {
		myTex = mModelTextures->texture(tex);
	}
	mesh->setTextureObject(myTex);
	mesh->setTextured(myTex != 0);

	// AB: about profiling: this doesn't really fit to loading display
	// lists...
#if USE_STRIP
	mesh->connectFaces();
#else
	mesh->addFaces();
#endif

	mesh->loadDisplayList();
 }


 createDisplayLists();
 boProfiling->stop(BosonProfiling::LoadModelDisplayLists);

 delete d->mLoader;
 d->mLoader = 0;
 boDebug(100) << k_funcinfo << "loaded from " << file() << endl;

 boProfiling->stop(BosonProfiling::LoadModel);
}

void BosonModel::createDisplayLists()
{
 if (d->mFrames.isEmpty()) {
	boWarning() << k_funcinfo << "no frames" << endl;
	return;
 }
 boDebug(100) << k_funcinfo << "creating " << d->mFrames.count() << " lists" << endl;
 GLuint listBase = glGenLists(d->mFrames.count());
 if (listBase == 0) {
	boError(100) << k_funcinfo << "NULL display lists created" << endl;
	return;
 }

 for (unsigned int i = 0; i < frames(); i++) {
	BoFrame* f = frame(i);

	GLuint list = listBase + i;
	glNewList(list, GL_COMPILE);

	for (int j = 0; j < f->meshCount(); j++) {
		glPushMatrix();

		BoMatrix* m = f->matrix(j);
		BoMesh* mesh = f->mesh(j);
		if (!m) {
			boError() << k_funcinfo << "NULL matrix at " << j << endl;
			continue;
		}
		if (!mesh) {
			boError() << k_funcinfo << "NULL mesh at " << j << endl;
			continue;
		}

		glMultMatrixf(m->data());

		// AB: try to merge the meshes into the frame display list. -->
		// bigger lists, but might be faster.
#define USE_MANY_LISTS 1
#if USE_MANY_LISTS
		glCallList(mesh->displayList());
#else

		if (mesh->textured()) {
			glBindTexture(GL_TEXTURE_2D, mesh->textureObject());
		}
		glBegin(mesh->type());
		mesh->renderMesh();
		glEnd();
#endif

		// AB: performance: we could try to use glLoadMatrix
		// instead of glMultMatrixf and then get rid of the
		// glPushMatrix()/glPopMatrix. try which version is
		// faster!
		glPopMatrix();
	}

	glEndList();

	f->setDisplayList(list);
 }
}

void BosonModel::generateConstructionLists()
{
 // construction lists are always generated from the 1st frame!
 BoFrame* frame0 = frame(0);
 if (!frame0) {
	boError(100) << k_funcinfo << "No frame was loaded yet!" << endl;
	return;
 }
 unsigned int nodes = frame0->meshCount();
 boDebug(100) << k_funcinfo << "Generating " << nodes << " construction lists" << endl;

 GLuint base = glGenLists(nodes);
 if (base == 0) {
	boError(100) << k_funcinfo << "NULL display lists created" << endl;
	return;
 }
 for (unsigned int i = 0; i < nodes; i++) {
	GLuint list = base + i;
	BoFrame* step = new BoFrame(*frame0, i); // copy the first i meshes from frame0

	// AB: FIXME: this code is pretty much redundant. we use the same code as
	// in createDisplayLists(), but for a limted number of nodes only. we
	// could merge both in a new function!
	glNewList(list, GL_COMPILE);

	for (unsigned int j = 0; j < i; j++) {
		glPushMatrix();

		BoMatrix* m = step->matrix(j);
		BoMesh* mesh = step->mesh(j);
		if (!m) {
			boError() << k_funcinfo << "NULL matrix at " << j << endl;
			continue;
		}
		if (!mesh) {
			boError() << k_funcinfo << "NULL mesh at " << j << endl;
			continue;
		}

		glMultMatrixf(m->data());

		glCallList(mesh->displayList());

		glPopMatrix();
	}
	glEndList();
	step->setDisplayList(list);
	d->mConstructionSteps.insert(i, step);
 }
}

unsigned int BosonModel::frames() const
{
 return d->mFrames.count();
}

BoFrame* BosonModel::constructionStep(unsigned int step)
{
 if (step >= d->mConstructionSteps.count()) {
	step = d->mConstructionSteps.count() - 1;
 }
 return d->mConstructionSteps[step];
}

unsigned int BosonModel::constructionSteps() const
{
 return d->mConstructionSteps.count();
}

void BosonModel::setTeamColor(const QColor& c)
{
 delete mTeamColor;
 mTeamColor = new QColor(c);
 for (unsigned int i = 0; i < meshCount(); i++) {
	mesh(i)->setTeamColor(*mTeamColor);
 }
}

void BosonModel::finishLoading()
{
 delete mTeamColor;
 mTeamColor = 0;
 delete d->mLoader;
 d->mLoader = 0;
 delete mTeamColor;
 mTeamColor = 0;
 d->mTextureNames.clear();
}

void BosonModel::reloadAllTextures()
{
 boDebug(100) << k_funcinfo << endl;
 if (!mModelTextures) {
	boError(100) << k_funcinfo << "NULL model textures ?!?!" << endl;
	return;
 }
 mModelTextures->reloadTextures();
}

void BosonModel::insertAnimationMode(int mode, int start, unsigned int range, unsigned int speed)
{
 if (mode == 0) {
	// mode == 0 is a special mode. we default to it when everything fails,
	// so this *must* be valid.
	if (start < 0 || range == 0 || speed == 0) {
		boWarning(100) << k_funcinfo << "invalid values for default mode! start=" << start << ",range=" << range << ",speed=" << speed << endl;
		start = 0;
		range = 1;
		speed = 1;
	}
	if (d->mAnimations[0]) {
		// default mode already there - replace it!
		d->mAnimations.remove(0);
	}
 } else {
	if (start < 0 || range == 0 || speed == 0) {
		return;
	}
 }
 BosonAnimation* anim = new BosonAnimation(start, range, speed);
 d->mAnimations.insert(mode, anim);
}

void BosonModel::loadAnimationMode(int mode, KSimpleConfig* conf, const QString& name)
{
 int start = -1;
 unsigned int range = 0;
 unsigned int speed = 0;
 // different default values for mode 0:
 if (mode == 0) {
	start = 0;
	range = 1;
	speed = 1;
 }
 start = conf->readNumEntry(QString::fromLatin1("FrameStart") + name, start);
 range = conf->readUnsignedNumEntry(QString::fromLatin1("FrameRange") + name, range);
 speed = conf->readUnsignedNumEntry(QString::fromLatin1("FrameSpeed") + name, speed);
 insertAnimationMode(mode, start, range, speed);
}

BosonAnimation* BosonModel::animation(int mode) const
{
 return d->mAnimations[mode];
}




void BosonModel::addMesh(BoMesh* mesh)
{
 d->mMeshes.insert(d->mMeshes.count(), mesh);
 if (mTeamColor) {
	mesh->setTeamColor(*mTeamColor);
 }
}

BoMesh* BosonModel::mesh(int index) const
{
 return d->mMeshes[index];
}

QIntDict<BoMesh> BosonModel::allMeshes() const
{
 return d->mMeshes;
}

unsigned int BosonModel::meshCount() const
{
 return d->mMeshes.count();
}

void BosonModel::setTexture(BoMesh* mesh, const QString& texture)
{
 d->mTextures.insert(mesh, texture);
}

int BosonModel::addFrames(int count)
{
 int offset = d->mFrames.count();
 for (int i = 0; i < count; i++) {
	BoFrame* f = new BoFrame;
	d->mFrames.insert(offset + i, f);
 }
 return offset;
}

BoFrame* BosonModel::frame(unsigned int index) const
{
 return d->mFrames[index];
}

void BosonModel::computeBoundings(BoFrame* frame, BoHelper* helper) const
{
 BO_CHECK_NULL_RET(frame);
 BO_CHECK_NULL_RET(helper);

 BoVector3 v;
 for (int i = 0; i < frame->meshCount(); i++) {
	BoMesh* mesh = frame->mesh(i);
	BoMatrix* m = frame->matrix(i);
	if (!mesh) {
		boError() << k_funcinfo << "NULL mesh at " << i << endl;
		continue;
	}
	if (!m) {
		boError() << k_funcinfo << "NULL matrix at " << i << endl;
		continue;
	}
	for (unsigned int j = 0; j < mesh->points(); j++) {
		BoVector3 vector(mesh->point(j));
		m->transform(&v, &vector);
		helper->addPoint(v);
	}
	
 }
}

void BosonModel::applyMasterScale()
{
 // note: all frame must share the same scaling factor. this means that the model mustn't grow in x or y direction (width or height). It can grow/shrink in z-direction however.
 float scale = 1.0f;
 boDebug(100) << k_funcinfo << scale << endl;

 if (frames() < 1) {
	boWarning() << k_funcinfo << "no frames found!" << endl;
	return;
 }

 for (unsigned int i = 0; i < frames(); i++) {
	BoFrame* f = frame(i);

	BoHelper helper;
	computeBoundings(f, &helper);
	if (i == 0) {
		scale = helper.scale(mWidth, mHeight);
	}

	for (int j = 0; j < f->meshCount(); j++) {
		BoMesh* mesh = f->mesh(j);
		BoMatrix* matrix = f->matrix(j);
		BoMatrix m;
		m.scale(scale,scale,scale);

		// we render from bottom to top - but for x and y in the center!
		// FIXME: this doesn't work 100% correctly - the quad e.g. is
		// still partially (parts of the wheels) in the grass.
		m.translate(0.0, 0.0, -helper.mMinZ * scale);

		m.multiply(matrix);
		matrix->loadMatrix(m);
	}
	f->setDepthMultiplier(helper.lengthZ() * scale / BO_GL_CELL_SIZE);
 }
}


