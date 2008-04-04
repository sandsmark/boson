/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BODEFAULTGROUNDRENDERER_H
#define BODEFAULTGROUNDRENDERER_H

#include "bogroundrendererbase.h"

#include <qmemarray.h>
#include <qvaluevector.h>

class PlayerIO;
class QString;

class BosonMap;
class BoMatrix;

class QRect;

class BoDefaultGroundRenderer : public BoGroundRendererBase
{
	Q_OBJECT
public:
	BoDefaultGroundRenderer();
	virtual ~BoDefaultGroundRenderer();

	virtual bool initGroundRenderer();

	virtual void cellFogChanged(int x1, int y1, int x2, int y2);
	virtual void cellTextureChanged(int x1, int y1, int x2, int y2);
	virtual void cellHeightChanged(int x1, int y1, int x2, int y2);

	virtual bool usable() const;

protected:
	virtual void renderVisibleCells(int* cells, unsigned int cellsCount, const BosonMap* map, RenderFlags flags);
	virtual void generateCellList(const BosonMap* map);

	virtual void updateMapCache(const BosonMap* map);

	void clearVBOs();
	void updateVertexVBO();
	void updateColorVBO();

private:
	void renderCellColors(int* cells, int count, const BosonMap* map);

	void calculateIndices(int* renderCells, unsigned int cellsCount, const BosonMap* map);

private:
	const BosonMap* mCurrentMap;
	unsigned int mVBOVertex;
	unsigned int mVBONormal;
	unsigned int mVBOColor;

	unsigned int* mIndicesArray;
	unsigned int mIndicesArraySize;
	unsigned int mIndicesCount;
	bool mIndicesDirty;

	QValueList<int> mIndicesCountList; // used by strips only atm

	QValueVector< QMemArray<unsigned int>* > mTextureIndices;
};

#endif

