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
#ifndef BOITEMLIST_H
#define BOITEMLIST_H

#include <qvaluelist.h>
class QCanvasItem;
class Unit;

/**
 * This list contains all collision-relevant items of a cell. This is not
 * perfectly true, since aircrafts (which are not collision relevant) are also
 * added to the list, but I hope you get the point. All units of a cell are in
 * the list of the cell, as well as all mines (not the ones where you find
 * minerals but the exploding ones) or other, similar items (maybe rocks or
 * something).
 *
 * Note that since nothing of the above except units is implemented in boson so
 * all items in the list are units, currently. 
 * @short A list of QCanvasItem
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoItemList : public QValueList<QCanvasItem*>
{
public:
	// FIXME: this *should* be Unit* instead of QCanvasItem but since we
	// must not include unit.h here we use QCanvasItem. pointer conversion
	// without unit.h included would cause problems here :-(
	iterator appendUnit(QCanvasItem* unit) { return QValueList<QCanvasItem*>::append((QCanvasItem*)unit); }
	uint removeUnit(QCanvasItem* unit) { return QValueList<QCanvasItem*>::remove((QCanvasItem*)unit); }

	/**
	 * @param collidingOnly if TRUE return only items that are interesting
	 * for collision detection. Please note that moving units are
	 * <em>not</em> returned for colldingOnly == TRUE!!
	 * @param forUnit The unit for which we test. This will not be included
	 * in the returned list. Some tests also depend on this (e.g. on
	 * @ref UnitBase::isFlying), if collidingOnly == TRUE. Ignored if you
	 * provide NULL
	 * @param nonUnit If not NULL all non-unit items are placed here. Used
	 * especially by @ref items to prevent code-duplication and to improve
	 * speed
	 * @param includeMoving Also include moving units in the returned list
	 * if TRUE, otherwise not
	 * @return The units on this cell
	 **/
	QValueList<Unit*> units(bool collidingOnly = true, bool includeMoving = true, Unit* forUnit = 0, QValueList<QCanvasItem*>* nonUnit = 0) const;
	
	/**
	 * @param collidingOnly if TRUE return only items that are interesting
	 * for collision detection
	 * @param forUnit The unit for which we test. This will not be included
	 * in the returned list. Some tests also depend on this (e.g. on
	 * @ref UnitBase::isFlying), if collidingOnly == TRUE. Ignored if you
	 * provide NULL
	 * @param includeMoving Also include moving units in the returned list
	 * if TRUE, otherwise not
	 * @return The items on this cell
	 **/
	QValueList<QCanvasItem*> items(bool collidingOnly = true, bool includeMoving = true, Unit* forUnit = 0) const;

	/**
	 * Please note that this function might get performance improvements!
	 * (see code)
	 * 
	 * @param forUnit the unit for which we test whether this is occupied.
	 * If non-null then it gets removed from the list of units.
	 * @param includeMoving Consider moving units as collisions, if TRUE,
	 * otherwise not
	 * @return if items(true, flying).count() != 0 
	 **/
	bool isOccupied(Unit* forUnit = 0, bool includeMoving = true) const;

protected:
	/**
	 * You are meant to use e.g. @ref appendUnit but not append. We change the
	 * access permissions to protected here to avoid adding of a
	 * QCanvasItem* directly.
	 **/
	QValueList<QCanvasItem*>::append;

	/**
	 * See above. You are meant to use e.g. @ref removeUnit
	 **/
	QValueList<QCanvasItem*>::remove;
};

#endif
