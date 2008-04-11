/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2008 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSON_H
#define BOSON_H

#include <kgame/kgame.h>
#include <sys/time.h>
#include <QList>
#include <Q3TextStream>
#include <Q3ValueList>
#include <Q3PtrList>
#include <QEvent>

class Player;
class PlayerIO;
class Unit;
class ProductionPlugin;
class BosonCanvas;
class BosonPlayField;
class QDomElement;
class QDomDocument;
class QDataStream;
class Q3TextStream;
class BosonSaveLoad;
class BosonPlayerListManager;
class BoMessage;
class BoEvent;
class BoEventManager;
class BoAdvanceMessageTimes;
class BosonMap;
class bofixed;
template<class T> class BoVector2;
typedef BoVector2<bofixed> BoVector2Fixed;
class BosonMessageEditorMove;
class BosonNetworkTraffic;

class QColor;

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
	bool createCanvas(BosonMap* map);

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

	PlayerIO* findPlayerIO(quint32 id) const;
	PlayerIO* playerIOAtAllIndex(unsigned int playerIndex) const;
	PlayerIO* playerIOAtGameIndex(unsigned int playerIndex) const;
	PlayerIO* playerIOAtActiveGameIndex(unsigned int playerIndex) const;

	BosonPlayerListManager* playerListManager() const;

	/**
	 * @return @ref BosonPlayerListManager::allPlayerCount
	 **/
	unsigned int allPlayerCount() const;

	/**
	 * @return @ref BosonPlayerListManager::gamePlayerCount
	 **/
	unsigned int gamePlayerCount() const;

	/**
	 * @return @ref BosonPlayerListManager::activeGamePlayerCount
	 **/
	unsigned int activeGamePlayerCount() const;

	/**
	 * @return @ref BosonPlayerListManager::allPlayerList
	 **/
	const QList<Player*>& allPlayerList() const;

	/**
	 * "game players" are players with ID >= 128 and <= 511. These are
	 * players who actually may own and move units in the game.
	 *
	 * This includes both, human controllable and neutral players.
	 *
	 * @return @ref BosonPlayerListManager::gamePlayerList
	 **/
	const QList<Player*>& gamePlayerList() const;

	/**
	 * "active game players" are players with ID >= 128 and <= 255. These
	 * players are relevant for winning conditions and therefore take an
	 * actual part in the game.
	 *
	 * They are in particular @em not neutral players.
	 *
	 * @return @ref BosonPlayerListManager::activeGamePlayerList
	 **/
	const QList<Player*>& activeGamePlayerList() const;

	/**
	 * Initialize a @ref BosonSaveLoad object with the relevant data.
	 **/
	void initSaveLoad(BosonSaveLoad*);

	void quitGame();

	static int advanceMessageInterval();

	int gameSpeed() const;
	bool gamePaused() const;

	/**
	 * Add the neutral player. This player contains all "dummy" objects and
	 * units (such as houses, trees, rocks, civilians, ...) that do nothing
	 * special, or at least that do not really fight :)
	 *
	 * The neutral player is (per definition) the last player in the @ref
	 * playerList(), i.e. is always at (playerList()->count() - 1). Note
	 * that the @ref Player::id is <em>not</em> predefined. It can be
	 * different for every game.
	 *
	 * There is only a single neutral player in the game.
	 *
	 * This method will send a network message using @ref bosonAddPlayer.
	 * The player is added once the message is received!
	 * @return 0 on failure, otherwise the player pointer
	 **/
	Player* addNeutralPlayer();

	void removeAllPlayers();

	Q3ValueList<QColor> availableTeamColors() const;

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
	Unit* findUnit(quint32 unitId, Player* searchIn) const;

	/**
	 * The factory completed to produce a unit and is now told to build
	 * (place) it.
	 * @param factory Where the unit is being build
	 * @param unitType which type of unit being buid. See @ref
	 * UnitProperties::typeId
	 * @param pos The position of the new unit.
	 **/
	bool buildProducedUnitAtTopLeftPos(ProductionPlugin* factory, quint32 unitType, BoVector2Fixed topLeftPos);

	virtual void networkTransmission(QDataStream&, int msgid, quint32 receiver, quint32 sender, quint32 clientID);

	/**
	 * Called by @ref networkTransmission. This method actually delivers the
	 * message to @ref KGame, i.e. call @ref KGame::networkTransmission.
	 *
	 * We do <em>not</em> check here whether the message is to be delayed
	 * any further. Do that in @ref networkTransmission.
	 **/
	void networkTransmission(BoMessage*);

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

	bool saveToFile(const QString& file);
	bool savePlayFieldToFile(const QString& file);

	virtual bool savegame(QDataStream& stream, bool network, bool saveplayers = true);
	virtual bool loadgame(QDataStream& stream, bool network, bool reset);

	virtual bool event(QEvent* event);
	virtual bool eventFilter(QObject* o, QEvent* event);

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

	void writeGameLog(Q3TextStream& log);
	bool saveGameLogs(const QString& prefix);

	/**
	 * Call @ref KGame::syncRandom
	 *
	 * You should normally not need this, as @ref KGame ensures that the
	 * random seeds are in sync.
	 **/
	void sendMessageSyncRandom();

	void syncNetwork();
	void clearDelayedMessages();
	void forcePauseGame();

	void queueEvent(BoEvent* event);

	BoEventManager* eventManager() const;

	bool loadCanvasConditions(const QDomElement& root);
	bool saveCanvasConditions(QDomElement& root) const;

	bool loadFromLog(Q3PtrList<BoMessage>* messages);

	/**
	 * Called when all messages from a "loadfromlog" run have been
	 * delivered. Network messages after this point will be accepted again.
	 *
	 * Calling this when not in "loadfromlog" mode is a noop.
	 **/
	void setLoadFromLogComplete();

	const BosonNetworkTraffic* networkTraffic() const;

	// for debugging
	const Q3PtrList<BoAdvanceMessageTimes>& advanceMessageTimes() const;

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
	using KGame::load;

	/**
	 * Dummy implementation that simply calls @ref KGame::save or @ref
	 * savegame, depending on the @ref KGame version. @ref KGame from KDE
	 * 3.0 didn't have @ref savegame, so we need to emulate it on our own
	 * here.
	 **/
	virtual bool save(QDataStream& stream, bool savePlayers = true);
	using KGame::save;

	/**
	 * @return The port that is used for network games. The port we listen
	 * to when we are server or the peerPort when we are connected to
	 * another client.
	 **/
	quint16 bosonPort();
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

	/**
	 * Add a system message for the local player only. Note that this does
	 * <em>not</em> send a chat message over network. It is displayed on
	 * this client only.
	 *
	 * You can find this method in this class, because it is accessible from
	 * mostly everywhere, where it might be needed
	 * @param forPlayer If non-NULL, the message is added only for this
	 * player. If @p forPlayer is not the local player, it is completely
	 * ignored.
	 **/
	void slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer = 0);


	/**
	 * Convinence method for the above slot. Simply uses "Boson" as
	 * fromName.
	 * @param forPlayer If non-NULL, the message is added only for this
	 * player. If @p forPlayer is not the local player, it is completely
	 * ignored.
	 **/
	void slotAddChatSystemMessage(const QString& text, const Player* forPlayer = 0);

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
	 * Emitted when the game is about to be started. The @p data contains
	 * all data necessary for starting the game and should be taken by a
	 * slot in @ref BosonStarting.
	 *
	 * The slot should set @p taken to TRUE, to let this object know that
	 * the data has been received.
	 **/
	void signalSetNewGameData(const QByteArray& data, bool* taken);

	/**
	 * Emitted when a client has sent a message that he completed starting
	 * (i.e. data loading). The game (i.e. advance messages) should start
	 * when all clients completed starting.
	 **/
	void signalStartingCompletedReceived(const QByteArray& message, quint32 sender);

	/**
	 * Order the canvas to call @ref BosonCanvas::slotAdvance
	 * @param advanceCallsCount The number of this advance call. This is used to
	 * decide what should be done - e.g. there is no need to check for new
	 * enemies every advance call. This value is increased after the signal
	 * was emitted
	 * @param advanceFlag see @ref advanceFlag
	 **/
	void signalAdvance(unsigned int advanceCallsCount, bool advanceFlag);

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
	 * This is only interesting for the new game dialog.
	 * Emitted when the "side" of the player changes, i.e. the @ref
	 * Player::bosonId.
	 *
	 * This is allowed to happen only before the game starts.
	 **/
	void signalSideChanged(Player* player);

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
	 * Emitted when the game is officially over and the winners are known.
	 *
	 * When this signal is emitted, no more advance calls will be processed.
	 * You most likely want to end the game now, e.g. call @ref quitGame
	 **/
	void signalGameOver();

	/**
	 * Tell the map to change @ref BosonMap::texMap at coordinates @p x, @p
	 * y.
	 *
	 * This is for editor use only!
	 **/
	void signalChangeTexMap(int x, int y, unsigned int textureCount, unsigned int* textures, unsigned char* alpha);
	void signalChangeHeight(int x, int y, float height);

	void signalAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer);

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

	void signalEditorClearUndoStack();
	void signalEditorClearRedoStack();
	void signalEditorNewUndoMessage(const BosonMessageEditorMove&, bool fromRedo);
	void signalEditorNewRedoMessage(const BosonMessageEditorMove&);

	/**
	 * See @ref BoAdvanceControl::signalUpdateGL
	 **/
	void signalUpdateGL();

protected:
	virtual bool playerInput(QDataStream& stream, KPlayer* player);

	virtual bool systemAddPlayer(KPlayer* p);
	virtual void systemRemovePlayer(KPlayer* p, bool deleteIt);

	/**
	 * Create a game log (see @ref writeGameLog) and store it for later use (see
	 * @ref saveGameLogs).
	 **/
	void makeGameLog();
	void makeUnitLog();

	bool loadFromLogFile(const QString& file);

	void clearUndoStacks();

	bool changeUserIdOfPlayer(Player* p, unsigned int newId);

	/**
	 * Redo the @ref allPlayerList, @ref gamePlayerList, @ref
	 * activeGamePlayerList lists.
	 *
	 * This must be called whenever these lists might change, i.e. whenever
	 * a player enters/leaves the game or whenever an Id of a player
	 * changes.
	 **/
	void recalculatePlayerLists();

	/**
	 * Like @ref recalculatePlayerLists, but pretends that @p removedPlayer
	 * was already removed from the players list.
	 **/
	void recalculatePlayerListsWithPlayerRemoved(KPlayer* removedPlayer);

protected slots:
	/**
	 * A network message arrived. Most game logic stuff is done here as
	 * nearly all functions just send a network request instead of doing the
	 * task theirselfes. This way we ensure that a task happens on
	 * <em>all</em> clients.
	 **/
	void slotNetworkData(int msgid, const QByteArray& buffer, quint32 receiver, quint32 sender);

	void slotClientLeftGame(int clientId, int oldgamestatus, KGame*);

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

	/**
	 * Called when the winning conditions are fullfilled. See @ref
	 * BosonCanvas::signalGameOver.
	 **/
	void slotGameOver();

	void slotChangeTexMap(int x, int y, unsigned int textureCount, unsigned int* textures, unsigned char* alpha);

private:
	friend class BoAdvance;
	friend class BoGameLogSaver;

	/**
	 * Use @ref allPlayerList instead
	 **/
	KGame::playerList;

	/**
	 * Use @ref allPlayerCount instead
	 **/
	KGame::playerCount;

private:
	class BosonPrivate;
	BosonPrivate* d;

	bool mGameMode;
	static Boson* mBoson;
};

struct timeval;
class BoAdvanceMessageTimes
{
public:
	BoAdvanceMessageTimes(int gameSpeed);
	~BoAdvanceMessageTimes();

	void receiveAdvanceCall();

	struct timeval mAdvanceMessage;
	struct timeval* mAdvanceCalls;
	int mGameSpeed;
	int mCurrentCall;
};

#endif

