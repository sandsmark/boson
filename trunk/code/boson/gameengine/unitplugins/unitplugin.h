/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2006 Rivo Laks (rivolaks@hot.ee)

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
#ifndef UNITPLUGIN_H
#define UNITPLUGIN_H

#include "../../global.h"
#include "../../bomath.h"

#include "../bogameproperty.h"

#include <qvaluelist.h>
#include <qmap.h>
#include <qpair.h>

class Unit;
class SpeciesTheme;
class UnitProperties;
class BosonCanvas;
class Cell;
class Player;
class PluginProperties;
class Boson;
class BosonItem;
class BosonWeapon;
class BoUpgradesCollection;
template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

class QDomElement;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitPlugin
{
public:
	enum UnitPlugins {
		PluginStart = 0, // MUST be the first!
		Production = 1,
		Repair = 2,
		Harvester = 3,
		Weapon = 4, // note: this won't end up in Unit::plugin()! weapons are stored separately. also note that rtti==Weapon is *not* unique! they have their own class and rttis - see BosonWeapon
		Bombing = 5,
		Mining = 6, // placing mine (the exploding ones)
		ResourceMine = 7,
		Refinery = 8,
		AmmunitionStorage = 9,
		Radar = 10,
		RadarJammer = 11,

		PluginEnd // MUST be the last entry!
	};

	UnitPlugin(Unit* unit);
	virtual ~UnitPlugin();

	inline Unit* unit() const { return mUnit; }

	/**
	 * Convenience method for unit()->speciesTheme()
	 **/
	SpeciesTheme* speciesTheme() const;

	/**
	 * Convenience method for unit()->owner()
	 **/
	Player* player() const;

	/**
	 * Convenience method for unit()->unitProperties()
	 **/
	const UnitProperties* unitProperties() const;

	/**
	 * Convenience method for unit()->properties()
	 **/
	const PluginProperties* properties(int propertyType) const;

	/**
	 * Convenience method for unit()->canvas()
	 **/
	BosonCanvas* canvas() const;

	/**
	 * Convenience method for unit()->dataHandler()
	 **/
	KGamePropertyHandler* dataHandler() const;

	/**
	 * Convenience method for player()->game()
	 **/
	Boson* game() const;

	/**
	 * Convenience method for unit()->upgradesCollection().
	 *
	 * This object can be used to implement @ref BoUpgradeableProperty
	 * objects in UnitPlugin objects.
	 **/
	const BoUpgradesCollection& upgradesCollection() const;

	virtual int pluginType() const = 0;

	/**
	 * See @ref Unit::unitDestroyed
	 **/
	virtual void unitDestroyed(Unit* unit) = 0;

	/**
	 * Called when @p item is about to be removed from the game. When your
	 * plugin stores a pointer to an item (e.g. a unit, such as a pointer to
	 * a refinery), you should set it at least to NULL now.
	 *
	 * Note that at this point @p item has not yet been deleted, but it will
	 * be soon!
	 **/
	virtual void itemRemoved(BosonItem* item) = 0;

	/**
	 * @param advanceCallsCount See @ref BosonCanvas::slotAdvance. You can use
	 * this to do expensive calculations only as seldom as possible. Note
	 * that there is still some overhead, since this advance method still
	 * gets called!
	 **/
	virtual void advance(unsigned int advanceCallsCount) = 0;

	/**
	 * Save the plugin into @p root. You must implement this in derived
	 * classes, but usually you will simply return true without touching @p
	 * root.
	 *
	 * Note that you are meant to use @ref KGameProperty for most of the
	 * properties. Don't use saveAsXML() for integer values or so.
	 *
	 * You should save e.g. IDs that help you to identify a pointer (e.g.
	 * the harvester plugin will save the ID of the refinery it is going
	 * to).
	 *
	 * See also @ref loadFromXML.
	 **/
	virtual bool saveAsXML(QDomElement& root) const = 0;

	/**
	 * See also @ref saveAsXML. You should use @ref KGameProperty for most
	 * properties.
	 *
	 * Here you will usually load pointers - e.g. you could save the ID of a
	 * target in @ref saveAsXML and here you could set the pointer of the
	 * target.
	 **/
	virtual bool loadFromXML(const QDomElement& root) = 0;

protected:
	bool isNextTo(const Unit* unit) const;

private:
	Unit* mUnit;
};

#endif
