/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "boson.h"
#include "boson.moc"

#include "defines.h"
#include "bosonmessage.h"
#include "player.h"
#include "unit.h"
#include "unitplugins.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosoncanvas.h"
#include "bosonstatistics.h"
#include "bosonplayfield.h"
#include "global.h"
#include "bosonprofiling.h"
#include "bodebug.h"
#include "bodebuglog.h"
#include "bosonsaveload.h"
#include "bosonconfig.h"
#include "bosonstarting.h"
#include "boevent.h"
#include "boeventmanager.h"
#include "bomessage.h"
#include "bosonplayerinput.h"
#include "bosonnetworksynchronizer.h"
#include "boeventloop.h"
#include "bosonpath.h"
#include "bosonmap.h"

#include <klocale.h>
#include <kdeversion.h>
#include <kcrash.h>
#include <kgame/kgameio.h>
#include <kgame/kgamemessage.h>
#include <kgame/kgamepropertyhandler.h>
#include <kgame/kmessageclient.h>

#include <qtimer.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qdatastream.h>
#include <qtextstream.h>
#include <qcstring.h>
#include <qfile.h>
#include <qapplication.h>

#include <stdlib.h>
#include <sys/time.h>

Boson* Boson::mBoson = 0;

#define ADVANCE_INTERVAL 250 // ms
//#define COLLECT_UNIT_LOGS

/**
 * Function that checks whether the ComputerIO list is still valid (i.e. players
 * still in the game). If it is not, boson is quit with an error message. Evil
 * error.
 *
 * This is a debugging function - invalid pointers must not (never!) appear in
 * the list.
 **/
static void ensureComputerIOListValid(Boson* boson, const QPtrList<KGameComputerIO>& computerIOList);

/**
 * See @ref KCrash::setEmergencyFunction
 *
 * This tries to save log messages, especially the player input that was made
 * until now. This might help at reproducing the crash.
 **/
static void emergencySave(int signal);
class BoMessageLogger;

BoAdvanceMessageTimes::BoAdvanceMessageTimes(int gameSpeed)
{
 if (gameSpeed <= 0) {
	boError(300) << k_funcinfo << "invalid advance message - gameSpeed " << gameSpeed << endl;
	gameSpeed = 1;
 }
 mCurrentCall = 0;
 mGameSpeed = gameSpeed;
 mAdvanceCalls = new struct timeval[mGameSpeed];
 gettimeofday(&mAdvanceMessage, 0);
}

BoAdvanceMessageTimes::~BoAdvanceMessageTimes()
{
 delete[] mAdvanceCalls;
}

void BoAdvanceMessageTimes::receiveAdvanceCall()
{
 if (mCurrentCall >= mGameSpeed) {
	boError(300) << k_funcinfo << "too many advance calls for this advance message" << endl;
	return;
 }
 gettimeofday(&mAdvanceCalls[mCurrentCall], 0);
 mCurrentCall++;
}

/**
 * @short Class that maintains advance messages and advance calls.
 *
 * A <em>advance message</em> is a message from the network with ID @ref
 * BoMessage::AdvanceN or similar. It tells boson to make a certain number of
 * <em>advance calls</em>, at the moment this number is defined by @ref
 * Boson::gameSpeed.
 *
 * An <em>advance call</em>, often referred to as <em>game cycle</em> is just a
 * call to the advance method (currently @ref receiveAdvanceCall), which causes
 * an advance signal (see @ref Boson::signalAdvance). When this signal is
 * emitted, all units are advanced. This is the central place where unit
 * movement and friends occur.
 *
 * In this class the precise process, how advance calls are made and how many
 * calls happen after a advance message was defined, now that is made in @ref
 * BoEventLoop.
 *
 * An advance message is received by @ref receiveAdvanceMessage. This will
 * result in an immediate advance call, and also in a certain number of
 * addition calls, that are delayed by a certain time. The exact amount of
 * time is interpolated from the expected time when the next advance message
 * arrives.
 *
 * An advance call is executed by @ref receiveAdvanceCall.
 **/
class BoAdvance
{
public:
	BoAdvance(Boson* boson)
	{
		mBoson = boson;

		initProperties(mBoson->dataHandler());

		mAdvanceMessageTimes.setAutoDelete(true);
		mCurrentAdvanceMessageTimes = 0;
	}

	static int advanceMessageInterval()
	{
		return ADVANCE_INTERVAL;
	}
	unsigned int advanceCallsCount() const
	{
		return mAdvanceCallsCount;
	}
	bool advanceFlag() const
	{
		return mAdvanceFlag;
	}
	void toggleAdvanceFlag()
	{
		mAdvanceFlag = !mAdvanceFlag;
	}

	void initProperties(KGamePropertyHandler* dataHandler)
	{
		mAdvanceFlag.registerData(Boson::IdAdvanceFlag, dataHandler,
				KGamePropertyBase::PolicyLocal, "AdvanceFlag");
		mAdvanceCallsCount.registerData(Boson::IdAdvanceCallsCount, dataHandler,
				KGamePropertyBase::PolicyLocal, "AdvanceCallsCount");

		mAdvanceFlag.setLocal(0);
		mAdvanceCallsCount.setLocal(0);
		mAdvanceCallsCount.setEmittingSignal(false);
		mAdvanceFlag.setEmittingSignal(false);
	}

	void receiveAdvanceMessage(int gameSpeed)
	{
		// we lock message delivery - only after all advance calls have
		// been made, user input is allowed.
		// this is important to keep network in sync
		mBoson->lock();

		boDebug(300) << "Advance - speed (calls per " << advanceMessageInterval()
				<< "ms)=" << gameSpeed << endl;

		mCurrentAdvanceMessageTimes = new BoAdvanceMessageTimes(gameSpeed);
		mAdvanceMessageTimes.append(mCurrentAdvanceMessageTimes);
		((BoEventLoop*)qApp->eventLoop())->receivedAdvanceMessage(gameSpeed);
	}

	void sendAdvance() // slot?
	{
		boDebug(300) << k_funcinfo << "advanceCallsCount=" << mBoson->advanceCallsCount() << " sending advance msg" << endl;
		mBoson->sendMessage(0, BosonMessage::AdvanceN);
	}

	/**
	 * Execute an advance call.
	 *
	 * This does the central advance call mechanism in Boson, most
	 * importantly it will make @ref Boson emit @ref Boson::signalAdvance.
	 *
	 * However the important task of calculating when the next advance call
	 * is being made, is not done here anymore. That is now done in the @ref
	 * BoEventLoop.
	 **/
	void receiveAdvanceCall();

	const QPtrList<BoAdvanceMessageTimes>& advanceMessageTimes() const
	{
		return mAdvanceMessageTimes;
	}

private:
	Boson* mBoson;

	QPtrList<BoAdvanceMessageTimes> mAdvanceMessageTimes;
	BoAdvanceMessageTimes* mCurrentAdvanceMessageTimes;

	KGamePropertyInt mAdvanceFlag;
	KGameProperty<unsigned int> mAdvanceCallsCount;
};


void BoAdvance::receiveAdvanceCall()
{
 BO_CHECK_NULL_RET(mCurrentAdvanceMessageTimes);

 BoEvent* advanceEvent = new BoEvent("Advance");
 mBoson->queueEvent(advanceEvent);

 mCurrentAdvanceMessageTimes->receiveAdvanceCall();
 boDebug(300) << k_funcinfo << advanceCallsCount() << endl;
 bool flag = advanceFlag();
 // we need to toggle the flag *now*, in case one of the Unit::advance*()
 // methods changes the advance function. this change must not appear to the
 // currently used function, but to the other one.
 toggleAdvanceFlag();

 emit mBoson->signalAdvance(advanceCallsCount(), flag);
 // AB: do _not_ connect to the signal!
 // -> slots may be called in random order, but we need well defined order
 // (otherwise network may get broken soon)
 mBoson->eventManager()->advance();

 boDebug(300) << k_funcinfo << advanceCallsCount() << " DONE" << endl;

 mAdvanceCallsCount = mAdvanceCallsCount + 1;
}


class Boson::BosonPrivate
{
public:
	BosonPrivate()
	{
		mStartingObject = 0;

		mGameTimer = 0;
		mCanvas = 0;
		mPlayField = 0;

		mLoadingStatus = BosonSaveLoad::NotLoaded;

		mAdvance = 0;
		mMessageDelayer = 0;

		mEventManager = 0;

		mPlayerInputHandler = 0;
		mNetworkSynchronizer = 0;
	}
	BosonStarting* mStartingObject;

	QTimer* mGameTimer;

	BosonCanvas* mCanvas;
	BosonPlayField* mPlayField;
	QPtrList<KGameComputerIO> mComputerIOList;

	KGamePropertyInt mGameSpeed;
	KGamePropertyBool mGamePaused;

	BosonSaveLoad::LoadingStatus mLoadingStatus;

	QValueList<QByteArray> mGameLogs;
	QValueList<QByteArray> mUnitLogs;
	BoMessageLogger mMessageLogger;

	BoAdvance* mAdvance;
	BoMessageDelayer* mMessageDelayer;

	BoEventManager* mEventManager;

	BosonPlayerInput* mPlayerInputHandler;
	BosonNetworkSynchronizer* mNetworkSynchronizer;
};

Boson::Boson(QObject* parent) : KGame(BOSON_COOKIE, parent)
{
 setPolicy(PolicyClean);
 d = new BosonPrivate;
 d->mPlayerInputHandler = new BosonPlayerInput(this);
 d->mAdvance = new BoAdvance(this);
 d->mMessageDelayer = new BoMessageDelayer(this);
 d->mNetworkSynchronizer = new BosonNetworkSynchronizer();

 d->mGameTimer = new QTimer(this);

 d->mNetworkSynchronizer->setGame(this);
 d->mNetworkSynchronizer->setMessageLogger(&d->mMessageLogger);


 mGameMode = true;

 connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)),
		this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)));
 connect(this, SIGNAL(signalReplacePlayerIO(KPlayer*, bool*)),
		this, SLOT(slotReplacePlayerIO(KPlayer*, bool*)));
 connect(this, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(this, SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SLOT(slotPlayerLeftGame(KPlayer*)));
 connect(this, SIGNAL(signalAdvance(unsigned int, bool)),
		this, SLOT(slotAdvanceComputerPlayers(unsigned int, bool)));
 connect(this, SIGNAL(signalClientLeftGame(int, int, KGame*)),
		this, SLOT(slotClientLeftGame(int, int, KGame*)));
 connect(dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
		this, SLOT(slotPropertyChanged(KGamePropertyBase*)));
 d->mGamePaused.setEmittingSignal(false); // make valgrind happy
 d->mGamePaused.registerData(IdGamePaused, dataHandler(),
		KGamePropertyBase::PolicyClean, "GamePaused");
 d->mGamePaused.setLocal(false);
 d->mGamePaused.setEmittingSignal(true);
 d->mGameSpeed.registerData(IdGameSpeed, dataHandler(),
		KGamePropertyBase::PolicyClean, "GameSpeed");
 d->mGameSpeed.setLocal(0);

 d->mEventManager = new BoEventManager(this);

 setMinPlayers(1);

 if (KCrash::emergencySaveFunction() != NULL) {
	boError() << k_funcinfo << "oops - already an emergencySaveFunction set! overwriting!" << endl;
 }
 KCrash::setEmergencySaveFunction(emergencySave);

 BoDebugLog* debugLog = BoDebugLog::debugLog();
 if (debugLog) {
	connect(debugLog, SIGNAL(signalError(const BoDebugMessage&)),
			this, SLOT(slotBoDebugError(const BoDebugMessage&)));
	connect(debugLog, SIGNAL(signalWarn(const BoDebugMessage&)),
			this, SLOT(slotBoDebugWarning(const BoDebugMessage&)));
	connect(debugLog, SIGNAL(signalDebug(const BoDebugMessage&)),
			this, SLOT(slotBoDebugOutput(const BoDebugMessage&)));

	// we could provide config entries for these, that allow enabling _INFO
	// as well
	debugLog->setEmitSignal(BoDebug::KDEBUG_ERROR, true);
	debugLog->setEmitSignal(BoDebug::KDEBUG_WARN, true);
 }

 ((BoEventLoop*)qApp->eventLoop())->setAdvanceMessageInterval(ADVANCE_INTERVAL);
 ((BoEventLoop*)qApp->eventLoop())->setAdvanceObject(this);
}

Boson::~Boson()
{
 ((BoEventLoop*)qApp->eventLoop())->setAdvanceObject(0);
 KCrash::setEmergencySaveFunction(NULL);
 delete d->mNetworkSynchronizer;
 delete d->mPlayerInputHandler;
 delete d->mMessageDelayer;
 delete d->mAdvance;
 delete d->mGameTimer;
 delete d->mCanvas;
 delete d;
}

void Boson::initBoson()
{
 mBoson = new Boson(0);
}

void Boson::deleteBoson()
{
 delete mBoson;
 mBoson = 0;
}

void Boson::createCanvas()
{
 if (d->mCanvas) {
	boWarning() << k_funcinfo << "there is already a canvas created! no touching that object..." << endl;
	// do NOT delete it, as it might be used somewhere else as wel else as
	// welll
	return;
 }
 d->mCanvas = new BosonCanvas(this);

 if (gameMode()) {
	connect(this, SIGNAL(signalAdvance(unsigned int, bool)),
			d->mCanvas, SLOT(slotAdvance(unsigned int, bool)));
 }
}

BosonCanvas* Boson::canvasNonConst() const
{
 return d->mCanvas;
}

const BosonCanvas* Boson::canvas() const
{
 return d->mCanvas;
}

bool Boson::event(QEvent* e)
{
 switch (e->type()) {
	case QEvent::User + QtEventAdvanceCall:
		slotReceiveAdvance();
		return true;
	case QEvent::User + QtEventAdvanceMessageCompleted:
		boDebug(300) << k_funcinfo << "delayed messages: "
				<< delayedMessageCount() << endl;
		unlock();
		return true;
	default:
		break;
 }
 return KGame::event(e);
}

bool Boson::eventFilter(QObject* o, QEvent* e)
{
 return KGame::eventFilter(o, e);
}

void Boson::setPlayField(BosonPlayField* p)
{
 boDebug() << k_funcinfo << endl;
 if (d->mPlayField) {
	boError() << k_funcinfo << "already have a playfield - unsetting is not yet supported" << endl;
	return;
 }
 d->mPlayField = p;
 if (d->mPlayField) {
	connect(this, SIGNAL(signalChangeTexMap(int, int, unsigned int, unsigned int*, unsigned char*)),
			d->mPlayField->map(), SLOT(slotChangeTexMap(int, int, unsigned int, unsigned int*, unsigned char*)));
 }
}

BosonPlayField* Boson::playField() const
{
 return d->mPlayField;
}

PlayerIO* Boson::findPlayerIO(Q_UINT32 id) const
{
 Player* p = (Player*)findPlayer(id);
 if (p) {
	return p->playerIO();
 }
 return 0;
}

void Boson::setStartingObject(BosonStarting* s)
{
 // AB: a NULL starting object unsets the object. it is totally valid
 d->mStartingObject =  s;
}

void Boson::quitGame()
{
 boDebug() << k_funcinfo << endl;
// reset everything
 if (d->mCanvas) {
	d->mCanvas->quitGame();
 }
 d->mGameTimer->stop();
 setGameStatus(KGame::End);

 // remove all players from game
 removeAllPlayers();

 setStartingObject(0);
 boDebug() << k_funcinfo << " done" <<  endl;
}

void Boson::removeAllPlayers()
{
 QPtrList<KPlayer> list = *playerList();
 for (unsigned int i = 0; i < list.count(); i++) {
	removePlayer(list.at(i)); // might not be necessary - sends remove over network
	systemRemovePlayer(list.at(i), true); // remove immediately, even before network removing is received.
 }
}

bool Boson::playerInput(QDataStream& stream, KPlayer* p)
{
 return d->mPlayerInputHandler->playerInput(stream, (Player*)p);
}

void Boson::slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 , Q_UINT32 sender)
{
 QDataStream stream(buffer, IO_ReadOnly);
 switch (msgid) {
	case BosonMessage::AdvanceN:
	{
		d->mNetworkSynchronizer->receiveAdvanceMessage(d->mCanvas);
		d->mAdvance->receiveAdvanceMessage(gameSpeed());
		break;
	}
	case BosonMessage::ChangeMap:
	{
		emit signalEditorNewMap(buffer);
		break;
	}
	case BosonMessage::IdNewGame:
	{
		if (isRunning()) {
			boError() << k_funcinfo << "received IdNewGame, but game is already running" << endl;
			return;
		}
		if (!d->mStartingObject) {
			boError() << k_funcinfo << "received IdNewGame, but starting a game is not allowed!" << endl;
			return;
		}
		Q_INT8 gameMode; // game/editor mode
		stream >> gameMode;
		if (gameMode == 1) {
			setGameMode(true);
		} else if (gameMode == 0) {
			setGameMode(false);
		} else {
			boError() << k_funcinfo << "invalid gameMode value " << gameMode << endl;
			return;
		}
		QByteArray data;
		stream >> data;
		d->mStartingObject->setNewGameData(data);
		QTimer::singleShot(0, this, SIGNAL(signalStartNewGame()));
		break;
	}
	case BosonMessage::IdStartGameClicked:
		// this is kind of a workaround.
		// for --start we need to call slotStart() in the start widgets
		// only once the (e.g.) playfield messages have arrived. for
		// this we use a message and *then* call slotStart() there.
		QTimer::singleShot(0, this, SIGNAL(signalStartGameClicked()));
		break;
	case BosonMessage::IdGameIsStarted:
	{
		int flag = 0;
		stream >> flag;
		if (sender != messageClient()->adminId()) {
			boError() << k_funcinfo << "only ADMIN is allowed to send IdGameIsStarted message! sender="
					<< sender << " ADMIN="
					<< messageClient()->adminId() << endl;
			break;
		}

		ensureComputerIOListValid(this, d->mComputerIOList);

		emit signalGameStarted();

		if (flag != 0 && flag != 1) {
			boError() << k_funcinfo << "invalid flag value " << flag << endl;
			flag = 0;
		}
		if (flag == 0) {
			if (isAdmin()) {
				connect(d->mGameTimer, SIGNAL(timeout()), this, SLOT(slotSendAdvance()));
			}
		} else if (flag == 1) {
			boWarning() << k_funcinfo << "starting to re-play from log..." << endl;

			if (!loadFromLogFile()) {
				slotAddChatSystemMessage(i18n("You requested to load from a log file, but the game could not be started using the specified file.\nThe current game is now unusable."));
//				KMessageBox::sorry(0, i18n("You requested to load from a log file, but the game could not be started using the specified file.\nThe current game is now unusable."));
			} else {
			}
		}
		break;
	}
	case BosonMessage::ChangeSpecies:
	{
		Q_UINT32 id;
		QString species;
		Q_UINT32 color;
		stream >> id;
		stream >> species;
		stream >> color;
		Player* p = (Player*)findPlayer(id);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << id << endl;
			return;
		}
		p->loadTheme(SpeciesTheme::speciesDirectory(species), QColor(color));
		emit signalSpeciesChanged(p);
		break;
	}
	case BosonMessage::ChangeTeamColor:
	{
		Q_UINT32 id;
		Q_UINT32 color;
		stream >> id;
		stream >> color;
		Player* p = (Player*)findPlayer(id);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << id << endl;
			return;
		}
		if (!p->speciesTheme()) {
			boError() << k_lineinfo << "NULL speciesTheme for " << id << endl;
			return;
		}
		if (p->speciesTheme()->setTeamColor(QColor(color))) {
			emit signalTeamColorChanged(p);
		} else {
			boWarning() << k_lineinfo << "could not change color for " << id << endl;
		}
		break;
	}
	case BosonMessage::ChangePlayField:
	{
		QString field;
		stream >> field;
		emit signalPlayFieldChanged(field);
		break;
	}
	case BosonMessage::IdChat:
	{
		break;
	}
	case BosonMessage::IdKillPlayer:
	{
		Player* p = 0;
		Q_UINT32 id = 0;
		stream >> id;
		p = (Player*)findPlayer(id);
		BO_CHECK_NULL_RET(p);
		BO_CHECK_NULL_RET(d->mCanvas);
		killPlayer(p);
		slotAddChatSystemMessage(i18n("Debug"), i18n("Killed player %1 - %2").arg(p->id()).arg(p->name()));
	}
	case BosonMessage::IdModifyMinerals:
	{
		Player* p = 0;
		Q_INT32 change = 0;
		Q_UINT32 id = 0;
		stream >> id;
		stream >> change;
		p = (Player*)findPlayer(id);
		BO_CHECK_NULL_RET(p);
		if ((Q_INT32)p->minerals() + change < 0) {
		}
		p->setMinerals(p->minerals() + change);
		break;
	}
	case BosonMessage::IdModifyOil:
	{
		Player* p = 0;
		Q_INT32 change = 0;
		Q_UINT32 id = 0;
		stream >> id;
		stream >> change;
		p = (Player*)findPlayer(id);
		BO_CHECK_NULL_RET(p);
		if ((Q_INT32)p->oil() + change < 0) {
			p->setOil(0);
		} else {
			p->setOil(p->oil() + change);
		}
		break;
	}
	case BosonMessage::IdGameStartingCompleted:
	{
		// this message is a kind of ACK from the client. it indicates
		// that the starting is done and we are waiting for the ADMIN to
		// start the game. he will do so once all clients have sent this
		// message.
		if (!d->mStartingObject) {
			BO_NULL_ERROR(d->mStartingObject);
			break;
		}
		if (gameStatus() != KGame::Init) {
			// the message is not allowed here
			boError() << k_funcinfo << "not in Init state" << endl;
			break;
		}
		d->mStartingObject->startingCompletedReceived(buffer, sender);
		break;
	}
	case BosonMessage::IdNetworkSyncCheck:
	{
		if (!d->mNetworkSynchronizer->receiveNetworkSyncCheck(stream)) {
			// the network is not in sync anymore.
			// note that we don't have to do anything here, it is
			// done in the network synchronizer class.
		}
		break;
	}
	case BosonMessage::IdNetworkSyncCheckACK:
	{
		if (!isAdmin()) {
			break;
		}
		bool inSync = d->mNetworkSynchronizer->receiveNetworkSyncCheckAck(stream, sender);
		if (inSync) {
			break;
		}
		syncNetwork();
		break;
	}
	case BosonMessage::IdNetworkRequestSync:
	{
		if (!d->mNetworkSynchronizer->receiveNetworkRequestSync(stream)) {
			slotAddChatSystemMessage(i18n("Could not synchronize clients. Cannot fix out-of-sync client. Sorry"));
			boError() << k_funcinfo << "unable to synchronize clients. cannot fix out-of-sync." << endl;
			break;
		}
		break;
	}
	case BosonMessage::IdNetworkSync:
	{
		if (!d->mNetworkSynchronizer->receiveNetworkSync(stream)) {
			slotAddChatSystemMessage(i18n("Could not load from sync stream. Game unusable now (loading error)"));

			// AB: at this point it makes no sense to unlock the
			// game.
			// when loading fails, then we aborted somewhere in the
			// middle of loading a game, so most probably we cannot
			// use that game anymore anyway.
			break;
		}
		if (!isAdmin()) {
			// the ADMIN has already a chat message containing the
			// success/failure of the sync
			// FIXME: we should tell the clients whether we
			// succeeded or failed at syncing
			// AB: maybe we can tell the player whether this client
			// is in sync (only ADMIN can know about the other
			// clients)
			slotAddChatSystemMessage(i18n("Network Sync completed. Success/failure unknown (only ADMIN knows). Unpause to continue the game"));
		}
		break;
	}
	case BosonMessage::IdNetworkSyncUnlockGame:
	{
		d->mNetworkSynchronizer->receiveNetworkSyncUnlockGame(stream);
		break;
	}
	default:
		boWarning() << k_funcinfo << "unhandled msgid " << msgid << endl;
		break;
 }
}

bool Boson::isServer() const
{
 return isAdmin(); // or isMaster() ??
}

void Boson::slotSendAdvance()
{
 d->mAdvance->sendAdvance();
}

Unit* Boson::findUnit(unsigned long int id, Player* searchIn) const
{
 if (searchIn) {
	return searchIn->findUnit(id);
 }
 QPtrListIterator<KPlayer> it(*playerList());
 while (it.current()) {
	Unit* unit = ((Player*)it.current())->findUnit(id);
	if (unit) {
		return unit;
	}
	++it;
 }
 return 0;
}

KPlayer* Boson::createPlayer(int , int , bool ) // AB: we don't use these params.
{
 boDebug() << k_funcinfo << endl;
 Player* p = new Player();
 p->setGame(this);
 if (d->mPlayField && d->mPlayField->map()) {
	// AB: this will never be reached. unused. can probably be removed.
	p->initMap(d->mPlayField->map(), boGame->gameMode());
 }
 return p;
}

int Boson::advanceMessageInterval()
{
 return BoAdvance::advanceMessageInterval();
}

int Boson::gameSpeed() const
{
 return d->mGameSpeed;
}

bool Boson::gamePaused() const
{
 return d->mGamePaused;
}

void Boson::slotSetGameSpeed(int speed)
{
 boDebug() << k_funcinfo << " speed = " << speed << endl;
 if (speed < 0) {
	boError() << "Invalid speed value " << speed << endl;
	return;
 }
 if ((speed < MIN_GAME_SPEED || speed > MAX_GAME_SPEED) && speed != 0) {
	boWarning() << "unexpected speed " << speed << " - pausing" << endl;
	d->mGameSpeed = 0;
	// we don't have a manual pause, so don't set d->mGamePaused!
	return;
 }
 boDebug() << k_funcinfo << "Setting speed to " << speed << endl;
 d->mGameSpeed = speed;
}

void Boson::slotTogglePause()
{
 boDebug() << k_funcinfo << endl;
 // note that this won't take immediate effect. the variable will change once it
 // is received from network!
 d->mGamePaused = !gamePaused();
}

void Boson::forcePauseGame()
{
 boDebug() << k_funcinfo << endl;

 // this has no effect until the message is received from net
 d->mGamePaused = true;

 // this takes effect immediately
 d->mGameTimer->stop();
}

void Boson::slotPropertyChanged(KGamePropertyBase* p)
{
 switch (p->id()) {
	case IdGameSpeed:
		boDebug() << k_funcinfo << "speed has changed, new speed: " << gameSpeed() << endl;
		if (isServer()) {
			if (d->mGameSpeed == 0) {
				if (d->mGameTimer->isActive()) {
					boDebug() << "pausing" << endl;
					d->mGameTimer->stop();
				}
			} else {
				if (!d->mGameTimer->isActive() && !gamePaused()) {
					boDebug() << "start timer - ms="
							<< advanceMessageInterval()
							<< endl;
					d->mGameTimer->start(advanceMessageInterval());
				}
			}
		}
		if (gamePaused()) {
			boProfiling->setGameSpeed(0);
		} else {
			boProfiling->setGameSpeed(gameSpeed());
		}
		break;
	case IdGamePaused:
		boDebug() << k_funcinfo << "game paused changed! now=" << d->mGamePaused << endl;
		if (d->mGamePaused) {
			slotAddChatSystemMessage(i18n("The game is paused now!"));
			d->mGameTimer->stop();
		} else if (d->mGameSpeed > 0) {
			boDebug() << k_funcinfo << "starting timer again" << endl;
			slotAddChatSystemMessage(i18n("The game is not paused anymore"));
			d->mGameTimer->start(advanceMessageInterval());
		}
		if (gamePaused()) {
			boProfiling->setGameSpeed(0);
		} else {
			boProfiling->setGameSpeed(gameSpeed());
		}
		break;
	default:
		break;
 }
}

void Boson::slotReplacePlayerIO(KPlayer* player, bool* remove)
{
 boDebug() << k_funcinfo << endl;
 if (gameStatus() == KGame::Init) {
	boDebug() << k_funcinfo << "game in Init state - remove player" << endl;
	// game has not yet started - we can safely remove the player now
	*remove = true;
	return;
 }
 boDebug() << k_funcinfo << "game not in Init state - keep player" << endl;

 // the game is still running, so don't remove the player.
 // -> one day we should add a new KGameIO object here, so that the computer
 //    plays instead of the network player now
 *remove = false;
 if (!player) {
	boError() << k_funcinfo << "NULL player" << endl;
	return;
 }
#if 0
 // AB: this can be called for the client, too!
 // -> when ADMIN leaves, the IOs of the clients have to be replaced.
 if (!isAdmin()) {
	boError() << k_funcinfo << "only ADMIN can do this" << endl;
	return;
 }
#endif
 slotAddChatSystemMessage(i18n("Player %1(%2) left the game. Units of that player remain on the map.").arg(player->name()).arg(player->id()));
// boDebug() << k_funcinfo << endl;
}

bool Boson::buildProducedUnit(ProductionPlugin* factory, unsigned long int unitType, BoVector2Fixed pos)
{
 if (!d->mCanvas) {
	BO_NULL_ERROR(d->mCanvas);
	return false;
 }
 if (!factory) {
	boError() << k_funcinfo << "NULL factory plugin cannot produce" << endl;
	return false;
 }
 Player* p = factory->player();
 if (!p) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return false;
 }
 if (!(d->mCanvas)->canPlaceUnitAt(p->unitProperties(unitType), pos, 0)) {
	boDebug() << k_funcinfo << "Cannot create unit here" << endl;
	return false;
 }
 BoVector3Fixed pos3(pos.x(), pos.y(), 0.0f);
 Unit* unit = (Unit*)d->mCanvas->createNewItem(RTTI::UnitStart + unitType, p, ItemType(unitType), pos3);
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return false;
 }
 if (unit->isFacility()) {
	p->statistics()->addProducedFacility((Facility*)unit, factory);
 } else {
	p->statistics()->addProducedMobileUnit((MobileUnit*)unit, factory);
 }

 BoEvent* productionPlaced = new BoEvent("ProducedUnitWithTypePlaced", QString::number(unit->type()));
 productionPlaced->setUnitId(unit->id());
 productionPlaced->setUnitId(unit->owner()->id());
 productionPlaced->setLocation(BoVector3Fixed(unit->x(), unit->y(), unit->z()));
 boGame->queueEvent(productionPlaced);

 // the current production is done.
 factory->removeProduction();
 emit signalUpdateProduction(factory->unit());
 return true;
}


void Boson::slotPlayerJoinedGame(KPlayer* p)
{
 boDebug() << k_funcinfo << endl;
 if (!p) {
	return;
 }
 KGameIO* io = p->findRttiIO(KGameIO::ComputerIO);
 if (io) {
	// note the IO is added on only *one* client!
	d->mComputerIOList.append((KGameComputerIO*)io);
 }
 slotAddChatSystemMessage(i18n("Player %1 - %2 joined").arg(p->id()).arg(p->name()));
}

void Boson::slotPlayerLeftGame(KPlayer* p)
{
 if (!p) {
	return;
 }
 KGameIO* io = p->findRttiIO(KGameIO::ComputerIO);
 if (io) {
	d->mComputerIOList.removeRef((KGameComputerIO*)io);
 }
 slotAddChatSystemMessage(i18n("Player %1 - %2 left the game").arg(p->id()).arg(p->name()));

 ensureComputerIOListValid(this, d->mComputerIOList);
}

void Boson::slotAdvanceComputerPlayers(unsigned int /*advanceCallsCount*/, bool /*advanceFlag*/)
{
 // we use this to "advance" the computer player. This is a completely new concept
 // introduced to KGameIO just for boson. See KGaneComputerIO documentation for
 // more. Basically this means - let the computer do something.
 QPtrListIterator<KGameComputerIO> it(d->mComputerIOList);
// boDebug() << "count = " << d->mComputerIOList.count() << endl;
 while (it.current()) {
	it.current()->advance();
	++it;
 }
}

QValueList<QColor> Boson::availableTeamColors() const
{
 QValueList<QColor> colors = SpeciesTheme::defaultColors();
 QPtrListIterator<KPlayer> it(*playerList());
 while (it.current()) {
	if (((Player*)it.current())->speciesTheme()) {
		boDebug() << k_funcinfo <<  endl;
		colors.remove(((Player*)it.current())->speciesTheme()->teamColor());
	}
	++it;
 }
 return colors;
}

void Boson::slotReceiveAdvance()
{
 // Log game state
 if (advanceCallsCount() % boConfig->intValue("GameLogInterval") == 0) {
	//makeGameLog();
 }
#ifdef COLLECT_UNIT_LOGS
 makeUnitLog();
#endif

 d->mAdvance->receiveAdvanceCall();
}

void Boson::networkTransmission(QDataStream& stream, int msgid, Q_UINT32 r, Q_UINT32 s, Q_UINT32 clientId)
{
 if (!d->mNetworkSynchronizer->acceptNetworkTransmission(msgid)) {
	// the game is locked. only certain messages are allowed currently.
	boDebug() << k_funcinfo << "game is locked for syncing. ignoring message with id=" << msgid << endl;
	return;
 }

 BoMessage* m = new BoMessage(stream, msgid, r, s, clientId, advanceCallsCount());

 if (!d->mMessageDelayer->processMessage(m)) {
	// the message got delayed. don't deliver it now.
	return;
 }
 // not delayed - deliver it
 networkTransmission(m);
}

void Boson::networkTransmission(BoMessage* m)
{
 if (!m) {
	BO_NULL_ERROR(m);
	return;
 }
 m->setDelivered();
 m->deliveredOnAdvanceCallsCount = advanceCallsCount();
 d->mMessageLogger.append(m);
 QDataStream s(m->byteArray, IO_ReadOnly);
 KGame::networkTransmission(s, m->msgid, m->receiver, m->sender, m->clientId);
}

void Boson::lock()
{
 d->mMessageDelayer->lock();
}

void Boson::unlock()
{
 d->mMessageDelayer->unlock();
}

bool Boson::isLocked() const
{
 return d->mMessageDelayer->isLocked();
}

// obsolete.
void Boson::slotProcessDelayed() // TODO: rename: processDelayed()
{
}

void Boson::initSaveLoad(BosonSaveLoad* b)
{
 if (b) {
	b->setCanvas(d->mCanvas);
	b->setPlayField(d->mPlayField);
 }
}

bool Boson::saveToFile(const QString& file)
{
 boDebug() << k_funcinfo << file << endl;
 QMap<QString, QByteArray> files;
 BosonSaveLoad* save = new BosonSaveLoad(this);
 bool ret = save->saveToFiles(files);
 delete save;
 if (!ret) {
	boError() << k_funcinfo << "saving failed" << endl;
	return ret;
 }
 ret = BosonSaveLoad::saveToFile(files, file);
 return ret;
}

bool Boson::savePlayFieldToFile(const QString& file)
{
 boDebug() << k_funcinfo << file << endl;
 QMap<QString, QByteArray> files;
 BosonSaveLoad* save = new BosonSaveLoad(this);
 bool ret = save->savePlayFieldToFiles(files);
 if (!ret) {
	boError() << k_funcinfo << "saving failed" << endl;
	return ret;
 }
 ret = BosonSaveLoad::saveToFile(files, file);
 return ret;
}

bool Boson::save(QDataStream& stream, bool saveplayers)
{
 return KGame::save(stream, saveplayers);
}


// AB: note: this is NOT called for saving to a file but for saving to stream
// only. saving to stream happens when a new player joins (we transfer the
// current game to that player)
bool Boson::savegame(QDataStream& stream, bool network, bool saveplayers)
{
 // AB: we need to save:
 // - cookie or something similar (to identify a boson non-kgame savegame)
 // - version of the savegame format
 // - KGame stores KGame::d->mUniquePlayerNumber. do we need to, too? i believe
 // not!
 // - KGame::d->mRandom->seed() !
 // - KGame::dataHandler() (use dataHandler()->save() or so. try to use xml)
 // - playerCount
 // - players
 //
 // + boson relevant data


 boDebug() << k_funcinfo << endl;
 // KGame::load() doesn't emit signalLoadPrePlayers in KDE 3.0.x, so we have to
 //  rewrite some code to be able to load map before players (because players
 //  need map)

 // First write some magic data
 // For filetype detection
 stream << (Q_UINT8)128;
 stream << (Q_UINT8)'B' << (Q_UINT8)'S' << (Q_UINT8)'G';  // BSG = Boson SaveGame
 // Magic cookie
 stream << (Q_INT32)cookie();
 // Version information (for future format changes and backwards compatibility)
 stream << (Q_UINT32)BosonSaveLoad::latestSavegameVersion();

 // Save KGame stuff
 if (!KGame::savegame(stream, network, saveplayers)) {
	boError() << k_funcinfo << "Can't save KGame!" << endl;
	return false;
 }

 // Save end cookie
 stream << (Q_UINT32)BOSON_SAVEGAME_END_COOKIE;


 boDebug() << k_funcinfo << " done" << endl;
 return true;
}

bool Boson::load(QDataStream& stream, bool reset)
{
// we can't use this directly cause of a KGame bug :-(
 return loadgame(stream, false, reset);
}

bool Boson::loadgame(QDataStream& stream, bool network, bool reset)
{
 // AB: KGame::loadgame() is called for exactly two things:
 // 1. loading a game from a file
 // 2. initializing a newly connected player
 //
 // 1. is not used by boson. we use loadFromFile() for this.
 // so for us only 2. is relevant.
 // it would be great if we could one day support that players can join even if
 // the game already started. but as we do not yet support this, we can keep
 // this as simple as possible
 boDebug() << k_funcinfo << endl;

 d->mLoadingStatus = BosonSaveLoad::LoadingInProgress;

 // Load magic data
 Q_UINT8 a, b1, b2, b3;
 Q_INT32 c;
 Q_UINT32 v;
 stream >> a >> b1 >> b2 >> b3;
 if ((a != 128) || (b1 != 'B' || b2 != 'S' || b3 != 'G')) {
	// Error - not Boson SaveGame
	boError() << k_funcinfo << "invalid magic cookie" << endl;
	d->mLoadingStatus = BosonSaveLoad::InvalidFileFormat;
	return false;
 }
 stream >> c;
 if (c != cookie()) {
	// Error - wrong cookie
	boError() << k_funcinfo << "Invalid cookie in header (found: " << c << "; should be: " << cookie() << ")" << endl;
	d->mLoadingStatus = BosonSaveLoad::InvalidCookie;
	return false;
 }
 stream >> v;
 if (v != BosonSaveLoad::latestSavegameVersion()) {
	// Error - older version
	boError() << k_funcinfo << "Unsupported format version (found: " << v << "; latest: " << BosonSaveLoad::latestSavegameVersion() << ")" << endl;
	d->mLoadingStatus = BosonSaveLoad::InvalidVersion;
	return false;
 }

 // Load KGame stuff
 boDebug() << "calling KGame::loadgame" << endl;
 if (!KGame::loadgame(stream, network, reset)) {
	// KGame loading error
	boError() << k_funcinfo << "KGame loading error" << endl;
	d->mLoadingStatus = BosonSaveLoad::KGameError;
	return false;
 }
 boDebug() << k_funcinfo << "kgame loading successful" << endl;

 // KGame::loadgame() also loads the gamestatus. some functions depend on KGame
 // to be in Init status as long as it is still loading, so we set it manually
 // here. we can't do this using KGame::setStatus(), as the policy is clean, but
 // we need Init state *now*. Changing policy would also change our property
 // policies (we use both clean and local policies, so this would not work).
 {
	// set gameStatus to Init. Will be set to Run later
	QByteArray b;
	QDataStream s(b, IO_WriteOnly);
	KGameMessage::createPropertyHeader(s, KGamePropertyBase::IdGameStatus);
	s << (int)KGame::Init;
	QDataStream readStream(b, IO_ReadOnly);
	dataHandler()->processMessage(readStream, dataHandler()->id(), false);
 }

 // Check end cookie
 Q_UINT32 endcookie;
 stream >> endcookie;
 if (endcookie != BOSON_SAVEGAME_END_COOKIE) {
	boError() << k_funcinfo << "Invalid end cookie!" << endl;
	return false;
 }

 d->mLoadingStatus = BosonSaveLoad::LoadingCompleted;
 boDebug() << k_funcinfo << " done" << endl;
 return true;
}

int Boson::loadingStatus() const
{
 return (int)d->mLoadingStatus;
}

bool Boson::advanceFlag() const
{
 return d->mAdvance->advanceFlag();
}

void Boson::slotUpdateProductionOptions()
{
 emit signalUpdateProductionOptions();
}

void Boson::slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* p)
{
 // just forward it to BosonWidgetBase
 emit signalAddChatSystemMessage(fromName, text, p);
}

void Boson::slotAddChatSystemMessage(const QString& text, const Player* p)
{
 slotAddChatSystemMessage(i18n("Boson"), text, p);
}

unsigned int Boson::delayedMessageCount() const
{
 return d->mMessageDelayer->delayedMessageCount();
}

unsigned int Boson::delayedAdvanceMessageCount() const
{
 return d->mMessageDelayer->delayedAdvanceMessageCount();
}

Q_UINT16 Boson::bosonPort()
{
 return KGame::port();
}

QString Boson::bosonHostName()
{
 return KGame::hostName();
}

void Boson::bosonAddPlayer(KPlayer* player)
{
 KGame::addPlayer(player);
}

void Boson::killPlayer(Player* player)
{
 if (d->mCanvas) {
	while (player->allUnits()->count() > 0) {
		d->mCanvas->destroyUnit(player->allUnits()->first());
	}
 }
 player->setMinerals(0);
 player->setOil(0);
 boDebug() << k_funcinfo << "player " << player->id() << " is out of game" << endl;
 emit signalPlayerKilled(player);
}

void Boson::makeGameLog()
{
 QByteArray log;
 QTextStream ts(log, IO_WriteOnly);
 writeGameLog(ts);
// boDebug() << k_funcinfo << "Log size: " << log.size() << endl;
 BosonProfiler p(BosonProfiling::MakeGameLog);
 QByteArray comp = qCompress(log);
 d->mGameLogs.append(comp);
// boDebug() << k_funcinfo << "Done, elapsed: " << p.stop() << endl;
// boDebug() << k_funcinfo << "Compressed log size: " << comp.size() << endl;
}

void Boson::makeUnitLog()
{
 QByteArray log;
 QTextStream ts(log, IO_WriteOnly);

 ts << "CYCLE " << advanceCallsCount() << ":" << endl;
 QPtrListIterator<KPlayer> it(*playerList());
 while (it.current()) {
	ts << "Player " << it.current()->id() << endl;;
	QPtrListIterator<Unit> uit(*((Player*)it.current())->allUnits());
	while (uit.current()) {
		Unit* u = uit.current();
		BosonPathInfo* path = u->pathInfo();
		// Generic info
		ts << "Unit " << u->id() << ":  pos(" << u->x() << "; " << u->y() << "; " << u->z() <<
				"); rot(" << u->xRotation() << "; " << u->yRotation() << "; " << u->rotation() <<
				"); speed: " << u->speed() << "; maxSpeed: " << u->maxSpeed() <<
				"); work: " << (int)u->work() << "; advWork: " << (int)u->advanceWork() <<
				"); movingStatus: " << (int)u->movingStatus() << "; advWork: " << (int)u->advanceWork() <<
				"; health: " << u->health();
		// Target
		if (u->target()) {
			ts << " target: " << u->target()->id();
		}
		ts << endl;
		// Waypoints
		ts << "        " << u->waypointCount() << " waypoints:";
		for (QValueList<BoVector2Fixed>::const_iterator pit = u->waypointList().begin(); pit != u->waypointList().end(); pit++) {
			ts << " (" << (*pit).x() << "; " << (*pit).y() << ")";
		}
		ts << endl;
		// Pathpoints
		ts << "        " << u->pathPointList().count() << " pathpoints:";
		for (QValueList<BoVector2Fixed>::const_iterator pit = u->pathPointList().begin(); pit != u->pathPointList().end(); pit++) {
			ts << " (" << (*pit).x() << "; " << (*pit).y() << ")";
		}
		ts << endl;
		// PathInfo
		if (path) {
			ts << "        " << "PathInfo:  start(" << path->start.x() << "; " << path->start.y() <<
					"); dest(" << path->dest.x() << "; " << path->dest.y() << "); hlstep: " << path->hlstep <<
					"; range: " << path->range << "; passable: " << path->passable <<
					"; canMove(" << path->canMoveOnLand << "; " << path->canMoveOnWater <<
					"); flying: " << path->flying << "; pass: " << (int)path->passability <<
					"; attack: " << path->moveAttacking << "; slow: " << path->slowDownAtDest <<
					"; waiting: " << path->waiting << "; recalc: " << path->pathrecalced << endl;
		}

		++uit;
	}
	ts << endl;
	++it;
 }
 ts << endl;
 ts << endl;

 QByteArray comp = qCompress(log);
 d->mUnitLogs.append(comp);
}

void Boson::writeGameLog(QTextStream& log)
{
 BosonProfiler p(BosonProfiling::WriteGameLog);

 log << "Advance calls count: " << advanceCallsCount() << endl;
 QPtrListIterator<KPlayer> it(*playerList());
 while (it.current()) {
	((Player*)it.current())->writeGameLog(log);
	++it;
 }

 log << endl << endl;
// boDebug() << k_funcinfo << "Done, elapsed: " << p.stop() << endl;
}

bool Boson::saveGameLogs(const QString& prefix)
{
 // AB: note that this could even be called _after_ boson has crashed!
 // try to avoid most methods/classes/pointers
 // (actually the profiling and boDebug stuff shouldnt be here, either)
 BosonProfiler p(BosonProfiling::SaveGameLogs);

 // this one can be used to reproduce a game.
 // therefore we start with this one, if everything else fails we still have it.
 QFile messageLog(prefix + ".messagelog");
 if (!messageLog.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << prefix << ".messagelog' for writing!" << endl;
	return false;
 }
 if (!d->mMessageLogger.saveMessageLog(&messageLog)) {
	boError() << k_funcinfo << "unable to write message log" << endl;
	return false;
 }
 messageLog.close();
 boDebug() << k_funcinfo << "message log saved to " << messageLog.name() << endl;

 // Write gamelog
 QFile gameLog(prefix + ".gamelog");
 if (!gameLog.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << gameLog.name() << "' for writing gamelog!" << endl;
	return false;
 }
 QValueList<QByteArray>::iterator it;
 for (it = d->mGameLogs.begin(); it != d->mGameLogs.end(); it++) {
	gameLog.writeBlock(qUncompress(*it));
 }
 gameLog.close();

#ifdef COLLECT_UNIT_LOGS
 // Write unitlog
 QFile unitLog(prefix + ".unitlog");
 if (!unitLog.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << unitLog.name() << "' for writing unitlog!" << endl;
	return false;
 }
 QValueList<QByteArray>::iterator uit;
 for (uit = d->mUnitLogs.begin(); uit != d->mUnitLogs.end(); uit++) {
	unitLog.writeBlock(qUncompress(*uit));
 }
 unitLog.close();
#endif

 // Write network message log
 QFile netLog(prefix + ".netlog");
 if (!netLog.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << netLog.name() << "' for writing!" << endl;
	return false;
 }
 if (!d->mMessageLogger.saveHumanReadableMessageLog(&netLog)) {
	boError() << k_funcinfo << "unable to write (human readable) message log" << endl;
	return false;
 }
 netLog.close();

 boDebug() << k_funcinfo << "Done, elapsed: " << p.stop() << endl;
 return true;
}

unsigned int Boson::advanceCallsCount() const
{
 return d->mAdvance->advanceCallsCount();
}

bool Boson::addNeutralPlayer()
{
 QPtrListIterator<KPlayer> it(*playerList());
 while (it.current()) {
	if (((Player*)it.current())->isNeutralPlayer()) {
		boWarning() << k_funcinfo << "already have a neutral player. removing." << endl;

		// note: this will _send_ a request to remove only. will get
		// removed once the message is received.
		removePlayer(it.current());
	}
	++it;
 }
 QValueList<QColor> colors = availableTeamColors();
 if (colors.count() == 0) {
	boError() << k_funcinfo << "no color for neutral player available. not enough colors." << endl;
	return false;
 }
 Player* p = new Player(true);
 p->setName(i18n("Neutral"));
 p->loadTheme(SpeciesTheme::speciesDirectory("Neutral"), colors.first());

 // will send a request for adding a player. player is added once the request is
 // received.
 bosonAddPlayer(p);
 return true;
}

static void ensureComputerIOListValid(Boson* boson, const QPtrList<KGameComputerIO>& computerIOList)
{
 if (!boson) {
	BO_NULL_ERROR(boson);
	return;
 }
 if (computerIOList.isEmpty()) {
	return;
 }
 QPtrList<KGameComputerIO> validList;
 QPtrListIterator<KPlayer> playerIt(*boson->playerList());
 while (playerIt.current()) {
	KGameComputerIO* io = (KGameComputerIO*)playerIt.current()->findRttiIO(KGameIO::ComputerIO);
	++playerIt;

	if (io) {
		validList.append(io);
	}
 }

 QPtrListIterator<KGameComputerIO> ioIt(computerIOList);
 while (ioIt.current()) {
	KGameComputerIO* io = ioIt.current();
	++ioIt;

	// we must not access the pointer directly, we first have to find out
	// that it is really valid
	if (!validList.contains(io)) {
		boError() << k_funcinfo << "ComputerIO " << io << " is an invalid pointer. quitting boson now. evil bug." << endl;
		exit(1);
		return;
	}

	// io should be a valid pointer. _if_ it still is invalid, it should
	// crash now.
	QString name = io->name();
	int reactionPeriod = io->reactionPeriod();
	boDebug() << k_funcinfo << name << " is a valid computer IO" << endl;
 }

}



// TODO: save all files in a certain directory, not in home
static void emergencySave(int signal)
{
 Boson* boson = Boson::boson();
 if (!boson) {
	return;
 }
 fprintf(stdout, "emergencySave(): retrieving current time for filenames\n");
 QDateTime time = QDateTime::currentDateTime();
 QString year, month, day, hour, minute, second;
 year.sprintf("%d", time.date().year());
 month.sprintf("%02d", time.date().month());
 day.sprintf("%02d", time.date().day());
 hour.sprintf("%02d", time.time().hour());
 minute.sprintf("%02d", time.time().minute());
 second.sprintf("%02d", time.time().second());
 QString prefix = QString("boson_crash-%1%2%3%4%5%6").
		arg(year).
		arg(month).
		arg(day).
		arg(hour).
		arg(minute).
		arg(second);
 fprintf(stdout, "emergencySave(): trying to save game logs\n");
 if (!boson->saveGameLogs(prefix)) {
	fprintf(stderr, "emergencySave(): game logs could not be saved\n");
 } else {
	fprintf(stdout, "emergencySave(): game logs saved\n");
 }
}


// At this point the game has been completely loaded/started already, all we
// have to do is to load the messages.
// (this is called after IdGameIsStarted is received)
bool Boson::loadFromLogFile()
{
 if (!d->mStartingObject) {
	BO_NULL_ERROR(d->mStartingObject);
	return false;
 }
 QString file = d->mStartingObject->logFile();
 if (file.isEmpty()) {
	boError() << k_funcinfo << "empty log filename" << endl;
	return false;
 }
 QFile f(file);
 if (!f.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "could not open " << file << " for reading" << endl;
	return false;
 }
 QPtrList<BoMessage> messages;
 messages.setAutoDelete(true);
 BoMessageLogger::loadMessageLog(&f, &messages);
 BoMessage* start = 0;
 QPtrListIterator<BoMessage> it(messages);
 while (it.current()) {
	if (it.current()->msgid == KGameMessage::IdUser + BosonMessage::IdGameIsStarted) {
		start = it.current();
	}
	++it;
 }
 if (!start) {
	boError() << k_funcinfo << "no IdGameIsStarted message found" << endl;
	return false;
 }

 while (!messages.isEmpty() && messages.getFirst() != start) {
	messages.removeFirst();
 }
 if (messages.getFirst() != start) {
	boError() << k_funcinfo << "oops - something went wrong" << endl;
	return false;
 }
 messages.removeFirst();

 while (!messages.isEmpty()) {
	BoMessage* m = messages.take(0);
	d->mMessageDelayer->delay(m);
 }

 d->mMessageDelayer->unlock();
 return true;
}

BoEventManager* Boson::eventManager() const
{
 return d->mEventManager;
}

void Boson::queueEvent(BoEvent* event)
{
 eventManager()->queueEvent(event);
}

bool Boson::loadCanvasConditions(const QDomElement& root)
{
 return canvasNonConst()->loadConditions(root);
}

bool Boson::saveCanvasConditions(QDomElement& root) const
{
 return canvas()->saveConditions(root);
}

const QPtrList<BoAdvanceMessageTimes>& Boson::advanceMessageTimes() const
{
 return d->mAdvance->advanceMessageTimes();
}

void Boson::slotBoDebugOutput(const BoDebugMessage& m)
{
 QString area = m.areaName();
 if (area.isEmpty()) {
	area = i18n("Debug");
 }
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 slotAddChatSystemMessage(area, i18n("DEBUG: %1").arg(message));
}

void Boson::slotBoDebugWarning(const BoDebugMessage& m)
{
 QString area = m.areaName();
 if (area.isEmpty()) {
	area = i18n("Debug");
 }
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 slotAddChatSystemMessage(area, i18n("WARNING: %1").arg(message));
}

void Boson::slotBoDebugError(const BoDebugMessage& m)
{
 QString area = m.areaName();
 if (area.isEmpty()) {
	area = i18n("Debug");
 }
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 slotAddChatSystemMessage(area, i18n("ERROR: %1").arg(message));
}

void Boson::slotClientLeftGame(int clientId, int oldgamestatus, KGame*)
{
 Q_UNUSED(clientId);
 Q_UNUSED(oldgamestatus);
 if (isServer()) {

	// AB: we don't know whether we had been the server before the client
	// left the game. so we just restart the timer and be happy in any way
	// (note that checking for d->mGameTimer->isActive() is not sufficient
	// as a) that behavior may change and b) it is not active when the game
	// is already paused)
	boDebug() << k_funcinfo << "we are the server now" << endl;
	d->mGameTimer->stop();
	QObject::disconnect(d->mGameTimer, SIGNAL(timeout()), this, SLOT(slotSendAdvance()));
	connect(d->mGameTimer, SIGNAL(timeout()), this, SLOT(slotSendAdvance()));

	// start the timer if game is not paused
	slotPropertyChanged(&d->mGamePaused);
 }
}

void Boson::clearDelayedMessages()
{
 d->mMessageDelayer->clearDelayedMessages();
}

void Boson::syncNetwork()
{
 boDebug() << k_funcinfo << endl;
 d->mNetworkSynchronizer->syncNetwork();
}

