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
#ifndef BOSON_H
#define BOSON_H

#include <kgame/kgame.h>

class Player;
class Unit;
class ProductionPlugin;
class BosonCanvas;
class BosonPlayField;
class QDomElement;
class QDomDocument;
class QDataStream;
class QTextStream;
class BosonSaveLoad;
class BosonStarting;

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
		IdGamePaused = 10001,
		IdNextUnitId = 10005,
		IdAdvanceCount = 10010,
		IdAdvanceFlag = 10011,
		IdAdvanceCallsCount = 10020
	};

protected:
	Boson(QObject* parent);
	~Boson();

public:
	/**
	 * @return The global Boson object
	 **/
	static Boson* boson() { return mBoson; }

	/**
	 * Initialize the global Boson object. See also @ref boson.
	 **/
	static void initBoson();

	/**
	 * Clean up the global Boson object.
	 **/
	static void deleteBoson();

public:
	/**
	 * Create a new @ref BosonCanvas object. The object will be deleted when
	 * this Boson object is destroyed, see @ref deleteBoson.
	 **/
	void createCanvas();

	/**
	 * @return The @ref BosonCanvas object. See also @ref createCanvas.
	 *
	 * Note That I do <em>not</em> want to see a non-const pointer returned
	 * in this class! Boson is a static object that is available
	 * _everywhere_ in this program and I do not want to allow access to the
	 * @ref BosonCanvas object all over this application.
	 *
	 * This pointer is meant to be used for e.g. statistic information (how
	 * many units are on the map, how many particle systems, ...)
	 **/
	const BosonCanvas* canvas() const;
	BosonCanvas* canvasNonConst() const; // FIXME: remove!

	void setPlayField(BosonPlayField*);
	BosonPlayField* playField() const;

	void setLocalPlayer(Player*);
	Player* localPlayer() const;

	/**
	 * Set the object that will get used to start the game. Some starting
	 * relevant network messages could get forwarded directly to this object
	 * and we will store some data from network messages there.
	 *
	 * Note that a NULL @p starting parameter will unset the object and
	 * therefore disable game starting. This can be done once the game has
	 * been started, in order to avoid starting it twice.
	 **/
	void setStartingObject(BosonStarting* starting);

	/**
	 * Initialize a @ref BosonSaveLoad object with the relevant data.
	 **/
	void initSaveLoad(BosonSaveLoad*);

	void quitGame();
	void startGame();

	int gameSpeed() const;
	bool gamePaused() const;

	void removeAllPlayers();

	QValueList<QColor> availableTeamColors() const;

	/**
	 * Kill the @p player, i.e. remove all it's units (if any) from the
	 * game. Once this was called the player is not able to do anything
	 * anymore.
	 *
	 * Note that the @p player will remain in the game, but can watch only
	 * after this point. The player is not removed from the network.
	 **/
	void killPlayer(Player* player);

	/**
	 * @param unitId The unit to search for
	 * @param searchIn The player to search the unit in. 0 for all players
	 **/
	Unit* findUnit(unsigned long int unitId, Player* searchIn) const;

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

	virtual void networkTransmission(QDataStream&, int msgid, Q_UINT32 receiver, Q_UINT32 sender, Q_UINT32 clientID);

	/**
	 * Lock message delivery until @ref unlock is called.
	 *
	 * Messages are not lost, but just delayed.
	 **/
	virtual void lock();

	/**
	 * Unlock message delivery (see @ref lock) and deliver all delayed
	 * messages (see @ref slotProcessDelayed)
	 **/
	virtual void unlock();

	bool isLocked() const;

	/**
	 * Set the game/editor mode. Use TRUE here for normal game mode, FALSE
	 * for editor mode.
	 **/
	void setGameMode(bool isGame = true) { mGameMode = isGame; }
	
	/**
	 * Note: not yet fully supported!
	 * @return TRUE if we are in game mode or FALSE if we are in editor mode
	 **/
	bool gameMode() const //TODO: disallow all actions in Boson that are invalid for this mode
	{
		return mGameMode;
	}

	bool loadFromFile(const QString& file);
	bool saveToFile(const QString& file);

	virtual bool savegame(QDataStream& stream, bool network, bool saveplayers = true);
	virtual bool loadgame(QDataStream& stream, bool network, bool reset);

	/**
	 * @return See @ref BosonSaveLoad::LoadingStatus
	 **/
	int loadingStatus() const;

	/**
	 * See @ref Unit::advanceFunction2 for more info about this.
	 * @return TRUE if @ref Unit::advanceFunction should get/is called or
	 * FALSE if @ref Unit::advanceFunction2 is active.
	 **/
	bool advanceFlag() const;

	/**
	 * Toggle the advance flag. See @ref advanceFlag.
	 **/
	void toggleAdvanceFlag();

	/**
	 * @return The number of delayed messages. When a certain number of
	 * messages has been delayed you are in serious trouble, as there will
	 * be a big delay before additional player input can be executed.
	 **/
	unsigned int delayedMessageCount() const;

	/**
	 * The number of delayed advance messages is really important in a game.
	 * In theory we should have correct timing, which ensures, that no
	 * advance message ever gets delayed. in practice, this just isn't the
	 * case.
	 *
	 * Once an advance message is delayed, everything in the game lags by a
	 * certain amount of time (the interval in which advance messages are
	 * sent). You should do everything to process all delayed messages asap,
	 * as multiple delayed advance messages can be noticed by the user (he
	 * clicks, to make a unit move, but the unit moves after some seconds
	 * only).
	 *
	 * Usually the best way to get rid of delayed advance messages is to
	 * drop frames. Frame rendering takes a huge amount of time!
	 * @return The number of advance messages in @ref delayedMessageCount.
	 **/
	unsigned int delayedAdvanceMessageCount() const;

	/**
	 * @return Number of advance calls that have been made in this game (aka
	 * cycles)
	 **/
	unsigned int advanceCallsCount() const;

	void writeGameLog(QTextStream& log);
	void saveGameLogs(const QString& prefix);


	/**
	 * Init the fog of war. This must not be called anymore, once a game is
	 * started, therefore it is allowed from a @ref BosonStarting object
	 * only.
	 **/
	void initFogOfWar(BosonStarting*);

	/**
	 * Start the scenario. This must not be called anymore, once a game is
	 * started, therefore it is allowed from a @ref BosonStarting object
	 * only.
	 **/
	void startScenario(BosonStarting*);


	unsigned long int nextUnitId();

public: // small KGame extenstions for boson
	/**
	 * Used internally by @ref KGame. Simply create and return a new @ref
	 * Player object.
	 **/
	virtual KPlayer* createPlayer(int rtti, int io, bool isVirtual);

	/**
	 * Simply calls @ref loadgame. This is an old function, that resides
	 * here due to an old KGame bug.
	 **/
	virtual bool load(QDataStream& stream, bool reset = true);

	/**
	 * Dummy implementation that simply calls @ref KGame::save or @ref
	 * savegame, depending on the @ref KGame version. @ref KGame from KDE
	 * 3.0 didn't have @ref savegame, so we need to emulate it on our own
	 * here.
	 **/
	virtual bool save(QDataStream& stream, bool savePlayers = true);

	/**
	 * @return The port that is used for network games. The port we listen
	 * to when we are server or the peerPort when we are connected to
	 * another client.
	 **/
	Q_UINT16 bosonPort();
	bool isServer() const;

	/**
	 * @return @ref KGameNetwork::hotName, if present (was added in KDE
	 * 3.2). If it is not present, we use an evil hack that should have the
	 * same result.
	 **/
	QString bosonHostName();

	/**
	 * Workaround for a KGame bug in KDE < 3.2. This just calls @ref
	 * KGame::addPlayer directly in KDE >= 3.2, otherwise it will first
	 * assign a correct ID to the player and then call it.
	 **/
	void bosonAddPlayer(KPlayer* player);

	/**
	 * Dummy systemAddPlayer function. This is meant to grant access to
	 * systemAddPlayer to @ref BosonSaveLoad, without making the class a
	 * friend of Boson (i.e. granting access to whole Boson).
	 *
	 * A method BosonSaveLoad::systemAddPlayer that is friend of Boson would
	 * have been better (a lot!) but requires us to include bosonsaveload.h
	 * here.
	 **/
	void systemAddPlayer_(BosonSaveLoad* b, KPlayer* player)
	{
		if (b) {
			systemAddPlayer(player);
		}
	}

public slots:
	void slotSetGameSpeed(int speed);
	void slotTogglePause();

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
	 * This signal is for the start game/editor widgets only. Don't use it
	 * outside.
	 *
	 * It is emitted once a message has arrived inidicating that the "start
	 * game" button was clicked.
	 **/
	void signalStartGameClicked();

	/**
	 * Emitted once the game admin starts the game, i.e. clicks on "start
	 * game" in the new game widget (see @ref BosonNewGameWidget)
	 *
	 * Now all clients should start loading tiles etc.
	 **/
	void signalStartNewGame();

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

	void signalLoadPlayerData(Player* player);
	void signalLoadingPlayersCount(int count);
	void signalLoadingPlayer(int current);
	void signalLoadingType(int type);

	/**
	 * Emitted when the editor is meant to change the map.
	 **/
	void signalEditorNewMap(const QByteArray&);

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

	/**
	 * Tell the map to change @ref BosonMap::texMap at coordinates @p x, @p
	 * y.
	 *
	 * This is for editor use only!
	 **/
	void signalChangeTexMap(int x, int y, unsigned int textureCount, unsigned int* textures, unsigned char* alpha);

	void signalAddChatSystemMessage(const QString& fromName, const QString& text);

	void signalLoadExternalStuff(QDataStream& stream);
	void signalSaveExternalStuff(QDataStream& stream);
	void signalLoadExternalStuffFromXML(const QDomElement& root);
	void signalSaveExternalStuffAsXML(QDomElement& root);

	/**
	 * Emitted when a player is killed, i.e. has lost. The player
	 * still remains in the network (i.e. the game), he just cannot
	 * play anymore.
	 * This is emitted mainly by @ref killPlayer.
	 *
	 * The player is <em>not</em> removed from network and can take part in
	 * the next game for example. When the player is removed from the
	 * network @ref signalPlayerLeftGame is emitted.
	 **/
	void signalPlayerKilled(Player*);

protected:
	virtual bool playerInput(QDataStream& stream, KPlayer* player);

	/**
	 * Load the XML file in @p xml into @p doc and display an error message
	 * if an error occured.
	 * @return TRUE on success
	 **/
	bool loadXMLDoc(QDomDocument* doc, const QString& xml);

	/**
	 * Create a game log (see @ref writeGameLog) and store it for later use (see
	 * @ref saveGameLogs).
	 **/
	void makeGameLog();

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

	void slotReplacePlayerIO(KPlayer* player, bool* remove);

	void slotPlayerJoinedGame(KPlayer*);
	void slotPlayerLeftGame(KPlayer*);

	void slotProcessDelayed();

	void slotPropertyChanged(KGamePropertyBase*);

	void slotReceiveAdvance();

	void slotDebugOutput(const QString& area, const char* data, int level);

private:
	friend class BoAdvance;

private:
	class BosonPrivate;
	BosonPrivate* d;

	bool mGameMode;
	static Boson* mBoson;
};

#endif

