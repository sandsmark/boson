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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOMESHRENDERERIMMEDIATE_H
#define BOMESHRENDERERIMMEDIATE_H

#include "../bomeshrenderer.h"

class BoMeshRendererImmediate : public BoMeshRenderer
{
	Q_OBJECT
public:
	BoMeshRendererImmediate();
	~BoMeshRendererImmediate();

	virtual void setModel(BosonModel* model);

protected:
	virtual void initFrame();
	virtual void deinitFrame();
	virtual unsigned int render(const QColor& teamColor, BoMesh* mesh, RenderFlags flags);

#if 0
	// immediate mode doesn't need extra data, so we use the default
	// implementations
	virtual BoMeshRendererModelData* createModelData() const;
	virtual BoMeshRendererMeshData* createMeshData() const;
	virtual BoMeshRendererMeshLODData* createMeshLODData() const;
	virtual void initModelData(BosonModel* model);
	virtual void initMeshData(BoMesh* mesh, unsigned int meshIndex);
	virtual void initMeshLODData(BoMeshLOD* meshLOD, unsigned int meshIndex, unsigned int lod);
#endif

};

#endif
