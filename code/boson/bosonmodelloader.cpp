/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonmodelloader.h"

#include "bo3dsload.h"
#include "boacload.h"
#include "bomesh.h"
#include "bomaterial.h"
#include "bosonmodel.h"
#include "bosonprofiling.h"
#include "bodebug.h"

#include <qstring.h>
#include <qintdict.h>
#include <qptrdict.h>
#include <qptrlist.h>
#include <qvaluevector.h>

class BosonModelLoaderOptimizer
{
public:
	BosonModelLoaderOptimizer()
	{
		mEqualLists.setAutoDelete(true);
		mData = 0;
		mOptimizedData = new BosonModelLoaderData;
	}
	~BosonModelLoaderOptimizer()
	{
		delete mOptimizedData;
	}
	BosonModelLoaderData* getOptimizedData()
	{
		BosonModelLoaderData* data = mOptimizedData;
		mOptimizedData = 0;
		return data;
	}

	void setData(const BosonModelLoaderData* data)
	{
		// AB: this MUST be const - we must not change it before
		// optimize() returns true.
		// this is important, because if it returns false, we require
		// data to be completely unchanged so that we can continue to
		// use it.
		mData = data;
	}
	bool optimize();

protected:
	/**
	 * Clone the materials from the original data pointer. Also makes sure
	 * that redundant materials are removed.
	 **/
	bool cloneMaterials();

	/**
	 * This is responsible for merging meshes that can be merged.
	 **/
	bool cloneMeshes();

	bool cloneFrames();

	/**
	 * Try to find a set of meshes that are "equal". See @ref meshesAreEqual
	 * @param equal An initial set of meshes (indices in the frame) that are
	 * known to be equal in at least their material. When the method
	 * returns, it places the actually equal meshes here.
	 * @param rest An initially equal list. When the method returns, all
	 * meshes that are not equal to those in @p equal are placed here. You
	 * should call findEqualMeshes again for this list.
	 **/
	void findEqualMeshes(QValueList<unsigned int>* equal, QValueList<unsigned int>* rest);

	/**
	 * For our uses here two * meshes are "equal" when they can be merged
	 * (that is if they share the * same material and the same matrix in all
	 * frames).
	 * @param i1 The index of the first mesh in the frame
	 * @param i2 The index of the second mesh in the frame
	 **/
	bool meshesAreEqual(unsigned int i1, unsigned int i2, const BoFrame* frame) const;

	bool mergeMeshes(QValueList<const BoMesh*>);

private:
	const BosonModelLoaderData* mData;
	BosonModelLoaderData* mOptimizedData;

	// maps data->material to mOptimizedData->material
	QMap<const BoMaterial*, BoMaterial*> mMaterial2Material;
	QMap<const BoMesh*, BoMesh*> mMesh2Mesh;

	// lists of mergable meshes in the frames.
	// if one valuelist contains more than one mesh, then all meshes in the
	// list can be merged - they use the same material and the same matrix
	// in all frames.
	// note that these are _frame indices_, you cannot use them with
	// model->mesh()
	QPtrList< QValueList<unsigned int> > mEqualLists;
};


class BosonModelLoaderDataPrivate
{
public:
	BosonModelLoaderDataPrivate()
	{
	}
	QIntDict<BoMesh> mMeshes;
	QIntDict<BoMaterial> mMaterials;
	QIntDict<BoFrame> mFrames;

};

BosonModelLoaderData::BosonModelLoaderData()
{
 d = new BosonModelLoaderDataPrivate;
 d->mMeshes.setAutoDelete(true);
 d->mMaterials.setAutoDelete(true);
 d->mFrames.setAutoDelete(true);
}

BosonModelLoaderData::~BosonModelLoaderData()
{
 clearMeshes(true);
 clearMaterials(true);
 clearFrames(true);
 delete d;
}

unsigned int BosonModelLoaderData::addMesh(BoMesh* mesh)
{
 unsigned int index = d->mMeshes.count();
 d->mMeshes.insert(index, mesh);
 return index;
}

unsigned int BosonModelLoaderData::meshCount() const
{
 return d->mMeshes.count();
}

BoMesh* BosonModelLoaderData::mesh(unsigned int i) const
{
 return d->mMeshes[i];
}

void BosonModelLoaderData::clearMeshes(bool delete_)
{
 d->mMeshes.setAutoDelete(delete_);
 d->mMeshes.clear();
 d->mMeshes.setAutoDelete(true);
}

unsigned int BosonModelLoaderData::addMaterial()
{
 unsigned int index = d->mMaterials.count();
 d->mMaterials.insert(index, new BoMaterial);
 return index;
}

unsigned int BosonModelLoaderData::materialCount() const
{
 return d->mMaterials.count();
}

BoMaterial* BosonModelLoaderData::material(unsigned int i) const
{
 return d->mMaterials[i];
}

void BosonModelLoaderData::clearMaterials(bool delete_)
{
 d->mMaterials.setAutoDelete(delete_);
 d->mMaterials.clear();
 d->mMaterials.setAutoDelete(true);
}

unsigned int BosonModelLoaderData::addFrame()
{
 unsigned int index = d->mFrames.count();
 d->mFrames.insert(index, new BoFrame);
 return index;
}

unsigned int BosonModelLoaderData::frameCount() const
{
 return d->mFrames.count();
}

BoFrame* BosonModelLoaderData::frame(unsigned int i) const
{
 return d->mFrames[i];
}

void BosonModelLoaderData::clearFrames(bool delete_)
{
 d->mFrames.setAutoDelete(delete_);
 d->mFrames.clear();
 d->mFrames.setAutoDelete(true);
}


BosonModelLoader::BosonModelLoader(const QString& dir, const QString& file, BosonModel* model)
{
 init();
 mDirectory = dir;
 if (mDirectory.right(1) != QString::fromLatin1("/")) {
	mDirectory += QString::fromLatin1("/");
 }
 mFile = file;
 mModel = model;
}

BosonModelLoader::~BosonModelLoader()
{
 delete mData;
}

void BosonModelLoader::init()
{
 mData = 0;
}

QString BosonModelLoader::file() const
{
 return baseDirectory() + mFile;
}

const QString& BosonModelLoader::baseDirectory() const
{
 return mDirectory;
}


bool BosonModelLoader::loadModel()
{
 if (mDirectory.isEmpty()) {
	boError() << k_funcinfo << "no directory set" << endl;
	return false;
 }
 if (mFile.isEmpty()) {
	boError() << k_funcinfo << "no file set" << endl;
	return false;
 }
 if (!mModel) {
	boError() << k_funcinfo << "no model set" << endl;
	return false;
 }
 if (mData) {
	boError() << k_funcinfo << "model already loaded?! deleting previous data" << endl;
	delete mData;
	mData = 0;
 }
 mData = new BosonModelLoaderData();
 if (mFile.right(4) == QString::fromLatin1(".3ds")) {
	// load a .3ds file
	Bo3DSLoad loader(mDirectory, mFile, mData);
	if (!loader.loadModel()) {
		boError() << k_funcinfo << "error while loading from .3ds file" << endl;
		return false;
	}
 } else if (mFile.right(3) == QString::fromLatin1(".ac")) {
	// load a .ac file
	BoACLoad loader(mDirectory, mFile, mData);
	if (!loader.loadModel()) {
		boError() << k_funcinfo << "error while loading from .ac file" << endl;
		return false;
	}
 } else {
	boError() << k_funcinfo << "don't know file suffix of " << file() << endl;
	return false;
 }

 if (!checkValidity()) {
	boError() << k_funcinfo << "data not loaded in valid format" << endl;
	return false;
 }

 // at this point the data has been loaded successfully. now let's modify it
 // (remove redundant meshes/vertices, ...)

 if (!optimizeModel()) {
	// loading the model was successfull, we just cannot use the optimized
	// data.
	return true;
 }

 return true;
}

bool BosonModelLoader::checkValidity() const
{
 if (!mData) {
	BO_NULL_ERROR(mData);
	return false;
 }
 if (mData->frameCount() < 1) {
	boError() << k_funcinfo << "no frame found" << endl;
	return false;
 }
 if (mData->meshCount() < 1) {
	boError() << k_funcinfo << "no mesh found" << endl;
	return false;
 }

 // make sure that frame0->mesh(j) == A, means framei->mesh(j) == A, too.
 // note that this is true in loading code only, in the game this is NOT always
 // true (e.g. not for construction frames)
 const BoFrame* frame0 = mData->frame(0);
 for (unsigned int i = 0; i < mData->frameCount(); i++) {
	BoFrame* framei = mData->frame(i);
	if (frame0->meshCount() != framei->meshCount()) {
		boError() << k_funcinfo << "meshCount of frame 0 does not match meshCount of frame " << i << endl;
		return false;
	}
	for (unsigned int j = 0; j < frame0->meshCount(); j++) {
		if (frame0->mesh(j) != framei->mesh(j)) {
			boError() << k_funcinfo << "mesh at index " << j << " in frame 0 does not match mesh in frame " << i << endl;
			return false;
		}
	}
 }

 return true;
}


bool BosonModelLoader::optimizeModel()
{
 static int profilingId = boProfiling->requestEventId("OptimizeModel");
 BosonProfiler profiler(profilingId);
 BosonModelLoaderOptimizer optimizer;
 optimizer.setData(mData);
 if (!optimizer.optimize()) {
	return false;
 }
 BosonModelLoaderData* data = optimizer.getOptimizedData();
// int diff = mData->frame(0)->meshCount() - data->frame(0)->meshCount();
// boDebug() << k_funcinfo << "saved " << diff << " meshes - left: " << data->frame(0)->meshCount() << endl;
 delete mData;
 mData = data;
 return true;
}



bool BosonModelLoaderOptimizer::optimize()
{
 if (!mData) {
	BO_NULL_ERROR(mData);
	return false;
 }
 if (!cloneMaterials()) {
	boError() << k_funcinfo << "could not clone materials" << endl;
	return false;
 }
 if (!cloneMeshes()) {
	boError() << k_funcinfo << "could not clone meshes" << endl;
	return false;
 }
 if (!cloneFrames()) {
	boError() << k_funcinfo << "could not clone frames" << endl;
	return false;
 }

 return true;
}

bool BosonModelLoaderOptimizer::cloneMaterials()
{
 if (!mData) {
	BO_NULL_ERROR(mData);
	return false;
 }
 QValueList<BoMaterial*> usedMaterials;
 QValueList<BoMaterial*> materials;
 for (unsigned int i = 0; i < mData->materialCount(); i++) {
	materials.append(mData->material(i));
 }
 QValueList<BoMaterial*>::Iterator it;
 for (it = materials.begin(); it != materials.end(); ++it) {
	for (unsigned int i = 0; i < mData->meshCount(); i++) {
		if (mData->mesh(i)->material() == *it) {
			usedMaterials.append(*it);
		}
	}
 }
 for (it = usedMaterials.begin(); it != usedMaterials.end(); ++it) {
	unsigned int index = mOptimizedData->addMaterial();
	BoMaterial* material = mOptimizedData->material(index);
	*material = *(*it);
	mMaterial2Material.insert(*it, material);
 }
 return true;
}

// AB: for this to work the frame->matrix()es must be the identity matrix
// whenever possible.
// the file loaders should ensure this if possible.
// AB: maybe we can pre-parse the file to ensure it?
bool BosonModelLoaderOptimizer::cloneMeshes()
{
 QPtrDict< QPtrList<BoMesh> > meshesByMaterial;
 meshesByMaterial.setAutoDelete(true);
 for (unsigned int i = 0; i < mData->meshCount(); i++) {
	BoMesh* mesh = mData->mesh(i);
	BoMaterial* mat = mesh->material();
	QPtrList<BoMesh>* list = meshesByMaterial[mat];
	if (!list) {
		list = new QPtrList<BoMesh>();
		meshesByMaterial.insert(mat, list);
	}
	list->append(mesh);
 }

 mEqualLists.clear();

 QPtrDictIterator< QPtrList<BoMesh> > it(meshesByMaterial);
 while (it.current()) {
	const BoFrame* frame0 = mData->frame(0);
	QValueList<unsigned int>* equal = new QValueList<unsigned int>;
	for (unsigned int i = 0; i < frame0->meshCount(); i++) {
		if (it.current()->contains(frame0->mesh(i))) {
			equal->append(i);
		}
	}
	mEqualLists.append(equal);
	++it;
 }

 for (QValueList<unsigned int>* equal = mEqualLists.first(); equal; equal = mEqualLists.next()) {
	QValueList<unsigned int>* rest = new QValueList<unsigned int>;
	findEqualMeshes(equal, rest);
	if (rest->count() == 0) {
		delete rest;
		rest = 0;
	} else {
		mEqualLists.append(rest);
		mEqualLists.find(equal); // set the current item again
	}
 }

 const BoFrame* frame0 = mData->frame(0);
 for (QValueList<unsigned int>* equal = mEqualLists.first(); equal; equal = mEqualLists.next()) {
	QValueList<const BoMesh*> equalMeshes;
	QValueList<unsigned int>::Iterator it;
	for (it = equal->begin(); it != equal->end(); ++it) {
		equalMeshes.append(frame0->mesh(*it));
	}
	if (!mergeMeshes(equalMeshes)) {
		boError() << k_funcinfo << "could not merge meshes!" << endl;
		return false;
	}
 }

 return true;
}

void BosonModelLoaderOptimizer::findEqualMeshes(QValueList<unsigned int>* equal, QValueList<unsigned int>* rest)
{
 rest->clear();
 if (equal->count() <= 1) {
	return;
 }
 for (unsigned int i = 0; i < mData->frameCount(); i++) {
	BoFrame* frame = mData->frame(i);
	QValueList<unsigned int> equal2;
	unsigned int first = equal->first();

	QValueList<unsigned int>::Iterator it;
	for (it = equal->begin(); it != equal->end(); ++it) {
		if (*it == first) {
			equal2.append(*it);
		} else if (meshesAreEqual(*it, first, frame)) {
			equal2.append(*it);
		} else {
			rest->append(*it);
		}
	}

	*equal = equal2;
 }
}

bool BosonModelLoaderOptimizer::mergeMeshes(QValueList<const BoMesh*> meshes)
{
 if (meshes.count() == 0) {
	return true;
 }
 unsigned int points = 0;
 unsigned int faces = 0;
 QValueList<const BoMesh*>::Iterator it;
 for (it = meshes.begin(); it != meshes.end(); ++it) {
	points += (*it)->points();
	faces += (*it)->facesCount(0);
 }
 const BoMesh* mesh0 = meshes[0];
 BoMesh* mesh = new BoMesh(faces, mesh0->name());
 mOptimizedData->addMesh(mesh);
 for (it = meshes.begin(); it != meshes.end(); ++it) {
	mMesh2Mesh.insert(*it, mesh);
 }

 mesh->setMaterial(mMaterial2Material[mesh0->material()]);
 mesh->setIsTeamColor(mesh0->isTeamColor());

 mesh->allocatePoints(points);
 QValueVector<BoVector3Float> vertices(points);
 QValueVector<BoVector3Float> texels(points);
 unsigned int pointIndex = 0;
 unsigned int faceIndex = 0;
 for (it = meshes.begin(); it != meshes.end(); ++it) {
	unsigned int meshPointOffset = pointIndex;
	for (unsigned int i = 0; i < (*it)->points(); i++) {
		vertices[pointIndex] = (*it)->vertex(i);
		texels[pointIndex] = (*it)->texel(i);
		pointIndex++;
	}
	for (unsigned int i = 0; i < (*it)->facesCount(0); i++) {
		BoFace face(*(*it)->face(i));
		int p[3];
		p[0] = face.pointIndex()[0] + meshPointOffset;
		p[1] = face.pointIndex()[1] + meshPointOffset;
		p[2] = face.pointIndex()[2] + meshPointOffset;
		face.setPointIndex(p);
		mesh->setFace(faceIndex, face);
		faceIndex++;
	}
 }

 mesh->setVertices(vertices);
 mesh->setTexels(texels);

 return true;
}

bool BosonModelLoaderOptimizer::meshesAreEqual(unsigned int i1, unsigned int i2, const BoFrame* frame) const
{
 BoMesh* mesh1 = frame->mesh(i1);
 BoMesh* mesh2 = frame->mesh(i2);
 BoMatrix* mat1 = frame->matrix(i1);
 BoMatrix* mat2 = frame->matrix(i2);
 if (!mesh1 || !mesh2 || !mat1 || !mat2) {
	boError() << k_funcinfo << "invalid indices" << endl;
	return false;
 }
 if (mesh1->isTeamColor() != mesh2->isTeamColor()) {
	return false;
 }
 if (!mat1->isEqual(*mat2)) {
	return false;
 }
 return true;
}

bool BosonModelLoaderOptimizer::cloneFrames()
{
 QIntDict< QValueList<unsigned int> > a;
 for (unsigned int i = 0; i < mData->frameCount(); i++) {
	unsigned int index = mOptimizedData->addFrame();
	if (index != i) {
		boError() << k_funcinfo << "frame has unexpected index " << index << " - should be " << i << endl;
		return false;
	}
	const BoFrame* origFrame = mData->frame(index);
	BoFrame* frame = mOptimizedData->frame(index);
	frame->allocMeshes(mEqualLists.count());

	unsigned int meshIndex = 0;
	QPtrListIterator< QValueList<unsigned int> > it(mEqualLists);
	while (it.current()) {
		unsigned int origMeshIndex = it.current()->first();
		const BoMesh* origMesh = origFrame->mesh(origMeshIndex);
		if (!origMesh) {
			BO_NULL_ERROR(origMesh);
			return false;
		}
		BoMesh* mesh = mMesh2Mesh[origMesh];
		if (!mesh) {
			BO_NULL_ERROR(mesh);
			return false;
		}

		frame->setMesh(meshIndex, mesh);
		BoMatrix* matrix = frame->matrix(meshIndex);
		*matrix = *origFrame->matrix(origMeshIndex);

		meshIndex++;
		++it;
	}
 }
 return true;
}

