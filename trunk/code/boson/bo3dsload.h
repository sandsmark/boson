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
#ifndef BO3DSLOAD_H
#define BO3DSLOAD_H

#include <qstring.h>
#include <qptrdict.h>

#include <GL/gl.h>

#include <lib3ds/types.h>

class KSimpleConfig;
class QString;
class QStringList;
class BoMatrix;
class BoMesh;
class BoFrame;
class BosonModelLoaderData;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;
template<class T> class QPtrList;
template<class T, class T2> class QMap;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class Bo3DSLoad
{
public:
	Bo3DSLoad(const QString& dir, const QString& file, BosonModelLoaderData* data);
	~Bo3DSLoad();

	bool loadModel();

	void finishLoading();

	/**
	 * @return The absolute filename to the .3ds file of this model.
	 **/
	QString file() const;

	/**
	 * @return A list of all textures used in this model. Note that these
	 * are the plain names, i.e. not necessarily the final filenames. Use
	 * @ref cleanTextureName to receive the final fileName.
	 **/
	static QStringList textures(Lib3dsFile* file);
	QStringList textures() const;

	static bool isTeamColor(const Lib3dsMesh* mesh);
	static QString textureName(const Lib3dsMesh* mesh, Lib3dsFile* file);
	static Lib3dsMaterial* material(const Lib3dsMesh* mesh, Lib3dsFile* file);

protected:
	void loadMesh(Lib3dsNode* node);
	bool loadFrame(int frame);
	void countNodes(Lib3dsNode* node, int* count);
	void loadFrameNode(BoFrame* frame, int* index, Lib3dsNode* node);
	void loadVertices(BoMesh* mesh, Lib3dsMesh* mesh);
	void loadTexels(BoMesh* mesh, Lib3dsMesh* mesh, Lib3dsMaterial* material);
	void loadFaces(BoMesh* mesh, Lib3dsMesh* mesh);
	bool loadMaterials(BosonModelLoaderData* data, Lib3dsMaterial* firstMaterial);

	/**
	 * Render the specified node according to the values for the current
	 * frame. You should call lib3ds_file_eval(frameNumber) before calling
	 * renderNode().
	 *
	 * Note that only one node (+ all child nodes) will be rendered - you
	 * should iterate all toplevel nodes and call renderNode() for all of
	 * them usually.
	 **/
	void renderNode(Lib3dsNode* node);

	/**
	 * @return The directory that contains the .3ds file. Usually the unit
	 * directory
	 **/
	const QString& baseDirectory() const;

public:
	/**
	 * @param v A single vector
	 **/
	void dumpVector(Lib3dsVector v);

	/**
	 * @param v An array of 3 BoVector3
	 * @param texture none if 0, otherwise the textue object
	 * @param tex if texture is non-null this must be the texture
	 * coordinates (array of 3) as provided for glTexCoord*()
	 **/
	static void dumpTriangle(BoVector3Float* v, GLuint texture = 0, Lib3dsTexel* tex = 0);
	static void dumpTriangle(Lib3dsVector* v, GLuint texture = 0, Lib3dsTexel* tex = 0);

	/**
	 * Create 3 vectors from @p face in @p mesh and place them into @p v.
	 * @param v An array of size 3 which will contain the vectors of the face.
	**/
	static void makeVectors(BoVector3Float* v, const Lib3dsMesh* mesh, const Lib3dsFace* face);

	/**
	 * Find adjacent faces in @p mesh and place them into @p adjacentFaces.
	 * The search will start at @p search if @p search is non-NULL,
	 * otherwise the first face of @p mesh is used.
	 *
	 * Once an adjacent face is found this function also searches for
	 * adjacent faces of the new face and so on.
	 **/
	static void findAdjacentFaces(QPtrList<Lib3dsFace>* adjacentFaces, Lib3dsMesh* mesh, Lib3dsFace* search = 0);

private:
	void init();

private:
	Lib3dsFile* m3ds;
	QString mDirectory;
	QString mFile;
	BosonModelLoaderData* mData;

	QPtrDict<BoMesh> mMesh2Mesh; // Lib3dsMesh to BoMesh
};


#endif

