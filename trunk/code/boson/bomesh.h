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

#ifndef BOMESH_H
#define BOMESH_H

#include "bo3dtools.h"

#include <qptrlist.h>

class BoMesh;
class BoMaterial;
class BoAdjacentDataBase;
class BoMeshLOD;
class QColor;

// AB: there are two different ways for normals: store one normal per face
// ("surface normal") or store one normal per vertex, i.e. 3 per face ("vertex
// normal").
// the "surface normal" is said to produce objects that appear "flat", whereas
// the "vertex normals" are used for "curved" objects. so vertex normals would
// be nicer, but take three times as much memory.
// i seriously think that we don't need vertex normals. our camera rarely moves
// *that* close that you can see whether the mesh appears slightly more flat or
// not.
// atm we don't have code for vertex normals anyway.
//
// use 0 here to use vertex normals.
#define BOMESH_USE_1_NORMAL_PER_FACE 1

class BoFace
{
public:
	BoFace();
	BoFace(const BoFace& face)
	{
		*this = face;
	}

	BoFace& operator=(const BoFace& face);

	void setPointIndex(const int* points)
	{
		mPointIndex[0] = points[0];
		mPointIndex[1] = points[1];
		mPointIndex[2] = points[2];
	}

	const int* pointIndex() const
	{
		return mPointIndex;
	}

	/**
	 * Use @þ normal for all vertices in this face
	 **/
	void setAllNormals(const BoVector3 normal)
	{
#if BOMESH_USE_1_NORMAL_PER_FACE
		setNormal(0, normal);
#else
		setNormal(0, normal);
		setNormal(1, normal);
		setNormal(2, normal);
#endif
	}
	void setNormal(unsigned int i, const BoVector3 normal)
	{
#if BOMESH_USE_1_NORMAL_PER_FACE
		i = 0;
#else
		// values > 2 are not allowed
		i = i % 3;
#endif
		mNormals[i] = normal;
	}
	inline const BoVector3& normal(unsigned int i) const
	{
#if BOMESH_USE_1_NORMAL_PER_FACE
		Q_UNUSED(i);
		return mNormals[0];
#else
		// we don't do i = i % 3; as of performance reasons
		return mNormals[i];
#endif
	}

private:
	int mPointIndex[3];

#if BOMESH_USE_1_NORMAL_PER_FACE
	BoVector3 mNormals[1];
#else
	BoVector3 mNormals[3];
#endif
};

/**
 * This class stores the way faces are connected. At the moment faces will
 * always be connected in a linear way (i.e. face1,face2,face3,...). But we
 * could also connect them so that we use triangle strips.
 **/
class BoFaceNode
{
public:
	/**
	 * Construct a node. Note that a node is 100% unusable without a valid
	 * @p BoFace object.
	 **/
	BoFaceNode(const BoFace* face);
	~BoFaceNode();

	void setPrevious(BoFaceNode* previous);
	void setNext(BoFaceNode* next);

	BoFaceNode* next() const
	{
		return mNext;
	}
	BoFaceNode* previous() const
	{
		return mPrevious;
	}

	int relevantPoint() const
	{
		return mRelevantPoint;
	}
	void setRelevantPoint(int p)
	{
		mRelevantPoint = p;
	}
	void encodeRelevantPoint(int firstPoint, int secondPoint, int thirdPoint)
	{
		int r = 0;
		r += firstPoint << 4;
		r += secondPoint << 2;
		r += thirdPoint;
		setRelevantPoint(r);
	}

	// the point is encoded for the *first* node only!
	// this will return stupid stuff for all other nodes!
	void decodeRelevantPoint(int* firstPoint, int* secondPoint, int* thirdPoint)
	{
		int r = relevantPoint();
		if (r < 0) {
			// oops!
			*firstPoint = -1;
			*secondPoint = -1;
			*thirdPoint = -1;
			return;
		}
		*firstPoint = r >> 4;
		r -= *firstPoint << 4;
		*secondPoint = r >> 2;
		r -= *secondPoint << 2;
		*thirdPoint = r;
	}

	/**
	 * @return The <em>local</em> index (i.e. 0..2) of the <em>point</em>
	 * index @p index (i.e. the index of the vertex in the vertex list). Or
	 * -1 if @p index is not a point of this node/face.
	 **/
	int findPointIndex(int index) const;

	inline const BoFace* face() const
	{
		return mFace;
	}
	const int* pointIndex() const
	{
		return face()->pointIndex();
	}

	void delNode();

	static bool isAdjacent(BoFaceNode* face1, BoFaceNode* face2);
	QString debugString() const;

private:
	void init();

private:
	BoFaceNode* mNext;
	BoFaceNode* mPrevious;
	int mRelevantPoint;
	const BoFace* mFace;
};


class BoMeshLODPrivate;
/**
 * This class is a collection of all data that depend on the current level of
 * detail (LOD).
 *
 * Note that some values/arrays (such as the point cache) are automatically
 * generated <em>after</em> the LOD has been generated. This happens for all
 * meshes and all LODs, so you don't have to worry about this for LOD. But it
 * also depends on the current LOD, so it is in this class.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMeshLOD
{
public:
	BoMeshLOD();
	~BoMeshLOD();

	void createFaces(unsigned int faces);

	inline BoFaceNode* nodes() const
	{
		return mNodes;
	}
	inline int type() const
	{
		return mType;
	}
	inline unsigned int* pointsCache() const
	{
		return mPointsCache;
	}
	inline unsigned int pointsCacheCount() const
	{
		return mPointsCacheCount;
	}

	/**
	 * @return The number of faces/triangles (i.e. nodes) in this mesh. Use
	 * the constructor to create the correct number.
	 **/
	unsigned int facesCount() const;

	void setFace(int index, const BoFace& face);
	const BoFace* face(unsigned int f) const;

	void setDisplayList(GLuint list)
	{
		mDisplayList = list;
	}
	inline GLuint displayList() const
	{
		return mDisplayList;
	}

	/**
	 * @param vertex The vertex in the face this normal applies to. This
	 * must be 0..2 or -1 for all vertices.
	 **/
	void setNormal(unsigned int face, int vertex, const BoVector3& normal);

	/**
	 * Called by @ref movePoints. This adds @p moveBy to every @ref
	 * BoFace::pointIndex.
	 **/
	void movePointIndices(int moveBy);

	void createPointCache();

	/**
	 * Ensure that all values in the point cache are valid. After this
	 * function is done, all entries in the point cache are greate or equal
	 * @p min and less or equal @p max.
	 *
	 * Note the "or equal" for @p max!
	 **/
	void ensurePointCacheValid(unsigned int min, unsigned int max);

	void disconnectNodes();
	void connectNodes();
	void addNodes();

private:
	BoMeshLODPrivate* d;

private:
	BoFaceNode* mNodes;
	int mType;

	// the list of points in the final order (after connectNodes() or
	// addNodes() was called). iterating through nodes() is equalivent (for
	// some modes the BoFaceNode::relevantPoint() will have to be used though)
	unsigned int* mPointsCache;
	unsigned int mPointsCacheCount;

	GLuint mDisplayList;
};


class BoMeshPrivate;
class BoMesh
{
public:
	/**
	 * @param faces The number of faces (triangles) in this mesh to be
	 * created. You must use @ref setFace to initialize them.
	 **/
	BoMesh(unsigned int faces);
	~BoMesh();

	/**
	 * The size of a single points (vertex and texel). Size means
	 * the number of floats here.
	 **/
	static int pointSize();

	/**
	 * @return The position of the vertex in a point (see @ref pointSize)
	 */
	static int vertexPos();

	static int texelPos();

	/**
	 * Prepare to load the points, i.e. allocate memory for them. You can
	 * set them using @ref setVertex and @ref setTexel.
	 **/
	void allocatePoints(unsigned int points);

	/**
	 * Move the points from the local array to the specified array. This
	 * will copy all points (vertices and texture coordinates). The points
	 * are inserted starting at @p index - all local indices are changed
	 * (i.e. increased by @p index).
	 *
	 * Note that the points are moved only, not changed. I.e. as long as you
	 * use the correct (maybe modified) index @ref point will return the
	 * same point.
	 *
	 * @return The number of points (!) that have been used. Note: a single
	 * point consists of @ref pointSize floats!
	 **/
	unsigned int movePoints(float* array, int index);

	/**
	 * Use material @p mat when rendering this mesh.
	 **/
	void setMaterial(BoMaterial* mat);

	BoMaterial* material() const;

	/**
	 * This adds the @p face to the default LOD (i.e. the full-detailed
	 * version) of this mesh. The face will be at @p index.
	 *
	 * Note that once @ref generateLOD was called you should not call
	 * setFace() anymore, as it will apply to the full-detailed version of
	 * the mesh only. As long as LODs aren't generated you don't have to
	 * care about LOD (@ref generateLOD will do everything).
	 **/
	void setFace(int index, const BoFace& face);

	/**
	 * You must call @ref allocatePoints before calling this!
	 * @param index The index of the vertex in the vertex pool (relative to
	 * this mesh). It must be < @ref points.
	 *
	 * Note that this changes the vertex at @p index for all LODs that
	 * reference that index!
	 **/
	void setVertex(unsigned int index, const BoVector3&);

	void calculateNormals();

	/**
	 * The third coordinate is discarded.
	 * @param index See @ref setVertex.
	 *
	 * Note that this changes the texel at @p index for all LODs that
	 * reference that index!
	 **/
	void setTexel(unsigned int index, const BoVector3&);

	/**
	 * Try to connect all faces in the mesh, so that we can use
	 * GL_TRIANGLE_STRIP. If that doesn't work this function will add all
	 * faces completely instead. See @ref addNodes.
	 *
	 * Note that this is not yet working!
	 **/
	void connectNodes();

	/**
	 * Add all nodes from @ref mesh, so that we can use GL_TRIANGLES. You
	 * should prefer @ref connectNodes usually (in case it was working).
	 **/
	void addNodes();

	/**
	 * Generate a point list (as it can be used by glDrawElements()) from
	 * the node list (see @ref nodes).
	 *
	 * The point cache is an array of all vertices that are referenced by
	 * the nodes/faces (exact: nodes, although the difference is not too
	 * big). So if face 1 references the vertices 2,4,7 and face 2
	 * references 7,5,9 then the point cache is 2,4,7,7,5,9
	 *
	 * The point cache can be used directly for the indices in
	 * glDrawElements().
	 *
	 * This cache will be invalid once @ref connectNodes or @ref addNodes
	 * gets called (i.e. the order of points get changed in any way).
	 **/
	void createPointCache();

	/**
	 * Disconnect all nodes to prepare another @ref connectNodes or @ref
	 * addNodes call. No node is deleted.
	 **/
	void disconnectNodes();

	/**
	 * Set whether this mesh is a teamcolor object or not.
	 **/
	void setIsTeamColor(bool teamColor);

	/**
	 * @return TRUE if this is a teamcolor object (which also is not textured)
	 * and FALSE if it is not a teamcolor object.
	 **/
	bool isTeamColor() const;

	/**
	 * @return material()->textureObject() if @ref material is non-null,
	 * otherwise 0.
	 **/
	GLuint textureObject() const;

	void renderMesh(const QColor* color);

	void loadDisplayList(const QColor* teamColor, bool reload = false);
	GLuint displayList(unsigned int lod) const;


	/**
	 * Create a BoVector3 at index @p p from the vertex pool.
	 *
	 * @param p The index of the vertex in the vertex pool. Must
	 * be < @ref points.
	 **/
	BoVector3 vertex(unsigned int p) const;


	/**
	 * @return The number of points in this mesh. See also @ref facesCount
	 **/
	unsigned int points() const;

	/**
	 * Calculate values for @ref maxZPoint and similar functions. This needs
	 * to get called whenever the values might change!
	 **/
	void calculateMaxMin();

	/**
	 * @return The maximal x value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float maxX() const;

	/**
	 * @return The minimal x value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float minX() const;

	/**
	 * @return The maximal y value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float maxY() const;

	/**
	 * @return The minimal y value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float minY() const;

	/**
	 * @return The maximal z value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float maxZ() const;

	/**
	 * @return The minimal z value in this mesh. Call @ref calculateMaxMin
	 * before you use this
	 **/
	float minZ() const;

	/**
	 * Compute a bounding object (usually a box) for the mesh.
	 **/
	void computeBoundingObject();

	/**
	 * Called by @ref BosonModel to generate LODs for the mesh.
	 **/
	void generateLOD();

	unsigned int facesCount(unsigned int lod) const;

protected:
	// this is meant to check whether the something on the screen will
	// change if we draw this mesh now.
	bool checkVisible();

	BoMeshLOD* levelOfDetail(unsigned int lod) const;
	unsigned int lodCount() const;

	/**
	 * @overload
	 *
	 * This will create a @ref BoVector3 for the vertex @p i in @p face.
	 *
	 * Note: this is protected, because I don't want to make the @p _lod
	 * public. If possible people who write e.g. fileloaders should not cope
	 * with lod at all.
	 * @param lod See @ref levelOfDetail
	 **/
	BoVector3 vertex(unsigned int face, unsigned int i, unsigned int lod) const;

	void calculateNormals(unsigned int lod);

	void setNormal(unsigned int face, int vertex, const BoVector3& normal);
	void loadDisplayList(BoMeshLOD* lod, const QColor* teamColor, bool reload = false);

private:
	void init();

private:
	BoMeshPrivate* d;
};

#endif
