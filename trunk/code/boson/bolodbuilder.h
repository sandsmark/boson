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

#ifndef BOLODBUILDER_H
#define BOLODBUILDER_H

class BoMesh;
class BoMeshLOD;
class BoFace;
class BoLODVertex;
class BoLODFace;
class BoLODHeap;
template<class T> class BoVector3;
typedef BoVector3<float> BoVector3Float;

template<class T> class QValueList;

#include <qptrvector.h>
#include <qvaluevector.h>


/**
 * Improved QPtrVector
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
template<class type> class BoLODVector : public QPtrVector<type>
{
public:
	BoLODVector() : QPtrVector<type>() {};
	BoLODVector(unsigned int size) : QPtrVector<type>(size) {};

	/**
	 * Appends item d to the vector.
	 * If vector is too small, it will be resized
	 **/
	void appendItem(const type* d)
	{
		if (this->count() == this->size()) {
			resize(this->size() ? (this->size() * 2) : 8);
		}
		insert(this->count(), d);
	}

	/**
	 * Removes item d from the vector and moves last item to the slot previously
	 * occupied by d, so that there won't be any "holes" in vector.
	 **/
	inline void removeItem(const type* d)
	{
		removeItem(findRef(d));
	}

	void removeItem(unsigned int i)
	{
		this->remove(i);
		if (i < this->count()) {
			insert(i, take(this->count()));
		}
	}

	/**
	 * Deletes all items in the vector and sets autoDelete to false
	 **/
	void deleteAllItems()
	{
		this->setAutoDelete(true);
		this->clear();
		this->setAutoDelete(false);
	}
};

/**
 * Builds LOD (Level Of Detail) mesh from full-detail mesh.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>, Rivo Laks <rivolaks@hot.ee>
 **/
class BoLODBuilder
{
public:
	/**
	 * @param mesh The mesh that the LOD is generated for.
	 * @param fullDetail The full-detailed version of the mesh.
	 **/
	BoLODBuilder(const BoMesh* mesh, const BoMeshLOD* fullDetail);
	~BoLODBuilder();

	/**
	 * This generates LOD for the mesh (see contructor). The level of detail
	 * is @p lod, where 0 is the full detailed version (it makes no sense to
	 * use 0 here).
	 **/
	QValueList<BoFace> generateLOD(unsigned int lod);

	/**
	 * Set additional data about the model that this mesh is in.
	 *
	 * Note that these data make use of the mesh matrices, i.e. they use the
	 * final outlook of the model, whereas the rest of the LOD code is (atm)
	 * based on the coordinates without the matrices only.
	 *
	 * @param thisVolume The volume of this mesh (the one the LOD builder
	 * works on)
	 * @param thisMaxSurface The max surface of this mesh (the one the LOD builder
	 * works on)
	 * @param modelVolume The volume of the model. Usually for simplicity
	 * reasons the volume of the bounding box is used here only.
	 * @param modelMaxSurface The maximal surface (i.e. either w*h or
	 * w*depth or h*depth) of the model. Usually for simplicity reasons we
	 * use the max surface of the bounding box only.
	 * @param largesMeshVolume The volume of the largest mesh in the model
	 * @param largesMeshSurface The maximal surface of the meshes in the
	 * model
	 **/
	void setModelData(float thisVolume, float thisMaxSurface, float modelVolume, float modelMaxSurface, float largesMeshVolume, float largestMeshSurface);

protected:
	/**
	 * @param index The index in the vertex pool. Must be 0 .. @ref
	 * pointCount
	 * * @return The vertex with index @p index in the vertex pool.
	 **/
	BoVector3Float vertex(unsigned int index) const;

	/**
	 * Note: this takes a _face_ as paremeter, @ref vertex an index!
	 * @param vertex See @ref BoFace::normal.
	 **/
	BoVector3Float normal(unsigned int face, unsigned int vertex) const;

	unsigned int pointCount() const;
	unsigned int facesCount() const;

	/**
	 * @return The face @p _face from the full-detailed LOD.
	 **/
	const BoFace* face(unsigned int _face) const;

	void computeEdgeCostAtVertex(BoLODVertex* v);
	float computeEdgeCollapseCost(BoLODVertex* u, BoLODVertex* v);
	void collapse(BoLODVertex* u, BoLODVertex* v, bool recompute = true);

	void buildLOD();
	QValueList<BoFace> getLOD(float percent, float maxcost);

private:
	const BoMesh* mMesh;
	const BoMeshLOD* mFullDetail;

	BoLODHeap* mHeap;
	BoLODVector<BoLODFace> mFaces;
	BoLODVector<BoLODVertex> mVertices;
	QValueVector<int> mMap;
	QValueVector<int> mOrder;
	QValueVector<float> mCost;

	float mThisVolume;
	float mThisMaxSurface;
	float mModelVolume;
	float mModelMaxSurface;
	float mLargestMeshVolume;
	float mLargestMeshSurface;
};

#endif

