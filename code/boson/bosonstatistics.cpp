/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonstatistics.h"

BosonStatistics::BosonStatistics()
{
 mShots = 0;
 mMinedMinerals = 0;
 mMinedOil = 0;
 mRefinedMinerals = 0;
 mRefinedOil = 0;
 mProducedMobileUnits = 0;
 mProducedFacilities = 0;
 mDestroyedMobileUnits = 0;
 mDestroyedFacilities= 0;
}

BosonStatistics::~BosonStatistics()
{
}

unsigned long int BosonStatistics::shots() const
{
// FIXME: in a long game this value can be greater than the maximal unsigned long
// value! we need to take care of this! One solution might be to add a variable
// mBillionShots and store every new billion there. THen we must return a
// QString here!
 return mShots;
}

void BosonStatistics::increaseShots()
{
// FIXME: in a long game this value can be greater than the maximal unsigned long
// value! we need to take care of this! One solution might be to add a variable
// mBillionShots and store every new billion there. 
// e.g.: mShots++; if (mShots == 1000000) { mBillionShots++; mShots = 0; } 

 mShots++;
}

void BosonStatistics::increaseMinedMinerals(unsigned int increaseBy)
{
 mMinedMinerals += increaseBy;
}

void BosonStatistics::increaseMinedOil(unsigned int increaseBy)
{
 mMinedOil += increaseBy;
}

void BosonStatistics::increaseRefinedMinerals(unsigned int increaseBy)
{
 mRefinedMinerals += increaseBy;
}

void BosonStatistics::increaseRefinedOil(unsigned int increaseBy)
{
 mRefinedOil += increaseBy;
}

void BosonStatistics::addDestroyedMobileUnit(MobileUnit*, Unit*)
{
 mDestroyedMobileUnits++;
}

void BosonStatistics::addDestroyedFacility(Facility*, Unit*)
{
 mDestroyedFacilities++;
}

void BosonStatistics::addProducedMobileUnit(MobileUnit*, Facility*)
{
 mProducedMobileUnits++;
}

void BosonStatistics::addProducedFacility(Facility*, Facility*)
{
 mProducedFacilities++;
}

