/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONSCENARIO_H
#define BOSONSCENARIO_H

#include <qstring.h>

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

	/**
	 * Set the minimal/maximal player numbers. Mostly used by the editor.
	 **/
	void setPlayers(unsigned int min, int max);

	/**
	 * @return The maximal number of players or -1 for unlimited.
	 **/
	int maxPlayers() const;

	/**
	 * @return The minimal number of players for this scenario.
	 **/
	unsigned int minPlayers() const;

	/**
	 * Load the scenario from a @ref QDomDocument. This string @p xml should
	 * come from a @ref saveScenarioToDocument. We use @ref loadScenario to
	 * actually load the scenario.
	 * @return TRUE on success, otherwise FALSE.
	 **/
	bool loadScenarioFromDocument(const QString& xml);

	/**
	 * Load the scenario from node.
	 *
	 * Note that this is not actually loaded, but should be fully parsed
	 * only (errors are reported). Actually we copy the node to a local xml
	 * document which is loaded in @ref startScenario.
	 **/
	bool loadScenario(const QDomElement& node);

	/**
	 * Create a scenario according to the current scenario settings. For
	 * example this will create @ref maxPlayers player nodes and give them
	 * the default value of oil/minerals and so on.
	 *
	 * This should be used on editor startup (at least currently), when
	 * creating a new map. See also @ref applyScenario, which should be
	 * used when saving a scenario.
	 * @return TRUE on success, otherwise FALSE
	 **/
	bool initializeScenario();

	/**
	 * Create a temporary @ref QDomDocument and save the scenario to it. See
	 * also @ref saveScenario.
	 *
	 * @return QDomDocument::toString() for the created scenario document,
	 * or @ref QString::null if an error occured.
	 **/
	QString saveScenarioToDocument() const;

	/**
	 * Save the local xml document (i.e. the scenario) to @p root.
	 **/
	bool saveScenario(QDomElement& root) const;

	/**
	 * Clear the local xml document and apply the scenario that is
	 * currently in boson.
	 **/
	void applyScenario(const Boson* boson);
	
	/**
	 * Add all available player units to the game, add minerals, ...
	 **/
	bool startScenario(Boson* boson) const;

	bool isValid() const;


	static bool loadUnit(const QDomElement& node, Unit* unit);
	static bool loadBasicUnit(const QDomElement& node, unsigned long int& unitType, unsigned int& x, unsigned int& y);

	static bool saveUnit(QDomElement& node, const Unit* unit);

	static bool saveBasicUnit(QDomElement& node, unsigned long int unitType, unsigned int x, unsigned int y);

	static bool savePlayer(QDomElement& node, const Player* p);


	bool modified() const { return mModified; }
	void setModified(bool m) { mModified = m; }

protected:
	bool saveScenarioSettings(QDomElement&) const;
	bool loadScenarioSettings(const QDomElement&);

	static bool saveFacility(QDomElement&, const Facility*);
	static bool saveMobile(QDomElement&, const MobileUnit*);

	/**
	 * Initialize the player node, i.e. set the default values for
	 * attributes (minerals, oil, ...)
	 **/
	void initPlayerNode(QDomElement& player, unsigned int playerNumber);

private:
	friend class BosonScenarioBuilder;
	void init();

private:
	class BosonScenarioPrivate;
	BosonScenarioPrivate* d;

	bool mModified;
};

#endif
