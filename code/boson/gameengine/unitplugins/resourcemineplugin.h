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
#ifndef RESOURCEMINEPLUGIN_H
#define RESOURCEMINEPLUGIN_H

#include "unitplugin.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;
class HarvesterPlugin;

class QDomElement;

/**
 * @short Plugin for mineral/oil mines
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class ResourceMinePlugin : public UnitPlugin
{
public:
	ResourceMinePlugin(Unit* owner);
	~ResourceMinePlugin();

	virtual int pluginType() const { return ResourceMine; }

	virtual bool loadFromXML(const QDomElement& root);
	virtual bool saveAsXML(QDomElement& root) const;
	virtual void advance(unsigned int advanceCallsCount);

	bool isUsableTo(const HarvesterPlugin* harvester) const;

	/**
	 * @return How much minerals are left here. -1 means unlimited.
	 **/
	int minerals() const;

	/**
	 * @return How much oil is left here. -1 means unlimited.
	 **/
	int oil() const;

	/**
	 * Mine minerals. The amount of @ref minerals is (if limited) reduced
	 * by the returned value.
	 * @return The amount of mined minerals
	 **/
	unsigned int mineMinerals(const HarvesterPlugin* harvester);

	/**
	 * Mine oil. The amount of @ref oil is (if limited) reduced
	 * by the returned value.
	 * @return The amount of mined oil
	 **/
	unsigned int mineOil(const HarvesterPlugin* harvester);

	void setMinerals(int m);
	void setOil(int m);

	/**
	 * @return See @ref ResourceMinePropeties::canProvideMinerals
	 **/
	bool canProvideMinerals() const;

	/**
	 * @return See @ref ResourceMinePropeties::canProvideOil
	 **/
	bool canProvideOil() const;

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

protected:
	/**
	 * @return The amount of resources that @ref mineMinerals or @ref
	 * mineOil will mine.
	 **/
	unsigned int mineStep(const HarvesterPlugin* harvester, int resourcesAvailable) const;

private:
	KGameProperty<int> mOil;
	KGameProperty<int> mMinerals;
};

#endif
