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

#include "unitbase.h"

#include <qdatastream.h>

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
 mDestroyedOwnMobileUnits = 0;
 mDestroyedOwnFacilities= 0;
 mLostMobileUnits = 0;
 mLostFacilities= 0;

 mPoints = 0;
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

void BosonStatistics::addDestroyedMobileUnit(UnitBase* destroyed, UnitBase* destroyedBy)
{
 if (destroyed->owner() == destroyedBy->owner()) {
	mDestroyedOwnMobileUnits++;
	mPoints += pointsPerDestroyedOwnMobileUnit();
	return;
 }
 mDestroyedMobileUnits++;
 mPoints += pointsPerDestroyedMobileUnit();
}

void BosonStatistics::addDestroyedFacility(UnitBase* destroyed, UnitBase* destroyedBy)
{
 if (destroyed->owner() == destroyedBy->owner()) {
	mDestroyedOwnFacilities++;
	mPoints += pointsPerDestroyedOwnFacility();
	return;
 }
 mDestroyedFacilities++;
 mPoints += pointsPerDestroyedFacility();
}

void BosonStatistics::addLostMobileUnit(UnitBase*)
{
 mLostMobileUnits++;
 mPoints += pointsPerLostMobileUnit();
}

void BosonStatistics::addLostFacility(UnitBase*)
{
 mLostFacilities++;
 mPoints += pointsPerLostFacility();
}

void BosonStatistics::addProducedMobileUnit(UnitBase*, ProductionPlugin*)
{
 mProducedMobileUnits++;
 mPoints += pointsPerProducedMobileUnit();
}

void BosonStatistics::addProducedFacility(UnitBase*, ProductionPlugin*)
{
 mProducedFacilities++;
 mPoints += pointsPerProducedFacility();
}

long int BosonStatistics::points() const
{
 long int points = mPoints;

// I'm not sure if shot's should get points at all!
// Probably it's better to use an inverse here. less shots is better, more shots
// are worse. but perhaps we simply don't give shots at all.
// points += (unsigned long int)(shots() * 0.001);

 points += (unsigned long int)(refinedMinerals() * pointsPerRefinedMinerals());
 points += (unsigned long int)(refinedOil() * pointsPerRefinedOil());

 return points;
}


float BosonStatistics::pointsPerRefinedMinerals()
{
 return 0.1;
}

float BosonStatistics::pointsPerRefinedOil()
{
 return 0.1;
}

unsigned int BosonStatistics::pointsPerDestroyedMobileUnit()
{
 return 2;
}

unsigned int BosonStatistics::pointsPerDestroyedFacility()
{
 return 5;
}

int BosonStatistics::pointsPerDestroyedOwnMobileUnit()
{
 return -2;
}

int BosonStatistics::pointsPerDestroyedOwnFacility()
{
 return -5;
}

unsigned int BosonStatistics::pointsPerProducedMobileUnit()
{
 return 5;
}

unsigned int BosonStatistics::pointsPerProducedFacility()
{
 return 10;
}

int BosonStatistics::pointsPerLostMobileUnit()
{
 return -2;
}

int BosonStatistics::pointsPerLostFacility()
{
 return -5;
}

unsigned int BosonStatistics::winningPoints()
{
 return 5000;
}

void BosonStatistics::load(QDataStream& stream)
{
 Q_UINT32 shots;
 Q_UINT32 minedMinerals;
 Q_UINT32 minedOil;
 Q_UINT32 refinedMinerals;
 Q_UINT32 refinedOil;
 Q_UINT32 producedMobUnits;
 Q_UINT32 producedFac;
 Q_UINT32 destroyedMobUnits;
 Q_UINT32 destroyedFac;
 Q_UINT32 destroyedOwnMobUnits;
 Q_UINT32 destroyedOwnFac;
 Q_UINT32 lostMobUnits;
 Q_UINT32 lostFac;
 Q_INT32 points;

 stream >> shots;
 stream >> minedMinerals;
 stream >> minedOil;
 stream >> refinedMinerals;
 stream >> refinedOil;
 stream >> producedMobUnits;
 stream >> producedFac;
 stream >> destroyedMobUnits;
 stream >> destroyedFac;
 stream >> destroyedOwnMobUnits;
 stream >> destroyedOwnFac;
 stream >> lostMobUnits;
 stream >> lostFac;
 stream >> points;

 mShots = shots;
 mMinedMinerals = minedMinerals;
 mMinedOil = minedOil;
 mRefinedMinerals = refinedMinerals;
 mRefinedOil = refinedOil;
 mProducedMobileUnits = producedMobUnits;
 mProducedFacilities = producedFac;
 mDestroyedMobileUnits = destroyedMobUnits;
 mDestroyedFacilities = destroyedFac;
 mDestroyedOwnMobileUnits = destroyedOwnMobUnits;
 mDestroyedOwnFacilities = destroyedOwnFac;
 mLostMobileUnits = lostMobUnits;
 mLostFacilities = lostFac;
 mPoints = points;
}

void BosonStatistics::save(QDataStream& stream)
{
 stream << (Q_UINT32)mShots;
 stream << (Q_UINT32)mMinedMinerals;
 stream << (Q_UINT32)mMinedOil;
 stream << (Q_UINT32)mRefinedMinerals;
 stream << (Q_UINT32)mRefinedOil;
 stream << (Q_UINT32)mProducedMobileUnits;
 stream << (Q_UINT32)mProducedFacilities;
 stream << (Q_UINT32)mDestroyedMobileUnits;
 stream << (Q_UINT32)mDestroyedFacilities;
 stream << (Q_UINT32)mDestroyedOwnMobileUnits;
 stream << (Q_UINT32)mDestroyedOwnFacilities;
 stream << (Q_UINT32)mLostMobileUnits;
 stream << (Q_UINT32)mLostFacilities;
 stream << (Q_INT32)mPoints;
}

