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
#ifndef BOMESHRENDERERVERTEXARRAY_H
#define BOMESHRENDERERVERTEXARRAY_H

#include "../bomeshrenderer.h"
#include "../bosonmodel.h"

class BoMeshRendererModelDataVA : public BoMeshRendererModelData
{
public:
	BoMeshRendererModelDataVA();
	~BoMeshRendererModelDataVA();

	float* mPoints;
	unsigned int mPointCount;

	// convenience for initialization
	unsigned int mMaxLODCount;
	unsigned int* mOffsets;
	unsigned int* mPointCounts;
};


class BoMeshRendererVertexArray: public BoMeshRenderer
{
	Q_OBJECT
public:
	BoMeshRendererVertexArray();
	~BoMeshRendererVertexArray();

	virtual void setModel(BosonModel*);

protected:
	virtual void initFrame();
	virtual void deinitFrame();
	virtual unsigned int render(const QColor* teamColor, BoMesh* mesh, BoMeshLOD* lod);

	virtual BoMeshRendererModelData* createModelData() const;
	virtual BoMeshRendererMeshData* createMeshData() const;
	virtual BoMeshRendererMeshLODData* createMeshLODData() const;

	virtual void initModelData(BosonModel* model);
	virtual void initMeshData(BoMesh* mesh, unsigned int meshIndex);
	virtual void initMeshLODData(BoMeshLOD* meshLOD, unsigned int meshIndex, unsigned int lod);

	unsigned int countModelPoints(const BosonModel* model) const;
	void fillModelPointsArray(BosonModel* model, float* array, unsigned int maxLODCount, unsigned int* offsets, unsigned int* pointCount);
};

#endif
