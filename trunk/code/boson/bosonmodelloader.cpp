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
#include "bodebug.h"

#include <qstring.h>
#include <qintdict.h>


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


 // at this point the data has been loaded successfully. now let's modify it
 // (remove redundant meshes/vertices, ...)


 return true;
}




