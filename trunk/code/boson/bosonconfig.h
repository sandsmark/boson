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
#ifndef __BOSONCONFIG_H__
#define __BOSONCONFIG_H__

#include <qstring.h>

class KConfig;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonConfig
{
public:
	BosonConfig() { }
	~BosonConfig() { }

	static QString localPlayerName(KConfig* conf = 0);
	static void saveLocalPlayerName(const QString& name, KConfig* conf = 0);

	static void saveGameSpeed(int speed, KConfig* conf = 0);
	static int gameSpeed(KConfig* conf = 0);

	static void saveCommandFramePosition(int pos, KConfig* conf = 0);
	static int commandFramePosition(KConfig* conf = 0);

protected:
	static void changeGroupGeneral(KConfig* conf);
};

#endif
