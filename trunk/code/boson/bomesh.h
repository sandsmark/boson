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
class QColor;

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

private:
	int mPointIndex[3];
};

/**
 * This class stores the way faces are connected. At the moment faces will
 * always be connected in a linear way (i.e. face1,face2,face3,...). But we
 * could also connect them so that we use triangle strips.
 **/
class BoNode
{
public:
	/**
	 * Construct a node. Note that a node is 100% unusable without a valid
	 * @p BoFace object.
	 **/
	BoNode(const BoFace* face);
	~BoNode();

	void setPrevious(BoNode* previous);
	void setNext(BoNode* next);

	BoNode* next() const
	{
		return mNext;
	}
	BoNode* previous() const
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

	static bool isAdjacent(BoNode* face1, BoNode* face2);
	QString debugString() const;

private:
	void init();

private:
	BoNode* mNext;
	BoNode* mPrevious;
	int mRelevantPoint;
	const BoFace* mFace;
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
	 * The size of a single points (vertex and texel and normal). Size means
	 * the number of floats here.
	 **/
	static int pointSize();

	/**
	 * @return The position of the vertex in a point (see @ref pointSize)
	 */
	static int vertexPos();

	static int texelPos();

	/**
	 * @return The position of the normal in a point (see @ref pointSize)
	 */
	static int normalPos();

	/**
	 * Use material @p mat when rendering this mesh.
	 **/
	void setMaterial(BoMaterial* mat);

	BoMaterial* material() const;

	/**
	 * @return The number of faces/triangles (i.e. nodes) in this mesh. Use
	 * the constructor to create the correct number.
	 **/
	unsigned int facesCount() const;

	/**
	 * @param See @ref BoNode::setFace. Remember to add the actual points to
	 * this mesh! See especially @ref setVertex
	 **/
	void setFace(int index, const int* points);
	void setFace(int index, const BoFace& face);

	/**
	 * Move the points from the local array to the specified array. This
	 * will copy all points (vertices and texture coordinates). The points
	 * are inserted starting at @p index - all local indices are changed
	 * (i.e. increased by @p index).
	 *
	 * Note that the points are moved only, not changed. I.e. as long as you
	 * use the correct (maybe modified) index @ref point will return the
	 * same point.
	 **/
	void movePoints(float* array, int index);

	/**
	 * You must call @ref allocatePoints before calling this!
	 **/
	void setVertex(unsigned int index, const BoVector3&);
	void setNormal(unsigned int index, const BoVector3&);

	void calculateNormals();

	/**
	 * The third coordinate is discarded.
	 **/
	void setTexel(unsigned int index, const BoVector3&);

	/**
	 * Try to connect all faces in @ref mesh, so that we can use
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
	 * Delete all nodes. See @ref nodes
	 * @obsolete
	 **/
	void deleteNodes();

	/**
	 * Delete all nodes starting at @p node. If it has a previous face,
	 * @ref BoNode::previous is set to 0 before deleting the node.
	 * @obsolete
	 **/
	void deleteNodes(BoNode* node);

	/**
	 * Disconnect all nodes to prepare another @ref connectNodes or @ref
	 * addNodes call. No node is deleted.
	 **/
	void disconnectNodes();

	int type() const;

	BoNode* nodes() const;

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
	GLuint displayList() const;


	/**
	 * Create a BoVector3 at index @p p
	 **/
	BoVector3 vertex(unsigned int p) const;

	/**
	 * @return The normal of of the point at @p p. Note that the normal
	 * applies to the entire face, which consists of all three points at
	 * (p - p % 3) to (p - p % 3 + 2)
	 **/
	BoVector3 normal(unsigned int p) const;

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

protected:
	void createFaces(unsigned int faces);
	bool connectNodes(const BoAdjacentDataBase* database, const QPtrList<BoNode>& nodes, QPtrList<BoNode>* found, BoNode* node) const;


	// this is meant to check whether the something on the screen will
	// change if we draw this mesh now.
	bool checkVisible();

	/**
	 * Prepare to load the points, i.e. allocate memory for them. You can
	 * set them using @ref setVertex and @ref setTexel.
	 **/
	void allocatePoints(unsigned int points);


private:
	void init();

private:
	BoMeshPrivate* d;
};

#endif
