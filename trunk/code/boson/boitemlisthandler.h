/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOITEMLISTHANDLER_H
#define BOITEMLISTHANDLER_H

class BoItemList;
class BoItemListHandlerPrivate;

#include <qobject.h>

/**
 * The item list handler cares about proper deletion of @ref BoItemList objects.
 * @ref BoItemList will register to this class (you need only a single object of
 * the handler per program). The handler then takes care that the list is
 * deleted as soon as the program returns to the event loop.
 *
 * BoItemListHandler uses a list internally, meaning that registering lists to
 * the handler can be done very fast (constant time).
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoItemListHandler : public QObject
{
	Q_OBJECT
public:
	BoItemListHandler();
	~BoItemListHandler();

	static BoItemListHandler* itemListHandler();

	void registerList(BoItemList* list);
	void unregisterList(BoItemList* list);


public slots:
	/**
	 * This immediately deletes all lists that have been reported to the
	 * handler using @ref registerList.
	 **/
	void slotDeleteLists();

private:
	BoItemListHandlerPrivate* d;
};

#endif

