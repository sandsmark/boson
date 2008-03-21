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

#ifndef BOSON_BOMATH_H
#define BOSON_BOMATH_H

// this file serves as a convenience include to math/bomath.h only.
// in addition, it provides Boson/KDE/Qt extensions to the things in
// math/bomath.h
//
// do NOT add anything else in here!

#include "math/bomath.h"

QDataStream& operator<<(QDataStream& stream, const bofixed& f);
QDataStream& operator>>(QDataStream& stream, bofixed& f);

#endif

