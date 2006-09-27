/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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
class BosonItem;
class QDomElement;
template<class T> class QIntDict;

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
	 * @param replace If TRUE (default) then the current selection is
	 * replaced. If FALSE, the new units are added to the selection instead.
	 **/
	void copy(BoSelection* selection, bool replace = true);

	/**
	 * Clear the selection and unselect all units.
	 * @param emitSignal Also emit @ref signalSelectionChanged if TRUE,
	 * otherwise don't emit it. Used internally, since the selection is
	 * cleared before it is changed.
	 **/
	void clear(bool emitSignal = true);

	/**
	 * Select a list of units
	 * @param replace If TRUE (default) then the current selection is
	 * replaced. If FALSE, the new units are added to the selection instead.
	 **/
	void selectUnits(const QPtrList<Unit>& list, bool replace = true);

	/**
	 * Select a single unit.
	 * @param replace If TRUE (default) then the current selection is
	 * replaced. If FALSE, the new unit is added to the selection instead.
	 **/
	void selectUnit(Unit*, bool replace = true);

	/**
	 * Remove unit from this selection and unselect unit.
	 **/
	void removeUnit(Unit* unit);

	/**
	 * @return The number of selected items.
	 **/
	uint count() const { return mSelection.count(); }

	/**
	 * @return TRUE if nothing is selected, otherwise FALSE
	 **/
	bool isEmpty() const { return mSelection.isEmpty(); }

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
	bool contains(Unit* unit) const { return mSelection.containsRef(unit); }

	/**
	 * @return All selected units
	 **/
	QPtrList<Unit> allUnits() const { return mSelection; }

	/**
	 * @return TRUE if at least one unit in the selection can shoot,
	 * otherwise FALSE
	 **/
	bool canShoot() const;

	/**
	 * @return TRUE if at least one unit in the selection can shoot at unit,
	 * otherwise FALSE
	 **/
	bool canShootAt(Unit* unit) const;

	/**
	 * @return TRUE if at least on unit in this selection can mine minerals
	 * otherwise FALSE.
	 **/
	bool hasMineralHarvester() const;

	/**
	 * @return TRUE if at least on unit in this selection can mine oil 
	 * otherwise FALSE.
	 **/
	bool hasOilHarvester() const;

	void activate(bool on);

	void saveAsXML(QDomElement& root);
	void loadFromXML(const QDomElement& root, bool activate = false);

public slots:
	void slotSelectSingleUnit(Unit* unit) { selectUnit(unit, true); }

	/**
	 * Remove @p item from the selection. This does nothing if @p item is
	 * not a unit.
	 *
	 * You can (and should) use this slot to ensure that an item/unit is
	 * actually removed from the selection when it is deleted.
	 **/
	void slotRemoveItem(BosonItem* item);

protected:
	void add(Unit* unit);
	/**
	 * @return TRUE if @p unit was in this selection, otherwise FALSE
	 **/
	bool remove(Unit* unit);

signals:
	/**
	 * @param selection this
	 **/
	void signalSelectionChanged(BoSelection* selection);

private:
	bool mIsActivated;
	QPtrList<Unit> mSelection;
};


/**
 * @short A collection of @ref BoSelection objects
 *
 * This class stores the selection groups that can usually be created using
 * CTRL+number.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoSelectionGroup : public QObject
{
	Q_OBJECT
public:
	BoSelectionGroup(int count, QObject* parent);
	~BoSelectionGroup();

	int count() const
	{
		return mCount;
	}

	void clearGroups();

	/**
	 * Set the active selection, i.e. the selection that displays the unit
	 * that are currently on the screen.
	 *
	 * This selection is used to copy data to/from when a selection group
	 * from this class is selected or created.
	 **/
	void setSelection(BoSelection* s) { mSelection = s; }

	/**
	 * @return See @ref setSelection
	 **/
	BoSelection* selection() const { return mSelection; }

	bool saveAsXML(QDomElement& root) const;
	bool loadFromXML(const QDomElement& root);

public slots:
	/**
	 * Connect @ref BosonCanvas::signalRemovedItem to this. It removes @p
	 * item from all selections.
	 **/
	void slotRemoveItem(BosonItem* item);

	/**
	 * Connect @ref BosonCanvas::signalUnitRemoved to this. It calls @ref
	 * BoSelection::removeUnit fro all selection groups.
	 **/
	void slotRemoveUnit(Unit* u);

	/**
	 * Select the specified selection group
	 * @param number The selection group to be selected. Must be in range 0..9 where 1
	 * is the first group and 0 the 10th group.
	 **/
	void slotSelectSelectionGroup(int number);

	/**
	 * Copy the current selection to the specified selection group.
	 * @param number The group to be created. Must be in range 0..9 where 1
	 * is the first group and 0 the 10th group.
	 **/
	void slotCreateSelectionGroup(int number);

	/**
	 * Clear the specified selection group.
	 * @param number The group to be created. Must be in range 0..9 where 1
	 * is the first group and 0 the 10th group.
	 **/
	void slotClearSelectionGroup(int number);

private:
	int mCount;
	QIntDict<BoSelection>* mSelectionGroups;
	BoSelection* mSelection;
};

#endif
