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

#include "bomesh.h"
#include "bodebug.h"
#include "bo3dtools.h"

#include <qptrlist.h>
#include <qcolor.h>

#include <GL/gl.h>

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
			boDebug() << "adjacent to " << it.current()->debugString() << " (Face " << mAllNodes.find(it.current()) + 1 << "):" << endl;
			QPtrList<BoNode> a = adjacent(it.current());
			QPtrListIterator<BoNode> it2(a);
			for (; it2.current(); ++it2) {
				boDebug() << it2.current()->debugString() << " (Face " << mAllNodes.find(it2.current()) + 1 << ")" << endl;
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


BoNode::BoNode(BoNode* previous)
{
 init();
 setPrevious(previous);
}

BoNode::BoNode()
{
 init();
}

BoNode::~BoNode()
{
}

void BoNode::setFace(const int* points)
{
 // we store the index of the point in the mesh only. that is necessary for
 // TRIANGLE_STRIPs
 // --> equal vertices don't have to use the same texture coordinates, so we
 // can't connect them.
 mPointIndex[0] = points[0];
 mPointIndex[1] = points[1];
 mPointIndex[2] = points[2];
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
 mPointIndex[0] = -1;
 mPointIndex[1] = -1;
 mPointIndex[2] = -1;
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
 s = QString("%1  %2  %3").arg(mPointIndex[0]).arg(mPointIndex[1]).arg(mPointIndex[2]);
 return s;
}

int BoNode::findPointIndex(int index) const
{
 if (mPointIndex[0] == index) {
	return 0;
 }
 if (mPointIndex[1] == index) {
	return 1;
 }
 if (mPointIndex[2] == index) {
	return 2;
 }
 return -1;
}


class BoMeshPrivate
{
public:
	BoMeshPrivate()
	{
		mFaces = 0;

		mAllocatedPoints = 0;
		mPoints = 0;

		mPointsCache = 0;
		mPointsCacheCount = 0;
	}
	int mType;
	BoNode* mFaces;
	bool mIsTextured;
	bool mIsTeamColor;

	GLuint mTexture;
	GLuint mDisplayList;

	QPtrList<BoNode> mAllNodes;

	unsigned int mPointCount;
	float* mPoints;
	float* mAllocatedPoints;

	// the list of points in the final order (after connectFaces() or
	// addFaces() was called). iterating through faces() is equalivent (for
	// some modes the BoNode::relevantPoint() will have to be used though)
	unsigned int* mPointsCache;
	unsigned int mPointsCacheCount;

	float mMinX;
	float mMaxX;
	float mMinY;
	float mMaxY;
	float mMinZ;
	float mMaxZ;
};

BoMesh::BoMesh(unsigned int faces)
{
 init();
 createNodes(faces);
}

BoMesh::~BoMesh()
{
 if (d->mDisplayList) {
	glDeleteLists(d->mDisplayList, 1);
 }
 d->mAllNodes.clear();
 delete[] d->mAllocatedPoints;
 d->mPoints = 0; // do NOT delete!
 delete d;
}

void BoMesh::init()
{
 d = new BoMeshPrivate;
 d->mAllNodes.setAutoDelete(true);
 d->mType = GL_TRIANGLES;
 d->mIsTextured = false;
 d->mIsTeamColor = false;
 d->mTexture = 0;
 d->mDisplayList = 0;
 d->mPointCount = 0;

 d->mMinX = 0.0f;
 d->mMaxX = 0.0f;
 d->mMinY = 0.0f;
 d->mMaxY = 0.0f;
 d->mMinZ = 0.0f;
 d->mMaxZ = 0.0f;
}

void BoMesh::createNodes(unsigned int faces)
{
 if (d->mAllNodes.count() > 0) {
	boDebug() << "nodes already created. nothing to do." << endl;
	return;
 }
 if (faces < 1) {
	boWarning() << k_funcinfo << "no faces in mesh" << endl;
	return;
 }

 for (unsigned int face = 0; face < faces; face++) {
	BoNode* node = new BoNode();
	d->mAllNodes.append(node);
 }
}

void BoMesh::setFace(int index, const int* points)
{
 BoNode* n = d->mAllNodes.at(index);
 BO_CHECK_NULL_RET(n);
 n->setFace(points);
}

void BoMesh::disconnectFaces()
{
 QPtrListIterator<BoNode> it(d->mAllNodes);
 for (; it.current(); ++it) {
	it.current()->setNext(0);
	it.current()->setPrevious(0);
 }
 d->mFaces = 0;
}

void BoMesh::addFaces()
{
 if (d->mFaces) {
	disconnectFaces();
 }
 if (d->mAllNodes.isEmpty()) {
	boError() << k_funcinfo << "no nodes for mesh" << endl;
	return;
 }
 BoNode* previous = 0;
 QPtrListIterator<BoNode> it(d->mAllNodes);
 d->mFaces = it.current(); // d->mFaces is the first node
 previous = d->mFaces;
 ++it;
 for (; it.current(); ++it) {
//	previous->setNext(it.current()); // TODO: do we need this?
	it.current()->setPrevious(previous);
	previous = it.current();
 }
 d->mType = GL_TRIANGLES;
}



bool BoMesh::connectFaces(const BoAdjacentDataBase* database, const QPtrList<BoNode>& faces, QPtrList<BoNode>* found, BoNode* node) const
{
 static int call = 0;
 static int max = 0;
 call++;
 if (call > max) {
	max = call;
 }
 boDebug() << "connectFaces() - call " << call << " max=" << max << endl;
 if (call == 11) {
	return true;
 }
 if (faces.isEmpty()) {
	return true;
 }
 if (!found->isEmpty()) {
	boError() << k_funcinfo << "found must be empty" << endl;
	return false;
 }
 if (node && faces.containsRef(node)) {
	boError() << k_funcinfo << "list shouldn't contain that node" << endl;
	return false;
 }
 QPtrList<BoNode> adjacent = database->adjacent(node);
 QPtrList<BoNode> remainingFaces = faces;
 QPtrListIterator<BoNode> it(adjacent);
 int x = 0;
 for (; it.current(); ++it) {
	if (!remainingFaces.containsRef(it.current())) {
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
				boDebug() << k_funcinfo << "no relevant point found" << endl;
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
			remainingFaces.removeRef(next);
			QPtrList<BoNode> newFound;
			ok = connectFaces(database, remainingFaces, &newFound, next);
			remainingFaces.append(next);
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
	remainingFaces.removeRef(next);
	QPtrList<BoNode> newFound;
	bool ok = connectFaces(database, remainingFaces, &newFound, next);
	if (!ok) {
		// reset next and remove next from node
		next->setNext(0);
		next->setPrevious(0);
		next->setRelevantPoint(-1);
		if (node) {
			node->setNext(0);
		}

		// next wasn't the next node - so it is available again
		remainingFaces.append(next);
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
// boDebug() << "no next in adjacent count=" << adjacent.count()  << endl;


 boDebug() << "none found (call " << call << ")" << endl;
 call--;
 return false;
}

void BoMesh::connectFaces()
{
 if (d->mFaces) {
	disconnectFaces();
 }
 if (d->mAllNodes.isEmpty()) {
	boError() << k_funcinfo << "no nodes for mesh" << endl;
	return;
 }
 boDebug() << "trying to connect faces" << endl;

 QPtrList<BoNode> allFaces = d->mAllNodes;

 // this constructs lists of all adjacent faces for all faces.
 BoAdjacentDataBase database(allFaces);

 // now we need to connect them if possible.
// BoNode* node = allFaces.first();
// allFaces.removeFirst();
 QPtrList<BoNode> connected;
 bool ok = connectFaces(&database, allFaces, &connected, 0);

 if (!ok) {
	boDebug() << "no connected faces" << endl;
	disconnectFaces();
	addFaces();
	return;
 }
 if (connected.count() == 0) {
	boError() << k_funcinfo << "no connected faces" << endl;
	disconnectFaces();
	addFaces();
	return;
 }
 /*
 if (ok != (allFaces.count() == connected.count())) {
	boError() << k_funcinfo << "wrong return value!" << endl;
	disconnectFaces();
	addFaces();
	return;
 }
 */

 BoNode* first = connected.first();
 while (first->previous()) {
	first = first->previous();
 }

 d->mFaces = first;

 if (!first->next()) {
	boError() << k_funcinfo << "oops - first node has no next node!" << endl;
	disconnectFaces();
	addFaces();
	return;
 }
 if (!first->next()->next()) {
	boError() << k_funcinfo << "oops - first node has no next node!" << endl;
	disconnectFaces();
	addFaces();
	return;
 }

 d->mFaces = first;
 d->mType = GL_TRIANGLE_STRIP;
}
/*
void findConnectable(const QPtrList<Lib3dsFace>& faces, QPtrList<Lib3dsFace>* connected, Lib3dsFace* face, Lib3dsMesh* mesh)
{
 BO_CHECK_NULL_RET(face)
 BO_CHECK_NULL_RET(connected)
 if (faces.isEmpty()) {
	connected->append(face);
	return;
 }
 if (faces.containsRef(face)) {
	boError() << k_funcinfo << "list must not contain the face that we search connectables for" << endl;
	return;
 }
 BoVector3 searchFace[3]; // the vectors of the face we search connectables for
 BoVector3::makeVectors(searchFace, mesh, face);

 BoVector3 v[3];
 QPtrList<Lib3dsFace> found;
 QPtrList<Lib3dsFace> facesLeft;

 QPtrListIterator<Lib3dsFace> it(faces);
 for (; it.current(); ++it) {
	BoVector3::makeVectors(v, mesh, it.current());
	if (BoVector3::isAdjacent(searchFace, v)) {
		found.clear();
		facesLeft = faces;
		facesLeft.removeRef(it.current());
		findConnectable(facesLeft, &found, it.current(), mesh);
		if (facesLeft.count() + 1 == found.count()) {
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

BoNode* BoMesh::faces() const
{
 return d->mFaces;
}

void BoMesh::deleteFaces()
{
 if (d->mFaces) {
	BoNode* first = d->mFaces;
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

void BoMesh::setTextured(bool tex)
{
 d->mIsTextured = tex;
}

void BoMesh::setTextureObject(GLuint tex)
{
 d->mTexture = tex;
}

bool BoMesh::textured() const
{
 return d->mIsTextured;
}

GLuint BoMesh::textureObject() const
{
 return d->mTexture;
}

void BoMesh::allocatePoints(unsigned int points)
{
 if (d->mAllocatedPoints) {
	boError() << k_funcinfo << "points already allocated!" << endl;
	if (d->mPoints == d->mAllocatedPoints) {
		d->mPoints = 0;
	}
	delete[] d->mAllocatedPoints;
	d->mAllocatedPoints = 0;
 }
 if (d->mPoints) {
	boError() << k_funcinfo << "non-NULL points array!" << endl;
	d->mPoints = 0;
 }
 d->mPointCount = points;
 // note that some space is wasted - some objects are not textured. but we can
 // render more efficient this way.
 d->mAllocatedPoints = new float[d->mPointCount * 5];
 d->mPoints = d->mAllocatedPoints;

 // AB: we initialize the array elements now. this is close to useless since we
 // overwrite it later anyway. but valgrind complains for meshes that don't use
 // textures (since we copy uninitialized values around).
 for (unsigned int i = 0; i < points * 5; i++) {
	d->mAllocatedPoints[i] = 0.0f;
 }
}

void BoMesh::setVertex(unsigned int index, const BoVector3& vertex)
{
 BO_CHECK_NULL_RET(d->mPoints);
 if (index >= points()) {
	boError() << k_funcinfo << "invalid index " << index << " max=" << points() << endl;
 }
 d->mPoints[index * 5 + 0] = vertex[0];
 d->mPoints[index * 5 + 1] = vertex[1];
 d->mPoints[index * 5 + 2] = vertex[2];
}

void BoMesh::setTexel(unsigned int index, const BoVector3& texel)
{
 BO_CHECK_NULL_RET(d->mPoints);
 if (index >= points()) {
	boError() << k_funcinfo << "invalid index " << index << " max=" << points() << endl;
 }
 d->mPoints[index * 5 + 3] = texel[0];
 d->mPoints[index * 5 + 4] = texel[1];
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

 if (!textured()) {
	glBindTexture(GL_TEXTURE_2D, 0);
	if (teamColor) {
		glPushAttrib(GL_CURRENT_BIT);
		glColor3ub(teamColor->red(), teamColor->green(), teamColor->blue());
		resetColor = true;
	}
 } else {
	glBindTexture(GL_TEXTURE_2D, textureObject());
 }


 if (!d->mPointsCache || d->mPointsCacheCount == 0) {
	boError() << k_funcinfo << "no point cache!" << endl;
 } else {
	glDrawElements(type(), d->mPointsCacheCount, GL_UNSIGNED_INT, d->mPointsCache);
 }


 if (resetColor) {
	// we need to reset the color (mainly for the placement preview)
	glPopAttrib();
	resetColor = false;
 }
}

void BoMesh::renderPoint(int point)
{
boWarning() << k_funcinfo << "obsolete!" << endl;
 if (textured()) {
	glTexCoord2fv(d->mPoints + 5 * point + 3);
 }
 glVertex3fv(d->mPoints + 5 * point);
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
 boDebug() << k_funcinfo << endl;

 glNewList(d->mDisplayList, GL_COMPILE);
 renderMesh(teamColor);
 glEndList();
}

unsigned int BoMesh::points() const
{
 return d->mPointCount;
}

BoVector3 BoMesh::point(unsigned int p) const
{
 if (p >= points()) {
	boError() << k_funcinfo << "invalid point " << p << endl;
	return BoVector3();
 }
 return BoVector3(&d->mPoints[p * 5]);
}

bool BoMesh::isTeamColor() const
{
 return d->mIsTeamColor;
}

void BoMesh::setIsTeamColor(bool c)
{
 d->mIsTeamColor = c;
 // AB: this implies setTextured(false) ! -> should be called for this, too!
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
 // first of all we fix the indices of all nodes.
 QPtrListIterator<BoNode> it(d->mAllNodes);
 for (; it.current(); ++it) {
	const int* orig = it.current()->pointIndex();
	int p[3];
	p[0] = index + orig[0];
	p[1] = index + orig[1];
	p[2] = index + orig[2];
	it.current()->setFace(p);
 }
 // now we move the vertices and texture coordinates to the new array
 for (unsigned int i = 0; i < points(); i++) {
	array[(index + i) * 5 + 0] = d->mPoints[i * 5 + 0];
	array[(index + i) * 5 + 1] = d->mPoints[i * 5 + 1];
	array[(index + i) * 5 + 2] = d->mPoints[i * 5 + 2];
	array[(index + i) * 5 + 3] = d->mPoints[i * 5 + 3];
	array[(index + i) * 5 + 4] = d->mPoints[i * 5 + 4];
 }
 delete[] d->mAllocatedPoints;
 d->mAllocatedPoints = 0;
 d->mPoints = array + index * 5;
 if (d->mPointsCache) {
	// we need to regenerate the cache
	createPointCache();
 }
}

void BoMesh::createPointCache()
{
 delete[] d->mPointsCache;
 d->mPointsCache = 0;
 d->mPointsCacheCount = 0;
 BoNode* node = faces();
 if (!node) {
	boError() << k_funcinfo << "NULL node" << endl;
	return;
 }
 if (points() < 1) {
	return;
 }
 int facesCount = 0;
 // count the number of nodes in our list.
 // note that we mustn't assume that all nodes are in that list!
 for (; node; node = node->next()) {
	facesCount++;
 }
 node = faces();
 if (type() == GL_TRIANGLE_STRIP) {
	boDebug() << "creating _STRIP" << endl;
	if (!node->next()) {
		boError() << k_funcinfo << "less than 2 faces in mesh! this is not supported" << endl;
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
	d->mPointsCacheCount = 3 + (facesCount - 3) * 1;
	d->mPointsCache = new unsigned int[d->mPointsCacheCount];

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
	d->mPointsCacheCount = facesCount * 3;
	d->mPointsCache = new unsigned int[d->mPointsCacheCount];
	int element = 0;
	for (; node; node = node->next()) {
		for (int i = 0; i < 3; i++) {
			d->mPointsCache[element] = (unsigned int)node->pointIndex()[i];
			element++;
		}
	}
 } else {
	boError() << k_funcinfo << "Invalid type: " << type() << endl;
 }
}

void BoMesh::calculateMaxMin()
{
 // NOT time critical! (called on startup only)
 BoVector3 v = point(0);
 d->mMinZ = v.z();
 d->mMaxZ = v.z();
 d->mMinX = v.x();
 d->mMaxX = v.x();
 d->mMinY = v.y();
 d->mMaxY = v.y();
 for (unsigned int i = 0; i < points(); i++) {
	v = point(i);
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

