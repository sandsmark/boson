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
#include "bosonmodel.h"

#include <qptrlist.h>

#include <lib3ds/mesh.h>
#include <GL/gl.h>


void findConnectable(const QPtrList<Lib3dsFace>& faces, QPtrList<Lib3dsFace>* connected, Lib3dsFace* face, Lib3dsMesh* mesh);

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

void BoNode::setFace(Lib3dsFace* face)
{
 mFace = face;
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
 mFace = 0;
 mRelevantPoint = -1;
}

class BoMeshPrivate
{
public:
	BoMeshPrivate()
	{
		mFaces = 0;
	}
	int mType;
	BoNode* mFaces;
};

BoMesh::BoMesh()
{
 d = new BoMeshPrivate;
 d->mType = GL_TRIANGLES;
}

BoMesh::~BoMesh()
{
 deleteFaces();
 delete d;
}

void BoMesh::addFaces(Lib3dsMesh* mesh)
{
 BO_CHECK_NULL_RET(mesh);
 if (d->mFaces) {
	deleteFaces();
 }
 if (mesh->faces < 1) {
	boWarning() << k_funcinfo << "no faces in " << mesh->name << endl;
	return;
 }
 d->mFaces = new BoNode();
 d->mFaces->setFace(&mesh->faceL[0]);
 BoNode* node = d->mFaces;
 for (unsigned int face = 1; face < mesh->faces; face++) {
	Lib3dsFace* f = &mesh->faceL[face];
	BoNode* next = new BoNode(node);
	next->setFace(f);
	node = next;
 }
 d->mType = GL_TRIANGLES;
}

void BoMesh::connectFaces(Lib3dsMesh* mesh)
{
 BO_CHECK_NULL_RET(mesh);
 if (d->mFaces) {
	deleteFaces();
 }
 if (mesh->faces < 1) {
	boWarning() << k_funcinfo << "no faces in " << mesh->name << endl;
	return;
 }
 boDebug() << "trying to connect faces for " << mesh->name << endl;

 // first we contruct a list of all faces
 QPtrList<Lib3dsFace> allFaces;
 for (unsigned int p = 0; p < mesh->faces; p++) {
	allFaces.append(&mesh->faceL[p]);
 }
 if (allFaces.count() == 0) {
	boError() << k_funcinfo << "no faces" << endl;
	return;
 }

 // now we need to connect them if possible.
 BoNode* first = 0;
 BoNode* last = 0;
 BoNode* current;
 d->mFaces = new BoNode();
 current = first = last = d->mFaces;
 current->setFace(allFaces.first());
 allFaces.removeFirst();

 do {
	QPtrListIterator<Lib3dsFace> it(allFaces);
	BoNode* next = 0;
	for (; it.current(); ++it) {
		int points = 0;
		BoVector3 currentPoints[3];
		BoVector3 otherPoints[3];
		BosonModel::makeVectors(currentPoints, mesh, current->face());
		BosonModel::makeVectors(otherPoints, mesh, it.current());
		for (int i = 0; i < 3; i++) {
			if (BosonModel::findPoint(currentPoints[i], otherPoints) >= 0) {
				points++;
			}
		}
		if (points >= 2) {
			BoNode* n = new BoNode();
			n->setFace(it.current());
			allFaces.remove(it);

			if (current == first) {
				current->setPrevious(n);
				first = n;
				next = n;
				current = n;
			} else if (current == last) {
				current->setNext(n);
				last = n;
				next = n;
				current = n;
			} else {
				boError() << k_funcinfo << "invalid current value" << endl;
			}
			break;
		}
	}
	if (next == 0) {
		if (current == first) {
			// the current node is the final first node. now we
			// search in the other direction for the last node.
			current = last;
		} else if (current == last) {
			// we found both first and last node. at this point
			// allFaces should be empty.
			// otherwise we could not connect all faces
			// if allFaces is empty then this won't be reached at
			// all!
			current = 0;
		}
	}
 } while (current != 0 && allFaces.count() > 0);
 if (!allFaces.isEmpty()) {
	boWarning() << k_funcinfo << "not all faces could be connected in " << mesh->name << "!" << endl;
	//TODO: delete faces and call addFaces()
	return;
 }
 if (!first || !last) {
	boError() << k_funcinfo << "Invalid first/last pointers" << endl;
	//TODO: delete faces and call addFaces()
	return;
 }

 // now we try to find the relevant (for rendering) points.
 // that is the point that is *not* in the previous face.
 // WARNING: we assume that this point *does* exist for all faces!
 first->setRelevantPoint(-1); // all are relevant in the first node
 for (BoNode* n = first->next(); n; n = n->next()) {
	BoNode* p = n->previous();

	// AB: we should store the vertices in BoNode. we could speed up much
	// stuff (also the loopthe loop  above) with that.
	BoVector3 points[3];
	BoVector3 previousPoints[3];
	BosonModel::makeVectors(points, mesh, n->face());
	BosonModel::makeVectors(previousPoints, mesh, p->face());
	int found = 0; // for error checking
	int relevant = -1;
	for (int i = 0; i < 3; i++) {
		int p = BosonModel::findPoint(points[i], previousPoints);
		if (p >= 0) {
			found++;
		} else {
			relevant = i;
		}
	}
	if (found < 2) {
		boError() << k_funcinfo << "faces are not adjacent!" << endl;
		//TODO: delete faces and call addFaces()
		return;
	} else if (found == 3) {
		// note that it *is* possible that all 3 points are equal!
		// e.g. for a vector ((1,2,3),(4,5,6),(1,2,3))
		// --> this would be a line only (represented as face/triangle). but
		// since we demand at least 2 points to be equal this would lead to
		// exactly 3 points equal (we have at least one model where this
		// happens)
		// now we need to find a relevant point that will get
		// rendered...
		boWarning() << k_funcinfo << "FIXME: random relevant point in mesh " << mesh->name << endl;
		relevant = 0; // FIXME
	}
	if (relevant < 0) {
		boError() << k_funcinfo << "no relevant point found" << endl;
	} else {
		n->setRelevantPoint(relevant);
	}
 }

 d->mFaces = first;
 d->mType = GL_TRIANGLE_STRIP;
 boDebug() << "faces connected" << endl;
}

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
 BosonModel::makeVectors(searchFace, mesh, face);

 BoVector3 v[3];
 QPtrList<Lib3dsFace> found;
 QPtrList<Lib3dsFace> facesLeft;

 QPtrListIterator<Lib3dsFace> it(faces);
 for (; it.current(); ++it) {
	BosonModel::makeVectors(v, mesh, it.current());
	if (BosonModel::isAdjacent(searchFace, v)) {
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
	BoNode* next = first;
	do {
		BoNode* current = next;
		next = current->next();
		delete current;
	} while (next);
 }
}
