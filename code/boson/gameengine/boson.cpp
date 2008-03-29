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
#include "boson.h"
#include "boson.moc"

#include "../defines.h"
#include "../../bomemory/bodummymemory.h"
#include "bosonmessageids.h"
#include "player.h"
#include "unit.h"
#include "unitplugins/productionplugin.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosoncanvas.h"
#include "bosongamestatistics.h"
#include "bosonstatistics.h"
#include "bosonplayfield.h"
#include "../global.h"
#include "../bosonprofiling.h"
#include "bodebug.h"
#include "bodebuglog.h"
#include "bosonsaveload.h"
#include "../bosonconfig.h"
#include "boevent.h"
#include "boeventmanager.h"
#include "bomessage.h"
#include "bosonplayerinputhandler.h"
#include "bosonnetworksynchronizer.h"
#include "bosonnetworktraffic.h"
#include "boeventloop.h"
#include "bosonpath.h"
#include "bosonmap.h"
#include "bosonplayerlistmanager.h"

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
 * See @ref KCrash::setEmergencyFunction
 *
 * This tries to save log messages, especially the player input that was made
 * until now. This might help at reproducing the crash.
 **/
static void emergencySave(int signal);
class BoMessageLogger;

BoAdvanceMessageTimes::BoAdvanceMessageTimes(int gameSpeed)
{
 if (gameSpeed <= 0 || gameSpeed >= 5000) {
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
		mBoson->sendMessage(0, BosonMessageIds::AdvanceN);
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
 mBoson->canvasNonConst()->setAdvanceFlag(mAdvanceFlag);

 if (mBoson->gameMode()) {
	// AB: slots connected to signals are not guaranteed to be executed in a
	//     certain order. But we must guarantee to execute
	//     BosonCanvas::slotAdvance() first, so we better call it manually.
	if (mBoson->canvasNonConst()) {
		mBoson->canvasNonConst()->slotAdvance(advanceCallsCount());
	}
	emit mBoson->signalAdvance(advanceCallsCount(), flag);
 }
 // AB: do _not_ connect to the signal!
 // -> slots may be called in random order, but we need well defined order
 // (otherwise network may get broken soon)
 mBoson->eventManager()->advance(advanceCallsCount());

 boDebug(300) << k_funcinfo << advanceCallsCount() << " DONE" << endl;

 mAdvanceCallsCount = mAdvanceCallsCount + 1;
}


class Boson::BosonPrivate
{
public:
	BosonPrivate()
	{
		mGameTimer = 0;
		mPlayerListManager = 0;
		mCanvas = 0;
		mPlayField = 0;

		mAdvance = 0;
		mMessageDelayer = 0;

		mEventManager = 0;

		mPlayerInputHandler = 0;
		mNetworkSynchronizer = 0;
		mNetworkTraffic = 0;

		mGameStatistics = 0;
	}
	QTimer* mGameTimer;

	BosonPlayerListManager* mPlayerListManager;
	BosonCanvas* mCanvas;
	BosonPlayField* mPlayField;

	KGamePropertyInt mGameSpeed;
	KGamePropertyBool mGamePaused;

	QValueList<QByteArray> mGameLogs;
	QValueList<QByteArray> mUnitLogs;
	BoMessageLogger mMessageLogger;

	BoAdvance* mAdvance;
	BoMessageDelayer* mMessageDelayer;

	BoEventManager* mEventManager;

	BosonPlayerInputHandler* mPlayerInputHandler;
	BosonNetworkSynchronizer* mNetworkSynchronizer;
	BosonNetworkTraffic* mNetworkTraffic;

	BosonGameStatistics* mGameStatistics;

	bool mGameIsOver;
	bool mLoadFromLogMode;
};


class BoGameLogSaver
{
public:
	BoGameLogSaver(Boson* boson, const QString& prefix);

	bool save();

protected:
	bool saveMessageLog();
	bool saveGameLog();
	bool saveUnitLog();
	bool saveNetworkLog();
	bool saveBoDebugLog();
	bool saveBacktrace();

private:
	QString mPrefix;
	Boson::BosonPrivate* d;
	Boson* mBoson;
};

BoGameLogSaver::BoGameLogSaver(Boson* boson, const QString& prefix)
{
 d = boson->d;
 mBoson = boson;
 mPrefix = prefix;
}

bool BoGameLogSaver::save()
{
 bool ret = true;
 // AB: note: usually we do not return if saving fails
 //           -> we should save as many logs as possible

 if (!saveMessageLog()) {
	boError() << k_funcinfo << "failed saving message log" << endl;
	ret = false;
 }
 if (!saveGameLog()) {
	boError() << k_funcinfo << "failed saving game log" << endl;
	ret = false;
 }
#ifdef COLLECT_UNIT_LOGS
 if (!saveUnitLog()) {
	boError() << k_funcinfo << "failed saving unit log" << endl;
	ret = false;
 }
#endif
 if (!saveNetworkLog()) {
	boError() << k_funcinfo << "failed saving network log" << endl;
	ret = false;
 }
 if (!saveBoDebugLog()) {
	boError() << k_funcinfo << "failed saving bodebug log" << endl;
	ret = false;
 }
 if (!saveBacktrace()) {
	boError() << k_funcinfo << "failed saving backtrace" << endl;
	ret = false;
 }


 return ret;
}

bool BoGameLogSaver::saveMessageLog()
{
 // this one can be used to reproduce a game.
 // therefore we start with this one, if everything else fails we still have it.
 QFile messageLog(mPrefix + ".messagelog");
 if (!messageLog.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << mPrefix << ".messagelog' for writing!" << endl;
	return false;
 }
 if (!d->mMessageLogger.saveMessageLog(&messageLog)) {
	boError() << k_funcinfo << "unable to write message log" << endl;
	return false;
 }
 messageLog.close();
 boDebug() << k_funcinfo << "message log saved to " << messageLog.name() << endl;
 return true;
}

bool BoGameLogSaver::saveGameLog()
{
 QFile gameLog(mPrefix + ".gamelog");
 if (!gameLog.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << gameLog.name() << "' for writing gamelog!" << endl;
	return false;
 }
 QValueList<QByteArray>::iterator it;
 for (it = d->mGameLogs.begin(); it != d->mGameLogs.end(); it++) {
	gameLog.writeBlock(qUncompress(*it));
 }
 gameLog.close();
 return true;
}

bool BoGameLogSaver::saveUnitLog()
{
 QFile unitLog(mPrefix + ".unitlog");
 if (!unitLog.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << unitLog.name() << "' for writing unitlog!" << endl;
	return false;
 }
 QValueList<QByteArray>::iterator uit;
 for (uit = d->mUnitLogs.begin(); uit != d->mUnitLogs.end(); uit++) {
	unitLog.writeBlock(qUncompress(*uit));
 }
 unitLog.close();
 return true;
}

bool BoGameLogSaver::saveNetworkLog()
{
 QFile netLog(mPrefix + ".netlog");
 if (!netLog.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << netLog.name() << "' for writing!" << endl;
	return false;
 }
 if (!d->mMessageLogger.saveHumanReadableMessageLog(&netLog)) {
	boError() << k_funcinfo << "unable to write (human readable) message log" << endl;
	return false;
 }
 netLog.close();
 return true;
}

bool BoGameLogSaver::saveBoDebugLog()
{
 BoDebugLog* debugLog = BoDebugLog::debugLog();
 if (!debugLog) {
	BO_NULL_ERROR(debugLog);
	return false;
 }
 const QPtrList<BoDebugMessage>* messages = debugLog->messageLogLevel(-1);
 if (!messages) {
	BO_NULL_ERROR(messages);
	return false;
 }
 QFile log(mPrefix + ".bodebuglog");
 if (!log.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << log.name() << "' for writing!" << endl;
	return false;
 }
 QTextStream stream(&log);
 for (QPtrListIterator<BoDebugMessage> it(*messages); it.current(); ++it) {
	QString level;
	switch (it.current()->level()) {
		case BoDebug::KDEBUG_INFO:
			level = "INFO";
			break;
		case BoDebug::KDEBUG_WARN:
			level = "WARNING";
			break;
		case BoDebug::KDEBUG_ERROR:
			level = "ERROR";
			break;
		case BoDebug::KDEBUG_FATAL:
			level = "FATAL";
			break;
		default:
			level = "UNKNOWN";
			break;
	}
	stream << level << " (" << it.current()->area() << ", " << it.current()->areaName() << "): " << it.current()->message();
	if (!it.current()->backtrace().isEmpty()) {
		stream << it.current()->backtrace();
	}
 }

 log.close();
 return true;
}

bool BoGameLogSaver::saveBacktrace()
{
 QFile log(mPrefix + ".backtrace");
 if (!log.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << log.name() << "' for writing!" << endl;
	return false;
 }
 QTextStream stream(&log);
 QString bt = boBacktrace();
 if (bt.isEmpty()) {
	stream << "No backtrace could be generated on this system. Use gdb to create a backtrace instead\n";
 } else {
	stream << bt;
 }
 return true;
}




Boson::Boson(QObject* parent) : KGame(BOSON_COOKIE, parent)
{
 setPolicy(PolicyClean);
 d = new BosonPrivate;
 d->mPlayerListManager = new BosonPlayerListManager(this);
 d->mPlayerInputHandler = new BosonPlayerInputHandler(this);
 connect(d->mPlayerInputHandler, SIGNAL(signalChangeTexMap(int, int, unsigned int, unsigned int*, unsigned char*)),
		this, SLOT(slotChangeTexMap(int, int, unsigned int, unsigned int*, unsigned char*)));
 connect(d->mPlayerInputHandler, SIGNAL(signalChangeHeight(int, int, float)),
		this, SIGNAL(signalChangeHeight(int, int, float)));

 connect(d->mPlayerInputHandler, SIGNAL(signalEditorNewUndoMessage(const BosonMessageEditorMove&, bool)),
		this, SIGNAL(signalEditorNewUndoMessage(const BosonMessageEditorMove&, bool)));
 connect(d->mPlayerInputHandler, SIGNAL(signalEditorNewRedoMessage(const BosonMessageEditorMove&)),
		this, SIGNAL(signalEditorNewRedoMessage(const BosonMessageEditorMove&)));
 d->mAdvance = new BoAdvance(this);
 d->mMessageDelayer = new BoMessageDelayer(this);
 d->mNetworkSynchronizer = new BosonNetworkSynchronizer();
 d->mNetworkTraffic = new BosonNetworkTraffic(this);
 d->mGameStatistics = new BosonGameStatistics(this);

 d->mGameTimer = new QTimer(this);

 d->mNetworkSynchronizer->setGame(this);
 d->mNetworkSynchronizer->setMessageLogger(&d->mMessageLogger);
 d->mNetworkTraffic->setBoson(this);
 d->mGameStatistics->setGame(this);


 mGameMode = true;
 d->mGameIsOver = false;
 d->mLoadFromLogMode = false;

 connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)),
		this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)));
 connect(this, SIGNAL(signalReplacePlayerIO(KPlayer*, bool*)),
		this, SLOT(slotReplacePlayerIO(KPlayer*, bool*)));
 connect(this, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(this, SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SLOT(slotPlayerLeftGame(KPlayer*)));
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

 // AB: emergencySaveFunction() most likely is useless currently.
 //     the log will be saved properly, but since the starting procedure has
 //     been changed completely, we most likely won't be able to load it anyway.
#warning FIXME: emergencySaveFunction log loading
#if 1
 if (KCrash::emergencySaveFunction() != NULL) {
	boError() << k_funcinfo << "oops - already an emergencySaveFunction set! overwriting!" << endl;
 }
 KCrash::setEmergencySaveFunction(emergencySave);
#endif

 ((BoEventLoop*)qApp->eventLoop())->setAdvanceMessageInterval(ADVANCE_INTERVAL);
 ((BoEventLoop*)qApp->eventLoop())->setAdvanceObject(this);
}

Boson::~Boson()
{
 ((BoEventLoop*)qApp->eventLoop())->setAdvanceObject(0);
 KCrash::setEmergencySaveFunction(NULL);
 delete d->mNetworkSynchronizer;
 delete d->mGameStatistics;
 delete d->mPlayerInputHandler;
 delete d->mMessageDelayer;
 delete d->mAdvance;
 delete d->mGameTimer;
 delete d->mCanvas;
 delete d->mPlayerListManager;
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

bool Boson::createCanvas(BosonMap* map)
{
 if (d->mCanvas) {
	boWarning() << k_funcinfo << "there is already a canvas created! not touching that object..." << endl;
	// do NOT delete it, as it might be used somewhere else as well
	return false;
 }
 d->mCanvas = new BosonCanvas(advanceFlag(), this, gameMode());
 connect(d->mCanvas, SIGNAL(signalGameOver()),
		this, SLOT(slotGameOver()));
 if (!d->mCanvas->init(map, playerListManager(), eventManager())) {
	boError() << k_funcinfo << "initializing the canvas failed" << endl;
	return false;
 }
 return true;
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
}

BosonPlayField* Boson::playField() const
{
 return d->mPlayField;
}

PlayerIO* Boson::findPlayerIO(Q_UINT32 id) const
{
 Player* p = (Player*)findPlayerByKGameId(id);
 if (p) {
	return p->playerIO();
 }
 return 0;
}

PlayerIO* Boson::playerIOAtAllIndex(unsigned int index) const
{
 unsigned int i = 0;
 for (QPtrListIterator<Player> it(allPlayerList()); it.current(); ++it) {
	if (index == i) {
		return it.current()->playerIO();
	}
	i++;
 }
 return 0;
}

PlayerIO* Boson::playerIOAtGameIndex(unsigned int index) const
{
 unsigned int i = 0;
 for (QPtrListIterator<Player> it(gamePlayerList()); it.current(); ++it) {
	if (index == i) {
		return it.current()->playerIO();
	}
	i++;
 }
 return 0;
}

PlayerIO* Boson::playerIOAtActiveGameIndex(unsigned int index) const
{
 unsigned int i = 0;
 for (QPtrListIterator<Player> it(activeGamePlayerList()); it.current(); ++it) {
	if (index == i) {
		return it.current()->playerIO();
	}
	i++;
 }
 return 0;
}

const QPtrList<Player>& Boson::allPlayerList() const
{
 return d->mPlayerListManager->allPlayerList();
}

const QPtrList<Player>& Boson::gamePlayerList() const
{
 return d->mPlayerListManager->gamePlayerList();
}

const QPtrList<Player>& Boson::activeGamePlayerList() const
{
 return d->mPlayerListManager->activeGamePlayerList();
}

unsigned int Boson::allPlayerCount() const
{
 return d->mPlayerListManager->allPlayerCount();
}

unsigned int Boson::gamePlayerCount() const
{
 return d->mPlayerListManager->gamePlayerCount();
}

unsigned int Boson::activeGamePlayerCount() const
{
 return d->mPlayerListManager->activeGamePlayerCount();
}

void Boson::sendMessageSyncRandom()
{
 syncRandom();
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

 boDebug() << k_funcinfo << " done" <<  endl;
}

void Boson::removeAllPlayers()
{
 QPtrList<Player> list = allPlayerList();
 for (unsigned int i = 0; i < list.count(); i++) {
	removePlayer(list.at(i)); // might not be necessary - sends remove over network
	systemRemovePlayer(list.at(i), true); // remove immediately, even before network removing is received.
 }
 recalculatePlayerLists();
}

bool Boson::playerInput(QDataStream& stream, KPlayer* p)
{
 return d->mPlayerInputHandler->playerInput(stream, (Player*)p);
}

void Boson::systemAddPlayer(KPlayer* p)
{
 BO_CHECK_NULL_RET(p);
 if (p->userId() == 0) {
	int userId = 128;
	while (findPlayerByUserId(userId) != 0) {
		userId++;
	}
	boDebug() << k_funcinfo << "player " << p->kgameId() << " gets userId " << userId << endl;
	p->setUserId(userId);
 } else {
	boDebug() << k_funcinfo << "player " << p->kgameId() << " already has a userId: " << p->userId() << endl;
 }
 bool have = false;
 if (playerList()->contains(p)) {
	have = true;
 }
 blockSignals(true);
 KGame::systemAddPlayer(p);
 blockSignals(false);

 recalculatePlayerLists();
 if (!have) {
	if (playerList()->contains(p)) {
		emit signalPlayerJoinedGame(p);
	}
 }
}

void Boson::systemRemovePlayer(KPlayer* p, bool deleteIt)
{
 // AB: make sure that the player lists do NOT include this player anymore, when
 //     signalPlayerLeftGame() is emitted
 recalculatePlayerListsWithPlayerRemoved(p);
 KGame::systemRemovePlayer(p, deleteIt);
}

void Boson::slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 , Q_UINT32 sender)
{
 QDataStream stream(buffer, IO_ReadOnly);
 switch (msgid) {
	case BosonMessageIds::AdvanceN:
	{
		BO_CHECK_NULL_RET(canvas());
		d->mNetworkSynchronizer->receiveAdvanceMessage(d->mCanvas);
		d->mGameStatistics->receiveAdvanceMessage(d->mCanvas);
		d->mAdvance->receiveAdvanceMessage(gameSpeed());
		break;
	}
	case BosonMessageIds::ChangeMap:
	{
		emit signalEditorNewMap(buffer);
		break;
	}
	case BosonMessageIds::IdNewGame:
	{
		clearUndoStacks();
		if (isRunning()) {
			boError() << k_funcinfo << "received IdNewGame, but game is already running" << endl;
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
		QByteArray compresseddata;
		stream >> compresseddata;
		QByteArray data = qUncompress(compresseddata);
		bool taken = false;
		emit signalSetNewGameData(data, &taken);
		if (!taken) {
			boError() << k_funcinfo << "newgame data not taken - slot not connected?" << endl;
			// TODO: message box ; back to newgame widget?
			return;
		}
		boGame->lock();
		QTimer::singleShot(0, this, SIGNAL(signalStartNewGame()));
		break;
	}
	case BosonMessageIds::IdStartGameClicked:
		clearUndoStacks();

		// this is kind of a workaround.
		// for --start we need to call slotStart() in the start widgets
		// only once the (e.g.) playfield messages have arrived. for
		// this we use a message and *then* call slotStart() there.
		QTimer::singleShot(0, this, SIGNAL(signalStartGameClicked()));
		break;
	case BosonMessageIds::IdGameIsStarted:
	{
		QString loadFromLog;
		stream >> loadFromLog; // AB: obsolete!!
		if (sender != messageClient()->adminId()) {
			boError() << k_funcinfo << "only ADMIN is allowed to send IdGameIsStarted message! sender="
					<< sender << " ADMIN="
					<< messageClient()->adminId() << endl;
			break;
		}
		d->mGameIsOver = false;

		emit signalGameStarted();

		if (!d->mLoadFromLogMode) {
			if (isAdmin()) {
				connect(d->mGameTimer, SIGNAL(timeout()), this, SLOT(slotSendAdvance()));
			}
		}
		clearUndoStacks();
		break;
	}
	case BosonMessageIds::ChangeSpecies:
	{
		Q_UINT32 id;
		QString species;
		Q_UINT32 color;
		stream >> id;
		stream >> species;
		stream >> color;
		Player* p = (Player*)findPlayerByUserId(id);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << id << endl;
			return;
		}
		p->loadTheme(SpeciesTheme::speciesDirectory(species), QColor(color));
		emit signalSpeciesChanged(p);
		break;
	}
	case BosonMessageIds::ChangeSide:
	{
		Q_UINT32 id;
		Q_UINT32 newId;
		stream >> id;
		stream >> newId;
		Player* p = (Player*)findPlayerByUserId(id);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << id << endl;
			return;
		}
		if (findPlayerByUserId(newId) && (Player*)findPlayerByUserId(newId) != p) {
			boError() << k_lineinfo << "user id " << newId << " already taken by a player. won't change to that Id." << endl;
			return;
		}
		if (newId <= 0) {
			boError() << k_lineinfo << "Id 0 is not allowed to be taken by a player" << endl;
			return;
		}
		if (newId > 255) {
			boError() << k_lineinfo << "Ids > 255 are not allowed to be taken by a real player. Use Ids between 1 and 127 to \"watch\" a game" << endl;
			return;
		}
		if (gameStatus() != KGame::Init) {
			boError() << k_lineinfo << "not in Init state" << endl;
			return;
		}
		changeUserIdOfPlayer(p, newId);
		break;
	}
	case BosonMessageIds::ChangeTeamColor:
	{
		Q_UINT32 id;
		Q_UINT32 color;
		stream >> id;
		stream >> color;
		Player* p = (Player*)findPlayerByUserId(id);
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
	case BosonMessageIds::ChangePlayField:
	{
		QString field;
		stream >> field;
		emit signalPlayFieldChanged(field);
		break;
	}
	case BosonMessageIds::IdChat:
	{
		break;
	}
	case BosonMessageIds::IdKillPlayer:
	{
		BO_CHECK_NULL_RET(canvas());
		Player* p = 0;
		Q_UINT32 id = 0;
		stream >> id;
		p = (Player*)findPlayerByUserId(id);
		BO_CHECK_NULL_RET(p);
		BO_CHECK_NULL_RET(d->mCanvas);
		killPlayer(p);
		slotAddChatSystemMessage(i18n("Debug"), i18n("Killed player %1 - %2").arg(p->bosonId()).arg(p->name()));
		break;
	}
	case BosonMessageIds::IdModifyMinerals:
	{
		BO_CHECK_NULL_RET(canvas());
		Player* p = 0;
		Q_INT32 change = 0;
		Q_UINT32 id = 0;
		stream >> id;
		stream >> change;
		p = (Player*)findPlayerByUserId(id);
		BO_CHECK_NULL_RET(p);
		if ((Q_INT32)p->minerals() + change < 0) {
		}
		p->setMinerals(p->minerals() + change);
		break;
	}
	case BosonMessageIds::IdModifyOil:
	{
		BO_CHECK_NULL_RET(canvas());
		Player* p = 0;
		Q_INT32 change = 0;
		Q_UINT32 id = 0;
		stream >> id;
		stream >> change;
		p = (Player*)findPlayerByUserId(id);
		BO_CHECK_NULL_RET(p);
		if ((Q_INT32)p->oil() + change < 0) {
			p->setOil(0);
		} else {
			p->setOil(p->oil() + change);
		}
		break;
	}
	case BosonMessageIds::IdGameStartingCompleted:
	{
		// this message is a kind of ACK from the client. it indicates
		// that the starting is done and we are waiting for the ADMIN to
		// start the game. he will do so once all clients have sent this
		// message.
		if (gameStatus() != KGame::Init) {
			// the message is not allowed here
			boError() << k_funcinfo << "not in Init state" << endl;
			break;
		}
		emit signalStartingCompletedReceived(buffer, sender);
		break;
	}
	case BosonMessageIds::IdNetworkSyncCheck:
	{
		BO_CHECK_NULL_RET(canvas());
		if (!d->mNetworkSynchronizer->receiveNetworkSyncCheck(stream)) {
			// the network is not in sync anymore.
			// note that we don't have to do anything here, it is
			// done in the network synchronizer class.
		}
		break;
	}
	case BosonMessageIds::IdNetworkSyncCheckACK:
	{
		BO_CHECK_NULL_RET(canvas());
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
	case BosonMessageIds::IdNetworkRequestSync:
	{
		BO_CHECK_NULL_RET(canvas());
		if (!d->mNetworkSynchronizer->receiveNetworkRequestSync(stream)) {
			slotAddChatSystemMessage(i18n("Could not synchronize clients. Cannot fix out-of-sync client. Sorry"));
			boError() << k_funcinfo << "unable to synchronize clients. cannot fix out-of-sync." << endl;
			break;
		}
		break;
	}
	case BosonMessageIds::IdNetworkSync:
	{
		BO_CHECK_NULL_RET(canvas());
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
	case BosonMessageIds::IdNetworkSyncUnlockGame:
	{
		d->mNetworkSynchronizer->receiveNetworkSyncUnlockGame(stream);
		break;
	}
	case BosonMessageIds::IdStatus:
	{
		// This message is only meant for the dedicated server. Ignore it.
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
 QPtrListIterator<Player> it(gamePlayerList());
 while (it.current()) {
	Unit* unit = it.current()->findUnit(id);
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
	p->initMap(d->mPlayField->map(), boGame->gameMode(), boGame->gameMode());
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
 slotAddChatSystemMessage(i18n("Player %1(%2) left the game. Units of that player remain on the map.").arg(player->name()).arg(((Player*)player)->bosonId()));
// boDebug() << k_funcinfo << endl;
}

bool Boson::buildProducedUnitAtTopLeftPos(ProductionPlugin* factory, unsigned long int unitType, BoVector2Fixed topLeftPos)
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
 if (!factory->unit()) {
	boError() << k_funcinfo << "NULL factory unit" << endl;
	return false;
 }
 if (!(d->mCanvas)->canPlaceUnitAtTopLeftPos(p->unitProperties(unitType), topLeftPos, 0)) {
	boDebug() << k_funcinfo << "Cannot create unit here" << endl;
	return false;
 }
 BoVector3Fixed pos3(topLeftPos.x(), topLeftPos.y(), 0.0f);
 Unit* unit = (Unit*)d->mCanvas->createNewItemAtTopLeftPos(RTTI::UnitStart + unitType, p, ItemType(unitType), pos3);
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return false;
 }

 if (!unit->isFlying()) {
	bofixed rotateX = 0.0f;
	bofixed rotateY = 0.0f;
	bofixed moveZ;
	unit->updateZ(0, 0, &moveZ, &rotateX, &rotateY);
	unit->setXRotation(rotateX);
	unit->setYRotation(rotateY);
	((BosonItem*)unit)->moveBy(0, 0, moveZ);
 }


 // the current production is done.
 factory->productionPlaced(unit);

 return true;
}


void Boson::slotPlayerJoinedGame(KPlayer* p)
{
 boDebug() << k_funcinfo << endl;
 if (!p) {
	return;
 }
 slotAddChatSystemMessage(i18n("Player %1 - %2 joined").arg(((Player*)p)->bosonId()).arg(p->name()));
}

void Boson::slotPlayerLeftGame(KPlayer* p)
{
 if (!p) {
	return;
 }
 slotAddChatSystemMessage(i18n("Player %1 - %2 left the game").arg(((Player*)p)->bosonId()).arg(p->name()));
}

QValueList<QColor> Boson::availableTeamColors() const
{
 QValueList<QColor> colors = SpeciesTheme::defaultColors();
 QPtrListIterator<Player> it(gamePlayerList());
 while (it.current()) {
	if (it.current()->speciesTheme()) {
		colors.remove(it.current()->speciesTheme()->teamColor());
	}
	++it;
 }
 return colors;
}

void Boson::slotReceiveAdvance()
{
 if (d->mGameIsOver) {
	return;
 }

 // Log game state
 if (advanceCallsCount() % boConfig->intValue("GameLogInterval") == 0) {
	//makeGameLog();
 }
#ifdef COLLECT_UNIT_LOGS
 makeUnitLog();
#endif

 d->mAdvance->receiveAdvanceCall();
}

void Boson::setLoadFromLogComplete()
{
 d->mLoadFromLogMode = false;
}

void Boson::networkTransmission(QDataStream& stream, int msgid, Q_UINT32 r, Q_UINT32 s, Q_UINT32 clientId)
{
 if (d->mLoadFromLogMode) {
	// ignoring any network messages while loading from log.
	return;
 }
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
 if (m->msgid == KGameMessage::IdSetupGame) {
	// AB: this (IdSetupGame) is the first step of KGame network initialization.
	//     this message is received by the client (non-ADMIN) only.


	QPtrList<Player> allPlayers = allPlayerList();
	for (unsigned int i = 0; i < allPlayers.count(); i++) {
		// AB: all client players will be added again by the KGame
		//     initialization:
		//     the setupGame() inactivates the players and in
		//     setupGameContinue() they are reactivated again (by the
		//     ADMIN).
		//
		//     this "reactivation" is very buggy in KGame: the ADMIN
		//     cannot activate it, as it doesn't have these player
		//     objects yet (those that are created into the
		//     newPlayerList are deleted at the end of
		//     setupGameContinue() due to auto deletion).
		//     however the "reactivation" is received by the client
		//     which calls addPlayer() and this one eventually adds the
		//     new player on the ADMIN, too.
		//
		//     however the player is then added with the (old)
		//     client-side user ID - the ADMIN has never a chance to
		//     assign the correct ID.
		//
		//     Therefore we assign the (invalid) ID 0 here which causes
		//     both clients to pick a new ID when the player is added.
		allPlayers.at(i)->setUserId(0);
		recalculatePlayerLists();
	}
 }

 // AB: we cannot do access control with "sender", as this field can be provided
 //     to KGame::sendSystemMessage(), i.e. is given by the sender.
 //     However the clientId parameter is provided on this side of the
 //     connection, i.e. by a trustworthy source.
 //     So we need to check if sender actually matches the clientId and then we
 //     can ignore clientId (KGame does so - after this point clientId will be
 //     dropped!)
 //
 //     code not activated, as it is untested.
#if 0
 if (KGameMessage::rawGameId(m->sender) != m->clientId) {
	boError() << k_funcinfo << "sender lied. sender said he was " << m->sender << " but actually " << m->clientId << " sent this message. not accepting." << endl;
	return;
 }
#endif

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
 delete save;
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

 // Load magic data
 Q_UINT8 a, b1, b2, b3;
 Q_INT32 c;
 Q_UINT32 v;
 stream >> a >> b1 >> b2 >> b3;
 if ((a != 128) || (b1 != 'B' || b2 != 'S' || b3 != 'G')) {
	// Error - not Boson SaveGame
	boError() << k_funcinfo << "invalid magic cookie" << endl;
	return false;
 }
 stream >> c;
 if (c != cookie()) {
	// Error - wrong cookie
	boError() << k_funcinfo << "Invalid cookie in header (found: " << c << "; should be: " << cookie() << ")" << endl;
	return false;
 }
 stream >> v;
 if (v != BosonSaveLoad::latestSavegameVersion()) {
	// Error - older version
	boError() << k_funcinfo << "Unsupported format version (found: " << v << "; latest: " << BosonSaveLoad::latestSavegameVersion() << ")" << endl;
	return false;
 }

 // Load KGame stuff
 boDebug() << "calling KGame::loadgame" << endl;
 if (!KGame::loadgame(stream, network, reset)) {
	// KGame loading error
	boError() << k_funcinfo << "KGame loading error" << endl;
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

 boDebug() << k_funcinfo << " done" << endl;
 return true;
}

bool Boson::advanceFlag() const
{
 return d->mAdvance->advanceFlag();
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
 boDebug() << k_funcinfo << "player " << player->bosonId() << " is out of game" << endl;
}

void Boson::makeGameLog()
{
 PROFILE_METHOD;
 QByteArray log;
 QTextStream ts(log, IO_WriteOnly);
 writeGameLog(ts);
// boDebug() << k_funcinfo << "Log size: " << log.size() << endl;
 boProfiling->push("compress log");
 QByteArray comp = qCompress(log);
 boProfiling->pop();
 d->mGameLogs.append(comp);
// boDebug() << k_funcinfo << "Done, elapsed: " << p.stop() << endl;
// boDebug() << k_funcinfo << "Compressed log size: " << comp.size() << endl;
}

void Boson::makeUnitLog()
{
 QByteArray log;
 QTextStream ts(log, IO_WriteOnly);

 ts << "CYCLE " << advanceCallsCount() << ":" << endl;
 QPtrListIterator<Player> it(gamePlayerList());
 while (it.current()) {
	ts << "Player " << it.current()->bosonId() << endl;
	QPtrListIterator<Unit> uit(*it.current()->allUnits());
	while (uit.current()) {
		Unit* u = uit.current();
		BosonPathInfo* path = u->pathInfo();
		// Generic info
		ts << "Unit " << u->id() << ":  pos(" << u->centerX() << "; " << u->centerY() << "; " << u->z() <<
				"); rot(" << u->xRotation() << "; " << u->yRotation() << "; " << u->rotation() <<
				"); speed: " << u->speed() << "; maxSpeed: " << u->maxSpeed() <<
				"); advWork: " << (int)u->advanceWork() <<
				"); movingStatus: " << (int)u->movingStatus() << "; advWork: " << (int)u->advanceWork() <<
				"; health: " << u->health();
		// Target
		ts << endl;
		// Pathpoints
		ts << "        " << u->pathPointList().count() << " pathpoints:";
		for (QValueList<BoVector2Fixed>::const_iterator pit = u->pathPointList().begin(); pit != u->pathPointList().end(); pit++) {
			ts << " (" << (*pit).x() << "; " << (*pit).y() << ")";
		}
		ts << endl;
		// PathInfo
		/*if (path) {
			ts << "        " << "PathInfo:  start(" << path->start.x() << "; " << path->start.y() <<
					"); dest(" << path->dest.x() << "; " << path->dest.y() << "); hlstep: " << path->hlstep <<
					"; range: " << path->range << "; passable: " << path->passable <<
					"; canMove(" << path->canMoveOnLand << "; " << path->canMoveOnWater <<
					"); flying: " << path->flying << "; pass: " << (int)path->passability <<
					"; attack: " << path->moveAttacking << "; slow: " << path->slowDownAtDest <<
					"; waiting: " << path->waiting << "; recalc: " << path->pathrecalced << endl;
		}*/

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
 PROFILE_METHOD;

 log << "Advance calls count: " << advanceCallsCount() << endl;
 QPtrListIterator<Player> it(gamePlayerList());
 while (it.current()) {
	it.current()->writeGameLog(log);
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
 BosonProfiler p("saveGameLogs");

 BoGameLogSaver logs(this, prefix);
 bool ret = logs.save();
 boDebug() << k_funcinfo << "Done, elapsed: " << p.elapsedSinceStart() << endl;
 return ret;
}

unsigned int Boson::advanceCallsCount() const
{
 return d->mAdvance->advanceCallsCount();
}

Player* Boson::addNeutralPlayer()
{
 QPtrListIterator<Player> it(gamePlayerList());
 while (it.current()) {
	if (it.current()->isNeutralPlayer()) {
		boWarning() << k_funcinfo << "already have a neutral player. removing." << endl;

		// note: this will _send_ a request to remove only. will get
		// removed once the message is received.
		removePlayer(it.current());
	}
	++it;
 }

#warning TODO: use a fixed color for neutral player (that cant be selected by other players)
 QValueList<QColor> colors = availableTeamColors();
 if (colors.count() == 0) {
	boError() << k_funcinfo << "no color for neutral player available. not enough colors." << endl;
	return 0;
 }
 Player* p = new Player(true);
 p->setUserId(256);
 p->setName(i18n("Neutral"));
 p->loadTheme(SpeciesTheme::speciesDirectory("Neutral"), colors.first());

 // will send a request for adding a player. player is added once the request is
 // received.
 bosonAddPlayer(p);
 return p;
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
bool Boson::loadFromLogFile(const QString& file)
{
 if (d->mLoadFromLogMode) {
	boError() << k_funcinfo << "already loading from log" << endl;
	return false;
 }
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
	if (it.current()->msgid == KGameMessage::IdUser + BosonMessageIds::IdGameIsStarted) {
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

 return loadFromLog(&messages);
}

bool Boson::loadFromLog(QPtrList<BoMessage>* messages)
{
 if (!messages) {
	return false;
 }
 boDebug() << k_funcinfo << "delaying " << messages->count() << " messages." << endl;
 while (!messages->isEmpty()) {
	BoMessage* m = messages->take(0);
	d->mMessageDelayer->delay(m);
 }
 boDebug() << k_funcinfo << "messages delayed." << endl;

 d->mLoadFromLogMode = true;

 boDebug() << k_funcinfo << "unlocking messagedelayer - processing messages now." << endl;
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

const BosonNetworkTraffic* Boson::networkTraffic() const
{
 return d->mNetworkTraffic;
}

const QPtrList<BoAdvanceMessageTimes>& Boson::advanceMessageTimes() const
{
 return d->mAdvance->advanceMessageTimes();
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

void Boson::clearUndoStacks()
{
 emit signalEditorClearRedoStack();
 emit signalEditorClearUndoStack();
}

void Boson::slotGameOver()
{
 boDebug() << k_funcinfo << endl;

 d->mGameIsOver = true;

 // do not send any additional advance messages
 d->mGameTimer->stop();
 QObject::disconnect(d->mGameTimer, SIGNAL(timeout()), this, SLOT(slotSendAdvance()));

 emit signalGameOver();
}

void Boson::slotChangeTexMap(int x, int y, unsigned int textureCount, unsigned int* textures, unsigned char* alpha)
{
 BO_CHECK_NULL_RET(d->mPlayField);
 BO_CHECK_NULL_RET(d->mPlayField->map());
 d->mPlayField->map()->slotChangeTexMap(x, y, textureCount, textures, alpha);
 emit signalChangeTexMap(x, y, textureCount, textures, alpha);
}

bool Boson::changeUserIdOfPlayer(Player* p, unsigned int newId)
{
 if (newId == 0) {
	boError() << k_funcinfo << "invalid Id " << newId << endl;
	return false;
 }
 Player* idPlayer = (Player*)findPlayerByUserId(newId);
 if (idPlayer && idPlayer != p) {
	boError() << k_funcinfo << "Id " << newId << " is already taken" << endl;
	return false;
 }
 if (newId > 511) {
	boError() << k_funcinfo << "Ids > 511 are not yet used by Boson." << endl;
	return false;
 }
 p->setUserId(newId);
 recalculatePlayerLists();
 emit signalSideChanged(p);
 return true;
}

void Boson::recalculatePlayerLists()
{
 d->mPlayerListManager->recalculatePlayerLists(*playerList());
}

void Boson::recalculatePlayerListsWithPlayerRemoved(KPlayer* removedPlayer)
{
 d->mPlayerListManager->recalculatePlayerListsWithPlayerRemoved(*playerList(), removedPlayer);
}

BosonPlayerListManager* Boson::playerListManager() const
{
 return d->mPlayerListManager;
}

