/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOVERSION_H
#define BOVERSION_H

#define BOSON_VERSION_MAJOR 0x00
#define BOSON_VERSION_MINOR 0x09
#define BOSON_VERSION_MICRO 0x01
#define BOSON_VERSION_STRING "0.9.1pre"

#define BOSON_VERSION ((BOSON_VERSION_MAJOR << 16) | (BOSON_VERSION_MINOR << 8) | BOSON_VERSION_MICRO)

// savegame versions
#define BOSON_MAKE_SAVEGAME_FORMAT_VERSION( a,b,c ) ( ((a) << 16) | ((b) << 8) | (c) )

// below we have a collection of "historic" versions, which will _never_ change
// again. they can be used for file converters.

#define BOSON_SAVEGAME_FORMAT_VERSION_0_8 \
	( BOSON_MAKE_SAVEGAME_FORMAT_VERSION (0x00, 0x01, 0x12) )

// version from boson 0.8.128 (development version that got never released)
#define BOSON_SAVEGAME_FORMAT_VERSION_0_8_128 \
	( BOSON_MAKE_SAVEGAME_FORMAT_VERSION (0x00, 0x02, 0x00) )

#define BOSON_SAVEGAME_FORMAT_VERSION_0_9 \
	( BOSON_MAKE_SAVEGAME_FORMAT_VERSION (0x00, 0x02, 0x01) )

#endif

