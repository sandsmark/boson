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

#include "boapplication.h"
#include "boglobal.h"

#include <config.h>

#include <kglobal.h>
#include <kstandarddirs.h>

BoApplication::BoApplication(bool allowStyles)
	: KApplication(allowStyles)
{
 BoGlobal::initStatic();
 BoGlobal::boGlobal()->initGlobalObjects();

 // this is for broken installations. people tend to install to /usr/local or
 // similar (which is 100% correct), but don't set $KDEDIRS (note that S)
 // correct. This is (I guess) a distribution bug in most (all?) distributions
 // out there.
 // we tell KDE here which our prefix is and add it this way to $KDEDIRS
 KGlobal::dirs()->addPrefix(BOSON_PREFIX);

}

BoApplication::~BoApplication()
{
 if (BoGlobal::boGlobal()) {
	BoGlobal::boGlobal()->destroyGlobalObjects();
 }
}

