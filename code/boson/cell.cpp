/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "defines.h"
#include "unitproperties.h"
#include "bodebug.h"

Cell::Cell()
	: mX(0),
	mY(0),
	mItems(BoItemList(2, false)),
	mAmountOfLand(0),
	mAmountOfWater(0),
	mRegion(0),
	mPassable(true)
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

void Cell::makeCell(unsigned char amountOfLand, unsigned char amountOfWater)
{
 if ((int)amountOfLand + (int)amountOfWater != 255) {
	boError() << k_funcinfo << "amountOfLand(==" << (int)amountOfLand
			<< ") + amountOfWater(==" << (int)amountOfWater
			<< ") != 255" << endl;
	amountOfLand = 0;
	amountOfWater = 255;
 }

 mAmountOfLand = amountOfLand;
 mAmountOfWater = amountOfWater;
}

bool Cell::canGo(const UnitProperties* prop) const
{ // probably a time critical function!
 if (!prop) {
	boError() << k_funcinfo << "NULL unit properties" << endl;
	return false;
 }
 if (mAmountOfLand >= 128) {
	return prop->canGoOnLand();
 } else {
	return prop->canGoOnWater();
 }
 return false;
}

int Cell::moveCost() const
{
 int cost = 0;
 return cost;
}

