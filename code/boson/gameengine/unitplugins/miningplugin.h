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
#ifndef MININGPLUGIN_H
#define MININGPLUGIN_H

#include "unitplugin.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

class QDomElement;

/**
 * @short Helper plugin for mining (mining = placing mines (mine = explosive
 * device ;-)))
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class MiningPlugin : public UnitPlugin
{
public:
	MiningPlugin(Unit* owner);
	~MiningPlugin();

	virtual int pluginType() const { return Mining; }

	void mine(int weaponId);

	virtual void advance(unsigned int);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*) {}
	virtual void itemRemoved(BosonItem*) {}

private:
	BosonWeapon* mWeapon; // FIXME: must be saved in Unit::save()
	KGameProperty<int> mPlacingCounter;
};

#endif
