/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef DEFINES_H
#define DEFINES_H

#include <config.h>

//#define NO_EDITOR

#define BO_TILE_SIZE 48
#define BO_GL_CELL_SIZE 1.0


#define BOSON_COOKIE 992


// obsolete
#define PIXMAP_PER_MOBILE 9 // 8 different directions + 1 "destroyed" pix
#define PIXMAP_PER_FIX 5 // number of construction steps + 1 "destroyed" pix
#define PIXMAP_FIX_DESTROYED (PIXMAP_PER_FIX - 1)
#define PIXMAP_MOBILE_DESTROYED (PIXMAP_PER_MOBILE - 1)
#define FACILITY_CONSTRUCTION_STEPS (PIXMAP_PER_FIX - 1)
#define SHOT_FRAMES 5
#define MOBILE_SHOT_FRAMES 16
#define FACILITY_SHOT_FRAMES 16
#define MOBILE_SHOT_NB 4 // FIXME hardcoded
#define FACILITY_SHOT_NB 4// FIXME hardcoded
// obsolete (end)

#define MAXIMAL_ADVANCE_COUNT 200 // maximal value of advanceCount param emitted by Boson::signalAdvance

#define BOSON_MAX_PLAYERS 10 // test if this is working - 2 is tested

#define MAX_MAP_HEIGHT 500
#define MAX_MAP_WIDTH 500

#define MAX_GAME_SPEED 25 // the advance period - lower means faster
#define MIN_GAME_SPEED 2 // the advance period - higher means slower
#define DEFAULT_GAME_SPEED 70

#define REMOVE_WRECKAGES_TIME 20 // remove wreckages after a certain time

#define BOSON_MINIMUM_WIDTH 640
#define BOSON_MINIMUM_HEIGHT 480

#define BUILD_RANGE 5 * BO_TILE_SIZE // units can be placed within 10 tiles from its factory

#define BOSON_PORT 5454

#endif

