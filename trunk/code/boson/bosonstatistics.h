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
#ifndef __BOSONSTATISTICS_H__
#define __BOSONSTATISTICS_H__

class Unit;
class Facility;
class MobileUnit;

/**
 * Whenever a unit shoots its weapon or whenever a new unit is produced we need
 * to store that fact that this happened. This is done here, for every player.
 *
 * Usually player or a unit of this player calls @ref Player:statistics and
 * supplies an updated value, e.g. @ref increaseShots when the unit fired its
 * weapon or @ref increaseMinedMinerals if the unit has mined more minerals.
 *
 * All these values are used at the end of the game (in @ref GameOverDialog) to
 * generate statistics and finally even to generate the points for every player.
 * @short Statistics about a player for the @ref GameOverDialog
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonStatistics
{
public:
	BosonStatistics();
	~BosonStatistics();

	void increaseShots();
	unsigned long int shots() const;

	/**
	 * Increase the value returned by @ref destroyedMobileUnits.
	 * @param destroyed Unused - might get used to show which unit destroyed
	 * most enemies, or to check whether an enemy or a friend was destroyed
	 * @param destroyedBy Unused (see above)
	 **/
	void addDestroyedMobileUnit(MobileUnit* destroyed, Unit* destroyedBy);

	/**
	 * Increase the value returned by @ref destroyedFacilities.
	 * @param destroyed Unused - might get used to show which unit destroyed
	 * most enemies, or to check whether an enemy or a friend was destroyed
	 * @param destroyedBy Unused (see above)
	 **/
	void addDestroyedFacility(Facility* destroyed, Unit* destroyedBy);
			
	/**
	 * Increase the value returned by @producedFacility
	 * @param produced Unused - might get used to show which special unit
	 * has been produced most by this player
	 * @param producer Unused (see above)
	 **/
	void addProducedFacility(Facility* produced, Facility* produced);

	/**
	 * Increase the value returned by @producedFacility
	 * @param produced Unused - might get used to show which special unit
	 * has been produced most by this player
	 * @param producer Unused (see above)
	 **/
	void addProducedMobileUnit(MobileUnit* produced, Facility* produced);

	void increaseMinedMinerals(unsigned int increaseBy  = 1);
	void increaseMinedOil(unsigned int increaseBy  = 1);

	void increaseRefinedMinerals(unsigned int increaseBy  = 1);
	void increaseRefinedOil(unsigned int increaseBy  = 1);


	unsigned long int producedMobileUnits() const { return mProducedMobileUnits; }
	unsigned long int producedFacilities() const { return mProducedFacilities; }
	unsigned long int producedUnits() const { return producedFacilities() + producedMobileUnits(); }
	unsigned long int minedMinerals() const { return mMinedMinerals; }
	unsigned long int minedOil() const { return mMinedOil; }
	unsigned long int refinedMinerals() const { return mRefinedMinerals; }
	unsigned long int refinedOil() const { return mRefinedOil; }
	unsigned long int destroyedMobileUnits() { return mDestroyedMobileUnits; }
	unsigned long int destroyedFacilities() { return mDestroyedFacilities; }
	unsigned long int destroyedUnits() { return destroyedMobileUnits() + destroyedFacilities(); }

private:
	unsigned long int mShots; // note: the unsigned is important here! long is about 2 billion, but that's really not much for shots!
	unsigned long int mMinedMinerals;
	unsigned long int mMinedOil;
	unsigned long int mRefinedMinerals;
	unsigned long int mRefinedOil;
	unsigned long int mProducedMobileUnits;
	unsigned long int mProducedFacilities;
	unsigned long int mDestroyedMobileUnits;
	unsigned long int mDestroyedFacilities;
	

};

#endif
