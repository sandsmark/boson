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
#ifndef REPAIRPLUGIN_H
#define REPAIRPLUGIN_H

#include "unitplugin.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

class QDomElement;

/**
 * Experimental plugin. At the current state id doesn't make any sense, since I
 * don't use any member variables anymore...
 *
 * Nevertheless I don't entegrate the functionality into Unit since it should
 * get some more testing
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class RepairPlugin : public UnitPlugin
{
public:
	RepairPlugin(Unit* owner);
	~RepairPlugin();

	virtual int pluginType() const { return Repair; }

	/**
	 * Order to repair unit. For a repairyard this means the unit will move
	 * to the repairyard and once it is in range it'll be repaired.
	 *
	 * For mobile repair-units this means that the <em>repairing</em> (i.e.
	 * the one that has this plugin) moves to unit and repairs it.
	 **/
	void repair(Unit* unit);

	/**
	 * Called from @ref Unit::advanceIdle. Repair the next unit that is in
	 * range. An alternative name might be "advance", just like in @ref
	 * ProducePlugin but since we don't have a WorkRepair in @ref Unit there
	 * is no advance call for it from @ref BosonCanvas::slotAdvance either.
	 *
	 * @ref Unit::advanceIdle is used for it instead.
	 **/
	void repairInRange();


	// does nothing, yet. plugin is experimental anyway.
	virtual void advance(unsigned int) {}

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

private:
};

#endif
