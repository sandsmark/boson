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
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/**
 * Here you won't find any classes. These are mostly enums which are used at
 * different places in boson by several reasons. Examples are mostly
 * configuration items.
 *
 * I am usually against global variables. Please try not to add any here. There
 * is usually a better way.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/

enum CommandFramePosition {
	CmdFrameLeft = 0,
	CmdFrameRight = 1,
	CmdFrameUndocked = 2
};
enum ChatFramePosition {
	ChatFrameTop = 0,
	ChatFrameBottom = 1
};
enum CursorMode {
	CursorSprite = 0,
	CursorNormal = 1,
	CursorExperimental = 2
};

enum Direction {
	North = 0,
	NorthEast = 1,
	East = 2, 
	SouthEast = 3,
	South = 4,
	SouthWest = 5,
	West = 6,
	NorthWest = 7,
	DirNone = 100 // used by BosonPath
};

/**
 * This enum specifies what kind of unit/tile is ordered by an order button in
 * the command frame.
 **/
enum OrderType {
	OrderFacilities = 0,
	OrderMobiles = 1,
	OrderPlainTiles = 2,
	OrderSmall = 3,
	OrderBig1 = 4,
	OrderBig2 = 5,
	
	OrderLast // should always be the last item - used by loops
};


#endif
