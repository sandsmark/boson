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
#ifndef __BOSONSCENARIO_H__
#define __BOSONSCENARIO_H__

#include <qstring.h>
#include <qptrlist.h>

class UnitBase;
class Boson;
class Player;

class QStringList;
class QIODevice;
class QDomElement;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonScenario
{
public:
	BosonScenario();
	~BosonScenario();

	int maxPlayers() const;
	unsigned int minPlayers() const;

	bool loadScenario(QDomElement& node);

	bool saveScenario(QDomElement& root);

	void applyScenario(Boson* boson);
	
	/**
	 * Add all available player units to the game, add minerals, ...
	 *
	 * This is like calling @ref initPlayer for all players 
	 * (0..maxPlayers()).
	 **/
	void startScenario(Boson* boson);

	bool isValid() const;

protected:
	/**
	 * Add the units of this player to boson, add minerals, ... 
	 *
	 * See @ref Boson::slotSendAddUnit
	 * @param player Player number. 0..maxPlayers() 
	 **/
	void initPlayer(Boson* boson, int playerNumber);

	bool saveScenarioSettings(QDomElement&);
	bool savePlayers(QDomElement&);

	bool loadScenarioSettings(QDomElement&);
	bool loadPlayers(QDomElement&);
	bool loadPlayer(QDomElement&);

private:
	void init();

private:
	class BosonScenarioPrivate;
	BosonScenarioPrivate* d;
};

#endif
