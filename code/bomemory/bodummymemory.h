/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BODUMMYMEMORY_H
#define BODUMMYMEMORY_H

#include <config.h>

#ifdef BOSON_USE_BOMEMORY

// AB: bomemory.h redefines "new" using macros. therefore we have to include
// files that redefine new using Class::operator new() before bomemory.h
// -> same about <new>
#include <string> // includes <new> and some other relevant files

#include <bomemory/bomemory.h>

#endif

#endif

