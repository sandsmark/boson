/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
class BoVector3;
class BoFace;
class BoLODVertex;
class BoLODFace;
class BoLODHeap;

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
		if (count() == size()) {
			resize(size() ? (size() * 2) : 8);
		}
		insert(count(), d);
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
		remove(i);
		if (i < count()) {
			insert(i, take(count()));
		}
	}

	/**
	 * Deletes all items in the vector and sets autoDelete to false
	 **/
	void deleteAllItems()
	{
		setAutoDelete(true);
		clear();
		setAutoDelete(false);
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

protected:
	/**
	 * @param index The index in the vertex pool. Must be 0 .. @ref
	 * pointCount
	 * * @return The vertex with index @p index in the vertex pool.
	 **/
	BoVector3 vertex(unsigned int index) const;

	/**
	 * Note: this takes a _face_ as paremeter, @ref vertex an index!
	 * @param vertex See @ref BoFace::normal.
	 **/
	BoVector3 normal(unsigned int face, unsigned int vertex) const;

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
	QValueList<BoFace> getLOD(float factor);

private:
	const BoMesh* mMesh;
	const BoMeshLOD* mFullDetail;

	BoLODHeap* mHeap;
	BoLODVector<BoLODFace> mFaces;
	BoLODVector<BoLODVertex> mVertices;
	QValueVector<int> mMap;
	QValueVector<int> mOrder;
	QValueVector<float> mCost;
};

#endif

