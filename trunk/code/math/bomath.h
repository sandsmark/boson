/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// note the copyright above: this is LGPL!

#ifndef BOMATH_H
#define BOMATH_H

#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * This file contains some maths functions and classes for boson.
 *
 * This file should be kept simple, I don't want it to depend on BosonConfig or
 * anything like that. Even QDom is not allowed here.
 * Please keep it like that, so that we can include this file everywhere without
 * drastically increasing the dependencies.
 */

// Define this when you want bofixed to use normal floats (so that essentially
//  bofixed = float)
//#define BOFIXED_IS_FLOAT



/**
 * Prints @p f in binary form followed by human readable sign, exponent and
 * mantissa.
 * (use (sign ? 1 : -1) * m * 2^e to get the actual number)
 **/
void floatToBin(float f);

// AB: we bofixed should be used as a replacement of float, so it should be
// accessed as simple as possible, so it should be included here
#include "math/bofixed.h"

#endif

