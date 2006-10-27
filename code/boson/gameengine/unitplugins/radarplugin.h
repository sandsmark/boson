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
#ifndef RADARPLUGIN_H
#define RADARPLUGIN_H

#include "unitplugin.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

class QDomElement;

class RadarPlugin : public UnitPlugin
{
public:
	RadarPlugin (Unit* owner);
	~RadarPlugin();

	virtual int pluginType() const { return Radar; }

	virtual bool loadFromXML(const QDomElement& root);
	virtual bool saveAsXML(QDomElement& root) const;
	virtual void advance(unsigned int advanceCallsCount);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

	/**
	 * Call this whenever unit's health changes. It recalculates radar's
	 *  transmitted power and range (which are dependant on health)
	 **/
	void unitHealthChanged();

	/**
	 * @return Power transmitted by the transmitter antenna.
	 * Note that this has _no_ correspondance to the power resource.
	 **/
	float transmittedPower() const { return mTransmittedPower; }

	/**
	 * @return Minimum received power to notice the target
	 **/
	float minReceivedPower() const;

	/**
	 * @return Range of the radar
	 * It's unlikely that any objects outside this range would be detected
	 **/
	bofixed range() const { return mRange; }

	bool detectsLandUnits() const;
	bool detectsAirUnits() const;

private:
	bofixed mRange;
	float mTransmittedPower;
};

#endif
