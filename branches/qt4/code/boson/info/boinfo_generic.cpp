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

#include "boinfo.h"

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"

void BoInfo::updateOSInfo()
{
}

float BoInfo::cpuSpeed() const
{
 return -1.0f;
}

bool BoInfo::haveMtrr() const
{
 return false;
}

bool BoCurrentInfo::memoryInUse(QString* vmSize, QString* vmLck, QString* vmRSS,
		QString* vmData, QString* vmStk, QString* vmExe, QString* vmLib,
		QString* vmPTE) const
{
 Q_UNUSED(vmSize);
 Q_UNUSED(vmLck);
 Q_UNUSED(vmRSS);
 Q_UNUSED(vmData);
 Q_UNUSED(vmStk);
 Q_UNUSED(vmExe);
 Q_UNUSED(vmLib);
 Q_UNUSED(vmPTE);
 return false;
}

bool BoCurrentInfo::cpuTime(unsigned long int* utime, unsigned long int* stime, long int* cutime, long int* cstime) const
{
 Q_UNUSED(utime);
 Q_UNUSED(stime);
 Q_UNUSED(cutime);
 Q_UNUSED(cstime);
 return false;
}

