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
#ifndef BOSON_H
#define BOSON_H

#include <kgame/kgame.h>

class Player;
class Unit;
class ProductionPlugin;
class BosonCanvas;
class QDomElement;
class BosonPlayField;

#define boGame Boson::boson()

/**
 * This is probably the most important class in Boson.
 * It is derived from KGame, so it is responsible for handling players' input
 * and all the network stuff.
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Boson : public KGame
{
	Q_OBJECT
public:
	enum PropertyIds {
		IdGameSpeed = 10000, // dont wanna #include <kgameproperty.h> - better: KGamePropertyBase::IdUser+...
		IdNextUnitId = 10001,
		IdAdvanceCount = 10002,
		IdAdvanceFlag = 10003
	};

	/**
	 * Describes current status when loading saved game
	 * This is mostly used for error checking
	 *
	 * Possible values:
	 * @li NotLoaded - Game is not yet loaded (loading method isn't called yet)
	 * @li LoadingInProgress - Loading is in progress
	 * @li LoadingCompleted - Loading is completed (successfully)
	 * @li InvalidFileFormat - File format was invalid (error)
	 * @li InvalidCookie - Cookie in the file header was invalid (error)
	 * @li InvalidVersion - SaveGame version was invalid. Probably the game was saved with too old version of Boson (error)
	 * @li KGameError - Error while loading KGame stuff
	 **/
	enum LoadingStatus {
		NotLoaded = 1,
		LoadingInProgress,
		LoadingCompleted,
		InvalidFileFormat,
		InvalidCookie,
		InvalidVersion,
		KGameError
	};

	Boson(QObject* parent);
	Boson(QObject* parent, const QString& fileName);
	~Boson();

	void setCanvas(BosonCanvas*);
	void setPlayField(BosonPlayField*);

	void setLocalPlayer(Player*);
	Player* localPlayer();

	void quitGame();
	void startGame();

	int gameSpeed() const;
	bool isServer() const;

	virtual KPlayer* createPlayer(int rtti, int io, bool isVirtual);
	void removeAllPlayers();

	Unit* createUnit(unsigned long int unitType, Player* owner); // public for Player::load
	Unit* loadUnit(unsigned long int unitType, Player* owner);

	QValueList<QColor> availableTeamColors() const;

	/**
	 * The factory completed to produce a unit and is now told to build
	 * (place) it.
	 * @param factory Where the unit is being build
	 * @param unitType which type of unit being buid. See @ref
	 * UnitProperties::typeId
	 * @param x The x-coordinate of the new unit.
	 * @param y The y-coordinate of the new unit.
	 **/
	bool buildProducedUnit(ProductionPlugin* factory, unsigned long int unitType, int x, int y);

	/**
	 * Behaves slightly similar to @ref slotSendAddUnit but this function
	 * takes an xml document and you can add several units at once.
	 **/
	void sendAddUnits(const QString& xmlDocument, Player* owner);

	virtual void networkTransmission(QDataStream&, int msgid, Q_UINT32 receiver, Q_UINT32 sender, Q_UINT32 clientID);
	virtual void lock();
	virtual void unlock();

	void setGameMode(bool isGame = true) { mGameMode = isGame; }
	
	/**
	 * Note: not yet fully supported!
	 * @return TRUE if we are in game mode or FALSE if we are in editor mode
	 **/
	bool gameMode() const //TODO: disallow all actions in Boson that are invalid for this mode
	{
		return mGameMode;
	}

	virtual bool savegame(QDataStream& stream, bool network, bool saveplayers = true);
	virtual bool load(QDataStream& stream, bool reset = true);
	virtual bool loadgame(QDataStream& stream, bool network, bool reset);

	LoadingStatus loadingStatus() const;

	/**
	 * See @ref Unit::advanceFunction2 for more info about this.
	 * @return TRUE if @ref Unit::advanceFunction should get/is called or
	 * FALSE if @ref Unit::advanceFunction2 is active.
	 **/
	bool advanceFlag() const;

	void toggleAdvanceFlag();

	virtual bool save(QDataStream& stream, bool savePlayers = true);

	static Boson* boson() { return mBoson; }
	static void initBoson();
	static void deleteBoson();

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
	void slotSendAddUnit(unsigned long int unitType, int x, int y, Player* owner);

	void slotAdvanceComputerPlayers(unsigned int advanceCount, bool advanceFlag);

	/**
	 * Emits @ref signalUpdateProductionOptions
	 **/
	void slotUpdateProductionOptions();

	/**
	 * Add a system message for the local player only. Note that this does
	 * <em>not</em> send a chat message over network. It is displayed on
	 * this client only.
	 *
	 * You can find this method in this class, because it is accessible from
	 * mostly everywhere, where it might be needed
	 **/
	void slotAddChatSystemMessage(const QString& fromName, const QString& text);

	/**
	 * Convinence method for the above slot. Simply uses "Boson" as
	 * fromName.
	 **/
	void slotAddChatSystemMessage(const QString& text);

signals:
	/**
	 * Emitted once the game admin starts the game, i.e. clicks on "start
	 * game" in the new game widget (see @ref BosonNewGameWidget)
	 *
	 * Now all clients should start loading tiles etc.
	 **/
	void signalStartNewGame();
	
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
	 * Order the canvas to call @ref BosonCanvas::slotAdvance
	 * @param advanceCount The number of this advance call. This is used to
	 * decide what should be done - e.g. there is no need to check for new
	 * enemies every advance call. This value is increased after the signal
	 * was emitted and reset to 0 when a certain value is reached.
	 * @param advanceFlag see @ref advanceFlag
	 **/
	void signalAdvance(unsigned int advanceCount, bool advanceFlag);

	void signalInitMap(const QByteArray&);

	/**
	 * Emitted when a new playfield in the new game dialog is selected
	 **/
	void signalPlayFieldChanged(const QString& playfield);

	/**
	 * This is only interesting for the new game dialog. Emitted when
	 * another species has been chosen.
	 **/
	void signalSpeciesChanged(Player* player);

	/**
	 * This is only interesting for the new game dialog. Emitted when
	 * another species has been chosen.
	 **/
	void signalTeamColorChanged(Player* player);

	void signalInitFogOfWar();

	/**
	 * Emitted when the game is started and ready for use - i.e. all units
	 * are added and so
	 **/
	void signalGameStarted();

	/**
	 * This signal is emitted whenever the production list of factory
	 * changes. This means whenever you add a unit to the queue (see @ref
	 * Factory::productionList), pause a production (using @ref
	 * Unit::setWork(Unit::WorkNone)) or continue a production
	 *
	 * This is also emitted once a production has been completed.
	 **/
	void signalUpdateProduction(Unit* factory);

	/**
	 * Emitted when unit's construction or when technology research has been
	 * completed to ensure that new production options become available.
	 **/
	void signalUpdateProductionOptions();

	void signalChangeCell(int x, int y, int groundType, unsigned char );

	void signalAddChatSystemMessage(const QString& fromName, const QString& text);

protected:
	virtual bool playerInput(QDataStream& stream, KPlayer* player);

	unsigned long int nextUnitId();

	/**
	 * @param unitId The unit to search for
	 * @param searchIn The player to search the unit in. 0 for all players
	 **/
	Unit* findUnit(unsigned long int unitId, Player* searchIn) const;

	/**
	 * Create a new unit. No resources of the player are reduced, the unit
	 * is created immediately.
	 **/
	Unit* addUnit(unsigned long int unitType, Player* owner, int x, int y);

	/**
	 * Create a unit from node. This behaves similar to the above function,
	 * but you can specify <em>every</em> property in the node, not just
	 * type and position. 
	 *
	 * This is used by @ref BosonScenario.
	 **/
	Unit* addUnit(QDomElement& node, Player* owner);

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
	 * BosonCanvasCanvas::slotAdvance ist called.
	 **/
	void slotSendAdvance();

	void slotSave(QDataStream& stream);
	void slotLoad(QDataStream& stream);

	void slotReplacePlayerIO(KPlayer* player, bool* remove);

	void slotPlayerJoinedGame(KPlayer*);
	void slotPlayerLeftGame(KPlayer*);

	void slotProcessDelayed();

	void slotPropertyChanged(KGamePropertyBase*);

	void slotReceiveAdvance();

private:
	class BosonPrivate;
	BosonPrivate* d;

	bool mGameMode;
	static Boson* mBoson;
};

#endif
