/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bolodbuilder.h"

#include "bomesh.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "bosonprofiling.h"

#include <qvaluelist.h>
#include <qptrvector.h>
#include <qvaluevector.h>
#include <qptrlist.h>


// Cost when collapsing from border to interior (very bad)
#define COLLAPSE_BORDER_TO_INTERIOR_COST 2.0f
// This will be added to curvature when collapsing along edge
#define COLLAPSE_ALONG_EDGE_PENALTY 0.75f
// Max cost when collapsing along edge
#define COLLAPSE_ALONG_EDGE_MULIPLIER 1.0f


BoLODBuilder::BoLODBuilder(const BoMesh* mesh, const BoMeshLOD* fullDetail)
{
 mMesh = mesh;
 mFullDetail = fullDetail;
 mThisVolume = 0.0f;
 mThisMaxSurface = 0.0f;
 mModelVolume = 0.0f;
 mModelMaxSurface = 0.0f;
 mLargestMeshVolume = 0.0f;
 mLargestMeshSurface = 0.0f;

 boDebug(120) << k_funcinfo << "mesh has " << pointCount() << " vertices in " << facesCount() <<
		" faces" << endl;

 BosonProfiler profiler(BosonProfiling::BuildLOD);

 buildLOD();

 long int elapsed = profiler.stop();
 boDebug(120) << k_funcinfo << "building lod took " << elapsed << "us" << endl;
}

BoLODBuilder::~BoLODBuilder()
{
}

BoVector3Float BoLODBuilder::vertex(unsigned int index) const
{
 return mMesh->vertex(index);
}

BoVector3Float BoLODBuilder::normal(unsigned int _face, unsigned int vertex) const
{
 const BoFace* f = face(_face);
 if (!f) {
	boError(120) << k_funcinfo << "NULL face " << _face << endl;
	return BoVector3Float();
 }
 return f->normal(vertex);
}

unsigned int BoLODBuilder::facesCount() const
{
 return mFullDetail->facesCount();
}

unsigned int BoLODBuilder::pointCount() const
{
 return mMesh->points();
}

const BoFace* BoLODBuilder::face(unsigned int f) const
{
 return mFullDetail->face(f);
}

QValueList<BoFace> BoLODBuilder::generateLOD(unsigned int lod)
{
 boDebug(121) << k_funcinfo << lod << endl;

 if (lod >= 5) {
	boWarning(120) << k_funcinfo << "Only 5 LOD levels are supported!" << endl;
	lod = 4;
 }

 // factor defines how many vertices (of original number) will be in the lod
 // Each LOD has 2/3 vertices of the last one
// const float percents[] = { 1.000, 0.666, 0.444, 0.296, 0.197 };
 const float percents[] = { 1.000, 0.750, 0.500, 0.300, 0.150 };
// const float factors[] = { 1.000, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1 };
 // maxcost defines maximum cost for vertices removed in the lod. No vertices
 //  with higher cost will be removed
 //  The higher cost is, the more model will change
 const float maxcosts[] = { 0.000, 0.200, 0.500, 0.800, 0.950 };
 //const float maxcosts[] = { 0.00, 0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.00,
 //   1.10, 1.20, 1.30, 1.40, 1.50, 1.75, 2.00 };
// const float maxcosts[] = { 0.000, 0.400, 0.600, 0.800, 0.900 };

 boDebug(120) << k_funcinfo << "factor is " << percents[lod] <<
		"; target is " << (int)(pointCount() * percents[lod]) << " of " << pointCount() << " vertices" <<
		"; max cost is " << maxcosts[lod] << endl;
 BosonProfiler profiler(BosonProfiling::GenerateLOD);
 QValueList<BoFace> faceList;
 if (lod == 0) {
	boWarning(100) << k_funcinfo << "lod==0 doesn't make any sense here!" << endl;
	for (unsigned int i = 0; i < facesCount(); i++) {
		faceList.append(*face(i));
	}
	return faceList;
 }

 // These define when mesh will be completely rejected from a lod, i.e. not
 //  drawn at all. Note that mesh will be rejected only if _all_ of test below
 //  are succesful for a mesh.
 // Mesh will be considered for rejection, if for lod x, it's volume is less
 //  than minvolumeofmodel[x]  of model's volume
 const float minvolumeofmodel[] = { 0.0, 0.0, 0.05, 0.15, 0.3 };
 // Mesh will be considered for rejection, if for lod x, it's volume is less
 //  than minvolumeoflargestmesh[x]  of largest mesh's volume
 const float minvolumeoflargestmesh[] = { 0.0, 0.0, 0.2, 0.35, 0.5 };
 // Mesh will be considered for rejection, if for lod x, it's surface area is
 //  less than minsurfaceofmodel[x]  of model's surface area
 const float minsurfaceofmodel[] = { 0.0, 0.0, 0.1, 0.25, 0.4 };
 // Mesh will be considered for rejection, if for lod x, it's surface area is
 //  less than minsurfaceoflargestmesh[x]  of largest mesh's surface area
 const float minsurfaceoflargestmesh[] = { 0.0, 0.0, 0.25, 0.5, 0.65 };

 bool reject = false;
 // Volume of the mesh is the most important criteria when checking if mesh
 //  should be rejected.
 if (mThisVolume < mModelVolume * minvolumeofmodel[lod]) {
	if (mThisVolume < mLargestMeshVolume * minvolumeoflargestmesh[lod]) {
		// Volume of mesh is small enough for the mesh to be rejected.
		// But we must also check for mesh's surface area - this makes sure meshes
		//  with small volume but big surface area (e.g. big rectangles, used for
		//  ground in some models), which are very visible because of their surface,
		//  won't be rejected.
		if (mThisMaxSurface < mModelMaxSurface * minsurfaceofmodel[lod]) {
			if (mThisMaxSurface < mLargestMeshSurface * minsurfaceoflargestmesh[lod]) {
				// This mesh has too small volume and surface area for this lod. Reject
				//  it.
				reject = true;
			}
		}
	}
 }


 if (!reject) {
	faceList = getLOD(percents[lod], maxcosts[lod]);
 }

 long int elapsed = profiler.stop();
 boDebug(120) << k_funcinfo << "lod: " << lod << "; took " << elapsed << "us" << endl;

 return faceList;
}

/**
 * Level Of Detail code is based on an article written by Stan Melax
 * <melax@cs.ualberta.ca> which is available at
 * http://www.melax.com/polychop/gdmag.pdf
 **/

class BoLODFace;
class BoLODVertex;

class BoLODFace
{
public:
	BoLODVertex* vertex[3];
	BoVector3Float normal;
	unsigned long int smoothGroup;

	BoLODFace(BoLODVertex* v0, BoLODVertex* v1, BoLODVertex* v2, unsigned long int smoothgroup = 0);
	~BoLODFace();

	void computeNormal();
	void replaceVertex(BoLODVertex* vold, BoLODVertex* vnew);
	bool hasVertex(BoLODVertex* v);
};


class BoLODVertex
{
public:
	BoLODVertex(BoVector3Float v, unsigned int id);
	~BoLODVertex();

	void removeIfNonNeighbor(BoLODVertex* n);
	bool isBorder();
	bool isManifoldEdgeWith(BoLODVertex* n);

	BoVector3Float position;
	unsigned int id;
	// adjacent vertices
	BoLODVector<BoLODVertex> neighbor;
	// adjacent triangles
	BoLODVector<BoLODFace> face;
	// cached cost of collapsing edge
	float cost;
	// candidate vertex for collapse
	BoLODVertex* collapse;

	int heapspot;
};



BoLODFace::BoLODFace(BoLODVertex* v0, BoLODVertex* v1, BoLODVertex* v2, unsigned long int sg)
{
 if (v0 == v1 || v1 == v2 || v2 == v0) {
	boError(120) << k_funcinfo << "Vertices (" << v0->id << ", " << v1->id << ", " << v2->id << ") must be unique!" << endl;
	// return here?
 }
 vertex[0] = v0;
 vertex[1] = v1;
 vertex[2] = v2;
 smoothGroup = sg;
 computeNormal();
 for (int i = 0; i < 3; i++) {
	vertex[i]->face.appendItem(this);
	for (int j = 0; j < 3; j++) {
		if (i != j && !vertex[i]->neighbor.containsRef(vertex[j])) {
			vertex[i]->neighbor.appendItem(vertex[j]);
		}
	}
 }
}

BoLODFace::~BoLODFace()
{
 int i;
 for (i = 0; i < 3; i++) {
	if (vertex[i]) {
		vertex[i]->face.removeItem(this);
	}
 }
 for (i = 0; i < 3; i++) {
	int i2 = (i + 1) % 3;
	if (!vertex[i] || !vertex[i2]) {
		continue;
	}
	vertex[i]->removeIfNonNeighbor(vertex[i2]);
	vertex[i2]->removeIfNonNeighbor(vertex[i]);
 }
}

bool BoLODFace::hasVertex(BoLODVertex* v)
{
 return (v == vertex[0] || v == vertex[1] || v == vertex[2]);
}

void BoLODFace::computeNormal()
{
 BoVector3Float v0 = vertex[0]->position;
 BoVector3Float v1 = vertex[1]->position;
 BoVector3Float v2 = vertex[2]->position;
 normal = BoVector3Float::crossProduct(v1 - v0, v2 - v1);
 normal.normalize();
}

void BoLODFace::replaceVertex(BoLODVertex* vold, BoLODVertex* vnew)
{
 if (!vold || !vnew) {
	boError(120) << k_funcinfo << "vold (" << vold << ") or vnew (" << vnew << ") is NULL!" << endl;
	return;
 }
 if (vold != vertex[0] && vold != vertex[1] && vold != vertex[2]) {
	boError(120) << k_funcinfo << "vold must be one of current vertices!" << endl;
	return;
 }
 if (vnew == vertex[0] || vnew == vertex[1] || vnew == vertex[2]) {
	boError(120) << k_funcinfo << "vnew must not be one of current vertices!" << endl;
	return;
 }
 if (vold == vertex[0]) {
	vertex[0] = vnew;
 } else if (vold == vertex[1]) {
	vertex[1] = vnew;
 } else {
	if (vold != vertex[2]) {
		boError(120) << k_funcinfo << "No such vertex: " << vold->id << endl;
		return;
	}
	vertex[2] = vnew;
 }
 int i;
 vold->face.removeItem(this);
 if (vnew->face.containsRef(this)) {
	boError(120) << "vnew must not have this face!" << endl;
	return;
 }
 vnew->face.appendItem(this);
 for (i = 0; i < 3; i++) {
	vold->removeIfNonNeighbor(vertex[i]);
	vertex[i]->removeIfNonNeighbor(vold);
 }
 for (i = 0; i < 3; i++) {
	if (vertex[i]->face.containsRef(this) != 1) {
		boError(120) << k_funcinfo << "vertex " << i << " doesn't have this face!" << endl;
		return;
	}
	for (int j = 0; j < 3; j++) {
		if (i != j && !vertex[i]->neighbor.containsRef(vertex[j])) {
			vertex[i]->neighbor.appendItem(vertex[j]);
		}
	}
 }
 computeNormal();
}



BoLODVertex::BoLODVertex(BoVector3Float v, unsigned int _id)
{
 position = v;
 id = _id;
}

BoLODVertex::~BoLODVertex()
{
 if (face.count() != 0) {
	boError(120) << k_funcinfo << "vertex " << id << " still has " << face.count() << " faces!" << endl;
 }
 while (neighbor.count()) {
	neighbor[0]->neighbor.removeItem(this);
	neighbor.removeItem((unsigned int)0);
 }
}

bool BoLODVertex::isBorder()
{
 for (unsigned int i = 0; i < neighbor.count(); i++) {
	int count = 0;
	for (unsigned int j = 0; j < face.count(); j++) {
		if (face[j]->hasVertex(neighbor[i])) {
			count++;
		}
	}
	if (count <= 0) {
		boError(120) << k_funcinfo << "count <= 0" << endl;
	}
	if (count == 1) {
		return true;
	}
 }
 return false;
}

void BoLODVertex::removeIfNonNeighbor(BoLODVertex* n)
{
 // removes n from neighbor list if n isn't a neighbor.
 if (!neighbor.containsRef(n)) {
	return;
 }
 for (unsigned int i = 0; i < face.count(); i++) {
	if (face[i]->hasVertex(n)) {
		return;
	}
 }
 neighbor.removeItem(n);
}

bool BoLODVertex::isManifoldEdgeWith(BoLODVertex* n)
{
 // If there's only 1 edge involving both these vertices, then it's a manifold
 //  edge
 int sidescount = 0;
 for (unsigned int i = 0; i < face.count(); i++) {
	if (face[i]->hasVertex(n)) {
		sidescount++;
	}
 }
 return (sidescount == 1);
}


class BoLODHeap : public QPtrList<BoLODVertex>
{
public:
	void addVertex(BoLODVertex* v);
	void sortUp(int k);
	void sortDown(int k);
	float heapVal(int i);
	BoLODVertex* heapPop();
protected:
	void swap(int v1, int v2);
};

void BoLODHeap::addVertex(BoLODVertex* v)
{
 int k = count();
 append(v);
 v->heapspot = k;
 sortUp(k);
}

void BoLODHeap::sortUp(int k)
{
 int k2;
 while (heapVal(k) < heapVal((k2 = (k - 1) / 2))) {
	swap(k, k2);
	at(k)->heapspot = k;
	at(k2)->heapspot = k2;
	k = k2;
 }
}

void BoLODHeap::sortDown(int k)
{
 int k2;
 while (heapVal(k) > heapVal(k2 = (k + 1) * 2) || heapVal(k) > heapVal(k2 - 1)) {
	k2 = (heapVal(k2) < heapVal(k2 - 1)) ? k2 : k2 - 1;
	swap(k, k2);
	at(k)->heapspot = k;
	if (at(k2)) {
		at(k2)->heapspot = k2;
	}
	k = k2;
 }
}

float BoLODHeap::heapVal(int i)
{
 if (i >= (int)count() || at(i) == 0) {
	return 9999999999999.9f;
 }
 return at(i)->cost;
}

BoLODVertex* BoLODHeap::heapPop()
{
 BoLODVertex* rv = take(0);
 if (!rv) {
	boError(120) << k_funcinfo << "First element of heap is NULL!" << endl;
	return 0;
 }
 rv->heapspot = -1;
 sortDown(0);
 return rv;
}

void BoLODHeap::swap(int v1, int v2)
{
 int tmp1 = v1;

 if (v1 < v2) {
	v1 = v2;
	v2 = tmp1;
 }
 BoLODVertex* tmp2 = take(v1);
 insert(v1 - 1, take(v2));
 insert(v2, tmp2);
}



void BoLODBuilder::buildLOD()
{
 //boDebug(120) << k_funcinfo << "mesh has " << pointCount() << " vertices in " << facesCount() <<
//		" faces" << endl;

 // Compute progressive mesh
 mMap.resize(pointCount());
 mOrder.resize(pointCount());

 mFaces.resize(facesCount());
 mVertices.resize(pointCount());
 mCost.resize(pointCount());
 mHeap = new BoLODHeap;


 //boDebug(120) << k_funcinfo << "adding vertices" << endl;
 // Make BoLODVertex objects for all vertices
 for (unsigned int i = 0; i < pointCount(); i++) {
	BoVector3Float p(vertex(i));
	BoLODVertex* v = new BoLODVertex(p, i);
	mVertices.insert(i, v);
	//boDebug(120) << k_funcinfo << "added vertex " << i << " at " << v << " with pos (" <<
	//		p.x() << ", " << p.y() << ", " << p.z() << ")" << endl;
 }
 //boDebug(120) << k_funcinfo << "vertices added" << endl;

 //boDebug(120) << k_funcinfo << "adding faces" << endl;
 // And make BoLODFace objects for all faces
 for (unsigned int i = 0; i < facesCount(); i++) {
	//boDebug(120) << k_funcinfo << "adding face " << i << " with vertices: " <<
	//		mVertices[face(i)->pointIndex()[0]]->id << ", " <<
	//		mVertices[face(i)->pointIndex()[1]]->id << " and " << mVertices[face(i)->pointIndex()[2]]->id << endl;
	BoLODFace* f = new BoLODFace(mVertices[face(i)->pointIndex()[0]],
			mVertices[face(i)->pointIndex()[1]], mVertices[face(i)->pointIndex()[2]], face(i)->smoothGroup());
	mFaces.insert(i, f);
 }
 //boDebug(120) << k_funcinfo << "faces added" << endl;

 // For all the edges, compute the difference it would make
 // to the model if it was collapsed.  The least of these
 // per vertex is cached in each vertex object.
 //boDebug(120) << k_funcinfo << "calculating costs" << endl;
 for (unsigned int i = 0; i < mVertices.count(); i++) {
	computeEdgeCostAtVertex(mVertices[i]);
	mHeap->addVertex(mVertices[i]);
 }
 //boDebug(120) << k_funcinfo << "cost calculated" << endl;

 //boDebug(120) << k_funcinfo << "generating lists" << endl;
 int count = 1;
 while (mVertices.count()) {
	// get the next vertex to collapse
	BoLODVertex* mn = mHeap->heapPop();
	boDebug(121) << k_funcinfo << "Collapsing " << count++ << ". vertex with id " << mn->id << " and cost "
			<< mn->cost << " to " << ((mn->collapse) ? (int)(mn->collapse->id) : -1) << endl;
	// Collapse this edge
	mOrder[mVertices.count() - 1] = mn->id;
	mMap[mn->id] = (mn->collapse) ? (int)(mn->collapse->id) : -1;
	mCost[mn->id] = mn->cost;
	collapse(mn, mn->collapse);
 }
 //boDebug(120) << k_funcinfo << "lists generated" << endl;
 if (mFaces.count() != 0) {
	boError(120) << k_funcinfo << mFaces.count() << " faces still in list!" << endl;
	mFaces.deleteAllItems();
 }
 if (mVertices.count() != 0) {
	boError(120) << k_funcinfo << mVertices.count() << " vertices still in list!" << endl;
	mVertices.deleteAllItems();
 }

 delete mHeap;
}

QValueList<BoFace> BoLODBuilder::getLOD(float percent, float maxcost)
{
 unsigned int target = (unsigned int)(percent * pointCount());
 //boDebug(120) << k_funcinfo << "factor is " << factor << "; target is " << target << " of " <<
//		pointCount() << " vertices" << endl;

 if (mOrder.count() != pointCount()) {
	boError(120) << k_funcinfo << "mOrder has only " << mOrder.count() << " of " << pointCount() << " elements!" << endl;
 }
 if (mMap.count() != pointCount()) {
	boError(120) << k_funcinfo << "mMap has only " << mMap.count() << " of " << pointCount() << " elements!" << endl;
 }

 unsigned int i;
 BoLODVector<BoLODVertex> vclist(pointCount());
 mVertices.resize(pointCount());
 mFaces.resize(facesCount());
 for (i = 0; i < pointCount(); i++) {
	BoVector3Float p(vertex(i));
	BoLODVertex* v = new BoLODVertex(p, i);
	mVertices.insert(i, v);
 }

	for (i = 0; i < pointCount(); i++) {
		mVertices[i]->collapse = (mMap[i] == -1) ? 0 : mVertices[mMap[i]];
		vclist.insert(i, mVertices[mOrder[i]]);
	}

 for (i = 0; i < facesCount(); i++) {
	BoLODFace* f = new BoLODFace(mVertices[face(i)->pointIndex()[0]],
			mVertices[face(i)->pointIndex()[1]], mVertices[face(i)->pointIndex()[2]], face(i)->smoothGroup());
	mFaces.insert(i, f);
 }

 int count = 1;
 // Main loop: collapse vertices as long as we have at most target vertices. At
 //  the same time, do _not_ remove vertices with cost higher than maxcost
 while (vclist.count() > target) {
	// Hard stop if there's only 6 faces left
	if (mFaces.count() <= 6) {
		boDebug(120) << k_funcinfo << "Only " << mFaces.count() << " faces left. Stop." << endl;
		break;
	}

	// get the next vertex to collapse
	BoLODVertex* mn = vclist.take(vclist.count() - 1);

	// Check if enough vertices have been removed
	if (mCost[mn->id] > maxcost) {
		// Vertex has too high cost. Don't remove it
		boDebug(120) << k_funcinfo << count << ". vertex with id " << mn->id <<
				" has cost (" << mCost[mn->id] << ") bigger than limit (" << maxcost << "). Break." << endl;
		break;
	}

	// Collapse this edge
	boDebug(121) << "Removing " << count++ << ". vertex with id " << mn->id << " and cost " << mCost[mn->id] << endl;
	collapse(mn, mn->collapse, false);
 }


 QValueList<BoFace> faceList;
 for (unsigned int i = 0; i < mFaces.count(); i++) {
	BoFace f;
	int points[3];
	points[0] = mFaces[i]->vertex[0]->id;
	points[1] = mFaces[i]->vertex[1]->id;
	points[2] = mFaces[i]->vertex[2]->id;
	f.setPointIndex(points);
	f.setAllNormals(mFaces[i]->normal);
	f.setSmoothGroup(mFaces[i]->smoothGroup);
	faceList.append(f);
 }

 mFaces.deleteAllItems();
 mVertices.deleteAllItems();

 return faceList;
}

void BoLODBuilder::computeEdgeCostAtVertex(BoLODVertex* v)
{
 // compute the edge collapse cost for all edges that start
 // from vertex v.  Since we are only interested in reducing
 // the object by selecting the min cost edge at each step, we
 // only cache the cost of the least cost edge at this vertex
 // (in member variable collapse) as well as the value of the
 // cost (in member variable cost).
 if (v->neighbor.count() == 0) {
	// v doesn't have neighbors so it costs nothing to collapse
	v->collapse = 0;
	v->cost = -0.01f;
	//boDebug(120) << k_funcinfo << "Vertex " << v->id << " doesn't have neighbors (cost is -0.01)" << endl;
	return;
 }
 v->cost = 1000000;
 v->collapse = 0;
 // search all neighboring edges for "least cost" edge
 float c;
 for (unsigned int i = 0; i < v->neighbor.count(); i++) {
	c = computeEdgeCollapseCost(v, v->neighbor[i]);
	if ((!v->collapse) || c < v->cost) {
		v->collapse = v->neighbor[i];  // candidate for edge collapse
		v->cost = c;             // cost of the collapse
	}
 }
 //boDebug(120) << k_funcinfo << "Collapse cost for vertex " << v->id << " (to " << v->collapse->id <<
//		") is " << v->cost << (v->isBorder() ? " (border)" : "") << endl;
}

float BoLODBuilder::computeEdgeCollapseCost(BoLODVertex* u, BoLODVertex* v)
{
 // if we collapse edge uv by moving u to v then how
 // much different will the model change, i.e. how much "error".
 // The method of determining cost was designed in order
 // to exploit small and coplanar regions for
 // effective polygon reduction.

 float curvature = 0.001f;

 // find the "sides" triangles that are on the edge uv
 BoLODVector<BoLODFace> sides;
 for (unsigned int i = 0; i < u->face.count(); i++) {
	if (u->face[i]->hasVertex(v)){
		sides.appendItem(u->face[i]);
	}
 }

 // Check how we're collapsing
 if (u->isBorder()) {
	// u is border vertex
	if (sides.count() > 1) {
		// Collapsing from border to interior -> very bad
		curvature = COLLAPSE_BORDER_TO_INTERIOR_COST;
	} else {
		// Collapsing along border -> not that bad, but not too good either
		// Code for this is taken from Ogre engine - http://ogre.sourceforge.net
		BoVector3Float collapseEdge, otherBorderEdge;
		BoVector3Float edgeVector = v->position - u->position;
		edgeVector.normalize();
		collapseEdge = edgeVector;
		float kinkiness, maxkinkiness;
		maxkinkiness = 0.0f;
		for (unsigned int i = 0; i < u->neighbor.count(); i++) {
			if ((u->neighbor[i] != v) && (u->neighbor[i]->isManifoldEdgeWith(u))) {
				otherBorderEdge = u->position - u->neighbor[i]->position;
				otherBorderEdge.normalize();
				// This time, the closer dot is to -1, the better, because that means
				//  the edges are opposite to each other
				kinkiness = BoVector3Float::dotProduct(collapseEdge, otherBorderEdge);
				maxkinkiness = QMAX(maxkinkiness, (1.002f + kinkiness) * 0.5f);
			}
		}
		// curvature will be between COLLAPSE_ALONG_EDGE_PENALTY and
		//  COLLAPSE_ALONG_EDGE_PENALTY + COLLAPSE_ALONG_EDGE_MULIPLIER
		curvature = COLLAPSE_ALONG_EDGE_PENALTY + (maxkinkiness * COLLAPSE_ALONG_EDGE_MULIPLIER);
	}
 } else {
	// use the triangle facing most away from the sides
	// to determine our curvature term
	for (unsigned int i = 0; i < u->face.count(); i++) {
		float mincurv = 1.0f; // curve for face i and closer side to it
		for (unsigned int j = 0; j < sides.count(); j++) {
			// use dot product of face normals.
			float dotprod = BoVector3Float::dotProduct(u->face[i]->normal, sides[j]->normal);
			mincurv = QMIN(mincurv, (1.002f - dotprod) * 0.5f);
		}
		curvature = QMAX(curvature, mincurv);
	}
 }

 return curvature;
}

void BoLODBuilder::collapse(BoLODVertex* u, BoLODVertex* v, bool recompute)
{
 // Collapse the edge uv by moving vertex u onto v
 // Actually remove tris on uv, then update tris that
 // have u to have v, and then remove u.
 if (!v) {
	// u is a vertex all by itself so just delete it
	mVertices.removeItem(u);
	delete u;
	return;
 }

 BoLODVector<BoLODVertex> tmp;

 // make tmp a list of all the neighbors of u
 for (unsigned int i = 0; i < u->neighbor.count(); i++) {
	tmp.appendItem(u->neighbor[i]);
 }
 BoLODVector<BoLODFace> sides;
 for (unsigned int i = 0; i < u->face.count(); i++) {
	if (u->face[i]->hasVertex(v)){
		sides.appendItem(u->face[i]);
	}
 }

 // delete triangles on edge uv:
 for (int i = u->face.count() - 1; i >= 0; i--) {
	if (u->face[i]->hasVertex(v)) {
		BoLODFace* f = u->face[i];
		mFaces.removeItem(f);
		delete(f);
	}
 }

 // update remaining triangles to have v instead of u
 for (int i = u->face.count() - 1; i >= 0; i--) {
	u->face[i]->replaceVertex(u, v);
 }
 mVertices.removeItem(u);
 delete u;

 // recompute the edge collapse costs for neighboring vertices
 if (recompute) {
	for (unsigned int i = 0; i < tmp.count(); i++) {
		computeEdgeCostAtVertex(tmp[i]);
		mHeap->sortUp(tmp[i]->heapspot);
		mHeap->sortDown(tmp[i]->heapspot);
	}
 }
}

void BoLODBuilder::setModelData(float thisVolume, float thisMaxSurface, float modelVolume, float modelMaxSurface, float largestMeshVolume, float largestMeshSurface)
{
 mThisVolume = thisVolume;
 mThisMaxSurface = thisMaxSurface;
 mModelVolume = modelVolume;
 mModelMaxSurface = modelMaxSurface;
 mLargestMeshVolume = largestMeshVolume;
 mLargestMeshSurface = largestMeshSurface;
}

