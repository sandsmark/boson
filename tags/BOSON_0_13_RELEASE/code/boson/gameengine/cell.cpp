/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
#include "cell.h"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "unitproperties.h"
#include "bodebug.h"

Cell::Cell()
	: mX(0),
	mY(0),
	mItems(BoItemList(2, false)),
	mIsWater(false)
{
}

Cell::~Cell()
{
}

void Cell::setPosition(int x, int y)
{
 mX = x;
 mY = y;
}

int Cell::moveCost() const
{
 int cost = 0;
 return cost;
}

