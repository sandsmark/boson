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

#include <lib3ds/types.h>
#include <qptrlist.h>

class BoMesh;
class BoAdjacentDataBase;

class BoNode
{
public:
	BoNode(BoNode* previous);
	BoNode();
	~BoNode();

	void setPrevious(BoNode* previous);
	void setNext(BoNode* next);

	void setFace(Lib3dsFace* f);

	Lib3dsFace* face() const
	{
		return mFace;
	}

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
	Lib3dsFace* mFace;
	int mRelevantPoint;
	int mPointIndex[3];
};


class BoMeshPrivate;
class BoMesh
{
public:
	BoMesh(Lib3dsMesh* mesh);
	~BoMesh();

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

	Lib3dsMesh* mesh() const;

protected:
	void createNodes();
	bool connectFaces(const BoAdjacentDataBase* database, const QPtrList<BoNode>& faces, QPtrList<BoNode>* found, BoNode* node) const;

private:
	BoMeshPrivate* d;
};

#endif
