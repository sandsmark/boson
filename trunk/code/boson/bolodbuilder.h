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

template<class T> class QValueList;

/**
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

	static float angle(const BoFace& face1, const BoFace& face2);

	/**
	 * @return The face @p _face from the full-detailed LOD.
	 **/
	const BoFace* face(unsigned int _face) const;

private:
	const BoMesh* mMesh;
	const BoMeshLOD* mFullDetail;

};

#endif

