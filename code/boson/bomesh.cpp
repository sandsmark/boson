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
	BoAdjacentDataBase(const QPtrList<BoNode>& allNodes)
	{
		addNodes(allNodes);
	}

	/**
	 * Add all nodes to the databse. construct the lists of adjacent
	 * nodes/faces.
	 **/
	void addNodes(const QPtrList<BoNode>& allNodes)
	{
		mAllNodes = allNodes;
		QPtrListIterator<BoNode> it(allNodes);
		for (; it.current(); ++it) {
			QPtrListIterator<BoNode> it2(allNodes);
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
				if (BoNode::isAdjacent(it.current(), it2.current())) {
					mDataBase[it.current()].append(it2.current());
					mDataBase[it2.current()].append(it.current());
				}
			}
		}
	}
	void debug()
	{
		QPtrListIterator<BoNode> it(mAllNodes);
		for (; it.current(); ++it) {
			boDebug(100) << "adjacent to " << it.current()->debugString() << " (Face " << mAllNodes.find(it.current()) + 1 << "):" << endl;
			QPtrList<BoNode> a = adjacent(it.current());
			QPtrListIterator<BoNode> it2(a);
			for (; it2.current(); ++it2) {
				boDebug(100) << it2.current()->debugString() << " (Face " << mAllNodes.find(it2.current()) + 1 << ")" << endl;
			}

		}
	}

	/**
	 * @return a list of faces that are adjacent to @p node
	 **/
	QPtrList<BoNode> adjacent(BoNode* node) const
	{
		if (!node) {
			return mAllNodes;
		}
		return mDataBase[node];
	}

private:
	QMap<BoNode*, QPtrList<BoNode> > mDataBase;
	QPtrList<BoNode> mAllNodes;
};


BoFace::BoFace()
{
 mPointIndex[0] = -1;
 mPointIndex[1] = -1;
 mPointIndex[2] = -1;
}

BoFace& BoFace::operator=(const BoFace& face)
{
 setPointIndex(face.pointIndex());
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
BoNode::BoNode(const BoFace* face)
{
 init();
 mFace = face;
}

BoNode::~BoNode()
{
 boMem->subBytes(BO_NODE_BYTE_SIZE);
}

void BoNode::setPrevious(BoNode* previous)
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

void BoNode::setNext(BoNode* next)
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

void BoNode::delNode()
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

void BoNode::init()
{
 mNext = 0;
 mPrevious = 0;
 mRelevantPoint = -1;
 mFace = 0;
 boMem->addBytes(BO_NODE_BYTE_SIZE);
}

bool BoNode::isAdjacent(BoNode* f1, BoNode* f2)
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

QString BoNode::debugString() const
{
 QString s;
 s = QString("%1  %2  %3").arg(pointIndex()[0]).arg(pointIndex()[1]).arg(pointIndex()[2]);
 return s;
}

int BoNode::findPointIndex(int index) const
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


class BoMeshPrivate
{
public:
	BoMeshPrivate()
	{
		mNodes = 0;

		mMaterial = 0;

		mAllocatedPoints = 0;
		mPoints = 0;

		mPointsCache = 0;
		mPointsCacheCount = 0;

		mBoundingObject = 0;
	}
	int mType;
	BoNode* mNodes;
	bool mIsTeamColor;

	BoMaterial* mMaterial;

	GLuint mDisplayList;

	QValueVector<BoFace> mAllFaces;
	QPtrList<BoNode> mAllNodes;

	unsigned int mPointCount;
	float* mAllocatedPoints;
	float* mPoints;
	unsigned int mPointsMovedBy;

	// the list of points in the final order (after connectNodes() or
	// addNodes() was called). iterating through nodes() is equalivent (for
	// some modes the BoNode::relevantPoint() will have to be used though)
	unsigned int* mPointsCache;
	unsigned int mPointsCacheCount;

	float mMinX;
	float mMaxX;
	float mMinY;
	float mMaxY;
	float mMinZ;
	float mMaxZ;

	BoundingObject* mBoundingObject;
};

BoMesh::BoMesh(unsigned int faces)
{
 init();
 createFaces(faces);
}

BoMesh::~BoMesh()
{
 if (d->mDisplayList) {
	glDeleteLists(d->mDisplayList, 1);
 }
 d->mAllNodes.clear();
 boMem->freeFloatArray(d->mAllocatedPoints);
 d->mPoints = 0; // do NOT delete!
 delete d->mBoundingObject;
 delete d;
}

void BoMesh::init()
{
 d = new BoMeshPrivate;
 d->mAllNodes.setAutoDelete(true);
 d->mType = GL_TRIANGLES;
 d->mIsTeamColor = false;
 d->mDisplayList = 0;
 d->mPointCount = 0;
 d->mPointsMovedBy = 0;

 d->mMinX = 0.0f;
 d->mMaxX = 0.0f;
 d->mMinY = 0.0f;
 d->mMaxY = 0.0f;
 d->mMinZ = 0.0f;
 d->mMaxZ = 0.0f;
}

int BoMesh::pointSize()
{
 // 3 vertex components, 2 texel components
 return (3 + 2);
}

int BoMesh::vertexPos()
{
 return 0;
}

int BoMesh::texelPos()
{
 return 3;
}

void BoMesh::setMaterial(BoMaterial* mat)
{
 d->mMaterial = mat;
}

BoMaterial* BoMesh::material() const
{
 return d->mMaterial;
}

unsigned int BoMesh::facesCount() const
{
 return d->mAllFaces.count();
}

void BoMesh::createFaces(unsigned int faces)
{
 if (faces < 1) {
	boWarning() << k_funcinfo << "no faces in mesh" << endl;
	return;
 }
 if (d->mAllFaces.count() != 0) {
	boWarning() << "faces already created. resizing" << endl;
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
	BoNode* node = new BoNode(&d->mAllFaces[face]);
	d->mAllNodes.append(node);
 }
}

void BoMesh::setFace(int index, const BoFace& face)
{
 d->mAllFaces[index] = face;
}

void BoMesh::disconnectNodes()
{
 QPtrListIterator<BoNode> it(d->mAllNodes);
 for (; it.current(); ++it) {
	it.current()->setNext(0);
	it.current()->setPrevious(0);
 }
 d->mNodes= 0;
}

void BoMesh::addNodes()
{
 if (d->mNodes) {
	disconnectNodes();
 }
 if (d->mAllNodes.isEmpty()) {
	boError() << k_funcinfo << "no nodes for mesh" << endl;
	return;
 }
 BoNode* previous = 0;
 QPtrListIterator<BoNode> it(d->mAllNodes);
 d->mNodes = it.current(); // d->mNodes is the first node
 previous = d->mNodes;
 ++it;
 for (; it.current(); ++it) {
//	previous->setNext(it.current()); // TODO: do we need this?
	it.current()->setPrevious(previous);
	previous = it.current();
 }
 d->mType = GL_TRIANGLES;
}



bool BoMesh::connectNodes(const BoAdjacentDataBase* database, const QPtrList<BoNode>& nodes, QPtrList<BoNode>* found, BoNode* node) const
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
 QPtrList<BoNode> adjacent = database->adjacent(node);
 QPtrList<BoNode> remainingNodes = nodes;
 QPtrListIterator<BoNode> it(adjacent);
 int x = 0;
 for (; it.current(); ++it) {
	if (!remainingNodes.containsRef(it.current())) {
		continue;
	}
	BoNode* next = it.current();

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
		BoNode* previous = node->previous();
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
			QPtrList<BoNode> newFound;
			ok = connectNodes(database, remainingNodes, &newFound, next);
			remainingNodes.append(next);
			if (ok) {
				found->append(next);
				QPtrListIterator<BoNode> it2(newFound);
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
	QPtrList<BoNode> newFound;
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
		QPtrListIterator<BoNode> it2(newFound);
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

void BoMesh::connectNodes()
{
 if (d->mNodes) {
	disconnectNodes();
 }
 if (d->mAllNodes.isEmpty()) {
	boError() << k_funcinfo << "no nodes for mesh" << endl;
	return;
 }
 boDebug(100) << k_funcinfo << "trying to connect nodes" << endl;

 QPtrList<BoNode> allNodes = d->mAllNodes;

 // this constructs lists of all adjacent faces for all faces.
 BoAdjacentDataBase database(allNodes);

 // now we need to connect them if possible.
// BoNode* node = allNodes.first();
// allNodes.removeFirst();
 QPtrList<BoNode> connected;
 bool ok = connectNodes(&database, allNodes, &connected, 0);

 if (!ok) {
	boDebug(100) << k_funcinfo << "no connected nodes" << endl;
	disconnectNodes();
	addNodes();
	return;
 }
 if (connected.count() == 0) {
	boError() << k_funcinfo << "no connected nodes" << endl;
	disconnectNodes();
	addNodes();
	return;
 }
 /*
 if (ok != (allNodes.count() == connected.count())) {
	boError() << k_funcinfo << "wrong return value!" << endl;
	disconnectNodes();
	addNodes();
	return;
 }
 */

 BoNode* first = connected.first();
 while (first->previous()) {
	first = first->previous();
 }

 d->mNodes = first;

 if (!first->next()) {
	boError() << k_funcinfo << "oops - first node has no next node!" << endl;
	disconnectNodes();
	addNodes();
	return;
 }
 if (!first->next()->next()) {
	boError() << k_funcinfo << "oops - first node has no next node!" << endl;
	disconnectNodes();
	addNodes();
	return;
 }

 d->mNodes = first;
 d->mType = GL_TRIANGLE_STRIP;
}
/*
void findConnectable(const QPtrList<Lib3dsFace>& nodes, QPtrList<Lib3dsFace>* connected, Lib3dsFace* face, Lib3dsMesh* mesh)
{
 BO_CHECK_NULL_RET(face)
 BO_CHECK_NULL_RET(connected)
 if (nodes.isEmpty()) {
	connected->append(face);
	return;
 }
 if (nodes.containsRef(face)) {
	boError() << k_funcinfo << "list must not contain the face that we search connectables for" << endl;
	return;
 }
 BoVector3 searchFace[3]; // the vectors of the face we search connectables for
 BoVector3::makeVectors(searchFace, mesh, face);

 BoVector3 v[3];
 QPtrList<Lib3dsFace> found;
 QPtrList<Lib3dsFace> nodesLeft;

 QPtrListIterator<Lib3dsFace> it(nodes);
 for (; it.current(); ++it) {
	BoVector3::makeVectors(v, mesh, it.current());
	if (BoVector3::isAdjacent(searchFace, v)) {
		found.clear();
		nodesLeft = nodes;
		nodesLeft.removeRef(it.current());
		findConnectable(nodesLeft, &found, it.current(), mesh);
		if (nodesLeft.count() + 1 == found.count()) {
			// all faces could be connected. first append the face
			// that we searched for to the list, then the faces that
			// have been found for that face.
			connected->append(face);
			QPtrListIterator<Lib3dsFace> foundIt(found);
			for (; foundIt.current(); ++foundIt) {
				connected->append(foundIt.current());
			}
			break;
		}
	}
 }
}
*/

int BoMesh::type() const
{
 return d->mType;
}

BoNode* BoMesh::nodes() const
{
 return d->mNodes;
}

void BoMesh::deleteNodes()
{
 if (d->mNodes) {
	BoNode* first = d->mNodes;
	while (first->previous()) {
		// WARNING: we mustn't have any node appear two times in the
		// list, otherwise we have an infinite loop here
		first = first->previous();
	}
 }
}

void BoMesh::deleteNodes(BoNode* node)
{
 // node is the *first* node!
 if (node) {
	node->setPrevious(0);
	BoNode* next = node;
	do {
		BoNode* current = next;
		next = current->next();
		delete current;
	} while (next);
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
 if (d->mAllocatedPoints) {
	boError() << k_funcinfo << "points already allocated!" << endl;
	if (d->mPoints == d->mAllocatedPoints) {
		d->mPoints = 0;
	}
	boMem->freeFloatArray(d->mAllocatedPoints);
	d->mAllocatedPoints = 0;
 }
 if (d->mPoints) {
	boError() << k_funcinfo << "non-NULL points array!" << endl;
	d->mPoints = 0;
 }
 d->mPointCount = points;

 // note that a lot of space is wasted - some objects are not textured
 // but we can render more efficient this way.
 d->mAllocatedPoints = boMem->allocateFloatArray(d->mPointCount * pointSize());
 d->mPoints = d->mAllocatedPoints;

 // AB: we initialize the array elements now. this is close to useless since we
 // overwrite it later anyway. but valgrind complains for meshes that don't use
 // textures (since we copy uninitialized values around).
 for (unsigned int i = 0; i < d->mPointCount; i++) {
	for (int j = 0; j < pointSize(); j++) {
		d->mAllocatedPoints[i * pointSize() + j] = 0.0f;
	}
 }
}

void BoMesh::setVertex(unsigned int index, const BoVector3& vertex)
{
 BO_CHECK_NULL_RET(d->mPoints);
 if (index >= points()) {
	boError() << k_funcinfo << "invalid index " << index << " max=" << points() - 1 << endl;
	return;
 }
 d->mPoints[index * pointSize() + vertexPos() + 0] = vertex[0];
 d->mPoints[index * pointSize() + vertexPos() + 1] = vertex[1];
 d->mPoints[index * pointSize() + vertexPos() + 2] = vertex[2];
}

BoVector3 BoMesh::vertex(unsigned int p) const
{
 if (!d->mPoints) {
	boError() << k_funcinfo << "no points allocated" << endl;
	return BoVector3();
 }
 if (p >= points()) {
	boError() << k_funcinfo << "invalid point " << p << " max=" << points() - 1 << endl;
	return BoVector3();
 }
 return BoVector3(&d->mPoints[p * pointSize() + vertexPos()]);
}

BoVector3 BoMesh::vertex(unsigned int face, unsigned int i) const
{
 if (face >= facesCount()) {
	boError() << k_funcinfo << "invalid face " << face << " have only " << facesCount() << endl;
	return BoVector3();
 }
 if (i >= 3) {
	boError() << k_funcinfo << " vertex index must be 0..2, not " << i << endl;
	i = i % 3;
 }
 return vertex(d->mAllFaces[face].pointIndex()[i] - d->mPointsMovedBy);
}

void BoMesh::setTexel(unsigned int index, const BoVector3& texel)
{
 BO_CHECK_NULL_RET(d->mPoints);
 if (index >= points()) {
	boError() << k_funcinfo << "invalid index " << index << " max=" << points() - 1 << endl;
	return;
 }
 d->mPoints[index * pointSize() + texelPos() + 0] = texel[0];
 d->mPoints[index * pointSize() + texelPos() + 1] = texel[1];
}

void BoMesh::setNormal(unsigned int face, int vertex, const BoVector3& normal)
{
 BO_CHECK_NULL_RET(d->mPoints);
 if (face >= facesCount()) {
	boError() << k_funcinfo << "invalid face " << face << " max=" << facesCount() << endl;
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

BoVector3 BoMesh::normal(unsigned int face, unsigned int vertex) const
{
 if (face >= facesCount()) {
	boError() << k_funcinfo << "invalid face " << face << " max= " << facesCount() << endl;
	return BoVector3();
 }
 if (vertex >= 3) {
	boError() << k_funcinfo << "invalid vertex " << vertex << " must be 0..2" << endl;
	vertex = vertex % 3;
 }
 return d->mAllFaces[face].normal(vertex);
}

BoVector3 BoMesh::normal(unsigned int p) const
{
 if (p >= points()) {
	boError() << k_funcinfo << "invalid point " << p << " max=" << points() - 1 << endl;
	return BoVector3();
 }
 int face = p / 3;
 int vertex = p % 3;
 return normal(face, vertex);
}

void BoMesh::calculateNormals()
{
 // we don't use lib3ds' normals so that we can easily support file formats that
 // dont store the normals. furthermore we depend on less external data :)
 //
 // note that we calculate 1 normal per face only at the moment!
 for (unsigned int face = 0; face < facesCount(); face++) {
	BoVector3 a(vertex(face, 0));
	BoVector3 b(vertex(face, 1));
	BoVector3 c(vertex(face, 2));
#if AB_DEBUG_1
	boDebug() << k_funcinfo << face << ": a=" << a.debugString() << endl;
	boDebug() << k_funcinfo << face << ": b=" << b.debugString() << endl;
	boDebug() << k_funcinfo << face << ": c=" << c.debugString() << endl;
#endif

	// AB: we use the same order as lib3ds does. check whether this is the
	// correct order.
	BoVector3 p, q;
	p = c - b;
	q = a - b;
#if AB_DEBUG_1
	boDebug() << k_funcinfo << face << ": p=" << p.debugString() << endl;
	boDebug() << k_funcinfo << face << ": q=" << q.debugString() << endl;
#endif
	BoVector3 normal = BoVector3::crossProduct(p, q);
#if AB_DEBUG_1
	boDebug() << k_funcinfo << face << ": normal1=" << normal.debugString() << endl;
#endif
	normal.normalize();
#if AB_DEBUG_1
	boDebug() << k_funcinfo << face << ": normal2=" << normal.debugString() << endl;
#endif

	setNormal(face, -1, normal);
 }
}

void BoMesh::renderMesh(const QColor* teamColor)
{
 // AB: it would be better to do most of this in BosonModel instead. we could
 // group several meshes into a single glBegin()/glEnd() pair

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
	if (!d->mPointsCache || d->mPointsCacheCount == 0) {
		boError() << k_funcinfo << "no point cache!" << endl;
	} else {
		glBegin(type());

		BoNode* node = nodes();
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

// there MUST be a valid context set already!!
void BoMesh::loadDisplayList(const QColor* teamColor, bool reload)
{
 // AB: it would be better to do most of this in BosonModel instead. we could
 // group several meshes into a single glBegin()/glEnd() pair
 if (!d->mDisplayList) {
	d->mDisplayList = glGenLists(1);
	if (d->mDisplayList == 0) {
		boError() << k_funcinfo << "NULL display list generated" << endl;
		return;
	}
 } else {
	if (reload) {
		// it is important that the list is deleted, but the
		// newly generated list must have the *same* number as
		// it had before!
		glDeleteLists(d->mDisplayList, 1);
	} else {
		boWarning() << k_funcinfo << "mesh was already loaded!" << endl;
		return;
	}
 }
 boDebug(100) << k_funcinfo << endl;

 glNewList(d->mDisplayList, GL_COMPILE);
 renderMesh(teamColor);
 glEndList();
}

unsigned int BoMesh::points() const
{
 return d->mPointCount;
}

bool BoMesh::isTeamColor() const
{
 return d->mIsTeamColor;
}

void BoMesh::setIsTeamColor(bool c)
{
 d->mIsTeamColor = c;
}

GLuint BoMesh::displayList() const
{
 return d->mDisplayList;
}

void BoMesh::movePoints(float* array, int index)
{
 if (!d->mAllocatedPoints) {
	boError() << k_funcinfo << "no points allocated" << endl;
	return;
 }
 // first of all we fix the indices of all faces.
 for (unsigned int i = 0; i < facesCount(); i++) {
	BoFace* face = &d->mAllFaces[i];
	const int* orig = face->pointIndex();
	int p[3];
	p[0] = index + orig[0];
	p[1] = index + orig[1];
	p[2] = index + orig[2];
	face->setPointIndex(p);
 }
 // now we move the vertices and texture coordinates to the new array
 for (unsigned int i = 0; i < points(); i++) {
	for (int j = 0; j < pointSize(); j++) {
		array[(index + i) * pointSize() + j] = d->mPoints[i * pointSize() + j];
	}
 }
#warning FIXME: memory fragmentation
 // we allocate a lot of floats and free them later (moving the values to
 // a single array). this will probably lead to quite some memory
 // fragmentation. we should change this design.
 // but remember that the array that is actually used (d->mPoints) is allocated
 // only once per model - so at this point there is no memory fragmentation (and
 // this is the important place)
 boMem->freeFloatArray(d->mAllocatedPoints);
 d->mAllocatedPoints = 0;
 d->mPoints = array + index * pointSize();
 d->mPointsMovedBy = index;
 if (d->mPointsCache) {
	// we need to regenerate the cache
	createPointCache();
 }
}

void BoMesh::createPointCache()
{
 // the point cache is an array of all vertex indices of the faces. so if face1
 // references vertices 2,3,1 and face 2 references vertices 5,4,7 then the
 // point cache will be 2,3,1,5,4,7 (assuming we don't use GL_TRIANGLES)
 boMem->freeUIntArray(d->mPointsCache);
 d->mPointsCacheCount = 0;
 BoNode* node = nodes();
 if (!node) {
	boError() << k_funcinfo << "NULL node" << endl;
	return;
 }
 if (points() < 1) {
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
	if (firstPoint < 0 || (unsigned int)firstPoint >= points()) {
		boError() << k_funcinfo << "invalid first point " << firstPoint << endl;
		return;
	}
	if (secondPoint < 0 || (unsigned int)secondPoint >= points()) {
		boError() << k_funcinfo << "invalid second point" << secondPoint << endl;
		return;
	}
	if (thirdPoint < 0 || (unsigned int)thirdPoint >= points()) {
		boError() << k_funcinfo << "invalid third point" << thirdPoint << endl;
		return;
	}

	// 3 basic points + one point per remaining face
	d->mPointsCacheCount = 3 + (nodesCount - 3) * 1;
	d->mPointsCache = boMem->allocateUIntArray(d->mPointsCacheCount);

	d->mPointsCache[0] = (unsigned int)node->pointIndex()[firstPoint];
	d->mPointsCache[1] = (unsigned int)node->pointIndex()[secondPoint];
	d->mPointsCache[2] = (unsigned int)node->pointIndex()[thirdPoint];

	int element = 3;
	// we skip the entire first face. all points have been rendered above.
	node = node->next();
	for (; node; node = node->next()) {
		int point = node->relevantPoint();
		if (point < 0 || point > 2) {
			boError( )<< k_funcinfo "oops - invalid point " << point << endl;
			continue;
		}
		d->mPointsCache[element] = (unsigned int)node->pointIndex()[point];
		element++;
	}
 } else if (type() == GL_TRIANGLES) {
	// 3 points per face
	d->mPointsCacheCount = nodesCount * 3;
	d->mPointsCache = boMem->allocateUIntArray(d->mPointsCacheCount);
	int element = 0;
	for (node = nodes(); node; node = node->next()) {
		for (int i = 0; i < 3; i++) {
			d->mPointsCache[element] = (unsigned int)node->pointIndex()[i];
			element++;
		}
	}
 } else {
	boError(100) << k_funcinfo << "Invalid type: " << type() << endl;
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

 d->mBoundingObject->render();

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

