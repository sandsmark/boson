/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOPUMENUBAR_H
#define BOPUMENUBAR_H

#define PU_USE_NONE
#include <plib/pu.h>

class bopuMenuBarPrivate;
class bopuMenuBar : public puInterface
{
public:
	bopuMenuBar();
	~bopuMenuBar();

	/**
	 * Convenience method for @ref addSubMenu followed by @ref addMenuItem
	 * for all @p items and finally calling @ref puPopupMenu::close.
	 **/
	void add_submenu(const char* str, char* items[], puCallback _cb[], void* _user_data[]);

	puPopupMenu* addSubMenu(const char* str)
	{
		return addSubMenu(0, str);
	}
	puPopupMenu* addSubMenu(puPopupMenu* p, const char* str);
	void closeSubMenu(puPopupMenu* p);

	/**
	 * Call this <em>after</em> @ref addSubMenu. The items will go to the
	 * previoulsy added submenu.
	 *
	 * Remember to @ref puPopupMenu::close() the submenu when you are done
	 * adding items.
	 **/
	void addMenuItem(puPopupMenu* p, const char* text, puCallback cb, void* userData = 0);

	virtual void close();

private:
	bopuMenuBarPrivate* d;
};


#endif

