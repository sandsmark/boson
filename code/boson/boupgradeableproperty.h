/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOUPGRADEABLEPROPERTY_H
#define BOUPGRADEABLEPROPERTY_H

#include <qstring.h>
#include "bomath.h"

template<class T> class QValueList;

class UnitProperties;
class BosonWeaponProperties;
class UpgradeProperties;
class SpeciesTheme;
class QDomElement;

class UpgradesCollection
{
public:
	UpgradesCollection();
	~UpgradesCollection();

	bool saveAsXML(QDomElement& root) const;
	bool loadFromXML(const SpeciesTheme* speciesTheme, const QDomElement& root);

	inline unsigned long int upgradesCacheCounter() const
	{
		return mUpgradesCacheCounter;
	}
	inline const QValueList<const UpgradeProperties*>* upgrades() const
	{
		return mUpgrades;
	}
	const UpgradeProperties* findUpgrade(unsigned long int id) const;

	void clearUpgrades();
	void addUpgrade(const UpgradeProperties* upgrade);
	bool removeUpgrade(const UpgradeProperties* upgrade);

private:
	QValueList<const UpgradeProperties*>* mUpgrades;
	unsigned long int mUpgradesCacheCounter;
};

class BoUpgradeablePropertyBase
{
public:
	/**
	 * @param name The name of the property, such as "Health" for health.
	 * Case sensitivie!
	 * @param type Whether this object represents the maximum or the minimum
	 * value of this property.
	 **/
	BoUpgradeablePropertyBase(const UnitProperties* unitProperties, const BosonWeaponProperties* weaponProperties, const QString& name, const QString& type = "MaxValue")
	{
		mCacheCounter = 0;
		mUnitProperties = unitProperties;
		mWeaponProperties = weaponProperties;
		mName = name;
		mType = type;
	}

	const QString& name() const
	{
		return mName;
	}
	const QString& type() const
	{
		return mType;
	}
	const UnitProperties* unitProperties() const
	{
		return mUnitProperties;
	}

	const BosonWeaponProperties* weaponProperties() const
	{
		return mWeaponProperties;
	}

	// AB: we cannot use a template method here, as we would have to
	// implement it in the header. i dont want to do this, as we would have
	// to include unitproperties.h and bosonweapon.h
	bool loadBaseValue(unsigned long int* v) const;
	bool loadBaseValue(long int* v) const;
	bool loadBaseValue(bofixed* v) const;

	bool upgradeValue(const QValueList<const UpgradeProperties*>* list, unsigned long int* v) const;
	bool upgradeValue(const QValueList<const UpgradeProperties*>* list, long int* v) const;
	bool upgradeValue(const QValueList<const UpgradeProperties*>* list, bofixed* v) const;

protected:
	/*
	 * "dirty" flag. When the upgrade cache counter of the player differs
	 * from this, then some upgrades have changed since the value was
	 * calculated the last time. We then need to recalculate the cache.
	 */
	mutable unsigned long int mCacheCounter;

private:
	QString mName;
	QString mType;
	const UnitProperties* mUnitProperties;
	const BosonWeaponProperties* mWeaponProperties;
};

template<class T> class BoUpgradeableProperty : public BoUpgradeablePropertyBase
{
public:
	BoUpgradeableProperty(const UnitProperties* unitProperties, const BosonWeaponProperties* weaponProperties, const QString& name, const QString& type = "MaxValue")
		: BoUpgradeablePropertyBase(unitProperties, weaponProperties, name, type)
	{
	}

	T value(const QValueList<const UpgradeProperties*>* upgrades, unsigned long int cacheCounter) const
	{
		if (mCacheCounter != cacheCounter || cacheCounter == 0) {
			T value;
			if (!loadBaseValue(&value)) {
				return value;
			}
			if (!upgradeValue(upgrades, &value)) {
				return value;
			}

			if (cacheCounter != 0) {
				mCacheCounter = cacheCounter;
				mCachedValue = value;
			}
		}
		return mCachedValue;
	}
	inline T value(const UpgradesCollection* c) const
	{
		return value(c->upgrades(), c->upgradesCacheCounter());
	}
	inline T value(const UpgradesCollection& c) const
	{
		return value(c.upgrades(), c.upgradesCacheCounter());
	}

private:
	mutable T mCachedValue;
};

#endif

