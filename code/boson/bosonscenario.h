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

class Boson;
class Player;
class UnitBase;
class Unit;
class Facility;
class MobileUnit;

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

	/**
	 * Load the scenario from node. 
	 *
	 * Note that this is not actually loaded, but should be fully parsed
	 * only (errors are reported). Actually we copy the node to a local xml
	 * document which is loaded in @ref startScenario.
	 **/
	bool loadScenario(QDomElement& node);

	/**
	 * Save the local xml document (i.e. the scenario) to root.
	 **/
	bool saveScenario(QDomElement& root);

	/**
	 * Clear the local xml document and apply the scenario that is
	 * currently in boson.
	 **/
	void applyScenario(Boson* boson);
	
	/**
	 * Add all available player units to the game, add minerals, ...
	 **/
	void startScenario(Boson* boson);

	bool isValid() const;



	static bool saveUnit(QDomElement& node, Unit* unit);
	static bool loadUnit(QDomElement& node, Unit* unit);

	static bool saveBasicUnit(QDomElement& node, int unitType, unsigned int x, unsigned int y);
	static bool loadBasicUnit(QDomElement& node, int& unitType, unsigned int& x, unsigned int& y);

	static bool savePlayer(QDomElement& node, Player* p);
	static bool loadPlayer(QDomElement& node, Player* p);

protected:
	bool saveScenarioSettings(QDomElement&);
	bool loadScenarioSettings(QDomElement&);

	static bool saveFacility(QDomElement&, Facility*);
	static bool loadFacility(QDomElement&, Facility*);
	static bool saveMobile(QDomElement&, MobileUnit*);
	static bool loadMobile(QDomElement&, MobileUnit*);

private:
	void init();

private:
	class BosonScenarioPrivate;
	BosonScenarioPrivate* d;
};

#endif
