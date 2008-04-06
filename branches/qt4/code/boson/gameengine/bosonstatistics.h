/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONSTATISTICS_H
#define BOSONSTATISTICS_H

class UnitBase;
class ProductionPlugin;
class Player;
class QDomElement;
class QString;

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
	quint32 shots() const;

	void addLostMobileUnit(UnitBase* unit);
	void addLostFacility(UnitBase* unit);

	/**
	 * Increase the value returned by @ref destroyedMobileUnits.
	 * @param destroyed Destroyed unit
	 * @param destroyedBy Player that destroyed the unit
	 **/
	void addDestroyedMobileUnit(UnitBase* destroyed, Player* destroyedBy);

	/**
	 * Increase the value returned by @ref destroyedFacilities.
	 * @param destroyed Destroyed unit
	 * @param destroyedBy Player that destroyed the unit
	 **/
	void addDestroyedFacility(UnitBase* destroyed, Player* destroyedBy);

	/**
	 * Increase the value returned by @producedFacility
	 * @param produced Unused - might get used to show which special unit
	 * has been produced most by this player
	 * @param producer Unused (see above)
	 **/
	void addProducedFacility(UnitBase* produced, ProductionPlugin* producedBy);

	/**
	 * Increase the value returned by @producedFacility
	 * @param produced Unused - might get used to show which special unit
	 * has been produced most by this player
	 * @param producer Unused (see above)
	 **/
	void addProducedMobileUnit(UnitBase* produced, ProductionPlugin* producedBy);

	void increaseMinedMinerals(unsigned int increaseBy  = 1);
	void increaseMinedOil(unsigned int increaseBy  = 1);

	void increaseRefinedMinerals(unsigned int increaseBy  = 1);
	void increaseRefinedOil(unsigned int increaseBy  = 1);


	quint32 producedMobileUnits() const { return mProducedMobileUnits; }
	quint32 producedFacilities() const { return mProducedFacilities; }
	quint32 producedUnits() const { return producedFacilities() + producedMobileUnits(); }
	quint32 minedMinerals() const { return mMinedMinerals; }
	quint32 minedOil() const { return mMinedOil; }
	quint32 refinedMinerals() const { return mRefinedMinerals; }
	quint32 refinedOil() const { return mRefinedOil; }

	quint32 lostMobileUnits() const { return mLostMobileUnits; }
	quint32 lostFacilities() const { return mLostFacilities; }
	quint32 lostUnits() const { return lostMobileUnits() + lostFacilities(); }
	
	quint32 destroyedMobileUnits() const { return mDestroyedMobileUnits; }
	quint32 destroyedFacilities() const { return mDestroyedFacilities; }
	quint32 destroyedUnits() const { return destroyedMobileUnits() + destroyedFacilities(); }
	quint32 destroyedOwnMobileUnits() const { return mDestroyedOwnMobileUnits; }
	quint32 destroyedOwnFacilities() const { return mDestroyedOwnFacilities; }
	quint32 destroyedOwnUnits() const { return destroyedOwnMobileUnits() + destroyedOwnFacilities(); }

	/**
	 * Note that one part of the points are calculated immediately, e.g. in
	 * @ref addDestroyedFacility the points are added immediately, but in
	 * @ref increaseRefinedMinerals the points are not touched. These 
	 * points are added "on-the-fly" when @ref points is called only
	 *
	 * Even negative values are possible here!
	 * @return How many points this player receives. Note that the "winning"
	 * factory is not yet taken into account! A winning player must get more
	 * points (<em>way</em> more) than a defeated player
	 **/
	qint32 points() const;


	static unsigned int winningPoints();

	/**
	 * Load all statistics from @ref QDomElement
	 * This is used for loading saved games
	 * @param root @ref QDomElement from where statistics are read
	 **/
	void load(const QDomElement& root);

	/**
	 * Save all statistics to @ref QDomElement
	 * This is used for saving games
	 * @param root @ref QDomElement where statistics are written to
	 **/
	void save(QDomElement& root) const;

protected:
	static float pointsPerRefinedMinerals();
	static float pointsPerRefinedOil();

	static unsigned int pointsPerDestroyedMobileUnit();
	static unsigned int pointsPerDestroyedFacility();
	static unsigned int pointsPerProducedMobileUnit();
	static unsigned int pointsPerProducedFacility();

	static int pointsPerLostMobileUnit();
	static int pointsPerLostFacility();
	/**
	 * Note: the @ref pointsPerLostMobileUnit is <em>also</em> called!
	 **/
	static int pointsPerDestroyedOwnMobileUnit();
	static int pointsPerDestroyedOwnFacility();


	/**
	 * Save @p value into @p root as an element with @p tagName
	 **/
	void saveULong(QDomElement& root, const QString& tagName, quint32 value) const;
	void saveLong(QDomElement& root, const QString& tagName, qint32 value) const;

	/**
	 * Search in @p root for a tag with @p tagName and store its value into
	 * @p value
	 **/
	void loadULong(const QDomElement& root , const QString& tagName, quint32* value);
	void loadLong(const QDomElement& root , const QString& tagName, qint32* value);

private:
	quint32 mShots; // note: the unsigned is important here! long is about 2 billion, but that's really not much for shots!
	quint32 mMinedMinerals;
	quint32 mMinedOil;
	quint32 mRefinedMinerals;
	quint32 mRefinedOil;
	quint32 mProducedMobileUnits;
	quint32 mProducedFacilities;
	quint32 mDestroyedMobileUnits;
	quint32 mDestroyedFacilities;
	quint32 mDestroyedOwnMobileUnits;
	quint32 mDestroyedOwnFacilities;
	quint32 mLostMobileUnits;
	quint32 mLostFacilities;

	qint32 mPoints;
};

#endif
