/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef DEFINES_H
#define DEFINES_H

#include <config.h>

#include <kgame/kgameversion.h>

#include "global.h"

// both must be > 0.0:
#define BO_GL_NEAR_PLANE 0.0625f
// AB: the largest possible map is 500x500. if you are at one corner (say
// topleft) you need to see the opposite corner (bottomright), so we need at
// least sqrt(500*500+500*500)=707. take the camera z into account (say 100) and
// you need at least sqrt(707*707+100*100)=714.
// so a value of 1000 should be sufficient for now. (04/02/11)
// AB: note that using 2^n values leads to higher precision (same for the NEAR
//     plane: use 2^-m values)
#define BO_GL_FAR_PLANE 1024.0f


#define BOSON_COOKIE 992
#define BOSON_PORT 5454


// magic cookies for binary files
#define BOSONMAP_MAP_MAGIC_COOKIE QString::fromLatin1("BosonMap")
#define BOSONMAP_TEXMAP_MAGIC_COOKIE QString::fromLatin1("BoTexMap")


#define BOSON_MAX_PLAYERS 10 // test if this is working - 2 is tested

#define MAX_GAME_SPEED 25 // the advance period - lower means faster
#define MIN_GAME_SPEED 2 // the advance period - higher means slower

#define REMOVE_WRECKAGES_TIME 10 // remove wreckages after a certain time (after 10 secs with normal game speed)
#define MAX_SHIELD_RELOAD_COUNT 10 // number of advance calls after that shield get reloaded by 1
#define MAX_WEAPONS_PER_UNIT 100

#define BUILD_RANGE 10 // units can be placed within 10 tiles from its factory
#define REFINERY_FORBID_RANGE 6 // refineries must be at least 6 cells away from resource mines

#define BOSON_MINIMUM_WIDTH 640
#define BOSON_MINIMUM_HEIGHT 480
#define MAX_PROFILING_ENTRIES 300


// random values for map - must be replaced by useful values! afaics these are
// cell values
#define MAX_MAP_HEIGHT 500
#define MAX_MAP_WIDTH 500

// the placement preview. alpha apply to both previews, the _DISALLOW_COLOR
// only if the facility can't placed at that point.
// AB: the red component will alway be 255 anyway.
#define PLACEMENTPREVIEW_ALPHA 120
#define PLACEMENTPREVIEW_DISALLOW_COLOR 100 // should get used for the green and blue component


// default values
// used for both BosonConfig and OptionsDialog
#define DEFAULT_GAME_SPEED 5

// not in BosonConfig (not necessarily in OptionsDialog):
#define DEFAULT_CHEAT_MODE true // by default cheating is *enabled* (debugging)

#endif

