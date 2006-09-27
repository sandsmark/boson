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

#ifndef BOSONITEMPROPERTYHANDLER_H
#define BOSONITEMPROPERTYHANDLER_H

#include <kgame/kgamepropertyhandler.h>

class BosonItemProperties;

/**
 * In @ref Player we need to know which unit/item belongs to a @ref
 * KGamePropertyHandler when it emits a signal indicating that a property has
 * changed. Unfortunately iterating the list of all units can take a very long
 * time (about 50-100ms for about 1000 units. this is a lot, since it happens
 * very often!). For this there is this class.
 *
 * All it does it to provide easy access to the owner of this handler - see @ref
 * item.
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Class that provides direct acces to the parent item of a @ref
 * KGamePropertyHandler
 **/
class BosonItemPropertyHandler : public KGamePropertyHandler
{
	Q_OBJECT
public:
	/**
	 * Construct a @ref KGamePropertyHandler with parent @p parent. Note
	 * that since @ref BosonItem isn't a @ref QObject there will be no auto
	 * deletion!
	 *
	 * You can retrieve the parent of this property handler using @ref item
	 **/
	BosonItemPropertyHandler(BosonItemProperties* parent) : KGamePropertyHandler()
	{
		mItem = parent;
	}

	/**
	 * @return The owner of this handler. You can safely cast it to @ref
	 * BosonItem (and to anything else if you take
	 * @ref BosonItemPropreties::rtti into account)
	 **/
	BosonItemProperties* item() const { return mItem; }

private:
	BosonItemProperties* mItem;
};

#endif

