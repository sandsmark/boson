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
#ifndef REFINERYPLUGIN_H
#define REFINERYPLUGIN_H

#include "unitplugin.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;
class HarvesterPlugin;
class ResourceMinePlugin;

class QDomElement;

class RefineryPlugin : public UnitPlugin
{
public:
	RefineryPlugin(Unit* owner);
	~RefineryPlugin();

	virtual int pluginType() const { return Refinery; }

	virtual bool loadFromXML(const QDomElement& root);
	virtual bool saveAsXML(QDomElement& root) const;
	virtual void advance(unsigned int advanceCallsCount);

	bool isUsableTo(const HarvesterPlugin* harvester) const;

	bool canRefineMinerals() const;
	bool canRefineOil() const;

	/**
	 * Try to refine @p minerals.
	 * @return The minerals that got actually refined. The harvester should
	 * reduce it's resources by exactly that amount only
	 **/
	unsigned int refineMinerals(unsigned int minerals);

	/**
	 * Try to refine @p oil.
	 * @return The oil that got actually refined. The harvester should
	 * reduce it's resources by exactly that amount only
	 **/
	unsigned int refineOil(unsigned int oil);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);
};


#endif
