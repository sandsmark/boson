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
#ifndef __BOSON_H__
#define __BOSON_H__

#include <kgame/kgame.h>

class Player;
class QCanvas;
class Unit;
class Facility;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Boson : public KGame
{
	Q_OBJECT
public:
	enum PropertyIds {
		IdGameSpeed = 10000, // dont wanna #include <kgameproperty.h> - better: KGamePropertyBase::IdUser+...
		IdNextUnitId = 10001
	};
	
	Boson(QObject* parent);
	Boson(QObject* parent, const QString& fileName);
	~Boson();

	void setCanvas(QCanvas*); 
	
	void quitGame();
	void startGame();

	int gameSpeed() const;
	bool isServer() const;

	virtual KPlayer* createPlayer(int rtti, int io, bool isVirtual);

	Unit* createUnit(int unitType, Player* owner); // public for Player::load

public slots:
	void slotSetGameSpeed(int speed);

	/**
	 * Doesn't actually add a unit to the game but sends a message to the
	 * network. The actual adding is done in @ref slotNetworkData
	 * @param unitType The type of the unit (see @ref UnitProperties::typeId) to be added
	 * @param x The x-coordinate (on the canvas) of the unit
	 * @param y The y-coordinate (on the canvas) of the unit
	 * @param owner The owner of the new unit.
	 **/
	void slotSendAddUnit(int unitType, int x, int y, Player* owner);

	void slotAdvanceComputerPlayers();

signals:
	/**
	 * Start a scenario. This should be done after loading map and scenario
	 * (a scenario is loaded first, <em>then</em> started). It is
	 * implemented using @ref KGame::sendMessage as the map must be loaded
	 * this way as well.
	 **/
	void signalStartScenario();

	/**
	 * @param unit The unit to be added
	 * @param x x-coordinate of the unit on the canvas
	 * @param y y-coordinate of the unit on the canvas
	 **/
	void signalAddUnit(Unit* unit, int x, int y);

	/**
	 * Order the canvas to call @ref QCanvas::advance
	 **/
	void signalAdvance();

	void signalInitMap(const QByteArray&);

	/**
	 * Emitted when a new map in the new game dialog is selected
	 **/
	void signalMapChanged(const QString& mapIdentifier);

	/**
	 * Emitted when a new scenario in the new game dialog is selected
	 **/
	void signalScenarioChanged(const QString& scenarioIdentifier);

	void signalInitFogOfWar();

	/**
	 * Emitted when the game is started and ready for use - i.e. all units
	 * are added and so
	 **/
	void signalGameStarted();

	/**
	 * Emitted when a factory has started to produce a unit. As a rection
	 * the cmd panel should grey out the order buttons or maybe completely
	 * disable them.
	 **/
	void signalProduceUnit(Facility* factory);

	/**
	 * Emitted when a factory has no units left to produce. Especially
	 * useful to re-enable the disabled orderbuttons. See @ref
	 * signalProduceUnit
	 **/
	void signalCompletedProduction(Facility* factory);

	void signalNotEnoughMinerals(Player* p);
	void signalNotEnoughOil(Player* p);

protected:
	virtual bool playerInput(QDataStream& stream, KPlayer* player);

	unsigned long int nextUnitId();

	/**
	 * @param unitId The unit to search for
	 * @param searchIn The player to search the unit in. 0 for all players
	 **/
	Unit* findUnit(unsigned long int unitId, Player* searchIn) const;

	/**
	 * The factory is told to build a unit.
	 * @param factory Where the unit is being build
	 * @param unitType which type of unit being buid. See @ref
	 * UnitProperties::typeId
	 * @param x The x-coordinate of the new unit.
	 * @param y The y-coordinate of the new unit.
	 **/
	bool buildUnit(Facility* factory, int unitType, int x, int y);

	/**
	 * Create a new unit. No resources of the player are reduced, the unit
	 * is created immediately.
	 **/
	Unit* addUnit(int unitType, Player* owner, int x, int y);

protected slots:
	/**
	 * A network message arrived. Most game logic stuff is done here as
	 * nearly all functions just send a network request instead of doing the
	 * task theirselfes. This way we ensure that a task happens on
	 * <em>all</em> clients.
	 **/
	void slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 receiver, Q_UINT32 sender);

	/**
	 * Send an advance message. When this is received by the clients @ref
	 * QCanvas::advance ist called.
	 **/
	void slotSendAdvance();

	void slotSave(QDataStream& stream);
	void slotLoad(QDataStream& stream);

	void slotReplacePlayerIO(KPlayer* player, bool* remove);

	void slotPlayerJoinedGame(KPlayer*);
	void slotPlayerLeftGame(KPlayer*);

private:
	class BosonPrivate;
	BosonPrivate* d;
};

#endif
