/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __BOSONSCENARIO_H__
#define __BOSONSCENARIO_H__

#include <qstring.h>
#include <qdatastream.h>

class UnitBase;
class Boson;
class Player;
class QDomElement;
class QStringList;

class BosonScenarioPrivate;

class BosonScenario
{
public:
	/*
	enum PropertyId {
		IdMaxPlayers = 0,
		IdMinPlayers = 1,
		IdMapHeight = 2,
		IdMapWidth = 3,
	};*/

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

	bool saveScenario(const QString& fileName, bool binary = false);
	
	/**
	 * @return The (hardcoded) default map
	 **/
	static QString defaultScenario();

	/**
	 * Add the units of this player to boson. See @ref Boson::slotSendAddUnit
	 * @param player Player number. 0..maxPlayers() 
	 **/
	void addPlayerUnits(Boson* boson, int playerNumber);

	/**
	 * Add all available player units to the game. This is like
	 * calling @ref addPlayerUnits for all players (0..maxPlayers()).
	 **/
	void startScenario(Boson* boson);

	bool isValid() const;

	static QString scenarioFileName(const QString& scenarioIdentifier);

protected:
	/**
	 * Save the scenario to the stream.
	 **/
	bool saveScenario(QDataStream& stream);

	/**
	 * Save the scenario as XML to dev
	 **/
	bool saveXMLScenario(QIODevice* dev);
	
	/**
	 * Add unit to the game. See also @ref Boson::slotSendAddUnit
	 **/
	void addUnit(Boson* boson, Player* owner, int unitType, int x, int y);

	/**
	 * Read the magic string from stream.
	 * @return TRUE if the magic string matches the expected value.
	 * Otherwise FALSE (not a boson map file)
	 **/
	bool verifyScenario(QDataStream& stream); // TODO
	bool loadScenarioSettings(QDataStream& stream);
	bool loadPlayers(QDataStream& stream);
	bool loadPlayer(QDataStream& stream, unsigned int playerNumber);


	bool saveValidityHeader(QDataStream& stream);
	bool saveScenarioSettings(QDataStream& stream);
	bool savePlayers(QDataStream& stream);
	bool savePlayer(QDataStream& stream, unsigned int playerNumber);

	bool saveScenarioSettings(QDomElement&);
	bool savePlayers(QDomElement&);
	bool savePlayer(QDomElement&, unsigned int playerNumber);

	bool loadScenarioSettings(QDomElement&);
	bool loadPlayers(QDomElement&);
	bool loadPlayer(QDomElement&);


private:
	void init();

private:
	BosonScenarioPrivate* d;
};

#endif
