/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSELECTION_H
#define BOSELECTION_H

#include <qobject.h>
#include <qptrlist.h>

class Unit;

/**
 * Represents a selection. Every @ref BosonBigDisplay has its own selection
 * which gets activated or deactivated whenever the big display is
 * activated/deactivated.
 * @short A selection of item (usually units) on a @ref BosonBigDisplay
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoSelection : public QObject
{
	Q_OBJECT
public:
	BoSelection(QObject* parent);
	~BoSelection();

	/**
	 * Clear the selection and unselect all units.
	 **/
	void clear();

	/**
	 * Select a list of units
	 **/
	void selectUnits(QPtrList<Unit> list);

	/**
	 * Select a single unit
	 **/
	void selectUnit(Unit*);

	/**
	 * Remove unit from this selection and unselect unit. Also emit @ref
	 * signalUnselectUnit
	 **/
	void removeUnit(Unit* unit);

	/**
	 * @return The number of selected items.
	 **/
	uint count() const;

	/**
	 * @return TRUE if nothing is selected, otherwise FALSE
	 **/
	bool isEmpty() const;

	/**
	 * @return Whether the current selection has at least one mobile unit
	 **/
	bool hasMobileUnit() const;

	/**
	 * @return The leader of the selection. This is usually the first unit
	 **/
	Unit* leader() const;

	/**
	 * @return TRUE if unit is in this selection, otherwise FALSE
	 **/
	bool contains(Unit* unit) const;

	/**
	 * @return All selected units
	 **/
	QPtrList<Unit> allUnits() const;

	void activate(bool on);

protected:
	void add(Unit* unit);
	void remove(Unit* unit);
	
signals:
	/**
	 * Emitted when a single unit (i.e. @ref count is 1) is selected. This
	 * may also happen when all except one units of a selection are
	 * destroyed.
	 *
	 * The game should display the image and further information about this
	 * unit and also order buttons (e.g. productions) which are suitable for
	 * the unit.
	 **/
	void signalSingleUnitSelected(Unit* unit);

	/**
	 * Emitted when a unit is removed from the selection. See @ref
	 * removeUnit. Note that this is <em>not</em> emitted when the list is
	 * cleared (see @ref clear)
	 **/
	void signalUnselectUnit(Unit* unit);

	/**
	 * Emitted when a new unit is added to the selection. This is done for
	 * every unit in a list.
	 *
	 * Note that this is not emitted when only a single unit is selected.
	 * See also @ref signalSingleUnitSelected
	 **/
	void signalSelectUnit(Unit* unit);

private:
	class BoSelectionPrivate;
	BoSelectionPrivate* d;
};

#endif
