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
	BosonScenario(const QString& fileName);
	~BosonScenario();

	static QStringList availableScenarios();
	static QStringList availableScenarios(const QString& map);

	int maxPlayers() const;
	unsigned int minPlayers() const;

	/**
	 * Load the specified scenario from a file. Note that this is just about
	 * the units - you have to load the map separately using @ref BosonMap!
	 *
	 * Note that BosonScenario does <em>not</em> check whether the scenario is
	 * valid for the current map!
	 * @param fileName the absolute filename of the map file.
	 **/
	bool loadScenario(const QString& fileName);

	bool saveScenario(const QString& fileName);
	
	/**
	 * @return The (hardcoded) default map
	 **/
	static QString defaultScenario();

	/**
	 * Add all available player units to the game, add minerals, ...
	 *
	 * This is like calling @ref initPlayer for all players 
	 * (0..maxPlayers()).
	 **/
	void startScenario(Boson* boson);

	bool isValid() const;

	static QString scenarioFileName(const QString& scenarioIdentifier);

protected:
	/**
	 * Add the units of this player to boson, add minerals, ... 
	 *
	 * See @ref Boson::slotSendAddUnit
	 * @param player Player number. 0..maxPlayers() 
	 **/
	void initPlayer(Boson* boson, int playerNumber);

	/**
	 * Save the scenario as XML to dev
	 **/
	bool saveXMLScenario(QIODevice* dev);
	
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
