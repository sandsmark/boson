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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef AMMUNITIONSTORAGEPLUGIN_H
#define AMMUNITIONSTORAGEPLUGIN_H

#include "unitplugin.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

class QDomElement;

class AmmunitionStoragePlugin : public UnitPlugin
{
public:
	AmmunitionStoragePlugin (Unit* owner);
	~AmmunitionStoragePlugin();

	virtual int pluginType() const { return AmmunitionStorage; }

	virtual bool loadFromXML(const QDomElement& root);
	virtual bool saveAsXML(QDomElement& root) const;
	virtual void advance(unsigned int advanceCallsCount);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

	/**
	 * @return TRUE if the ammo @p type must be picked up from the unit that has
	 * this plugin. If FALSE, the ammo can be used by a unit even if it is
	 * on the other side of the map.
	 **/
	bool mustBePickedUp(const QString& type) const;

	bool canStore(const QString& type) const;

	unsigned long int requestAmmunitionGlobally(const QString& type, unsigned long int requested);

	/**
	 * Like @ref requestAmmunitionGlobally, but this implies that the unit
	 * requesting the ammunition is close enough to actually pick the ammo
	 * up itself.
	 *
	 * @param picksUp The unit that picks up the ammunition. Note that the
	 * plugin may decide not to give any ammunition to that unit. See @p
	 * denies
	 * @param denied If non-NULL, then this is set to TRUE if the plugin
	 * decided not to give ammunition to the unit @p picksUp, most likely it
	 * is not close enough.
	 **/
	unsigned long int pickupAmmunition(Unit* picksUp, const QString& type, unsigned long int requested, bool* denied = 0);

	unsigned long int ammunitionStored(const QString& type) const;

	/**
	 * Try to put an amount of @p ammo of type @p type into the storage.
	 * This is usually successful, but if there is limited capacity, this
	 * might fail
	 * @return The amount of ammo that was actually stored. Always <= @p
	 * ammo.
	 **/
	unsigned long int tryToFillStorage(const QString& type, unsigned long int ammo);

protected:
	int changeAmmunition(const QString& type, int change);

	/**
	 * @internal
	 **/
	unsigned long int giveAmmunition(const QString& type, unsigned long int requested);

private:
	QMap<QString, unsigned long int> mAmmunitionStorage;
};

#endif
