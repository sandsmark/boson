#ifndef __UNITPROPERTIES_H__
#define __UNITPROPERTIES_H__

#include <qstring.h>

#include <qvaluelist.h>

class KSimpleConfig;

class UnitPropertiesPrivate;

/**
 * Represents the config file of a unit. See README of the config file for
 * infos.
 **/
class UnitProperties
{
public:
	/**
	 * Only public cause of the private d-pointer! Will become protected as
	 * soon as the d pointer is removed. Don't use outside this class!
	 **/
	enum TerrainType {
		Land=0,
		Water=1,
		Air=2
	};
	
	UnitProperties();

	/**
	 * @param fileName The filename of the config file for this unit type
	 **/
	UnitProperties(const QString& fileName);
	~UnitProperties();

	unsigned long int health() const;
	unsigned long int shields() const;
	unsigned long int armor() const;
	unsigned long int prize() const;

	/**
	 * @return The weapon range of this unit.
	 **/
	unsigned long int range() const;

	/**
	 * @return The number of advance calls until the weapon is reloaded
	 **/
	unsigned int reload() const;

	/**
	 * The damage this unit makes to other units. Negative values means
	 * repairing
	 **/
	long int damage() const;
	
	/**
	 * @return The Type ID of the unit. This ID is unique for this
	 * UnitProperties. There is no other unit with the same type ID. Note
	 * that you can construct several units of the same type ID in a game -
	 * they will all be of the same type (e.g. they are all ships).
	 **/
	int typeId() const;  // we MUST use int (not unsigned int) as -1 is used for invalid
	const QString& name() const;

	void loadUnitType(const QString& fileName);

	/**
	 * @return If this is a mobile unit. Better use @ref Unit::isMobile()
	 **/
	bool isMobile() const;

	/**
	 * @return If this is a facility. Better use @ref Unit::isFacility()
	 **/
	bool isFacility() const;

// only if this unit is a mobile unit:
	double speed() const;

	/**
	 * @return Whether this unit can go over land
	 **/
	bool canGoOnLand() const; // FIXME is there a shorter and better name?

	/**
	 * @return Whether this unit can go on water - currently only ships.
	 **/
	bool canGoOnWater() const;

	/**
	 * @return Whether this is an aircraft unit. Currently there is only
	 * one.
	 **/
	bool isAircraft() const;

	/**
	 * @return Whether this is a ship
	 **/
	bool isShip() const;

	/**
	 * @return Whether this is a land unit.
	 **/
	bool isLand() const;

	/**
	 * @return Whether this facility (if it is one) can produce anything.
	 **/
	bool canProduce() const;

	/**
	 * @return A list of units which can be produced by this facility (if
	 * any).
	 **/
	QValueList<int> produceList() const;

	const QString& unitPath() const;

protected:
	void loadMobileProperties(KSimpleConfig* conf);
	void loadFacilityProperties(KSimpleConfig* conf);
	
private:
	UnitPropertiesPrivate* d;
};

#endif
