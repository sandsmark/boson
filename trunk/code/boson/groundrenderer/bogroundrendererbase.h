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
#ifndef BOGROUNDRENDERERBASE_H
#define BOGROUNDRENDERERBASE_H

#include "../bogroundrenderer.h"

class Cell;
class PlayerIO;
class QString;
class BosonMap;
class BoMatrix;
class bofixed;
template<class T> class BoVector3;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;

class QRect;
class CellListBuilder;

class BoGroundRendererBase : public BoGroundRenderer
{
	Q_OBJECT
public:
	BoGroundRendererBase(bool useCellTree);
	virtual ~BoGroundRendererBase();

	/**
	 * Generate a list of cells that are (or may) be visible at the moment.
	 * @param map The map that contains the @ref Cell pointers. Use 0 to
	 * delete the current list of cells.
	 **/
	virtual void generateCellList(const BosonMap* map);

	virtual QString debugStringForPoint(const BoVector3Fixed& pos) const;

protected:
	float* mHeightMap2;

private:
	CellListBuilder* mCellListBuilder;
	const BosonMap* mMap;
};

#endif

