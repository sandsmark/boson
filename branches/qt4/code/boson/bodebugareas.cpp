/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "bodebugareas.h"

#include "bodebug.h"

void BoDebugAreas::initAreas()
{
 BoDebug::registerAreaName(100, QString::fromLatin1("BosonModel"));
 BoDebug::registerAreaName(110, QString::fromLatin1("Boson Textures"));
 BoDebug::registerAreaName(120, QString::fromLatin1("BoLOD"));
 BoDebug::registerAreaName(121, QString::fromLatin1("BoLOD verbose"));
 BoDebug::registerAreaName(130, QString::fromLatin1("BoShader"));
 BoDebug::registerAreaName(150, QString::fromLatin1("Boson Particles"));
 BoDebug::registerAreaName(200, QString::fromLatin1("Boson Sound"));
 BoDebug::registerAreaName(210, QString::fromLatin1("OptionsDialog"));
 BoDebug::registerAreaName(220, QString::fromLatin1("CommandFrame"));
 BoDebug::registerAreaName(230, QString::fromLatin1("Camera"));
 BoDebug::registerAreaName(250, QString::fromLatin1("BosonScenario"));

 // 260 and 270 are equal. the difference has historic reasons only - pick your
 // favourite :)
 // feel free to remove one completely
 BoDebug::registerAreaName(260, QString::fromLatin1("LoadGame"));
 BoDebug::registerAreaName(270, QString::fromLatin1("LoadGame"));
 BoDebug::registerAreaName(300, QString::fromLatin1("Boson Advance"));
 BoDebug::registerAreaName(310, QString::fromLatin1("Collisions"));
 BoDebug::registerAreaName(350, QString::fromLatin1("BosonShot"));
 BoDebug::registerAreaName(360, QString::fromLatin1("Event"));
 BoDebug::registerAreaName(370, QString::fromLatin1("NetworkSync"));
 BoDebug::registerAreaName(380, QString::fromLatin1("Movement"));

 // Use 400-429 for Unit::advance*()
 BoDebug::registerAreaName(401, QString::fromLatin1("AdvanceMove"));

 // Use 430-459 for UnitPlugin's advance() methods
 BoDebug::registerAreaName(430, QString::fromLatin1("Harvester Advance"));

 BoDebug::registerAreaName(500, QString::fromLatin1("BosonPath"));
 BoDebug::registerAreaName(600, QString::fromLatin1("Boson Upgrades"));
 BoDebug::registerAreaName(610, QString::fromLatin1("Ammunition"));

 BoDebug::registerAreaName(700, QString::fromLatin1("BoScript"));
 BoDebug::registerAreaName(800, QString::fromLatin1("PluginManager"));



 BoDebug::registerAreaName(11000, QString::fromLatin1("libkdegames"));
 BoDebug::registerAreaName(11001, QString::fromLatin1("libkdegames (KGame)"));
}

