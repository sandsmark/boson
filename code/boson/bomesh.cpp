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

#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/file.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>

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
 // we store the index of the point in the mesh only. that is necessary for
 // TRIANGLE_STRIPs
 // --> equal vertices don't have to use the same texture coordinates, so we
 // can't connect them.
 mPointIndex[0] = face->points[0];
 mPointIndex[1] = face->points[1];
 mPointIndex[2] = face->points[2];
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
		mMesh = 0;
		mMaterial = 0;

		mTexels = 0;
		mVertices = 0;
	}
	int mType;
	BoNode* mFaces;
	bool mTextured;

	Lib3dsMesh* mMesh;
	Lib3dsMaterial* mMaterial;
	GLuint mTexture;

	QPtrList<BoNode> mAllNodes;

	//  TODO: combine with vertex data into a single array! we can optimize
	//  vertex arrays this way!
	float* mTexels;
	float* mVertices;
};

BoMesh::BoMesh(Lib3dsMesh* mesh)
{
 init();
 d->mMesh = mesh;
 createNodes();
}

BoMesh::~BoMesh()
{
 d->mAllNodes.clear();
 delete[] d->mTexels;
 delete[] d->mVertices;
 delete d;
}

void BoMesh::init()
{
 d = new BoMeshPrivate;
 d->mAllNodes.setAutoDelete(true);
 d->mType = GL_TRIANGLES;
 d->mTextured = false;
 d->mTexture = 0;
}

void BoMesh::createNodes()
{
 BO_CHECK_NULL_RET(d->mMesh);
 if (d->mAllNodes.count() > 0) {
	boDebug() << "nodes already created. nothing to do." << endl;
	return;
 }
 if (d->mMesh->faces < 1) {
	boWarning() << k_funcinfo << "no faces in " << d->mMesh->name << endl;
	return;
 }

 for (unsigned int face = 0; face < d->mMesh->faces; face++) {
	Lib3dsFace* f = &d->mMesh->faceL[face];
	BoNode* node = new BoNode();
	node->setFace(f);
	d->mAllNodes.append(node);
 }
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
 BO_CHECK_NULL_RET(d->mMesh);
 if (d->mFaces) {
	disconnectFaces();
 }
 if (d->mAllNodes.isEmpty()) {
	boError() << k_funcinfo << "no nodes for " << d->mMesh->name << endl;
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
 BO_CHECK_NULL_RET(d->mMesh);
 if (d->mFaces) {
	disconnectFaces();
 }
 if (d->mAllNodes.isEmpty()) {
	boError() << k_funcinfo << "no nodes for " << d->mMesh->name << endl;
	return;
 }
 boDebug() << "trying to connect faces for " << d->mMesh->name << endl;

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

Lib3dsMesh* BoMesh::mesh() const
{
 return d->mMesh;
}

QString BoMesh::textureName(Lib3dsMesh* mesh, Lib3dsFile* file)
{
 if (!mesh || mesh->faces == 0) {
	return QString::null;
 }
 if (mesh->texels == 0) {
	return QString::null;
 }
 Lib3dsMaterial* mat = BoMesh::material(mesh, file);
 if (!mat) {
	return QString::null;
 }
 if (BoMesh::isTeamColor(mesh)) {
	// teamcolor objects are not textured.
	return QString::null;
 }

 // this is the texture map of the object.
 // t->name is the (file-)name and in
 // mesh->texelL you can find the texture
 // coordinates for glTexCoord*()
 // note that mesh->texels can be 0 - then the
 // mesh doesn't have any texture. otherwise it
 // must be equal to mesh->points
 Lib3dsTextureMap* t = &mat->texture1_map;

 // AB: note that we use BosonModel::cleanTextureName() for the final name. here
 // we return the name from the 3ds file only.
 return QString(t->name);
}

Lib3dsMaterial* BoMesh::material(Lib3dsMesh* mesh, Lib3dsFile* file)
{
if (!mesh || mesh->faces == 0) {
	return 0;
 }
 // AB: all faces in this mesh must use the same material!
 Lib3dsFace* f = &mesh->faceL[0];
 Lib3dsMaterial* mat = 0;
 if (f->material[0]) {
	mat = lib3ds_file_material_by_name(file, f->material);
 }
 return mat;
}

bool BoMesh::isTeamColor(const Lib3dsMesh* mesh)
{
 if (!mesh) {
	BO_NULL_ERROR(mesh);
	return false;
 }
 if (QString::fromLatin1(mesh->name).find("teamcolor", 0, false) == 0) {
	return true;
 }
 return false;
}

void BoMesh::setMaterial(Lib3dsMaterial* mat)
{
 d->mMaterial = mat;
}

void BoMesh::setTextured(bool tex)
{
 d->mTextured = tex;
}

void BoMesh::setTextureObject(GLuint tex)
{
 d->mTexture = tex;
}

void BoMesh::loadVertices()
{
 BO_CHECK_NULL_RET(mesh());
 if (mesh()->faces == 0) {
	return;
 }
 if (d->mVertices) {
	boWarning() << k_funcinfo << "vertices already loaded" << endl;
	delete[] d->mVertices;
 }
 d->mVertices = new float[mesh()->points * 3];
 Lib3dsMatrix invMeshMatrix;
 lib3ds_matrix_copy(invMeshMatrix, mesh()->matrix);
 lib3ds_matrix_inv(invMeshMatrix);
 BoMatrix matrix(&invMeshMatrix[0][0]);

 BoVector3 vector;
 BoVector3 v;
 for (unsigned int i = 0; i < mesh()->points; i++) {
	vector.set(mesh()->pointL[i].pos);
	matrix.transform(&v, &vector);
	d->mVertices[i * 3] = v[0];
	d->mVertices[i * 3 + 1] = v[1];
	d->mVertices[i * 3 + 2] = v[2];
 }
}

void BoMesh::loadTexels()
{
 BO_CHECK_NULL_RET(mesh());
 if (d->mTexels) {
	boWarning() << k_funcinfo << "texels already loaded" << endl;
	delete[] d->mTexels;
 }
 if (mesh()->texels == 0) {
	return;
 }
 if (!d->mMaterial) {
	return;
 }
 if (!d->mTextured) {
	return;
 }
 d->mTexels = new float[mesh()->texels * 2];


 BoMatrix texMatrix;
 // *ggg* this is a nice workaround.
 // it's hard to do this with a Lib3dsMatrix by several
 // reasons - so we do these calculations in OpenGL
 // (*NOT* in a display list - immediate mode) and then
 // get the texture matrix.
 // With this matrix we can easily calculate the actual
 // texture coordinates.
 // btw: this means that all calculations are done only
 // once (on startup) and therefore we'll have some
 // speedup

 // AB: this part isn't time critical anymore. you can
 // call even OpenGL commands without runtime slowdowns.
 // We calculate the actual texture map coordinates only
 // once and then use the final calculations in the
 // display list.
 glMatrixMode(GL_TEXTURE);
 glPushMatrix();
 glLoadIdentity(); // should already be there
 Lib3dsTextureMap* t = &d->mMaterial->texture1_map;
 if ((t->scale[0] || t->scale[1]) && (t->scale[0] != 1.0 || t->scale[1] != 1.0)) {
	// 3ds does these things pretty unhandy. it doesn't
	// scale as opengl does, but rather emulates scaling the
	// texture itself (i.e. when the texture is centered on
	// an object it will still be centered after scaling).
	// so we need to translate them before scaling.
	glTranslatef((1.0 - t->scale[0]) / 2, (1.0 - t->scale[1]) / 2, 0.0);
	glScalef(t->scale[0], t->scale[1], 1.0);
 }
 if (t->rotation != 0.0) {
	glRotatef(-t->rotation, 0.0, 0.0, 1.0);
 }
 glTranslatef(-t->offset[0], -t->offset[1], 0.0);
 glTranslatef(mesh()->map_data.pos[0], mesh()->map_data.pos[1], mesh()->map_data.pos[2]);
 float scale = mesh()->map_data.scale;
 if (scale != 0.0 && scale != 1.0) {
	// doesn't seem to be used in our models
	glScalef(scale, scale, 1.0);
 }
 texMatrix.loadMatrix(GL_TEXTURE_MATRIX);
 if (texMatrix.isNull()) {
	boWarning(100) << k_funcinfo << "Invalid texture matrix was generated!" << endl;
	texMatrix.loadIdentity();
 }
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);


 // now we have the final texture matrix in texMatrix.
 // all texel coordinates have to be transformed using this matrix.
 BoVector3 a;
 BoVector3 b;
 for (unsigned int i = 0; i < mesh()->texels; i++) {
	a.set(mesh()->texelL[i][0], mesh()->texelL[i][1], 0.0);
	texMatrix.transform(&b, &a);
	d->mTexels[i * 2] = b[0];
	d->mTexels[i * 2 + 1] = b[1];
 }
}

void BoMesh::renderMesh()
{
 // TODO: count number of nodes in this mesh and compare to mesh->faces
 BO_CHECK_NULL_RET(mesh());
 BO_CHECK_NULL_RET(d->mVertices);
 BoNode* node = faces();
 if (!node) {
	boError() << k_funcinfo << "NULL node" << endl;
	return;
 }
 if (type() == GL_TRIANGLE_STRIP && false) {
	boDebug() << "painting _STRIP" << endl;
	if (!node->next()) {
		boError() << k_funcinfo << "less than 2 faces in mesh " << mesh()->name << "! this is not supported" << endl;
		return;
	}
	int firstPoint;
	int secondPoint;
	int thirdPoint;
	node->decodeRelevantPoint(&firstPoint, &secondPoint, &thirdPoint);

	renderPoint(node->face()->points[firstPoint]);
	renderPoint(node->face()->points[secondPoint]);
	renderPoint(node->face()->points[thirdPoint]);

	// we skip the entire first face. all points have been rendered above.
	node = node->next();
	for (; node; node = node->next()) {
		int point = node->relevantPoint();
		if (point < 0 || point > 2) {
			boError( )<< k_funcinfo "oops - invalid point " << point << endl;
			continue;
		}
		renderPoint(node->face()->points[point]);
	}
 } else {
	for (; node; node = node->next()) {
		for (int i = 0; i < 3; i++) {
			renderPoint(node->face()->points[i]);
		}
	}
 }
}

void BoMesh::renderPoint(int point)
{
 if (point < 0 || (unsigned int)point >= mesh()->points) {
	boError() << k_funcinfo << "invalid point " << point << endl;
	return;
 }

 if (d->mTexels) {
	 // basiclly this is a
	 // glTexCoord2f(d->mTexels[point*2], d->mTexels[point*2+1])
	glTexCoord2fv(d->mTexels + 2 * point);
 }
 glVertex3fv(d->mVertices + 3 * point);
}


