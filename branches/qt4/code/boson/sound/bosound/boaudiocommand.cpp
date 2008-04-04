/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "boaudiocommand.h"

#include <qdeepcopy.h>

BoAudioCommand::BoAudioCommand(int command, int dataInt, const QString& dataString1, const QString& dataString2)
{
 mCommand = command;
 mDataInt = dataInt;
 mDataString1 = QDeepCopy<QString>(dataString1);
 mDataString2 = QDeepCopy<QString>(dataString2);
}

BoAudioCommand::BoAudioCommand(int command, const QString& species, int dataInt, const QString& dataString1, const QString& dataString2)
{
 mCommand = command;
 mDataInt = dataInt;
 mDataString1 = QDeepCopy<QString>(dataString1);
 mDataString2 = QDeepCopy<QString>(dataString2);
 mSpecies = QDeepCopy<QString>(species);
}


