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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOUPGRADEABLEPROPERTY_H
#define BOUPGRADEABLEPROPERTY_H

#include <qstring.h>
//Added by qt3to4:
#include <Q3ValueList>
#include "../bomath.h"

template<class T> class Q3ValueList;

class UnitProperties;
class BosonWeaponProperties;
class UpgradeProperties;
class SpeciesTheme;
class QDomElement;

class BoBaseValueCollectionPrivate;
/**
 * This class provides a collection of "base" values of upgradeable properties.
 * Providing a value in an object of this class is mandatory for @ref
 * BoUpgradeableProperty objects.
 *
 * A "base" value of a property is the value that is read from for example the
 * index.unit file. This value will be used as base for all upgrades (see @ref
 * BoUpgradeableProperty).
 * @short Collection of base values for @ref BoUpgradeableProperty objects
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoBaseValueCollection
{
public:
	BoBaseValueCollection();
	~BoBaseValueCollection();

//#warning TODO: rename *LongBaseValue -> *IntBaseValue
	bool insertULongBaseValue(quint32 v, const QString& name, const QString& type = "MaxValue", bool replace = true);
	bool insertLongBaseValue(qint32 v, const QString& name, const QString& type = "MaxValue", bool replace = true);
	bool insertBoFixedBaseValue(bofixed v, const QString& name, const QString& type = "MaxValue", bool replace = true);

	/**
	 * Same as @ref insertULongBaseValue, but this name provides overloaded
	 * versions of the method for all data types. For type safety you should
	 * prefer @ref insertULongBaseValue instead.
	 **/
	bool insertBaseValue(quint32 v, const QString& name, const QString& type = "MaxValue", bool replace = true)
	{
		return insertULongBaseValue(v, name, type, replace);
	}
	/**
	 * @overload
	 **/
	bool insertBaseValue(qint32 v, const QString& name, const QString& type = "MaxValue", bool replace = true)
	{
		return insertLongBaseValue(v, name, type, replace);
	}
	/**
	 * @overload
	 **/
	bool insertBaseValue(bofixed v, const QString& name, const QString& type = "MaxValue", bool replace = true)
	{
		return insertBoFixedBaseValue(v, name, type, replace);
	}

	/**
	 * @return The base value of an upgradeable property. This value is the
	 * start-value, before any upgrades are applied.
	 * @param name The name of the property
	 * @param type Either "MaxValue" or "MinValue". Use "MaxValue" if you
	 * are not sure.
	 **/
	bool getBaseValue(quint32* ret, const QString& name, const QString& type = "MaxValue") const;
	bool getBaseValue(qint32* ret, const QString& name, const QString& type = "MaxValue") const;
	bool getBaseValue(bofixed* ret, const QString& name, const QString& type = "MaxValue") const;

	/**
	 * @return @ref getBaseValue or @p defaultValue if the @p
	 * name @p type pair does not exist.
	 **/
	quint32 ulongBaseValue(const QString& name, const QString& type = "MaxValue", quint32 defaultValue = 0) const;
	qint32 longBaseValue(const QString& name, const QString& type = "MaxValue", qint32 defaultValue = 0) const;
	bofixed bofixedBaseValue(const QString& name, const QString& type = "MaxValue", bofixed defaultValue = 0) const;


private:
	BoBaseValueCollectionPrivate* d;
};

/**
 * @short This class provides a collection of upgrades.
 * A @ref BoUpgradeableProperty requires a list of upgrades to be applied to the
 * property in order to calculate the real value. Such a list is provided by
 * this class.
 *
 * This class also provides a hint to @ref BoUpgradeableProperty whether it
 * can use the cached value of the property, or whether it has to recalculate
 * the value. Whenever @ref addUpgrade or @ref removeUpgrade is called, all @ref
 * BoUpgradeableProperty objects will recalculate their value once they are
 * accessed.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUpgradesCollection
{
public:
	BoUpgradesCollection();
	~BoUpgradesCollection();

	bool saveAsXML(QDomElement& root) const;
	bool loadFromXML(const SpeciesTheme* speciesTheme, const QDomElement& root);

	inline quint32 upgradesCacheCounter() const
	{
		return mUpgradesCacheCounter;
	}
	inline const Q3ValueList<const UpgradeProperties*>* upgrades() const
	{
		return mUpgrades;
	}
	const UpgradeProperties* findUpgrade(quint32 id) const;

	void clearUpgrades();
	void addUpgrade(const UpgradeProperties* upgrade);
	bool removeUpgrade(const UpgradeProperties* upgrade);

private:
	Q3ValueList<const UpgradeProperties*>* mUpgrades;
	quint32 mUpgradesCacheCounter;
};

/**
 * @short This is the base class for @ref BoUpgradeableProperty.
 *
 * @ref BoUpgradeableProperty uses @ref loadBaseValue to load the initial value
 * of the property and then @ref upgradeValue to apply the upgrades to this
 * value.
 *
 * Every property has a @ref name and a @ref type.
 *
 * The @ref name is a unique internal string that identifies the property. It
 * usually makes sense to use the same name as in the index.technologies (or
 * similar) files. For example "Health" for the health property.
 *
 * The @ref type describes whether the property is a "MaxValue" or a "MinValue"
 * property. This has no internal meaning, it is meant as a help for the user.
 * In Boson we currently make no use of  "MinValue", it might be used for things
 * like "minimal weapon range".
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUpgradeablePropertyBase
{
public:
	/**
	 * @param name The name of the property, such as "Health" for health.
	 * Case sensitivie!
	 * @param type Whether this object represents the maximum or the minimum
	 * value of this property.
	 **/
	BoUpgradeablePropertyBase(const BoBaseValueCollection* baseValueSource, const QString& name, const QString& type = "MaxValue")
	{
		mCacheCounter = 0;
		mBaseValueSource = baseValueSource;
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
	const BoBaseValueCollection* baseValueCollection() const
	{
		return mBaseValueSource;
	}

protected:
	template<class T>bool loadBaseValue(T* v) const
	{
		if (!baseValueCollection()) {
			return false;
		}
		return baseValueCollection()->getBaseValue(v, name(), type());
	}

	bool upgradeValue(const Q3ValueList<const UpgradeProperties*>* list, quint32* v) const;
	bool upgradeValue(const Q3ValueList<const UpgradeProperties*>* list, qint32* v) const;
	bool upgradeValue(const Q3ValueList<const UpgradeProperties*>* list, bofixed* v) const;

protected:
	/*
	 * "dirty" flag. When the upgrade cache counter of the player differs
	 * from this, then some upgrades have changed since the value was
	 * calculated the last time. We then need to recalculate the cache.
	 */
	mutable quint32 mCacheCounter;

private:
	QString mName;
	QString mType;
	const BoBaseValueCollection* mBaseValueSource;
};

/**
 * @short Upgradeable properties
 *
 * An object of this class is a property that can make use of upgrades, i.e. is
 * upgradeable.
 *
 * WARNING: An "upgradeable" property refers to the values a property can be set
 *          to, i.e. their minimum and maximum values. It does NOT refer to the
 *          actual current value. For example the "Health" property should NOT
 *          be an object of this class, whereas the MaxHealth property should be
 *          (MinHealth does of course make no sense, as it is always 0).
 *
 * You must provide an object of @ref BoBaseValueCollection to this class, this
 * is where the propertiy retrieves its "base" values from. Note that you have
 * to fill it with data, under the same @ref name and @ref type (and with the
 * same data type) as the object of this class uses.
 *
 * You can retrieve the actual value of this property using @ref value. This is
 * the value of the property after all upgrades have been applied.
 *
 * The value is calculated on the fly, but is cached internally. Therefore
 * calling @ref value is very fast, only when upgrades changed @ref value
 * requires recalculations.
 *
 * WARNING: Note that you should NOT use @ref value with two different @ref
 *          BoUpgradeCollection objects! This might cause the cache to get
 *          out of sync!
 *
 * Objects of this class are never saved to a stream or a file, as they are
 * always calculated on the fly from the current upgrades (the upgrades @em are
 * saved).
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
template<class T> class BoUpgradeableProperty : public BoUpgradeablePropertyBase
{
public:
	BoUpgradeableProperty(const BoBaseValueCollection* baseValueSource, const QString& name, const QString& type = "MaxValue")
		: BoUpgradeablePropertyBase(baseValueSource, name, type)
	{
	}
	BoUpgradeableProperty(BoBaseValueCollection* baseValueSource, const QString& name, const QString& type = "MaxValue")
		: BoUpgradeablePropertyBase(baseValueSource, name, type)
	{
		// AB: this makes sure that there actually is a base value for
		//     this property.
		//     note that because of replace=fase, this will be a noop, if
		//     such a base value is already provided.
		// AB: FIXME: this requires that we can cast "0" into this data
		//    type. is there a different way to achieve this?
		baseValueSource->insertBaseValue((T)0, name, type, false);
	}

	inline T value(const BoUpgradesCollection* c) const
	{
		return value(c->upgrades(), c->upgradesCacheCounter());
	}
	inline T value(const BoUpgradesCollection& c) const
	{
		return value(c.upgrades(), c.upgradesCacheCounter());
	}

protected:
	T value(const Q3ValueList<const UpgradeProperties*>* upgrades, quint32 cacheCounter) const
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

private:
	mutable T mCachedValue;
};

#endif

