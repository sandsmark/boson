/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonmodelloader.h"
#include "bosonglwidget.h" // BoContext
#include "bodebug.h"
#include "bomesh.h"
#include "bomaterial.h"
#include "bosonconfig.h"
#include "bomemorytrace.h"
#include "bomeshrenderer.h"
#include "bomeshrenderermanager.h"

#include <ksimpleconfig.h>

#include <qstringlist.h>
#include <qvaluelist.h>
#include <qintdict.h>
#include <qptrvector.h>

#include <math.h>
#if HAVE_GL_GLEXT_H
#include <GL/glext.h>
#endif

// use GL_TRIANGLE_STRIP ? (experimental and not working!)
// AB: there are 2 different optimizing approaches. one is to use
// GL_TRIANGLE_STRIP, the other one is using a single glBegin()/glEnd() call (we
// need to place all textures of that model into a single big texture map and
// adjust the coordinates accordingly). unfortunately they exclude each other.
// so we need to be compare both approaches to find out which is better.
// i am expecting the strips to be faster
#define USE_STRIP 0

// some testings for me. this is here for my own use only currently.
// #define AB_TEST 1


class BoMeshSorter
{
public:
	class Mesh
	{
	public:
		Mesh()
		{
			matrix = 0;
			mesh = 0;
			hidden = false;
		}
		Mesh(const Mesh& m)
		{
			*this = m;
		}
		Mesh& operator=(const Mesh& m)
		{
			mesh = m.mesh;
			matrix = m.matrix;
			hidden = m.hidden;
			return *this;
		}
		BoMatrix* matrix;
		BoMesh* mesh;
		bool hidden;
	};

	BoMeshSorter()
	{
	}
	~BoMeshSorter()
	{
	}

	static void sortByMaxZ(BoFrame* frame, QValueList<Mesh>* meshes)
	{
		makeList(frame, meshes);
		sortByMaxZ(meshes);
	}
	static void sortByMinZ(BoFrame* frame, QValueList<Mesh>* meshes)
	{
		makeList(frame, meshes);
		sortByMinZ(meshes);
	}

	static void sortByMaxZ(QValueList<Mesh>* meshes)
	{
		sortByZ(meshes, true);
	}
	static void sortByMinZ(QValueList<Mesh>* meshes)
	{
		sortByZ(meshes, false);
	}
	static void sortByMaxSize(BoFrame* frame, QValueList<Mesh>* meshes) // sort by volume
	{
		makeList(frame, meshes);
		sortByMaxSize(meshes);
	}
	static void sortByMaxSize(QValueList<Mesh>* meshes); // sort by volume

protected:
	static void sortByZ(QValueList<Mesh>* meshes, bool byMaxZ);
	static void makeList(BoFrame* frame, QValueList<Mesh>* meshes)
	{
		if (!frame || !meshes) {
			return;
		}
		meshes->clear();
		for (unsigned int i = 0; i < frame->meshCount(); i++) {
			BoMeshSorter::Mesh mesh;
			mesh.mesh = frame->mesh(i);
			mesh.matrix = frame->matrix(i);
			mesh.hidden = frame->hidden(i);
			meshes->append(mesh);
		}
	}
	/**
	 * @return The volume of the box with the specified values
	 **/
	static float volume(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
	{
		float dx = maxX - minX;
		float dy = maxY - minY;
		float dz = maxZ - minZ;
		float volume = dx * dy * dz;
		if (volume < 0.0f) {
			return -volume;
		}
		return volume;
	}
};

void BoMeshSorter::sortByMaxSize(QValueList<BoMeshSorter::Mesh>* meshes)
{
 BO_CHECK_NULL_RET(meshes);
 if (meshes->count() == 0) {
	boError() << k_funcinfo << "no meshes" << endl;
	return;
 }
 if (meshes->count() == 1) {
	return;
 }
 QMap<float, QValueList<BoMeshSorter::Mesh>* > map;

 QValueList<BoMeshSorter::Mesh>::Iterator it;
 for (it = meshes->begin(); it != meshes->end(); ++it) {
	BoMeshSorter::Mesh mesh = *it;

	if (!mesh.mesh) {
		boError() << k_funcinfo << "NULL mesh" << endl;
		continue;
	}

	// do some necessary calculations
	// AB: this is a naive implementation.
	// we just use min/max x/y/z. but imagine a perfect cube which
	// is rotated by 45 degree - it's actual volume would be still
	// the same, but here we would generate a different box which
	// the original cube fits in (and isn't rotated). that volume
	// would be a lot bigger than...
	float minX, maxX, minY, maxY, minZ, maxZ;
	mesh.mesh->getBoundingBox(*mesh.matrix, &minX, &maxX, &minY, &maxY, &minZ, &maxZ);
	float v = volume(minX, maxX, minY, maxY, minZ, maxZ);

	QValueList<BoMeshSorter::Mesh>* list = 0;
	if (!map.contains(v)) {
		list = new QValueList<BoMeshSorter::Mesh>();
		map.insert(v, list);
	} else {
		list = map[v];
	}
	list->append(mesh);
 }

 unsigned int meshesCount = meshes->count();
 meshes->clear();
 QMap<float, QValueList<BoMeshSorter::Mesh>* >::Iterator mapIt;
 for (mapIt = map.begin(); mapIt != map.end(); ++mapIt) {
	QValueList<BoMeshSorter::Mesh>* list = mapIt.data();
	QValueList<BoMeshSorter::Mesh>::Iterator listIt;
	for (listIt = list->begin(); listIt != list->end(); ++listIt) {
		meshes->prepend(*listIt);
	}
	delete list;
 }
 map.clear(); // all inserted pointers are already deleted

 if (meshesCount != meshes->count()) {
	boError() << k_funcinfo << "invalid result! count=" << meshes->count() << " should be: " << meshesCount << endl;
 }
}

void BoMeshSorter::sortByZ(QValueList<BoMeshSorter::Mesh>* meshes, bool byMaxZ)
{
 BO_CHECK_NULL_RET(meshes);
 if (meshes->count() == 0) {
	boError() << k_funcinfo << "no meshes" << endl;
	return;
 }
 if (meshes->count() == 1) {
	return;
 }
 QMap<float, QValueList<BoMeshSorter::Mesh>* > map;

 QValueList<BoMeshSorter::Mesh>::Iterator it;
 for (it = meshes->begin(); it != meshes->end(); ++it) {
	BoMeshSorter::Mesh mesh = *it;

	// do some necessary calculations
	float tmp, minZ, maxZ;
	mesh.mesh->getBoundingBox(*mesh.matrix, &tmp, &tmp, &tmp, &tmp, &minZ, &maxZ);

	float z = 0.0f;
	if (byMaxZ) {
		z = maxZ;
	} else {
		z = minZ;
	}

	QValueList<BoMeshSorter::Mesh>* list = 0;
	if (!map.contains(z)) {
		list = new QValueList<BoMeshSorter::Mesh>();
		map.insert(z, list);
	} else {
		list = map[z];
	}
	list->append(mesh);
 }

 unsigned int meshesCount = meshes->count();
 meshes->clear();
 QMap<float, QValueList<BoMeshSorter::Mesh>* >::Iterator mapIt;
 for (mapIt = map.begin(); mapIt != map.end(); ++mapIt) {
	QValueList<BoMeshSorter::Mesh>* list = mapIt.data();
	if (byMaxZ) {
		QValueList<BoMeshSorter::Mesh>::Iterator listIt;
		for (listIt = list->begin(); listIt != list->end(); ++listIt) {
			meshes->prepend(*listIt);
		}
	} else {
		QValueList<BoMeshSorter::Mesh>::Iterator listIt;
		for (listIt = list->begin(); listIt != list->end(); ++listIt) {
			meshes->append(*listIt);
		}
	}
	delete list;
 }
 map.clear(); // all inserted pointers are already deleted

 if (meshesCount != meshes->count()) {
	boWarning() << k_funcinfo << "oops - something weird happened: meshesCount=" << meshesCount << " meshes->count()=" << meshes->count() << endl;
 }
}


BoFrame::BoFrame()
{
 init();
}

BoFrame::BoFrame(const BoFrame& f, unsigned int firstMesh, unsigned int meshCount)
{
 init();
 mDepthMultiplier = f.mDepthMultiplier;
 mRadius = f.mRadius;

 if (meshCount == 0) {
	boWarning() << k_funcinfo << "no mesh copied" << endl;
	return;
 }
 if (firstMesh + meshCount > f.mMeshCount) {
	boError() << k_funcinfo << "can't copy " << meshCount
			<< " meshes starting at " << firstMesh
			<< ", as there are only " << f.mMeshCount
			<< " meshes!" << endl;
	meshCount = f.mMeshCount - firstMesh;
 }
 unsigned int* meshes = new unsigned int[meshCount];
 for (unsigned int i = 0; i < meshCount; i++) {
	meshes[i] = meshes[firstMesh + i];
 }

 copyMeshes(f, meshes, meshCount);
 delete[] meshes;
}

BoFrame::BoFrame(const BoFrame& f, unsigned int* meshes, unsigned int meshCount)
{
 init();
 mDepthMultiplier = f.mDepthMultiplier;
 mRadius = f.mRadius;
 copyMeshes(f, meshes, meshCount);
}

void BoFrame::init()
{
 mDepthMultiplier = 0.0f;
 mRadius = 0.0f;
 mMatrices = 0;
 mMeshes = 0;
 mMeshCount = 0;
 mHidden = 0;
}

BoFrame::~BoFrame()
{
 if (mMatrices) {
	for (unsigned int i = 0; i < mMeshCount; i++) {
		delete mMatrices[i];
	}
 }
 delete[] mMeshes;
 delete[] mMatrices;
 delete[] mHidden;
}

void BoFrame::copyMeshes(const BoFrame& f, unsigned int* meshes, unsigned int count)
{
 if (f.mMeshCount == 0 || !f.mMeshes || !f.mMatrices || !f.mHidden) {
	boError() << k_funcinfo << "oops - can't copy from invalid mesh!" << endl;
	return;
 }
 BO_CHECK_NULL_RET(meshes);
 if (count == 0) {
	boWarning() << k_funcinfo << "no mesh copied" << endl;
	return;
 }
 if (count > f.mMeshCount) {
	boError() << k_funcinfo << "cannot copy " << count
			<< " meshes as frame contains " << f.mMeshCount
			<< " meshes only" << endl;
	return;
 }

 // before allocating anything we first check whether all indices are valid
 for (unsigned int i = 0; i < count; i++) {
	if (meshes[i] >= f.mMeshCount) {
		boError() << k_funcinfo << "index " << meshes[i] << " at " << i
				<< " is not valid! only " << f.mMeshCount
				<< " available in the frame" << endl;
		return;
	}
 }

 allocMeshes(count);
 for (unsigned int i = 0; i < count; i++) {
	unsigned int index = meshes[i];
	mMeshes[i] = f.mMeshes[index]; // copy the pointer only
	mMatrices[i]->loadMatrix(*f.mMatrices[index]);
	mHidden[i] = f.mHidden[index];
 }
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
 if (mHidden) {
	boError() << k_funcinfo << "\"hidden\" flags already allocated??" << endl;
	delete[] mHidden;
 }
 mMeshes = new BoMesh*[meshes];
 mMatrices = new BoMatrix*[meshes];
 mHidden = new bool[meshes];
 mMeshCount = meshes; // unused?
 for (int i = 0; i < meshes; i++) {
	mMeshes[i] = 0;
	mMatrices[i] = new BoMatrix;
	mHidden[i] = false;
 }
}

void BoFrame::setMesh(unsigned int index, BoMesh* mesh)
{
 if (index >= mMeshCount) {
	boError() << k_funcinfo << "invalid mesh " << index << " , count=" << mMeshCount << endl;
	return;
 }
 BO_CHECK_NULL_RET(mesh);
 BO_CHECK_NULL_RET(mMeshes);
 // we store the *pointer* only!
 mMeshes[index] = mesh;
}

void BoFrame::setHidden(unsigned int index, bool hidden)
{
 if (index >= mMeshCount) {
	boError() << k_funcinfo << "invalid mesh " << index << " , count=" << mMeshCount << endl;
	return;
 }
 BO_CHECK_NULL_RET(mHidden);
 mHidden[index] = hidden;
}

BoMatrix* BoFrame::matrix(int index) const
{
 return mMatrices[index];
}

void BoFrame::renderFrame(const QColor* teamColor, unsigned int lod, int mode)
{
 for (unsigned int i = 0; i < mMeshCount; i++) {
	if (mHidden[i]) {
		continue;
	}
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
	if (mode == GL_SELECT) {
		glLoadName(i);
	}
	mesh->renderMesh(m, teamColor, lod);
 }
}

void BoFrame::mergeMeshes()
{
#ifdef AB_TEST
 QValueList<int> redundantMatrices;
 for (unsigned int i = 0; i < meshCount(); i++) {
	for (unsigned int j = i + 1; j < meshCount(); j++) {
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

 QValueList<BoMeshSorter::Mesh> meshes;
// BoMeshSorter::sortByMaxZ(this, &meshes);
 BoMeshSorter::sortByMaxSize(this, &meshes);

 BO_CHECK_NULL_RET(mMeshes);
 BO_CHECK_NULL_RET(mMatrices);
 BO_CHECK_NULL_RET(mHidden);

 // meshes is now sorted by maxZ.
 QValueList<BoMeshSorter::Mesh>::Iterator it;
 int i = 0;
 for (it = meshes.begin(); it != meshes.end(); ++it) {
	BoMesh* mesh = (*it).mesh;
	BoMatrix* matrix = (*it).matrix;
	bool hidden = (*it).hidden;
	if (!mesh || !matrix) {
		boError() << k_funcinfo << "oops" << endl;
		continue;
	}
	mMeshes[i] = mesh;
	mMatrices[i] = matrix;
	mHidden[i] = hidden;
	i++;
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

	/**
	 * @return the scaling factor required for the data in this object to
	 * fit a width of @p w and a height of @p h. When all points added using
	 * @ref addPoints are scaled by that value, the model won't be higher
	 * than @p h * and won't be wider than @p w.
	 **/
	float scale(float w, float h) const
	{
		if (fabsf(lengthX() < 0.0001)) {
			boWarning() << k_funcinfo << "invalid length in x direction: " << lengthX() << endl;
			return 1.0f;
		}
		if (fabsf(lengthY() < 0.0001)) {
			boWarning() << k_funcinfo << "invalid length in y direction: " << lengthY() << endl;
			return 1.0f;
		}
		float scaleX = w / lengthX();
		float scaleY = h / lengthY();
		// we don't care about z-size here!
		float scale = QMIN(scaleX, scaleY);
		if (fabsf(scale) < 0.0001) {
			return 1.0f;
		}
		return scale;
	}



	float mMaxX;
	float mMinX; // lowest (i.e. negative) x
	float mMaxY;
	float mMinY;
	float mMaxZ;
	float mMinZ;
};


class BosonModelPrivate
{
public:
	BosonModelPrivate()
	{
		mPoints = 0;

	}

	QPtrVector<BoMesh> mMeshes;
	QPtrVector<BoFrame> mFrames;

	QPtrVector<BoMaterial> mAllMaterials;

	QPtrVector<BoFrame> mConstructionSteps;
	QIntDict<BosonAnimation> mAnimations;
	QMap<QString, QString> mTextureNames;
	QString mDirectory;
	QString mFile;

	unsigned int mPointArraySize;
	float* mPoints;

	unsigned int mLODCount;
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
 d = new BosonModelPrivate;
 mWidth = 0;
 mHeight = 0;
 d->mMeshes.setAutoDelete(true);
 d->mFrames.setAutoDelete(true);
 d->mConstructionSteps.setAutoDelete(true);
 d->mAnimations.setAutoDelete(true);
 d->mAllMaterials.setAutoDelete(true);
 d->mLODCount = 1;
 d->mPointArraySize = 0;
 mMeshRendererModelData = 0;

 // add the default mode 0
 insertAnimationMode(0, 0, 1, 1);
}

BosonModel::~BosonModel()
{
 boDebug(100) << k_funcinfo << endl;
 finishLoading();
 BoMeshRendererManager::manager()->removeModel(this);
 BosonModelTextures::modelTextures()->removeModel(this);
 if (mMeshRendererModelData) {
	boWarning(100) << k_funcinfo << "meshrenderer forgot to delete model data" << endl;
 }
 delete mMeshRendererModelData;
 boDebug(100) << k_funcinfo << "delete " << d->mFrames.count() << " frames" << endl;
 d->mFrames.clear();
 boDebug(100) << k_funcinfo << "delete " << d->mConstructionSteps.count() << " construction frames" << endl;
 d->mConstructionSteps.clear();
 d->mAnimations.clear();
 boDebug(100) << k_funcinfo << "delete meshes" << endl;
 d->mMeshes.clear();
 delete[] d->mPoints;
 delete d;
 boDebug(100) << k_funcinfo << "done" << endl;
}

BoMaterial* BosonModel::material(unsigned int index) const
{
 return d->mAllMaterials[index];
}

unsigned int BosonModel::materialCount() const
{
 return d->mAllMaterials.count();
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
	BosonModelTextures::modelTextures()->insert(this, cleanTextureName(*it));
 }
}

void BosonModel::loadModel()
{
 if (d->mFile.isEmpty() || d->mDirectory.isEmpty()) {
	boError(100) << k_funcinfo << "No file has been specified for loading" << endl;
	return;
 }
 BosonProfiler profiler(BosonProfiling::LoadModel);

 BosonModelLoader loader(d->mDirectory, d->mFile, this);

 // TODO: add a profiling entry for this
 if (!loader.loadModel()) {
	boError() << k_funcinfo << "model " << d->mFile << " could not be loaded!" << endl;
	return;
 }

 BosonModelLoaderData* data = loader.data();
 if (!data) {
	BO_NULL_ERROR(data);
	return;
 }

 if (!loadModelData(data)) {
	boError() << k_funcinfo << "unable to load model data for " << d->mFile << endl;
	return;
 }


 if (frames() == 0) {
	boError() << k_funcinfo << "0 frames loaded for model " << d->mFile << endl;
	return;
 }
 if (meshCount() == 0) {
	boError() << k_funcinfo << "0 meshes loaded for model " << d->mFile << endl;
	return;
 }

 boDebug(100) << k_funcinfo << "calculate normals" << endl;
 for (unsigned int i = 0; i < meshCount(); i++) {
	BoMesh* m = mesh(i);
	if (!m) {
		BO_NULL_ERROR(m);
		continue;
	}
	m->calculateNormals();
 }

 // scale the model to mWidth * mHeight. vertices are not touched, the scaling
 // factor is integrated into the mesh matrices.
 applyMasterScale();
 computeBoundingObjects();

 boDebug(100) << k_funcinfo << "generating LODs for meshes" << endl;
 // AB: first apply the master scale, then the LOD: in order to find out how
 // much surface / volume the model can take at most, we need the final
 // matrices.
 // computing the master scaling factor first, we can easily use width() and
 // height().
 generateLOD();
 boDebug(100) << k_funcinfo << "generating LODs for meshes done" << endl;

 boDebug(100) << k_funcinfo << "merge arrays" << endl;
 mergeArrays();
 mergeMeshesInFrames();
 sortByDepth();

 QStringList modelTextures;
 for (unsigned int i = 0; i < materialCount(); i++) {
	BoMaterial* mat = material(i);
	if (!mat) {
		BO_NULL_ERROR(mat);
		continue;
	}
	QString tex = cleanTextureName(mat->textureName());
	if (!tex.isEmpty()) {
		modelTextures.append(tex);
	}
 }

 boProfiling->start(BosonProfiling::LoadModelTextures);
 loadTextures(modelTextures);
 boProfiling->stop(BosonProfiling::LoadModelTextures);

 if (!BoContext::currentContext()) {
	boError(100) << k_funcinfo << "NULL current context" << endl;
	return;
 }

 for (unsigned int i = 0; i < materialCount(); i++) {
	BoMaterial* mat = material(i);
	if (!mat) {
		BO_NULL_ERROR(mat);
		continue;
	}
	QString tex = cleanTextureName(mat->textureName());
	GLuint myTex = 0;
	if (!tex.isEmpty()) {
		myTex = BosonModelTextures::modelTextures()->texture(tex);
	}
	mat->setTextureObject(myTex);
 }

 for (unsigned int i = 0; i < meshCount(); i++) {
	BoMesh* m = mesh(i);
	if (!m) {
		BO_NULL_ERROR(m);
		continue;
	}
#if USE_STRIP
	m->connectNodes();
#else
	m->addNodes();
#endif
	m->createPointCache();
 }


 // must happen when the model has been loaded completely
 boDebug(100) << k_funcinfo << "adding model to meshrenderer" << endl;
 BoMeshRendererManager::manager()->addModel(this);

 boDebug(100) << k_funcinfo << "loaded from " << file() << endl;
}

bool BosonModel::loadModelData(BosonModelLoaderData* data)
{
 if (!data) {
	BO_NULL_ERROR(data);
	return false;
 }

 // AB: note that we must copy the pointers, not just the data.
 // the objects might reference each other through their pointers.
 // AB: also note that when we copy a pointer, we take ownership and delete it
 // when the model is deleted

 if (data->meshCount() == 0) {
	boError() << k_funcinfo << "no mesh in model!" << endl;
	return false;
 }
 if (d->mMeshes.count() != 0) {
	boWarning() << k_funcinfo << "meshes already loaded?? deleting existing meshes - this might crash if the pointers are still used!" << endl;
	d->mMeshes.resize(0);
 }
 d->mMeshes.resize(data->meshCount());
 for (unsigned int i = 0; i < data->meshCount(); i++) {
	d->mMeshes.insert(i, data->mesh(i));
 }
 data->clearMeshes(false);

 if (data->frameCount() == 0) {
	boError() << k_funcinfo << "no frame in model" << endl;
	return false;
 }
 if (d->mFrames.count() != 0) {
	boWarning() << k_funcinfo << "frames already loaded?? deleting existing frames - this might crash if the pointers are still used!" << endl;
	d->mFrames.resize(0);
 }
 d->mFrames.resize(data->frameCount());
 for (unsigned int i = 0; i < data->frameCount(); i++) {
	d->mFrames.insert(i, data->frame(i));
 }
 data->clearFrames(false);

 if (data->materialCount() != 0) {
	if (d->mAllMaterials.count() != 0) {
		boWarning() << k_funcinfo << "materials already loaded?! deleting them - this might crash when they are still needed" << endl;
		d->mAllMaterials.resize(0);
	}
	d->mAllMaterials.resize(data->materialCount());
	for (unsigned int i = 0; i < data->materialCount(); i++) {
		d->mAllMaterials.insert(i, data->material(i));
	}
 }
 data->clearMaterials(false);

 return true;
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

 QValueList<BoMeshSorter::Mesh> meshes;
 BoMeshSorter::sortByMinZ(frame0, &meshes);

 unsigned int* indices = new unsigned int[meshes.count()];
 d->mConstructionSteps.resize(meshes.count());
 for (unsigned int i = 0; i < meshes.count(); i++) {
	for (unsigned int j = 0; j < frame0->meshCount(); j++) {
		// a damn slow linear search... but who cares for a 20 entry
		// lists on startup...
		BoMesh* mesh = meshes[i].mesh;
		if (frame0->mesh(j) == mesh) {
			indices[i] = j;
		}
	}
	BoFrame* step = new BoFrame(*frame0, indices, i + 1);
	d->mConstructionSteps.insert(i, step);
 }
 delete[] indices;
}

unsigned int BosonModel::frames() const
{
 return d->mFrames.count();
}

BoFrame* BosonModel::constructionStep(unsigned int step)
{
 if (d->mConstructionSteps.count() == 0) {
	return 0;
 }
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
 d->mTextureNames.clear();
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



BoMesh* BosonModel::mesh(unsigned int index) const
{
 return d->mMeshes[index];
}

unsigned int BosonModel::meshCount() const
{
 return d->mMeshes.count();
}

QIntDict<BoMesh> BosonModel::allMeshes() const
{
 QIntDict<BoMesh> meshes;
 for (unsigned int i = 0; i < meshCount(); i++) {
	BoMesh* m = mesh(i);
	if (!m) {
		BO_NULL_ERROR(m);
		continue;
	}
	meshes.insert(i, m);
 }
 return meshes;
}

BoFrame* BosonModel::frame(unsigned int index) const
{
 if (index >= d->mFrames.count()) {
	return 0;
 }
 return d->mFrames[index];
}

void BosonModel::computeBoundings(BoFrame* frame, BoHelper* helper) const
{
 BO_CHECK_NULL_RET(frame);
 BO_CHECK_NULL_RET(helper);

 BoVector3 v;
 for (unsigned int i = 0; i < frame->meshCount(); i++) {
	const BoMesh* mesh = frame->mesh(i);
	const BoMatrix* m = frame->matrix(i);
	if (!mesh) {
		boError() << k_funcinfo << "NULL mesh at " << i << endl;
		continue;
	}
	if (!m) {
		boError() << k_funcinfo << "NULL matrix at " << i << endl;
		continue;
	}
	for (unsigned int j = 0; j < mesh->points(); j++) {
		BoVector3 vector(mesh->vertex(j));
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

	for (unsigned int j = 0; j < f->meshCount(); j++) {
		BoMatrix* matrix = f->matrix(j);
		BoMatrix m;
		m.scale(scale,scale,scale);

		// we render from bottom to top - but for x and y in the center!
		// FIXME: this doesn't work 100% correctly - the quad e.g. is
		// still partially (parts of the wheels) in the grass.
		// FIXME:do we actually have to multiply mMinZ by scale? scale
		// is already in m... AB: I do not change this as I am not
		// totally sure and we dont have models that behave incorrectly
		// atm
		m.translate(0.0, 0.0, -helper.mMinZ * scale);

		m.multiply(matrix);
		matrix->loadMatrix(m);
	}
	f->setDepthMultiplier(helper.lengthZ() * scale);
 }
}

void BosonModel::computeBoundingObjects()
{
 // we use bounding boxes everywhere at the moment.
 // most (culling-)algorithms can be implemented easier with boxes, but
 // sometimes (e.g. for the hp culling extension) the object must be as small as
 // possible - such as spheres for certain meshes. one day we may support that
 // (the hp extension is too slow for us, but maybe we have other uses one day).

 // compute a bounding box for all meshes first.
 for (unsigned int i = 0; i < meshCount(); i++) {
	BoMesh* m = mesh(i);
	m->computeBoundingObject();
 }

 // here we should compute a bounding object for the complete frame, which take
 // care of frame matrices.
 // we will need this object (box) for borender, for a grid.
 for (unsigned int i = 0; i < frames(); i++) {
	BoFrame* f = frame(i);

 }
}

void BosonModel::generateLOD()
{
 unsigned int lodCount = boConfig->defaultLodCount();
 if (lodCount == 0) {
	boWarning() << k_funcinfo << "LOD count of 0 is not allowed" << endl;
	lodCount = 1;
 }
 if (lodCount > 5) {
	// AB: this is a BoLODBuilder limitation.
	// (not that as of 03/12/13 the same limitation is in choosing the
	// LOD that is used, as the LODLevels array has 5 entries only)
	boWarning() << k_funcinfo << "more than 5 LODs not supported" << endl;
	lodCount = 5;
 }

 d->mLODCount = lodCount;
 for (unsigned int i = 0; i < meshCount(); i++) {
	BoMesh* m = mesh(i);
	if (!m) {
		BO_NULL_ERROR(m);
		continue;
	}
	m->generateLOD(lodCount, this);
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
 for (unsigned int i = 0; i < meshCount(); i++) {
	BoMesh* m = mesh(i);
	if (!m) {
		BO_NULL_ERROR(m);
		continue;
	}
	size += m->points() * BoMesh::pointSize();
 }

 delete[] d->mPoints;
 d->mPoints = new float[size];
 d->mPointArraySize = size;
 int index = 0;
 for (unsigned int i = 0; i < meshCount(); i++) {
	BoMesh* m = mesh(i);
	if (!m) {
		BO_NULL_ERROR(m);
		continue;
	}
	unsigned int pointsMoved = m->movePoints(d->mPoints, index);
	index += pointsMoved;
 }
}

void BosonModel::mergeMeshesInFrames()
{
#ifdef AB_TEST
// boDebug() << k_funcinfo << file() << endl;
 for (unsigned int i = 0; i < frames(); i++) {
	BoFrame* f = frame(i);
	if (!f) {
		BO_NULL_ERROR(i);
		continue;
	}
	f->mergeMeshes();
 }
#endif
}

void BosonModel::sortByDepth()
{
 for (unsigned int i = 0; i < frames(); i++) {
	BoFrame* f = frame(i);
	if (!f) {
		BO_NULL_ERROR(i);
		continue;
	}
	f->sortByDepth();
 }
}

float* BosonModel::pointArray() const
{
 return d->mPoints;
}

unsigned int BosonModel::pointArraySize() const
{
 return d->mPointArraySize;
}

void BosonModel::prepareRendering()
{
 BoMeshRendererManager* manager = BoMeshRendererManager::manager();
 if (!manager->checkCurrentRenderer()) {
	boError() << k_funcinfo << "unable to load a renderer" << endl;
	return;
 }
 if (manager->currentRenderer()) {
	// AB: maybe we add a setModel() to the manager, then we can avoid
	// #including the renderer file.
	manager->currentRenderer()->setModel(this);
 }
}

unsigned int BosonModel::lodCount() const
{
 return d->mLODCount;
}

unsigned int BosonModel::preferredLod(float dist) const
{
 unsigned int lod = 0;

 // LOD distance levels
 // If distance between item and camera is less than LODLevels[x], lod level
 // x is used
 static const float LODLevels[] = { 7.0f, 12.0f, 20.0f, 30.0f, 1000.0f };

 while (lod < lodCount() - 1) {
	if (dist > LODLevels[lod]) {
		// Object is too far for this lod
		lod++;
	} else {
		// Use this lod
		break;
	}
 }

 return lod;
}

void BosonModel::setMeshRendererModelData(BoMeshRendererModelData* data)
{
 delete mMeshRendererModelData;
 mMeshRendererModelData = data;
}

void BosonModel::startModelRendering()
{
 if (!BoMeshRendererManager::checkCurrentRenderer()) {
	boError() << k_funcinfo << "unable to load a renderer" << endl;
	return;
 }
 BoMeshRenderer* renderer = BoMeshRendererManager::manager()->currentRenderer();
 BO_CHECK_NULL_RET(renderer);
 renderer->startModelRendering();
}

void BosonModel::stopModelRendering()
{
 BoMeshRenderer* renderer = BoMeshRendererManager::manager()->currentRenderer();
 BO_CHECK_NULL_RET(renderer);
 renderer->stopModelRendering();
}

BoVector3 BosonModel::vertex(unsigned int i) const
{
 BoVector3 v;
 if (!pointArray()) {
	return v;
 }
 if (i >= pointArraySize()) {
	boError() << k_funcinfo << "invalid index " << i << " maximal " << pointArraySize() << " points available" << endl;
	return v;
 }
 v.setX(d->mPoints[i * BoMesh::pointSize() + BoMesh::vertexPos() + 0]);
 v.setY(d->mPoints[i * BoMesh::pointSize() + BoMesh::vertexPos() + 1]);
 v.setZ(d->mPoints[i * BoMesh::pointSize() + BoMesh::vertexPos() + 2]);
 return v;
}

BoVector3 BosonModel::texel(unsigned int i) const
{
 BoVector3 t;
 if (!pointArray()) {
	return t;
 }
 if (i >= pointArraySize()) {
	boError() << k_funcinfo << "invalid index " << i << " maximal " << pointArraySize() << " points available" << endl;
	return t;
 }
 t.setX(d->mPoints[i * BoMesh::pointSize() + BoMesh::texelPos() + 0]);
 t.setY(d->mPoints[i * BoMesh::pointSize() + BoMesh::texelPos() + 1]);
 t.setZ(0.0f);
 return t;
}

