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

#include "bomesh.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "bomaterial.h"
#include "bomemorytrace.h"
#include "bolodbuilder.h"

#include <qptrlist.h>
#include <qcolor.h>
#include <qvaluevector.h>

#include <GL/gl.h>

#define AB_DEBUG_1 0

class BoundingObject
{
public:
	BoundingObject()
	{
	}
	virtual ~BoundingObject()
	{
	}

	virtual void render() = 0;
};
class BoundingBox : public BoundingObject
{
public:
	BoundingBox() : BoundingObject()
	{
	}
	~BoundingBox()
	{
	}
	virtual void render()
	{
		glBegin(GL_QUADS);
			renderQuad(0);
			renderQuad(4);
			renderQuad(8);
			renderQuad(12);
			renderQuad(16);
			renderQuad(20);
		glEnd();
	}
	void setVertex(int i, float* coords)
	{
		mVectors[i].set(coords);
	}

protected:
	inline void renderQuad(int startVertex)
	{
		glVertex3fv(mVectors[startVertex].data());
		glVertex3fv(mVectors[startVertex + 1].data());
		glVertex3fv(mVectors[startVertex + 2].data());
		glVertex3fv(mVectors[startVertex + 3].data());
	}

private:
	BoVector3 mVectors[24];
};

// this should be able to generate bounding boxes, spheres or maybe just
// bounding polygons.
// it can be used to check visibility, i.e. so that we do not render the mesh if
// it is behind other meshes/units/whatever.
class BoundingObjectBuilder
{
public:
	BoundingObjectBuilder()
	{
	}
	~BoundingObjectBuilder()
	{
	}

	/**
	 * @return A new'ed object (you are responsible of deleting it!) - the
	 * best kind of object should be chosen.
	 **/
	BoundingObject* generateBoundingObject(BoMesh* mesh)
	{
		// we support bounding boxes only :-(
		return (BoundingObject*)generateBoundingBox(mesh);
	}

	BoundingBox* generateBoundingBox(BoMesh* mesh)
	{
		BoundingBox* box = new BoundingBox();

		// note: we use a very naive algorithm, which constructs boxes
		// that are always parallel to the x/y/z axis. this means we
		// have bad bounding boxes - they could easily be smaller.
		// but that would be harder to code...
		float v[3];
		mesh->calculateMaxMin(); // probably redundant! we need to ensure that these values are final!

		v[0] = mesh->maxX();
		v[1] = mesh->maxY();
		v[2] = mesh->maxZ();
		box->setVertex(0, v);
		box->setVertex(4, v);
		box->setVertex(8, v);

		v[0] = mesh->maxX();
		v[1] = mesh->maxY();
		v[2] = mesh->minZ();
		box->setVertex(1, v);
		box->setVertex(11, v);
		box->setVertex(12, v);

		v[0] = mesh->maxX();
		v[1] = mesh->minY();
		v[2] = mesh->maxZ();
		box->setVertex(3, v);
		box->setVertex(5, v);
		box->setVertex(16, v);

		v[0] = mesh->maxX();
		v[1] = mesh->minY();
		v[2] = mesh->minZ();
		box->setVertex(2, v);
		box->setVertex(15, v);
		box->setVertex(17, v);

		v[0] = mesh->minX();
		v[1] = mesh->maxY();
		v[2] = mesh->maxZ();
		box->setVertex(9, v);
		box->setVertex(7, v);
		box->setVertex(20, v);

		v[0] = mesh->minX();
		v[1] = mesh->maxY();
		v[2] = mesh->minZ();
		box->setVertex(23, v);
		box->setVertex(10, v);
		box->setVertex(13, v);

		v[0] = mesh->minX();
		v[1] = mesh->minY();
		v[2] = mesh->maxZ();
		box->setVertex(6, v);
		box->setVertex(19, v);
		box->setVertex(21, v);

		v[0] = mesh->minX();
		v[1] = mesh->minY();
		v[2] = mesh->minZ();
		box->setVertex(14, v);
		box->setVertex(18, v);
		box->setVertex(22, v);

		return box;
	}
};

class BoAdjacentDataBase
{
public:
	BoAdjacentDataBase(const QPtrList<BoFaceNode>& allNodes)
	{
		addNodes(allNodes);
	}

	/**
	 * Add all nodes to the databse. construct the lists of adjacent
	 * nodes/faces.
	 **/
	void addNodes(const QPtrList<BoFaceNode>& allNodes)
	{
		mAllNodes = allNodes;
		QPtrListIterator<BoFaceNode> it(allNodes);
		for (; it.current(); ++it) {
			QPtrListIterator<BoFaceNode> it2(allNodes);
			it2 = it;
			++it2; // we don't care about it==it2, so skip it
			for (; it2.current(); ++it2) {
				if (it.current() == it2.current()) {
					continue;
				}
				if (mDataBase[it.current()].containsRef(it2.current())) {
					continue;
				}
				// mDataBase[it2.current()].containsRef(it.current())
				// doesn't need to be checked, as we always add
				// adjacent nodes/faces to both lists
				if (BoFaceNode::isAdjacent(it.current(), it2.current())) {
					mDataBase[it.current()].append(it2.current());
					mDataBase[it2.current()].append(it.current());
				}
			}
		}
	}
	void debug()
	{
		QPtrListIterator<BoFaceNode> it(mAllNodes);
		for (; it.current(); ++it) {
			boDebug(100) << "adjacent to " << it.current()->debugString() << " (Face " << mAllNodes.find(it.current()) + 1 << "):" << endl;
			QPtrList<BoFaceNode> a = adjacent(it.current());
			QPtrListIterator<BoFaceNode> it2(a);
			for (; it2.current(); ++it2) {
				boDebug(100) << it2.current()->debugString() << " (Face " << mAllNodes.find(it2.current()) + 1 << ")" << endl;
			}

		}
	}

	/**
	 * @return a list of faces that are adjacent to @p node
	 **/
	QPtrList<BoFaceNode> adjacent(BoFaceNode* node) const
	{
		if (!node) {
			return mAllNodes;
		}
		return mDataBase[node];
	}

private:
	QMap<BoFaceNode*, QPtrList<BoFaceNode> > mDataBase;
	QPtrList<BoFaceNode> mAllNodes;
};


class BoFaceConnector
{
public:
	BoFaceConnector()
	{
	}
	static void disconnectNodes(const QPtrList<BoFaceNode>& allNodes, BoFaceNode*& first)
	{
		QPtrListIterator<BoFaceNode> it(allNodes);
		for (; 	it.current(); ++it) {
			it.current()->setNext(0);
			it.current()->setPrevious(0);
		}
		first = 0;
	}

	/**
	 * @return The OpenGL-type of what has been created. Always GL_TRIANGLES
	 * here.
	 **/
	static int addNodes(const QPtrList<BoFaceNode>& allNodes, BoFaceNode*& first)
	{
		if (first) {
			disconnectNodes(allNodes, first);
		}
		if (allNodes.isEmpty()) {
			// AB: actually we don't have triangles here, but as
			// there is nothing at all here we could also render
			// triangles.
			return GL_TRIANGLES;
		}
		BoFaceNode* previous = 0;
		QPtrListIterator<BoFaceNode> it(allNodes);
		first = it.current();
		previous = first;
		++it;
		for (; it.current(); ++it) {
			// previous->setNext(it.current()); // TODO: do we need this?
			it.current()->setPrevious(previous);
			previous = it.current();
		}
		return GL_TRIANGLES;
	}

	/**
	 * @return The OpenGL-type of what has been created. That is
	 * GL_TRIANGLE_STRIP on success and @ref addNodes on failure.
	 *
	 * Note: this is <em>not</em> working at the moment! Do NOT use it!
	 **/
	static int connectNodes(const QPtrList<BoFaceNode>& allNodes, BoFaceNode*& first);

protected:
	static bool connectNodes(const BoAdjacentDataBase* database, const QPtrList<BoFaceNode>& nodes, QPtrList<BoFaceNode>* found, BoFaceNode* node);
};

int BoFaceConnector::connectNodes(const QPtrList<BoFaceNode>& _allNodes, BoFaceNode*& first)
{
 if (first) {
	disconnectNodes(_allNodes, first);
 }
 if (_allNodes.isEmpty()) {
	boError() << k_funcinfo << "no nodes for mesh" << endl;
	return GL_TRIANGLES;
 }
 boDebug(100) << k_funcinfo << "trying to connect nodes" << endl;

 QPtrList<BoFaceNode> allNodes = _allNodes;

 // this constructs lists of all adjacent faces for all faces.
 BoAdjacentDataBase database(allNodes);

 // now we need to connect them if possible.
// BoFaceNode* node = allNodes.first();
// allNodes.removeFirst();
 QPtrList<BoFaceNode> connected;
 bool ok = connectNodes(&database, allNodes, &connected, 0);

 if (!ok) {
	boDebug(100) << k_funcinfo << "no connected nodes" << endl;
	disconnectNodes(allNodes, first);
	return addNodes(allNodes, first);
 }
 if (connected.count() == 0) {
	boError(100) << k_funcinfo << "no connected nodes" << endl;
	disconnectNodes(allNodes, first);
	return addNodes(allNodes, first);
 }
 /*
 if (ok != (allNodes.count() == connected.count())) {
	boError() << k_funcinfo << "wrong return value!" << endl;
	disconnectNodes();
	addNodes();
	return;
 }
 */

 first = connected.first();
 while (first->previous()) {
	first = first->previous();
 }

 if (!first->next()) {
	boError(100) << k_funcinfo << "oops - first node has no next node!" << endl;
	disconnectNodes(allNodes, first);
	return addNodes(allNodes, first);
 }
 if (!first->next()->next()) {
	boError(100) << k_funcinfo << "oops - first node has no next node!" << endl;
	disconnectNodes(allNodes, first);
	return addNodes(allNodes, first);
 }

 return GL_TRIANGLE_STRIP;
}

bool BoFaceConnector::connectNodes(const BoAdjacentDataBase* database, const QPtrList<BoFaceNode>& nodes, QPtrList<BoFaceNode>* found, BoFaceNode* node)
{
 static int call = 0;
 static int max = 0;
 call++;
 if (call > max) {
	max = call;
 }
 boDebug(100) << k_funcinfo << "- call " << call << " max=" << max << endl;
 if (call == 11) {
	return true;
 }
 if (nodes.isEmpty()) {
	return true;
 }
 if (!found->isEmpty()) {
	boError() << k_funcinfo << "found must be empty" << endl;
	return false;
 }
 if (node && nodes.containsRef(node)) {
	boError() << k_funcinfo << "list shouldn't contain that node" << endl;
	return false;
 }
 QPtrList<BoFaceNode> adjacent = database->adjacent(node);
 QPtrList<BoFaceNode> remainingNodes = nodes;
 QPtrListIterator<BoFaceNode> it(adjacent);
 int x = 0;
 for (; it.current(); ++it) {
	if (!remainingNodes.containsRef(it.current())) {
		continue;
	}
	BoFaceNode* next = it.current();

	if (node) {
		if (node->relevantPoint() < 0) {
			boError() << k_funcinfo << "negative relevant (node) point" << endl;
			continue;
		}
		int nodePoint = next->findPointIndex(node->pointIndex()[node->relevantPoint()]);
		if (nodePoint < 0) {
			// the point that has just been rendered isn't in this
			// face. not the next face.
			continue;
		}
		BoFaceNode* previous = node->previous();
		if (previous) {
			if (previous->relevantPoint() < 0) {
				boError() << k_funcinfo << "negative relevant (previous) point" << endl;
				continue;
			}
			int previousPoint = next->findPointIndex(previous->pointIndex()[previous->relevantPoint()]);
			if (previousPoint < 0) {
				continue;
			}
			if (nodePoint == previousPoint) {
				boError() << k_funcinfo << "node and previous point are the same point!" << endl;
				continue;
			}
			int relevantPoint = -1;
			for (int i = 0; i < 3; i++) {
				if (i == nodePoint) {
					continue;
				}
				if (i == previousPoint) {
					continue;
				}
				if (relevantPoint != -1) {
					boError() << k_funcinfo << "two relevant points - bad!" << endl;
				}
				relevantPoint = i;
			}
			if (relevantPoint < 0) {
				boDebug(100) << k_funcinfo << "no relevant point found" << endl;
				continue;
			}
			next->setRelevantPoint(relevantPoint);
		} else {
			const int* pointIndex = next->pointIndex();
			int relevantPoint = -1;
			for (int i = 0; i < 3; i++) {
				if (i == nodePoint) {
					continue;
				}
				if (node->findPointIndex(pointIndex[i]) >= 0) {
					// the next relevant point must not be
					// in the current node
					continue;
				}
				if (relevantPoint != -1) {
					boError() << "oops" << endl;
				}
				relevantPoint = i;
			}
			if (relevantPoint < 0) {
				boError() << k_funcinfo << "no relevant point found" << endl;
				continue;
			}
			next->setRelevantPoint(relevantPoint);
		}
		node->setNext(next);
	} else {
		// the relevant point for the first node are *all* points.
		// we use only one here - this is the last rendered point (i.e.
		// the third point) before the next face.
		bool ok = false;
		for (int i = 0; i < 3 && !ok; i++) {
			next->setRelevantPoint(i);
			remainingNodes.removeRef(next);
			QPtrList<BoFaceNode> newFound;
			ok = connectNodes(database, remainingNodes, &newFound, next);
			remainingNodes.append(next);
			if (ok) {
				found->append(next);
				QPtrListIterator<BoFaceNode> it2(newFound);
				for (; it2.current(); ++it2) {
					found->append(it2.current());
				}
				int firstPoint = -1;
				int secondPoint = -1;
				int thirdPoint = i;
				// note: next == first at this point
				for (int j = 0; j < 3; j++) {
					if (j == thirdPoint) {
						continue;
					}
					if (next->next()->findPointIndex(next->pointIndex()[j]) >= 0) {
						secondPoint = j;
						continue;
					}
					firstPoint = j;
				}
				if (secondPoint < 0 || firstPoint < 0) {
					boError() << k_funcinfo << "invalid points: " << firstPoint << " " << secondPoint << endl;
				} else {
					next->encodeRelevantPoint(firstPoint, secondPoint, thirdPoint);
				}
				return true;
			}
		}
		next->setNext(0);
		next->setPrevious(0);
	}
	remainingNodes.removeRef(next);
	QPtrList<BoFaceNode> newFound;
	bool ok = connectNodes(database, remainingNodes, &newFound, next);
	if (!ok) {
		// reset next and remove next from node
		next->setNext(0);
		next->setPrevious(0);
		next->setRelevantPoint(-1);
		if (node) {
			node->setNext(0);
		}

		// next wasn't the next node - so it is available again
		remainingNodes.append(next);
	} else {
		// that is good - n actually was the next node. now we happend
		// all nodes to found and return.
		found->append(next);
		QPtrListIterator<BoFaceNode> it2(newFound);
		for (; it2.current(); ++it2) {
			found->append(it2.current());
		}
		boWarning() << "next node found (call " << call << ")" << endl;
		call--;
		return true;
	}
	x++;
 }
// boDebug(100) << "no next in adjacent count=" << adjacent.count()  << endl;


 boDebug(100) << k_funcinfo << "none found (call " << call << ")" << endl;
 call--;
 return false;
}



BoFace::BoFace()
{
 mPointIndex[0] = -1;
 mPointIndex[1] = -1;
 mPointIndex[2] = -1;
 mSmoothGroup = 0;
}

BoFace& BoFace::operator=(const BoFace& face)
{
 setPointIndex(face.pointIndex());
 setSmoothGroup(face.smoothGroup());
#if BOMESH_USE_1_NORMAL_PER_FACE
 mNormals[0] = face.mNormals[0];
#else
 mNormals[0] = face.mNormals[0];
 mNormals[1] = face.mNormals[1];
 mNormals[2] = face.mNormals[2];
#endif
 return *this;
}



#define BO_NODE_BYTE_SIZE ((1 + 1 + 1 + 3) * 4) // 2 pointer, one int, one 3-int array, each 4 bytes
BoFaceNode::BoFaceNode(const BoFace* face)
{
 init();
 mFace = face;
}

BoFaceNode::~BoFaceNode()
{
 boMem->subBytes(BO_NODE_BYTE_SIZE);
}

void BoFaceNode::setPrevious(BoFaceNode* previous)
{
 mPrevious = previous;
 if (!mPrevious) {
	return;
 }
 if (mPrevious->next()) {
	boError() << k_funcinfo << "previous node already has a next node!!" << endl;
 }
 mPrevious->mNext = this;
}

void BoFaceNode::setNext(BoFaceNode* next)
{
 mNext = next;
 if (!mNext) {
	return;
 }
 if (mNext->previous()) {
	boError() << k_funcinfo << "next node already has a previous node!!" << endl;
 }
 mNext->mPrevious = this;
}

void BoFaceNode::delNode()
{
 if (previous() && next()) {
	previous()->mNext = next();
	next()->mPrevious = previous();
 } else {
	if (previous()) {
		previous()->mNext = 0;
	}
	if (next()) {
		next()->mPrevious = 0;
	}
 }
}

void BoFaceNode::init()
{
 mNext = 0;
 mPrevious = 0;
 mRelevantPoint = -1;
 mFace = 0;
 boMem->addBytes(BO_NODE_BYTE_SIZE);
}

bool BoFaceNode::isAdjacent(BoFaceNode* f1, BoFaceNode* f2)
{
 const int* v1 = f1->pointIndex();
 const int* v2 = f2->pointIndex();
 int equal = 0;
 for (int i = 0; i < 3; i++) {
	if (v1[i] == v2[0] || v1[i] == v2[1] || v1[i] == v2[2]) {
		equal++;
	}
 }
 return equal >= 2;
}

QString BoFaceNode::debugString() const
{
 QString s;
 s = QString("%1  %2  %3").arg(pointIndex()[0]).arg(pointIndex()[1]).arg(pointIndex()[2]);
 return s;
}

int BoFaceNode::findPointIndex(int index) const
{
 if (pointIndex()[0] == index) {
	return 0;
 }
 if (pointIndex()[1] == index) {
	return 1;
 }
 if (pointIndex()[2] == index) {
	return 2;
 }
 return -1;
}



/**
 * This class is a collection of points for @ref BoMesh. You start by calling
 * @ref allocatePoints and then modify the points using @ref setVertex and @ref
 * setTexel (and maybe friends - depends on current implementation).
 *
 * This class supports moving all points to a different array - see @ref
 * movePoints. This can be used to maintain a single huge array, instead of
 * multiple small arrays. This can be more handy and more efficient.
 *
 * But note that you cannot add additional points (other than those that were
 * initially allocated), so you have to allocate all points that you may need
 * right from the beginning.
 *
 * @short points collection for @ref BoMesh
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMeshPoints
{
public:
	BoMeshPoints()
	{
		mPointCount = 0;
		mPoints = 0;
		mAllocatedPoints = 0;
		mPointsMovedBy = 0;
	}
	~BoMeshPoints()
	{
		boMem->freeFloatArray(mAllocatedPoints);
		mPoints = 0;
	}
	inline static int pointSize()
	{
		// 3 vertex components, 2 texel components
		return (3 + 2);
	}
	inline static int vertexPos()
	{
		return 0;
	}
	inline static int texelPos()
	{
		return 3;
	}

	void setVertex(unsigned int index, const BoVector3& vertex)
	{
		BO_CHECK_NULL_RET(mPoints);
		if (index >= points()) {
			boError() << k_funcinfo << "invalid index " << index
					<< " max=" << points() - 1 << endl;
			return;
		}
		mPoints[index * pointSize() + vertexPos() + 0] = vertex[0];
		mPoints[index * pointSize() + vertexPos() + 1] = vertex[1];
		mPoints[index * pointSize() + vertexPos() + 2] = vertex[2];
	}
	void setTexel(unsigned int index, const BoVector3& texel)
	{
		BO_CHECK_NULL_RET(mPoints);
		if (index >= points()) {
			boError() << k_funcinfo << "invalid index " << index
					<< " max=" << points() - 1 << endl;
			return;
		}
		mPoints[index * pointSize() + texelPos() + 0] = texel[0];
		mPoints[index * pointSize() + texelPos() + 1] = texel[1];
	}
	inline BoVector3 vertex(unsigned int p) const
	{
		if (!mPoints) {
			boError() << k_funcinfo << "no points allocated" << endl;
			return BoVector3();
		}
		if (p >= points()) {
			boError() << k_funcinfo << "invalid point " << p
					<< " max=" << points() - 1 << endl;
			return BoVector3();
		}
		return BoVector3(&mPoints[p * pointSize() + vertexPos()]);
	}

	inline unsigned int points() const
	{
		return mPointCount;
	}

	/**
	 * You should be careful when you use this - you may be doing something
	 * wrong.
	 * This class stores the points with indices 0 .. @ref points, whereas
	 * @ref BoFace::pointIndex references indices pointsMovedBy ..
	 * pointsMovedBy + @ref points. This is why you may think you need this.
	 *
	 * But usually you are meant to use glArrayElement() with parameters
	 * directly from @ref BoFace::pointIndex or the @ref BoMesh::vertex
	 * methods instead. BoMesh::vertex may use pointsMovedBy under certain
	 * circumstances - you should not if you can avoid it.
	 *
	 * @return How much the points got moved by @ref movePoints. This is
	 * just the index parameter of @ref movePoints.
	 **/
	inline unsigned int pointsMovedBy() const
	{
		return mPointsMovedBy;
	}

	void allocatePoints(int points)
	{
		if (mAllocatedPoints) {
			boError() << k_funcinfo << "points already allocated!" << endl;
			if (mPoints == mAllocatedPoints) {
				mPoints = 0;
			}
			boMem->freeFloatArray(mAllocatedPoints);
			mAllocatedPoints = 0;
		}
		if (mPoints) {
			boError() << k_funcinfo << "non-NULL points array!" << endl;
			mPoints = 0;
		}
		mPointCount = points;

		const int pointSize = BoMesh::pointSize();

		// note that a lot of space is wasted - some objects are not textured
		// but we can render more efficient this way.
		mAllocatedPoints = boMem->allocateFloatArray(mPointCount * pointSize);
		mPoints = mAllocatedPoints;

		for (unsigned int i = 0; i < mPointCount; i++) {
			for (int j = 0; j < pointSize; j++) {
				mAllocatedPoints[i * pointSize + j] = 0.0f;
			}
		}
	}

	unsigned int movePoints(float* array, int index)
	{
		unsigned int pointsMoved = 0;
		if (!mAllocatedPoints) {
			boError() << k_funcinfo << "no points allocated" << endl;
			return pointsMoved;
		}
		const int pointSize = BoMesh::pointSize();
		for (unsigned int i = 0; i < points(); i++) {
			for (int j = 0; j < pointSize; j++) {
				array[(index + i) * pointSize + j] = mPoints[i * pointSize + j];
			}
		}
		pointsMoved = points();

		#warning FIXME: memory fragmentation
		// we allocate a lot of floats and free them later (moving the values to
		// a single array). this will probably lead to quite some memory
		// fragmentation. we should change this design.
		// but remember that the array that is actually used (d->mPoints) is allocated
		// only once per model - so at this point there is no memory fragmentation (and
		// this is the important place)
		boMem->freeFloatArray(mAllocatedPoints);
		mAllocatedPoints = 0;
		mPoints = array + index * pointSize;
		mPointsMovedBy = index;

		return pointsMoved;
	}

private:
	float* mPoints;
	float* mAllocatedPoints;
	unsigned int mPointsMovedBy;
	unsigned int mPointCount;
};

class BoMeshLODPrivate
{
public:
	BoMeshLODPrivate()
	{
	}
	QValueVector<BoFace> mAllFaces;
	QPtrList<BoFaceNode> mAllNodes;
};

BoMeshLOD::BoMeshLOD()
{
 d = new BoMeshLODPrivate;
 mNodes = 0;
 mPointsCache = 0;
 mPointsCacheCount = 0;
 d->mAllNodes.setAutoDelete(true);
 mDisplayList = 0;

 mType = GL_TRIANGLES;
}

BoMeshLOD::~BoMeshLOD()
{
 d->mAllNodes.clear();
 delete[] mPointsCache;
 if (mDisplayList) {
	glDeleteLists(mDisplayList, 1);
 }
 delete d;
}

void BoMeshLOD::createFaces(unsigned int faces)
{
 if (faces < 1) {
	boWarning(100) << k_funcinfo << "no faces in mesh" << endl;
	return;
 }
 if (d->mAllFaces.count() != 0) {
	boWarning(100) << "faces already created. resizing" << endl;
 }
 d->mAllFaces.resize(faces);

 // there are exactly as many nodes as faces in a mesh.
 // Every node represents exactly one face. Nodes represent the "connections" of
 // faces.
 if (d->mAllNodes.count() > 0) {
	boDebug(100) << "nodes already created. deleting." << endl;
	d->mAllNodes.clear();
 }
 for (unsigned int face = 0; face < faces; face++) {
	BoFaceNode* node = new BoFaceNode(&d->mAllFaces[face]);
	d->mAllNodes.append(node);
 }
}

unsigned int BoMeshLOD::facesCount() const
{
 return d->mAllFaces.count();
}

void BoMeshLOD::setFace(int index, const BoFace& face)
{
 d->mAllFaces[index] = face;
}

const BoFace* BoMeshLOD::face(unsigned int f) const
{
 if (f >= facesCount()) {
	return 0;
 }
 return &d->mAllFaces[f];
}

void BoMeshLOD::setNormal(unsigned int face, int vertex, const BoVector3& normal)
{
 if (face >= facesCount()) {
	boError() << k_funcinfo << "invalid face " << face
			<< " max=" << facesCount() << endl;
	return;
 }
 if (vertex >= 3) {
	boError() << k_funcinfo << "vertex must be 0..2  or -1 not " << vertex << endl;
	vertex = vertex % 3;
 }

 if (vertex < 0) {
	d->mAllFaces[face].setAllNormals(normal);
 } else {
	d->mAllFaces[face].setNormal(vertex, normal);
 }
}

void BoMeshLOD::movePointIndices(int moveBy)
{
 for (unsigned int i = 0; i < facesCount(); i++) {
	BoFace* face = &d->mAllFaces[i];
	const int* orig = face->pointIndex();
	int p[3];
	p[0] = moveBy + orig[0];
	p[1] = moveBy + orig[1];
	p[2] = moveBy + orig[2];
	face->setPointIndex(p);
 }
 if (mPointsCache) {
	// we need to regenerate the cache
	createPointCache();
 }
}

void BoMeshLOD::ensurePointCacheValid(unsigned int min, unsigned int max)
{
 if (!mPointsCache) {
	return;
 }
 for (unsigned int i = 0; i < mPointsCacheCount; i++) {
	if (mPointsCache[i] < min) {
		boWarning(100) << k_funcinfo << "point cache at "
				<< i << " is less than " << min
				<< ": " << mPointsCache[i]
				<< ". Fixing." << endl;
		mPointsCache[i] = min;
	} else if (mPointsCache[i] > max) {
		boWarning(100) << k_funcinfo << "point cache at "
				<< i << " is greater than " << max
				<< ": " << mPointsCache[i]
				<< ". Fixing." << endl;
		mPointsCache[i] = max;
	}
 }
}

void BoMeshLOD::disconnectNodes()
{
 mType = BoFaceConnector::connectNodes(d->mAllNodes, mNodes);
}
void BoMeshLOD::connectNodes()
{
 mType = BoFaceConnector::connectNodes(d->mAllNodes, mNodes);
}
void BoMeshLOD::addNodes()
{
 mType = BoFaceConnector::addNodes(d->mAllNodes, mNodes);
}

void BoMeshLOD::createPointCache()
{
 // the point cache is an array of all vertex indices of the faces. so if face1
 // references vertices 2,3,1 and face 2 references vertices 5,4,7 then the
 // point cache will be 2,3,1,5,4,7 (assuming we don't use GL_TRIANGLES)
 boMem->freeUIntArray(mPointsCache);
 mPointsCacheCount = 0;
 if (d->mAllFaces.count() < 1) {
	return;
 }
 BoFaceNode* node = nodes();
 if (!node) {
	boError(100) << k_funcinfo << "NULL node" << endl;
	return;
 }
 int nodesCount = 0;
 // count the number of nodes in our list.
 // note that we mustn't assume that all nodes are in that list!
 for (node = nodes(); node; node = node->next()) {
	nodesCount++;
 }
 node = nodes();
 if (type() == GL_TRIANGLE_STRIP) {
	boDebug(100) << "creating _STRIP" << endl;
	if (!node->next()) {
		boError() << k_funcinfo << "less than 2 nodes in mesh! this is not supported" << endl;
		return;
	}
	int firstPoint;
	int secondPoint;
	int thirdPoint;
	node->decodeRelevantPoint(&firstPoint, &secondPoint, &thirdPoint);

	// 3 basic points + one point per remaining face
	mPointsCacheCount = 3 + (nodesCount - 3) * 1;
	mPointsCache = boMem->allocateUIntArray(mPointsCacheCount);

	mPointsCache[0] = (unsigned int)node->pointIndex()[firstPoint];
	mPointsCache[1] = (unsigned int)node->pointIndex()[secondPoint];
	mPointsCache[2] = (unsigned int)node->pointIndex()[thirdPoint];

	int element = 3;
	// we skip the entire first face. all points have been rendered above.
	node = node->next();
	for (; node; node = node->next()) {
		int point = node->relevantPoint();
		if (point < 0 || point > 2) {
			boError( )<< k_funcinfo "oops - invalid point " << point << endl;
			continue;
		}
		mPointsCache[element] = (unsigned int)node->pointIndex()[point];
		element++;
	}
 } else if (type() == GL_TRIANGLES) {
	// 3 points per face
	mPointsCacheCount = nodesCount * 3;
	mPointsCache = boMem->allocateUIntArray(mPointsCacheCount);
	int element = 0;
	for (node = nodes(); node; node = node->next()) {
		for (int i = 0; i < 3; i++) {
			mPointsCache[element] = (unsigned int)node->pointIndex()[i];
			element++;
		}
	}
 } else {
	boError(100) << k_funcinfo << "Invalid type: " << type() << endl;
 }
}



class BoMeshPrivate
{
public:
	BoMeshPrivate()
	{
		mMaterial = 0;

		mBoundingObject = 0;

		mLODs = 0;
	}
	bool mIsTeamColor;

	BoMaterial* mMaterial;

	BoMeshPoints mMeshPoints;


	BoMeshLOD** mLODs;
	unsigned int mLODCount;

	float mMinX;
	float mMaxX;
	float mMinY;
	float mMaxY;
	float mMinZ;
	float mMaxZ;

	BoundingObject* mBoundingObject;

	QString mName;
};

BoMesh::BoMesh(unsigned int faces, const QString& name)
{
 init();

 d->mName = name;

 BoMeshLOD* lod = levelOfDetail(0);
 BO_CHECK_NULL_RET(lod);
 lod->createFaces(faces);
}

BoMesh::~BoMesh()
{
 for (unsigned int i = 0; i < d->mLODCount; i++) {
	delete d->mLODs[i];
 }
 delete[] d->mLODs;
 delete d->mBoundingObject;
 delete d;
}

void BoMesh::init()
{
 d = new BoMeshPrivate;
 d->mIsTeamColor = false;
 d->mLODCount = 0;

 d->mMinX = 0.0f;
 d->mMaxX = 0.0f;
 d->mMinY = 0.0f;
 d->mMaxY = 0.0f;
 d->mMinZ = 0.0f;
 d->mMaxZ = 0.0f;

 // a mesh *never* has less than 1 LOD. LOD 0 is always the full-detailed
 // version.
 d->mLODs = new BoMeshLOD*[1];
 d->mLODs[0] = new BoMeshLOD;
 d->mLODCount = 1;
}

int BoMesh::pointSize()
{
 return BoMeshPoints::pointSize();
}

int BoMesh::vertexPos()
{
 return BoMeshPoints::vertexPos();
}

int BoMesh::texelPos()
{
 return BoMeshPoints::texelPos();
}

void BoMesh::setMaterial(BoMaterial* mat)
{
 d->mMaterial = mat;
}

BoMaterial* BoMesh::material() const
{
 return d->mMaterial;
}

// this is used for mesh-loading and therefore operates on LOD 0 only.
void BoMesh::setFace(int index, const BoFace& face)
{
 BoMeshLOD* lod = levelOfDetail(0);
 if (!lod) {
	boError(100) << k_funcinfo << "NULL default LOD" << endl;
	return;
 }
 lod->setFace(index, face);
}

void BoMesh::disconnectNodes()
{
 for (unsigned int i = 0; i < d->mLODCount; i++) {
	if (!d->mLODs[i]) {
		boError(100) << k_funcinfo << "NULL LOD at " << i << endl;
		continue;
	}
	d->mLODs[i]->disconnectNodes();
 }
}

void BoMesh::addNodes()
{
 for (unsigned int i = 0; i < d->mLODCount; i++) {
	if (!d->mLODs[i]) {
		boError(100) << k_funcinfo << "NULL LOD at " << i << endl;
		continue;
	}
	d->mLODs[i]->addNodes();
 }
}

void BoMesh::connectNodes()
{
 for (unsigned int i = 0; i < d->mLODCount; i++) {
	if (!d->mLODs[i]) {
		boError(100) << k_funcinfo << "NULL LOD at " << i << endl;
		continue;
	}
	d->mLODs[i]->connectNodes();
 }
}

GLuint BoMesh::textureObject() const
{
 if (material()) {
	return material()->textureObject();
 }
 return 0;
}

void BoMesh::allocatePoints(unsigned int points)
{
 d->mMeshPoints.allocatePoints(points);
}

void BoMesh::setVertex(unsigned int index, const BoVector3& vertex)
{
 d->mMeshPoints.setVertex(index, vertex);
}

BoVector3 BoMesh::vertex(unsigned int p) const
{
 return d->mMeshPoints.vertex(p);
}

BoVector3 BoMesh::vertex(unsigned int face, unsigned int i, unsigned int _lod) const
{
 BoMeshLOD* lod = levelOfDetail(_lod);
 if (!lod) {
	boError() << k_funcinfo << "NULL LOD " << _lod << endl;
	return BoVector3();
 }
 if (face >= lod->facesCount()) {
	boError() << k_funcinfo << "invalid face " << face << " have only " << lod->facesCount() << endl;
	return BoVector3();
 }
 if (i >= 3) {
	boError() << k_funcinfo << " vertex index must be 0..2, not " << i << endl;
	i = i % 3;
 }
 const BoFace* f = lod->face(face);
 if (!f) {
	boError(100) << k_funcinfo << "NULL face " << face << endl;
	return BoVector3();
 }
 return vertex(f->pointIndex()[i] - d->mMeshPoints.pointsMovedBy());
}

void BoMesh::setTexel(unsigned int index, const BoVector3& texel)
{
 d->mMeshPoints.setTexel(index, texel);
}

void BoMesh::calculateNormals()
{
 for (unsigned int i = 0; i < d->mLODCount; i++) {
	calculateNormals(i);
 }
}

void BoMesh::calculateNormals(unsigned int _lod)
{
 // we don't use lib3ds' normals so that we can easily support file formats that
 // dont store the normals. furthermore we depend on less external data :)
 //
 // note that we calculate 1 normal per face only at the moment!

 BoMeshLOD* lod = levelOfDetail(_lod);
 BO_CHECK_NULL_RET(lod);

 // Normals are calculated in two passes:
 // First we calculate normals for all faces
 // Then, we calculate normals for vertices
 //  If face is not in any smoothing group, it's vertices will have same normal
 //   as the face (flat smoothing)
 //  If face is in smoothing group, we take the average of the normals of all
 //   faces that have this vertex and that are in the same smoothing group. This
 //   way we don't have to recalculate face normals all the time.

 // Pass 1: calculate face normals
 BoVector3* facenormals = new BoVector3[lod->facesCount()];
 BoVector3 a, b, c;
 for (unsigned int face = 0; face < lod->facesCount(); face++) {
	a = vertex(face, 0, _lod);
	b = vertex(face, 1, _lod);
	c = vertex(face, 2, _lod);

	facenormals[face] = BoVector3::crossProduct(c - b, a - b);
	facenormals[face].normalize();
 }

 // Pass 2: calculate vertex normals. If face is not in any smoothing group,
 //  it's normal is it's face's normal
 for (unsigned int face = 0; face < lod->facesCount(); face++) {
	if (lod->face(face)->smoothGroup()) {
#if !BOMESH_USE_1_NORMAL_PER_FACE
		// Face is in some smoothing group. Use smooth shading
		for (int p = 0; p < 3; p++) {
			BoVector3 normal;
			for (unsigned int face2 = 0; face2 < lod->facesCount(); face2++) {
				if(lod->face(face)->smoothGroup() & lod->face(face2)->smoothGroup()) {
					// Two faces have same smooth groups
					if (lod->face(face2)->hasPoint(lod->face(face)->pointIndex()[p])) {
						// And it has our point
						normal += facenormals[face2];
					}
				}
			}
			normal.normalize();
			lod->setNormal(face, p, normal);
		}
#else
		lod->setNormal(face, -1, facenormals[face]);
#endif
	} else {
		// Face is not in any smoothing group. Use flat shading
		lod->setNormal(face, -1, facenormals[face]);
	}
 }
}

void BoMesh::renderMesh(const QColor* teamColor, unsigned int _lod)
{
 if (_lod >= lodCount()) {
	_lod = lodCount() - 1;
 }
 BoMeshLOD* lod = levelOfDetail(_lod);
 if (!lod) {
	BO_NULL_ERROR(lod);
	return;
 }
 unsigned int* pointsCache = lod->pointsCache();
 unsigned int pointsCacheCount = lod->pointsCacheCount();
 int type = lod->type();
 BoFaceNode* nodes = lod->nodes();

 if (!nodes) {
	// nothing to do.
	return;
 }


 bool resetColor = false; // needs to be true after we changed the current color

 // AB: we have *lots* of faces! in numbers the maximum i found
 // so far (only a short look) was about 25 toplevel nodes and
 // rarely child nodes. sometimes 2 child nodes or so - maybe 10
 // per model (if at all).
 // but we have up to (short look only) 116 faces *per node*
 // usually it's about 10-20 faces (minimum) per node!
 //
 // so optimization should happen here - if possible at all...

 BoMaterial::activate(material());
 if (!material() && isTeamColor()) {
	if (teamColor) {
		glPushAttrib(GL_CURRENT_BIT);
		glColor3ub(teamColor->red(), teamColor->green(), teamColor->blue());
		resetColor = true;
	}
 }

#define USE_OCCLUSION_CULLING 0
#if USE_OCCLUSION_CULLING
 if (checkVisible())
#endif
 {
//	boDebug() << k_funcinfo << "not culled" << endl;
	if (!pointsCache || pointsCacheCount == 0) {
		boError() << k_funcinfo << "no point cache!" << endl;
	} else {
		glBegin(type);

		BoFaceNode* node = nodes;
		while (node) {
			const BoFace* face = node->face();
			const int* points = face->pointIndex();
#if BOMESH_USE_1_NORMAL_PER_FACE
			// here we assume that the same normal is used for all
			// vertices of this face. this is the default.
			glNormal3fv(face->normal(0).data());
			glArrayElement(points[0]);
			glArrayElement(points[1]);
			glArrayElement(points[2]);
#else
			glNormal3fv(face->normal(0).data());
			glArrayElement(points[0]);
			glNormal3fv(face->normal(1).data());
			glArrayElement(points[1]);
			glNormal3fv(face->normal(2).data());
			glArrayElement(points[2]);
#endif

			node = node->next();
		}

		glEnd();

		// reset the normal...
		// (better solution: don't enable light when rendering
		// selection rect!)
		const float n[] = { 0.0f, 0.0f, 1.0f };
		glNormal3fv(n);
	}
 }


 if (resetColor) {
	// we need to reset the color (mainly for the placement preview)
	glPopAttrib();
	resetColor = false;
 }

 // we need this currently, because of the selection rects. we should avoid
 // this.
 // maybe place BoMaterial::deactivate() into SelectBox ?
 if (material()) {
	material()->deactivate();
 }
}

void BoMesh::renderBoundingObject()
{
 if (!d->mBoundingObject) {
	computeBoundingObject();
	if (!d->mBoundingObject) {
		return;
	}
 }
 d->mBoundingObject->render();
}

void BoMesh::renderVertexPoints(unsigned int _lod)
{
 if (_lod >= lodCount()) {
	_lod = lodCount() - 1;
 }
 BoMeshLOD* lod = levelOfDetail(_lod);
 if (!lod) {
	BO_NULL_ERROR(lod);
	return;
 }

 BoFaceNode* node = lod->nodes();
 if (!node) {
	return;
 }
 glBegin(GL_POINTS);
	while (node) {
		const BoFace* face = node->face();
		const int* points = face->pointIndex();
		BoVector3 v1 = vertex(points[0] - d->mMeshPoints.pointsMovedBy());
		glVertex3f(v1[0], v1[1], v1[2]);
		BoVector3 v2 = vertex(points[1] - d->mMeshPoints.pointsMovedBy());
		glVertex3f(v2[0], v2[1], v2[2]);
		BoVector3 v3 = vertex(points[2] - d->mMeshPoints.pointsMovedBy());
		glVertex3f(v3[0], v3[1], v3[2]);
		node = node->next();
	}
 glEnd();
}

// there MUST be a valid context set already!!
void BoMesh::loadDisplayList(const QColor* teamColor, bool reload)
{
 for (unsigned int i = 0; i < d->mLODCount; i++) {
	loadDisplayList(d->mLODs[i], teamColor, reload);
 }
}

void BoMesh::loadDisplayList(BoMeshLOD* lod, const QColor* teamColor, bool reload)
{
 BO_CHECK_NULL_RET(lod);

 if (!lod->displayList()) {
	lod->setDisplayList(glGenLists(1));
	if (lod->displayList() == 0) {
		boError() << k_funcinfo << "NULL display list generated" << endl;
		return;
	}
 } else {
	if (reload) {
		// it is important that the list is deleted, but the
		// newly generated list must have the *same* number as
		// it had before!
		glDeleteLists(lod->displayList(), 1);
	} else {
		boWarning() << k_funcinfo << "mesh was already loaded!" << endl;
		return;
	}
 }
 boDebug(100) << k_funcinfo << endl;

 glNewList(lod->displayList(), GL_COMPILE);
 renderMesh(teamColor);
 glEndList();
}

unsigned int BoMesh::points() const
{
 return d->mMeshPoints.points();
}

bool BoMesh::isTeamColor() const
{
 return d->mIsTeamColor;
}

void BoMesh::setIsTeamColor(bool c)
{
 d->mIsTeamColor = c;
}

GLuint BoMesh::displayList(unsigned int lod) const
{
 BoMeshLOD* l = levelOfDetail(lod);
 if (!l) {
	boError(100) << k_funcinfo << "NULL lod " << lod << endl;
	return 0;
 }
 return l->displayList();
}

unsigned int BoMesh::movePoints(float* array, int index)
{
 // move all points to the new array
 unsigned int pointsMoved = d->mMeshPoints.movePoints(array, index);

 // now we fix the (point-)indices of all faces in all LODs.
 for (unsigned int i = 0; i < d->mLODCount; i++) {
	if (!d->mLODs[i]) {
		boError(100) << k_funcinfo << "NULL LOD " << i << endl;
		continue;
	}
	d->mLODs[i]->movePointIndices(index);
 }

 return pointsMoved;
}

void BoMesh::createPointCache()
{
 for (unsigned int i = 0; i < d->mLODCount; i++) {
	d->mLODs[i]->createPointCache();
	d->mLODs[i]->ensurePointCacheValid(d->mMeshPoints.pointsMovedBy(),
			d->mMeshPoints.pointsMovedBy() + points());
 }
}

void BoMesh::calculateMaxMin()
{
 // NOT time critical! (called on startup only)
 BoVector3 v = vertex(0);
 d->mMinZ = v.z();
 d->mMaxZ = v.z();
 d->mMinX = v.x();
 d->mMaxX = v.x();
 d->mMinY = v.y();
 d->mMaxY = v.y();
 for (unsigned int i = 0; i < points(); i++) {
	v = vertex(i);
	if (v.x() < d->mMinX) {
		d->mMinX = v.x();
	} else if (v.x() > d->mMaxX) {
		d->mMaxX = v.x();
	}
	if (v.y() < d->mMinY) {
		d->mMinY = v.y();
	} else if (v.y() > d->mMaxY) {
		d->mMaxY = v.y();
	}
	if (v.z() < d->mMinZ) {
		d->mMinZ = v.z();
	} else if (v.z() > d->mMaxZ) {
		d->mMaxZ = v.z();
	}
 }
}

float BoMesh::minX() const
{
 return d->mMinX;
}

float BoMesh::maxX() const
{
 return d->mMaxX;
}

float BoMesh::minY() const
{
 return d->mMinY;
}

float BoMesh::maxY() const
{
 return d->mMaxY;
}

float BoMesh::minZ() const
{
 return d->mMinZ;
}

float BoMesh::maxZ() const
{
 return d->mMaxZ;
}

bool BoMesh::checkVisible()
{
 if (!d->mBoundingObject) {
	return false;
 }
 GLboolean result;
#if 1
 glDisable(GL_TEXTURE_2D);
 glDisable(GL_DEPTH_TEST);
#else
 glDepthMask(GL_FALSE);
 glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
#endif
 glEnable(GL_OCCLUSION_TEST_HP);

 renderBoundingObject();

 glGetBooleanv(GL_OCCLUSION_TEST_RESULT_HP, &result);

 glDisable(GL_OCCLUSION_TEST_HP);
#if 1
 // AB: we should NOT do this! it might have been disabled before already!
 // (think about rendering a wireframe only!)
 glEnable(GL_TEXTURE_2D);
 glEnable(GL_DEPTH_TEST);
#else
 glDepthMask(GL_TRUE);
 glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
 return result;
}

void BoMesh::computeBoundingObject()
{
 delete d->mBoundingObject;
 d->mBoundingObject = 0;

 BoundingObjectBuilder builder;
 d->mBoundingObject = builder.generateBoundingObject(this);
}

void BoMesh::generateLOD()
{
 // how many LODs should be generated. maybe use a parameter for this?
 // these numbers can be used by renderMesh() (or indirectly by renderFrame())
 // to choose which LOD should be rendered at. 0 is always the default, i.e.
 // render all points and faces.
 unsigned int LODCount = 5; // must be at least 1, as we have at leas the full-detailed version

 unsigned int oldCount = d->mLODCount;
 if (LODCount < oldCount) {
	boError(100) << k_funcinfo << "must at least preserve existing LODs (" << oldCount << ")!" << endl;
	LODCount = oldCount;
 }
 if (LODCount == 0) {
	// default rendering is LOD 0 !
	LODCount = 1;
	boWarning(100) << k_funcinfo << "must create at least one LOD" << endl;
 }
 BoMeshLOD** lod = new BoMeshLOD*[LODCount];
 for (unsigned int i = 0; i < oldCount; i++) {
	lod[i] = d->mLODs[i];
 }
 if (oldCount == 0) {
	lod[0] = 0;
 }
 boDebug(100) << k_funcinfo << "lods=" << LODCount
		<< " exisiting: " << oldCount
		<< " generating: " << LODCount - oldCount
		<< " name: " << name()
		<< endl;

 for (unsigned int i = oldCount; i < LODCount; i++) {
	lod[i] = new BoMeshLOD();
 }

 delete[] d->mLODs;
 d->mLODs = lod;
 d->mLODCount = LODCount;

 BoLODBuilder builder(this, lod[0]);
 for (unsigned int i = oldCount; i < LODCount; i++) {
	QValueList<BoFace> faces = builder.generateLOD(i);
	if (faces.count() > 0) {
		lod[i]->createFaces(faces.count());
	}
	boDebug(100) << k_funcinfo << "generated LOD " << i << " which has " << faces.count() << " faces" << endl;
	for (unsigned int j = 0; j < faces.count(); j++) {
		lod[i]->setFace(j, faces[j]);
	}
	calculateNormals(i);
 }
}

BoMeshLOD* BoMesh::levelOfDetail(unsigned int lod) const
{
 if (!d->mLODs) {
	boError(100) << k_funcinfo << "NULL LOD pointer" << endl;
	return 0;
 }
 if (lod >= lodCount()) {
	boError(100) << k_funcinfo << "invalid lod: " << lod << " have only " << lodCount() << endl;
	return 0;
 }
 return d->mLODs[lod];
}

unsigned int BoMesh::lodCount() const
{
 return d->mLODCount;
}

unsigned int BoMesh::facesCount(unsigned int lod) const
{
 BoMeshLOD* l = levelOfDetail(lod);
 if (!l) {
	boError(100) << k_funcinfo << "NULL LOD " << lod << endl;
	return 0;
 }
 return l->facesCount();
}

const QString& BoMesh::name() const
{
 return d->mName;
}

