/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann <b_mann@gmx.de>

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

#include "bocurrentinfo.h"
#include "bodebug.h"

#include "../../bomemory/bodummymemory.h"
#include "boinfo.h"

BoCurrentInfo::BoCurrentInfo()
{
 mInfo = BoInfo::boInfo();
 if (!mInfo) {
	boError() << k_funcinfo << "global BoInfo object not yet initialized" << endl;
 }
}

BoCurrentInfo::~BoCurrentInfo()
{
}

float BoCurrentInfo::cpuSpeed() const
{
 if (!mInfo) {
	return 0.0f;
 }
 return mInfo->cpuSpeed();
}

