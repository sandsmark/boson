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

#ifndef BOMESH_H
#define BOMESH_H

#include "bo3dtools.h"

#include <qptrlist.h>

class BoMesh;
class BoAdjacentDataBase;
class QColor;

class BoNode
{
public:
	BoNode(BoNode* previous);
	BoNode();
	~BoNode();

	void setPrevious(BoNode* previous);
	void setNext(BoNode* next);

	/**
	 * @param points An array of three with inidces to the vertices and
	 * texels (in case the mesh is textured) of this face in @ref BoMesh.
	 **/
	void setFace(const int* points);

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

	const int* pointIndex() const { return &mPointIndex[0]; }

	void delNode();

	static bool isAdjacent(BoNode* face1, BoNode* face2);
	QString debugString() const;

private:
	void init();

private:
	BoNode* mNext;
	BoNode* mPrevious;
	int mRelevantPoint;
	int mPointIndex[3];
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
	 * @return The number of faces/triangles (i.e. nodes) in this mesh. Use
	 * the constructor to create the correct number.
	 **/
	unsigned int facesCount() const;

	/**
	 * @param See @ref BoNode::setFace. Remember to add the actual points to
	 * this mesh! See especially @ref setVertex
	 **/
	void setFace(int index, const int* points);

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
	 **/
	void movePoints(float* array, int index);

	/**
	 * You must call @ref allocatePoints before calling this!
	 **/
	void setVertex(unsigned int index, const BoVector3&);

	/**
	 * The third coordinate is discarded.
	 **/
	void setTexel(unsigned int index, const BoVector3&);

	/**
	 * Try to connect all faces in @ref mesh, so that we can use
	 * GL_TRIANGLE_STRIP. If that doesn't work this function will add all
	 * faces completely instead. See @ref addFaces
	 **/
	void connectFaces();

	/**
	 * Add all faces from @ref mesh, so that we can use GL_TRIANGLES. You
	 * should prefer @ref connectFaces usually.
	 **/
	void addFaces();

	/**
	 * Generate a point list (as it can be used by glDrawElements()) from
	 * the node list (see @ref faces).
	 *
	 * This cache will be invalid once @ref connectFaces or @ref addFaces
	 * gets called (i.e. the order of points get changed in any way).
	 **/
	void createPointCache();

	/**
	 * Delete all faces. See @ref faces
	 * @obsolete
	 **/
	void deleteFaces();

	/**
	 * Delete all nodes starting at @p node. If it has a previous face,
	 * @ref BoNode::previous is set to 0 before deleting the node.
	 * @obsolete
	 **/
	void deleteNodes(BoNode* node);

	/**
	 * Disconnect all faces to prepare another @ref connectFaces or @ref
	 * addFaces call. No node/face is deleted.
	 **/
	void disconnectFaces();

	int type() const;

	BoNode* faces() const;

	/**
	 * Set whether this mesh is a teamcolor object or not.
	 **/
	void setIsTeamColor(bool teamColor);

	/**
	 * @return TRUE if this is a teamcolor object (which also is not textured)
	 * and FALSE if it is not a teamcolor object.
	 **/
	bool isTeamColor() const;
	
	void setTextured(bool isTextured);
	void setTextureObject(GLuint tex);
	GLuint textureObject() const;
	bool textured() const;

	void renderMesh(const QColor* color);

	void loadDisplayList(const QColor* teamColor, bool reload = false);
	GLuint displayList() const;


	/**
	 * Create a BoVector3 at index @p p
	 **/
	BoVector3 point(unsigned int p) const;
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

protected:
	void createNodes(unsigned int faces);
	bool connectFaces(const BoAdjacentDataBase* database, const QPtrList<BoNode>& faces, QPtrList<BoNode>* found, BoNode* node) const;


	// this is meant to check whether the something on the screen will
	// change if we draw this mesh now.
	bool checkVisible();

private:
	void init();

private:
	BoMeshPrivate* d;
};

#endif
