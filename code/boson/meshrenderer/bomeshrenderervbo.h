/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOMESHRENDERERVBO_H
#define BOMESHRENDERERVBO_H

#include "bomeshrenderervertexarray.h"

class BoMeshRendererVBO: public BoMeshRendererVertexArray
{
	Q_OBJECT
public:
	BoMeshRendererVBO();
	~BoMeshRendererVBO();

	virtual void setModel(BosonModel* model);

protected:
	virtual void render(const QColor* teamColor, BoMesh* mesh, BoMeshLOD* lod);

	virtual BoMeshRendererModelData* createModelData() const;
	virtual BoMeshRendererMeshData* createMeshData() const;
	virtual BoMeshRendererMeshLODData* createMeshLODData() const;

	virtual void initModelData(BosonModel* model);
	virtual void initMeshData(BoMesh* mesh, unsigned int meshIndex);
	virtual void initMeshLODData(BoMeshLOD* meshLOD, unsigned int meshIndex, unsigned int lod);

	virtual void deinitModelData(BosonModel* model);
	virtual void deinitMeshData(BoMesh* mesh);
	virtual void deinitMeshLODData(BoMeshLOD* meshLOD);

	bool useVBO() const;

};

#endif
