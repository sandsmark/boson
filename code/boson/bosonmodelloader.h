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
#ifndef BOSONMODELLOADER_H
#define BOSONMODELLOADER_H

#include <qstring.h>

class BoVector3;
class BoMatrix;
class BoMesh;
class BoFrame;
class BoMaterial;
class BosonModel;

class KSimpleConfig;

class QString;
class QStringList;
template<class T> class QPtrList;
template<class T, class T2> class QMap;


class BosonModelLoaderDataPrivate;
class BosonModelLoaderData
{
public:
	BosonModelLoaderData();
	~BosonModelLoaderData();

	/**
	 * Add a mesh. This class takes ownership, i.e. will delete the pointer!
	 * @return The index of the mesh. The index is increased sequentially
	 * after every call, i.e. will be 0, then 1, 2, 3, ...
	 **/
	unsigned int addMesh(BoMesh*);
	BoMesh* mesh(unsigned int i) const;
	unsigned int meshCount() const;
	/**
	 * Remove all meshes from this class. This prevents the meshes to be
	 * deleted on destruction.
	 **/
	void clearMeshes(bool delete_ = true);

	unsigned int addMaterial();
	BoMaterial* material(unsigned int i) const;
	unsigned int materialCount() const;
	void clearMaterials(bool delete_ = true);

	unsigned int addFrame();
	BoFrame* frame(unsigned int i) const;
	unsigned int frameCount() const;
	void clearFrames(bool delete_ = true);

private:
	BosonModelLoaderDataPrivate* d;
};

/**
 * This class takes care of the actual from-file loading. It will also do some
 * additional work, once file loading is done, such as removing unused meshes,
 * removing duplicates vertices and so on. But all non-trivial post-processing
 * will get done in @ref BosonModel (such as LOD).
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class BosonModelLoader
{
public:
	/**
	 * Load the file @p file, that resided in @p dir into @p model.
	 * @param dir Must be an absolute path to a directory containing @p
	 * file.
	 **/
	BosonModelLoader(const QString& dir, const QString& file, BosonModel* model);
	~BosonModelLoader();

	/**
	 * @return The data of the model. Use @ref loadModel before you call
	 * this (will be NULL otherwise) !
	 **/
	BosonModelLoaderData* data() const
	{
		return mData;
	}

	bool loadModel();

	/**
	 * @return The absolute filename to the .3ds file of this model.
	 **/
	QString file() const;

protected:
	/**
	 * @return The directory that contains the .3ds file. Usually the unit
	 * directory
	 **/
	const QString& baseDirectory() const;

	bool optimizeModel();
	bool checkValidity() const;

private:
	void init();

private:
	QString mDirectory;
	QString mFile;
	BosonModel* mModel;

	BosonModelLoaderData* mData;
};


#endif

