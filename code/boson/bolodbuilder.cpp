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

#include "bolodbuilder.h"

#include "bomesh.h"
#include "bodebug.h"
#include "bo3dtools.h"

#include <qvaluelist.h>

BoLODBuilder::BoLODBuilder(const BoMesh* mesh, const BoMeshLOD* fullDetail)
{
 mMesh = mesh;
 mFullDetail = fullDetail;
}

BoLODBuilder::~BoLODBuilder()
{
}

BoVector3 BoLODBuilder::vertex(unsigned int index) const
{
 return mMesh->vertex(index);
}

BoVector3 BoLODBuilder::normal(unsigned int _face, unsigned int vertex) const
{
 const BoFace* f = face(_face);
 if (!f) {
	boError() << k_funcinfo << "NULL face " << _face << endl;
	return BoVector3();
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
 QValueList<BoFace> faceList;
 if (lod == 0) {
	boWarning(100) << k_funcinfo << "lod==0 doesn't make any sense here!" << endl;
	for (unsigned int i = 0; i < facesCount(); i++) {
		faceList.append(*face(i));
	}
	return faceList;
 }

#if 1
 boWarning() << k_funcinfo << "not yet implemented: LOD " << lod << endl;
 faceList = generateLOD(0);
#else
// TODO
#endif

 return faceList;
}

