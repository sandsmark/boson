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

// we use vertex arrays - we can also use display lists to make it faster.
// they are disabled currently cause plain vertex arrays are more flexible.
#define USE_DISPLAYLISTS 0

// some testings for me. this is here for my own use only currently.
// #define AB_TEST 1

BosonModelTextures* BosonModel::mModelTextures = 0;
static KStaticDeleter<BosonModelTextures> sd;

BoFrame::BoFrame()
{
 init();
}

BoFrame::BoFrame(const BoFrame& f, int firstMesh, int meshCount)
{
 init();
 mDisplayList = f.mDisplayList;
 mDepthMultiplier = f.mDepthMultiplier;
 mRadius = f.mRadius;

 if (meshCount == 0) {
	boWarning() << k_funcinfo << "no mesh copied" << endl;
	return;
 }
 if (firstMesh < 0) {
	boError() << k_funcinfo << "first mesh: " << firstMesh << " < 0" << endl;
	firstMesh = 0;
 }
 if (f.mMeshCount == 0 || !f.mMeshes || !f.mMatrices) {
	boError() << k_funcinfo << "oops - can't copy from invalid mesh!" << endl;
	return;
 }
 if (firstMesh + meshCount > f.mMeshCount) {
	boError() << k_funcinfo << "can't copy " << meshCount
			<< " meshes starting at " << firstMesh
			<< ", as there are only " << f.mMeshCount
			<< " meshes!" << endl;
	meshCount = f.mMeshCount - firstMesh;
 }
 allocMeshes(meshCount);
 int pos = 0;
 for (int i = firstMesh; i < firstMesh + meshCount; i++) {
	mMeshes[pos] = f.mMeshes[i]; // copy the pointer only
	mMatrices[pos]->loadMatrix(*f.mMatrices[i]);
	pos++;
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
 if (mMatrices) {
	for (int i = 0; i < mMeshCount; i++) {
		delete mMatrices[i];
	}
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
 mMatrices = new BoMatrix*[meshes];
 mMeshCount = meshes; // unused?
 for (int i = 0; i < meshes; i++) {
	mMeshes[i] = 0;
	mMatrices[i] = new BoMatrix;
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
 return mMatrices[index];
}

void BoFrame::renderFrame(const QColor* teamColor)
{
 for (int i = 0; i < mMeshCount; i++) {
	BoMatrix* m = mMatrices[i];
	BoMesh* mesh = mMeshes[i];
	if (!m) {
		boError() << k_funcinfo << "NULL matrix at " << i << endl;
		continue;
	}
	if (!mesh) {
		boError() << k_funcinfo << "NULL mesh at " << i << endl;
		continue;
	}
	glPushMatrix();
	glMultMatrixf(m->data());
#if USE_DISPLAYLISTS
	if (mesh->displayList()) {
		glCallList(mesh->displayList());
	} else
#endif
	{
		mesh->renderMesh(teamColor);
	}
	glPopMatrix();
 }
}

void BoFrame::mergeMeshes()
{
#ifdef AB_TEST
 QValueList<int> redundantMatrices;
 for (int i = 0; i < meshCount(); i++) {
	for (int j = i + 1; j < meshCount(); j++) {
		if (mMeshes[i]->textured() && !mMeshes[j]->textured()) {
			continue;
		}
		if (mMeshes[i]->textureObject() != mMeshes[j]->textureObject()) {
			continue;
		}
		if (mMatrices[i]->isEqual(*mMatrices[j])) {
			if (!redundantMatrices.contains(j)) {
				redundantMatrices.append(j);
			}
		}
	}
 }
#endif
// boDebug() << "redundant matrices: " << redundantMatrices.count() << endl;
}

void BoFrame::sortByDepth()
{
 // sort the meshes by depth, so that closer meshes are drawn first, then
 // meshes that are farther away.
 // note that this function is not time critical, so you don't have to care
 // about speed of the algorithms (it is called once per model on startup only).

 QPtrList<BoMesh> meshes;
 QPtrList<BoMatrix> matrices;
 for (int i = 0; i < meshCount(); i++) {
	BoMesh* mesh = mMeshes[i];
	BoMatrix* matrix = mMatrices[i];

	// do some necessary calculations
	mesh->calculateMaxMin();

	float z = mesh->maxZ();
	bool found = false;
	for (unsigned int j = 0; j < meshes.count() && !found; j++) {
		if (z >= meshes.at(j)->maxZ()) {
			meshes.insert(j, mesh);
			matrices.insert(j, matrix);
			found = true;
		}
	}
	if (!found) {
		meshes.append(mesh);
		matrices.append(matrix);
	}
 }

 // meshes is now sorted by maxZ.
 for (unsigned int i = 0; i < meshes.count(); i++) {
	BoMesh* mesh = meshes.at(i);
	BoMatrix* matrix = matrices.at(i);
	if (!mesh || !matrix) {
		boError() << k_funcinfo << "oops" << endl;
		continue;
	}
	mMeshes[i] = mesh;
	mMatrices[i] = matrix;
 }
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

		mPoints = 0;
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

	// consits of vertices and texture coordinates:
	float* mPoints;
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
 delete[] d->mPoints;
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

 mergeArrays();
 mergeMeshesInFrames();
 sortByDepth();

 applyMasterScale();

 boProfiling->start(BosonProfiling::LoadModelTextures);
 loadTextures(d->mLoader->textures());
 boProfiling->stop(BosonProfiling::LoadModelTextures);

 if (!BoContext::currentContext()) {
	boError(100) << k_funcinfo << "NULL current context" << endl;
	return;
 }

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
	mesh->createPointCache();
 }

 delete d->mLoader;
 d->mLoader = 0;
 boDebug(100) << k_funcinfo << "loaded from " << file() << endl;

 boProfiling->stop(BosonProfiling::LoadModel);
}

void BosonModel::createDisplayLists(const QColor* teamColor)
{
#if !USE_DISPLAYLISTS
 return;
#endif

#warning TODO: different teamcolors!
 // we need to maintain an internal map for the displaylists and the teamcolor!
 // createDisplayLists() will get called several times with different teamColor
 // params!

 if (d->mFrames.isEmpty()) {
	boWarning() << k_funcinfo << "no frames" << endl;
	return;
 }

 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);
 // a display list makes our vertex pointer totally useless! the data are
 // evaluated *now* when we compile the list - not later anymore!
 enablePointer();


 // AB: instead of creating display lists for every mesh we could generate a
 // single display list per frame containing all of the data. this would save
 // several glCallLists() calls and might be faster - but takes a lot more
 // memory for many frames
 boDebug(100) << k_funcinfo << "creating lists for " << d->mMeshes.count() << " meshes" << endl;
 QIntDictIterator<BoMesh> it(d->mMeshes);
 for (; it.current(); ++it) {
	it.current()->loadDisplayList(teamColor);
 }

 boDebug(100) << k_funcinfo << "creating " << d->mFrames.count() << " lists" << endl;
 GLuint listBase = glGenLists(d->mFrames.count() + d->mConstructionSteps.count());
 if (listBase == 0) {
	boError(100) << k_funcinfo << "NULL display lists created" << endl;
	return;
 }

 GLuint list = listBase;
 for (unsigned int i = 0; i < frames(); i++) {
	BoFrame* f = frame(i);

	glNewList(list, GL_COMPILE);
	f->renderFrame(teamColor);
	glEndList();
	f->setDisplayList(list);

	list++;
 }
 for (unsigned int i = 0; i < constructionSteps(); i++) {
	BoFrame* step = constructionStep(i);

	glNewList(list, GL_COMPILE);
	step->renderFrame(teamColor);
	glEndList();
	step->setDisplayList(list);

	list++;
 }

 glDisableClientState(GL_VERTEX_ARRAY);
 glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void BosonModel::generateConstructionFrames()
{
 // construction frames are always generated from the 1st frame!
 BoFrame* frame0 = frame(0);
 if (!frame0) {
	boError(100) << k_funcinfo << "No frame was loaded yet!" << endl;
	return;
 }
 if (d->mConstructionSteps.count() != 0) {
	boWarning(100) << k_funcinfo << "construction frames already generated" << endl;
	return;
 }
 unsigned int nodes = frame0->meshCount();
 boDebug(100) << k_funcinfo << "Generating " << nodes << " construction frames" << endl;

 for (unsigned int i = 0; i < nodes; i++) {
	// copy i meshes, starting at start (until the end)
	// --> meshes are sorted by z. first mesh has lowest z.
	int start = (nodes - 1) - i;
	BoFrame* step = new BoFrame(*frame0, start, i + 1);
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

void BosonModel::finishLoading()
{
 delete d->mLoader;
 d->mLoader = 0;
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

void BosonModel::mergeArrays()
{
 if (d->mPoints) {
	// this is dangerous!
	// crash is probably close (broken indices)
	boError() << k_funcinfo << "points already allocated" << endl;
	return;
 }
 // count the points in the meshes first.
 unsigned int size = 0;
 QIntDictIterator<BoMesh> it(d->mMeshes);
 for (; it.current(); ++it) {
	// 3 vertices per point + 2 texels per point
	// note that we waste some space here - some objects are not textured.
	// but when we allocate this  way we can render more efficient.
	size += it.current()->points() * 5;
 }

 d->mPoints = new float[size];
 it.toFirst();
 int index = 0;
 for (; it.current(); ++it) {
	it.current()->movePoints(d->mPoints, index);
	index += it.current()->points();
 }
}

void BosonModel::mergeMeshesInFrames()
{
#ifdef AB_TEST
// boDebug() << k_funcinfo << file() << endl;
 QIntDictIterator<BoFrame> it(d->mFrames);
 for (; it.current(); ++it) {
	it.current()->mergeMeshes();
 }
#endif
}

void BosonModel::sortByDepth()
{
 QIntDictIterator<BoFrame> it(d->mFrames);
 for (; it.current(); ++it) {
	it.current()->sortByDepth();
 }
}

float* BosonModel::pointArray() const
{
 return d->mPoints;
}

void BosonModel::enablePointer()
{
 // TODO: performance:
 // we should manage a single (giantic) array, which contains the of ALL models.
 // then we could set the pointers once only and can render after that.
 // additionally we could search for redundant indices - i.e. if there are
 // different points with exactly the same coordinates (vertices and texture)
 // then we could replace the index of one of them by the index of the other one
 //
 // TODO: performance: interleaved arrays
 glVertexPointer(3, GL_FLOAT, 5 * sizeof(float), d->mPoints);
 glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(float), d->mPoints + 3);
}

