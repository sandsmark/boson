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

#if HAVE_KGAME_VERSION_H
#include <kgame/kgameversion.h>
#endif

#include "global.h"

#define BO_TILE_SIZE 48
#define BO_GL_CELL_SIZE 1.0

// both must be > 0.0:
#define BO_GL_NEAR_PLANE 1.0f
// AB: the largest possible map is 500x500. if you are at one corner (say
// topleft) you need to see the opposite corner (bottomright), so we need at
// least sqrt(500*500+500*500)=707. take the camera z into account (say 100) and
// you need at least sqrt(707*707+100*100)=714.
// so a value of 1000 should be sufficient for now. (04/02/11)
#define BO_GL_FAR_PLANE 1000.0f


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

#define BUILD_RANGE 10 * BO_TILE_SIZE // units can be placed within 10 tiles from its factory

#define BOSON_MINIMUM_WIDTH 640
#define BOSON_MINIMUM_HEIGHT 480
#define MAX_PROFILING_ENTRIES 300


// random values for map - must be replaced by useful values! afaics these are
// cell values
#define MAX_MAP_HEIGHT 500
#define MAX_MAP_WIDTH 500

// playfield that gets used by default
#define DEFAULT_PLAYFIELD QString::fromLatin1("basic2.bpf")

// the placement preview. alpha apply to both previews, the _DISALLOW_COLOR
// only if the facility can't placed at that point.
// AB: the red component will alway be 255 anyway.
#define PLACEMENTPREVIEW_ALPHA 120
#define PLACEMENTPREVIEW_DISALLOW_COLOR 100 // should get used for the green and blue component


// default values
// used for both BosonConfig and OptionsDialog
#define DEFAULT_SOUND true
#define DEFAULT_MUSIC true
#define DEFAULT_USE_RMB_MOVE true
#define DEFAULT_USE_MMB_MOVE true
#define DEFAULT_GAME_SPEED 5
#define DEFAULT_CURSOR CursorOpenGL
#define DEFAULT_CURSOR_DIR QString::null // QString::null means use BosonCursor::defaultTheme
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
#define DEFAULT_MIPMAP_MINIFICATION_FILTER GL_LINEAR_MIPMAP_NEAREST
#define DEFAULT_ALIGN_SELECTION_BOXES true
#define DEFAULT_RMB_MOVES_WITH_ATTACK true
#define DEFAULT_MOUSE_WHEEL_ACTION CameraZoom
#define DEFAULT_MOUSE_WHEEL_SHIFT_ACTION CameraRotate
#define DEFAULT_DEACTIVATE_WEAPON_SOUNDS false
#define DEFAULT_USE_LIGHT true // not really supported yet
#define DEFAULT_USE_MATERIALS false
#define DEFAULT_USE_MIPMAPS_FOR_MODELS true
#define DEFAULT_TOOLTIP_UPDATE_PERIOD 300
#define DEFAULT_TOOLTIP_CREATOR 1 // FIXME: should be BoToolTipCreator::Extended, but I don't want to include the file here
#define DEFAULT_USE_LOD true
#define DEFAULT_USE_VBO false  // Nvidia drivers doesn't properly support VBOs :-(

// not in BosonConfig (not necessarily in OptionsDialog):
#define DEFAULT_CHEAT_MODE true // by default cheating is *enabled* (debugging)
#define DEFAULT_GROUND_RENDERER "BoDefaultGroundRenderer"
#define DEFAULT_MESH_RENDERER "BoMeshRendererVertexArray"

#endif

