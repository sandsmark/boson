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

#define BO_TILE_SIZE 48
#define BO_GL_CELL_SIZE 1.0


#define BOSON_COOKIE 992
#define BOSON_PORT 5454

#define MAXIMAL_ADVANCE_COUNT 20 // maximal value of advanceCount param emitted by Boson::signalAdvance

#define BOSON_MAX_PLAYERS 10 // test if this is working - 2 is tested

#define MAX_GAME_SPEED 25 // the advance period - lower means faster
#define MIN_GAME_SPEED 2 // the advance period - higher means slower

#define REMOVE_WRECKAGES_TIME 10 // remove wreckages after a certain time (after 10 secs with normal game speed)

#define BUILD_RANGE 5 * BO_TILE_SIZE // units can be placed within 10 tiles from its factory

#define BOSON_MINIMUM_WIDTH 640
#define BOSON_MINIMUM_HEIGHT 480


// random values for map - must be replaced by useful values! afaics these are
// cell values
#define MAX_MAP_HEIGHT 500
#define MAX_MAP_WIDTH 500


// default values
// used for both BosonConfig and OptionsDialog
#define DEFAULT_SOUND true
#define DEFAULT_MUSIC true
#define DEFAULT_USE_RMB_MOVE true
#define DEFAULT_USE_MMB_MOVE true
#define DEFAULT_GAME_SPEED 5
#define DEFAULT_CURSOR CursorOpenGL
#define DEFAULT_CURSOR_EDGE_SENSITY 20
#define DEFAULT_ARROW_SCROLL_SPEED 10
#define DEFAULT_CMD_BUTTONS_PER_ROW 3
#define DEFAULT_MINIMAP_SCALE 2.0
#define DEFAULT_MINIMAP_ZOOM 1.0
#define DEFAULT_UPDATE_INTERVAL 20
#define DEFAULT_CHAT_SCREEN_REMOVE_TIME 10
#define DEFAULT_CHAT_SCREEN_MAX_ITEMS 5
#define DEFAULT_MAGNIFICATION_FILTER GL_LINEAR
#define DEFAULT_MINIFICATION_FILTER GL_LINEAR
#define DEFAULT_MIPMAP_MINIFICATION_FILTER GL_NEAREST_MIPMAP_NEAREST

#endif

