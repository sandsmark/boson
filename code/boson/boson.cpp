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
#include "boson.h"

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
#include "upgradeproperties.h"
#include "bosonprofiling.h"
#include "bofile.h"
#include "bodebug.h"
#include "bosonweapon.h"
#include "bosonsaveload.h"
#include "bosonconfig.h"
#include "bosonstarting.h"
#include "startupwidgets/bosonloadingwidget.h"

#include <klocale.h>
#include <kdeversion.h>
#include <kgame/kgameio.h>
#include <kgame/kgamemessage.h>
#include <kgame/kgamepropertyhandler.h>
#include <kgame/kmessageclient.h>

#include <qbuffer.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qptrqueue.h>
#include <qdom.h>
#include <qdatastream.h>
#include <qtextstream.h>
#include <qcstring.h>
#include <qfile.h>
//#include <qintdict.h>

#ifndef KGAME_HAVE_KGAME_HOSTNAME
// AB: !KGAME_HAVE_KGAME_PORT implies !KGAME_HAVE_KGAME_HOSTNAME
// (KGame::hostName() was added after KGame::port())
#include <qsocket.h>
#include <qserversocket.h>
#include <qobjectlist.h>
#endif

#include "boson.moc"

Boson* Boson::mBoson = 0;

#define ADVANCE_INTERVAL 250 // ms


#if KDE_VERSION <= KDE_MAKE_VERSION(3,1,4)
#include <kgame/kmessageio.h>
#include <kmessagebox.h>
#include <qsocket.h>
static QMap<KMessageSocket*, BoMyKMessageSocket*> KMessageSocket2BoMyKMessageSocket;
static QMap<QSocket*, KMessageSocket*> QSocket2KMessageSocket;

static void fixKMessageSocketIsRecursive()
{
 if (!boGame) {
	return;
 }
 const QObjectList* list = QObject::objectTrees();
 QObjectListIt it(*list);
 for (; it.current(); ++it) {
	if (qstrcmp((*it)->className(), "KMessageSocket") != 0) {
		continue;
	}
	boDebug() << k_funcinfo << "found a KMessageSocket" << endl;
	if (KMessageSocket2BoMyKMessageSocket.contains((KMessageSocket*)(*it))) {
		boDebug() << k_funcinfo << "KMessageSocket already fixed" << endl;
		continue;
	}
	boDebug() << k_funcinfo << "fixing KMessageSocket" << endl;
	BoMyKMessageSocket* s = new BoMyKMessageSocket();
	QSocket* sock = 0;
	const QObjectList* list2 = QObject::objectTrees();
	QObjectListIt it2(*list2);
	for (; it2.current() && !sock; ++it2) {
		if (qstrcmp((*it2)->className(), "QSocket") != 0) {
			continue;
		}
		if (QSocket2KMessageSocket[(QSocket*)(*it2)] != 0) {
			continue;
		}
		if (sock) {
			boError() << k_funcinfo << "already a QSocket object found! baaaaaad! your netowork will probably be broken!" << endl;
			KMessageBox::sorry(0, i18n("A workaround for a bug in libkdegames from KDE < 3.2 failed. Your network will be broken."));
		}
		sock = (QSocket*)(*it2);
	}

	if (!sock) {
		boError() << k_funcinfo << "QSocket object not found" << endl;
		return;
	}

	s->setSocket((KMessageSocket*)(*it), sock);
	KMessageSocket2BoMyKMessageSocket.insert((KMessageSocket*)(*it), s);
	QSocket2KMessageSocket.insert(sock, (KMessageSocket*)(*it));
 }
}

void BoMyKMessageSocket::setSocket(KMessageSocket* s, QSocket* sock)
{
 mSocket = s;
 mSocketSocket = sock;
 connect(this, SIGNAL(signalReceived(const QByteArray&)), mSocket, SIGNAL(received(const QByteArray&)));

 disconnect(mSocketSocket, SIGNAL(readyRead()), mSocket, SLOT(processNewData()));
 connect(mSocketSocket, SIGNAL(readyRead()), this, SLOT(slotProcessNewData()));
 mNextBlockLength = 0;
 mAwaitingHeader = 0;
}

void BoMyKMessageSocket::slotProcessNewData()
{
 BO_CHECK_NULL_RET(mSocket);
 static bool isRecursive = false;
 if (isRecursive) {
	return;
 }

 isRecursive = true;

 QDataStream str(mSocketSocket);
 while (mSocketSocket->bytesAvailable() > 0) {
	if (mAwaitingHeader) {
		// Header = magic number + packet length = 5 bytes
		if (mSocketSocket->bytesAvailable() < 5) {
			isRecursive = false;
			return;
		}

		// Read the magic number first. If something unexpected is found,
		// start over again, ignoring the data that was read up to then.

		Q_UINT8 v;
		str >> v;
		if (v != 'M') {
			kdWarning(11001) << k_funcinfo << "Received unexpected data, magic number wrong!" << endl;
			continue;
		}

		str >> mNextBlockLength;
		mAwaitingHeader = false;
	} else {
		// Data not completly read => wait for more
		if (mSocketSocket->bytesAvailable() < (Q_ULONG) mNextBlockLength) {
			isRecursive = false;
			return;
		}

		QByteArray msg (mNextBlockLength);
		str.readRawBytes (msg.data(), mNextBlockLength);

		// send the received message
		emit signalReceived (msg);

		// Waiting for the header of the next message
		mAwaitingHeader = true;
	}
 }

 isRecursive = false;
}

#else
void BoMyKMessageSocket::setSocket(KMessageSocket* s, QSocket* sock)
{
}
void BoMyKMessageSocket::slotProcessNewData()
{
 // nothing. KMessageSocket is fixed in this version.
}
#endif


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
 * calls happen after a advance message is defined.
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

		mAdvanceDividerCount = 0;
		mAdvanceDivider = 1;
		mAdvanceMessageSent = false;

		initProperties(mBoson->dataHandler());
	}

	unsigned int advanceCount() const
	{
		return mAdvanceCount;
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
		mAdvanceCount.registerData(Boson::IdAdvanceCount, dataHandler,
				KGamePropertyBase::PolicyLocal, "AdvanceCount");
		mAdvanceCount.registerData(Boson::IdAdvanceCount, dataHandler,
				KGamePropertyBase::PolicyLocal, "AdvanceCount");
		mAdvanceFlag.registerData(Boson::IdAdvanceFlag, dataHandler,
				KGamePropertyBase::PolicyLocal, "AdvanceFlag");
		mAdvanceCallsCount.registerData(Boson::IdAdvanceCallsCount, dataHandler,
				KGamePropertyBase::PolicyLocal, "AdvanceCallsCount");

		mAdvanceCount.setLocal(0);
		mAdvanceFlag.setLocal(0);
		mAdvanceCallsCount.setLocal(0);
		mAdvanceCount.setEmittingSignal(false); // wo don't need it and it would be bad for performance.
		mAdvanceCallsCount.setEmittingSignal(false);
		mAdvanceFlag.setEmittingSignal(false);
	}

	void receiveAdvanceMessage(int gameSpeed)
	{
		mAdvanceMessageSent = false;
		mAdvanceDivider = gameSpeed;
		mAdvanceDividerCount = 0;
		mBoson->lock();
		boDebug(300) << "Advance - speed (calls per " << ADVANCE_INTERVAL
				<< "ms)=" << gameSpeed << " elapsed: "
				<< mAdvanceReceived.elapsed() << endl;
		mAdvanceReceived.restart();
		mBoson->slotReceiveAdvance();
	}

	void sendAdvance() // slot?
	{
		// If advance message has been sent, but not yet received, don't send another
		//  one, because it would get delayed.
		// Note that this is for single-player only, it would probably break network
		if (mAdvanceMessageSent && !mBoson->isNetwork()) {
			boDebug(300) << k_funcinfo << mBoson->advanceCallsCount() << "message has already been sent, returning" << endl;
			return;
		}
		// Also don't send advance message if we're in locked() state.
		// Otherwise, scripted sequences may broke
		if (mBoson->isLocked() && !mBoson->isNetwork()) {
			boDebug(300) << k_funcinfo << mBoson->advanceCallsCount() << "message delivery locked, returning" << endl;
			return;
		}
		boDebug(300) << k_funcinfo << mBoson->advanceCallsCount() << "sending advance msg" << endl;
		mBoson->sendMessage(0, BosonMessage::AdvanceN);
		mAdvanceMessageSent = true;
	}

	/**
	 * Execute an advance call.
	 *
	 * This does the central advance call mechanism in Boson, most
	 * importantly it will make @ref Boson emit @ref Boson::signalAdvance.
	 *
	 * Once the signal is emitted, this function is meant to calculate the
	 * time until the next advanceCall should be made, if there are any
	 * calls left from the last advance message. If this was the last
	 * advance call for the advance message, we are done and wait for the
	 * next message.
	 *
	 * In an optimal case, the time until the next advance call should be
	 * made is exactly message_interval / calls_per_message. E.g. when we
	 * send an advance message ever second and want 10 calls per message, we
	 * do an advance call every 0.6 seconds. But in practice this won't
	 * work, due to several unpredictable delays at about every place. So we
	 * need to do some magic here.
	 *
	 * Therefore this is a <em>complex</em> method! This can be improved
	 * greatly, but you can also do a lot wrong here (e.g. concerning
	 * networking). Take care when you change something here.
	 **/
	void receiveAdvanceCall();

private:
	Boson* mBoson;

	QTime mAdvanceReceived; // when the last advance *message* was received
	int mAdvanceDivider; // pretty much exactly gameSpeed()
	int mAdvanceDividerCount; // how many advance *calls* have been made since the last advance *message*
	bool mAdvanceMessageSent;


	KGameProperty<unsigned int> mAdvanceCount;

	KGamePropertyInt mAdvanceFlag;

	KGameProperty<unsigned int> mAdvanceCallsCount;
};


void BoAdvance::receiveAdvanceCall()
{
// boDebug() << k_funcinfo << advanceCallsCount() << endl;
 bool flag = advanceFlag();
 // we need to toggle the flag *now*, in case one of the Unit::advance*()
 // methods changes the advance function. this change must not appear to the
 // currently used function, but to the other one.
 toggleAdvanceFlag();
 emit mBoson->signalAdvance(advanceCount(), flag);

 mAdvanceCallsCount = mAdvanceCallsCount + 1;
 mAdvanceCount = mAdvanceCount + 1;
 if (mAdvanceCount > MAXIMAL_ADVANCE_COUNT) {
	mAdvanceCount = 0;
 }

 // we also have "mAdvanceDividerCount". the mAdvanceCount is important in Unit,
 // mAdvanceDividerCount is limited to boson only. some explanations:
 // Only a single advance message is sent over network every ADVANCE_INTERVAL
 // ms, independant from the game speed.
 // This single advance message results in a certain number of advance calls
 // (btw: in codes, comments, logs and so on I always make a different between
 // "advance message" and "advance calls" as explained here). mAdvanceDivider is
 // this number of advance calls to-be-generated and is reset to the gameSpeed()
 // when the message is received.
 // The mAdvanceDividerCount is reset to 0 once the advance message is received.
 //
 // The code below tries to make one advance call every
 // ADVANCE_INTERVAL/mAdvanceDivider ms. This means in the ideal case all
 // advance calls from this and from the next advance message would be in the
 // same interval.
 //
 // Please remember that there are several additional tasks that need to be done
 // - e.g. unit moving, OpenGL rendering, ... so 
 if (mAdvanceDividerCount + 1 == mAdvanceDivider)  {
	boDebug(300) << k_funcinfo << "delayed messages: "
			<< mBoson->delayedMessageCount() << endl;
	mBoson->unlock();
 } else if (mAdvanceDividerCount + 1 < mAdvanceDivider) {
	int next;
	if (mBoson->delayedAdvanceMessageCount() == 0) {
		int t = ADVANCE_INTERVAL * mAdvanceDividerCount / mAdvanceDivider;// time that should have been elapsed
		int diff = QMAX(5, mAdvanceReceived.elapsed() - t + 5); // we are adding 5ms "safety" diff
		next = QMAX(0, ADVANCE_INTERVAL / mAdvanceDivider - diff);
	} else {
		// we have delayed advance messages already, so we should flush
		// the remaining calls for the current message asap
		next = 0;
	}
	QTimer::singleShot(next, mBoson, SLOT(slotReceiveAdvance()));
	mAdvanceDividerCount++;
 } else {
	boError() << k_funcinfo << "count > divider --> This must never happen!!" << endl;
 }
}



class BoMessage
{
public:
	QByteArray byteArray;
	int msgid;
	Q_UINT32 receiver;
	Q_UINT32 sender;
	Q_UINT32 clientId;
	unsigned int advanceCallsCount;
	QTime mArrivalTime;
	QTime mDeliveryTime;

	/**
	 * Construct a new message. The tinmstamp is set to the current time.
	 * @param _advanceCallsCount See @ref Boson::advanceCallsCount. This
	 * parameter is optional as it is informational only (not an actual part
	 * of the message, but handy for debugging/logging).
	 **/
	BoMessage(QByteArray& _message, int _msgid, Q_UINT32 _receiver, Q_UINT32 _sender, Q_UINT32 _clientId, unsigned int _advanceCallsCount = 0)
		: byteArray(_message),
		msgid(_msgid),
		receiver(_receiver),
		sender(_sender),
		clientId(_clientId),
		advanceCallsCount(_advanceCallsCount)
	{
		mArrivalTime = QTime::currentTime();
	}

	BoMessage(QDataStream& stream, int _msgid, Q_UINT32 _receiver, Q_UINT32 _sender, Q_UINT32 _clientId, unsigned int _advanceCallsCount = 0)
		: byteArray(((QBuffer*)stream.device())->readAll()),
		msgid(_msgid),
		receiver(_receiver),
		sender(_sender),
		clientId(_clientId),
		advanceCallsCount(_advanceCallsCount)
	{
		mArrivalTime = QTime::currentTime();
	}

	/**
	 * Set the delivery time to the current time. Both, arrival and delivery
	 * time are (nearly) equal, except if a message has been delayed.
	 **/
	void setDelivered()
	{
		mDeliveryTime = QTime::currentTime();
	}

	QString debug(KGame* game)
	{
		if (!game) {
			return QString::null;
		}
		QString m = QString("msgid=%1").arg(msgid);
		QString r = QString("receiver=%3").arg(receiver);
		QString s = QString("sender=%2").arg(sender);
		if (KGameMessage::isPlayer(receiver)) {
			KPlayer* p = game->findPlayer(receiver);
			if (!p) {
				r += QString("(player cant be found)");
			} else {
				r += QString("(player %1)").arg(p->name());
			}
		} else if (KGameMessage::isGame(receiver)) {
			if (receiver == 0) {
				r += "(broadcast games)";
			} else if (receiver == game->gameId()) {
				r += "(local game)";
			} else {
				r += "(remote game)";
			}
		}

		if (KGameMessage::isPlayer(sender)) {
			KPlayer* p = game->findPlayer(receiver);
			if (!p) {
				r += QString("(player cant be found)");
			} else {
				r += QString("(player %1)").arg(p->name());
			}
		} else if (KGameMessage::isGame(sender)) {
			if (sender == 0) {
				r += "(broadcast games)";
			} else if (sender == game->gameId()) {
				s += "(local game)";
			} else {
				s += "(remote game)";
			}
		}

		QString mname = KGameMessage::messageId2Text(msgid);
		if (!mname.isEmpty()) {
			m += QString("(%1)").arg(mname);
		}

		QString d;
		d = m + " " + r + " " + s;

		d += debugMore(game);
		return d;
	}

	QString debugMore(KGame* game)
	{
		QString m;
		QDataStream s(byteArray, IO_ReadOnly);
		if (msgid == KGameMessage::IdGameProperty) {
			int propId;
			KGameMessage::extractPropertyHeader(s, propId);
			if (propId == KGamePropertyBase::IdCommand) {
				m = " is a property command";
				// we could use
				// KGameMessage::extractPropertyCommand() here
				// now
			} else {
				KGamePropertyBase* p;
				p = game->dataHandler()->dict().find(propId);
				if (!p) {
					m = QString(" property %1 can't be found").arg(propId);
				} else {
					m = QString(" property: %1(%2)").arg(propId).arg(game->dataHandler()->propertyName(propId));
				}
			}
		}
		return m;
	}
};


class BoMessageDelayer
{
public:
	BoMessageDelayer(Boson* b)
	{
		mBoson = b;
		mDelayedMessages.setAutoDelete(true);
		mIsLocked = false;
		mDelayedWaiting = false;
		mAdvanceMessageWaiting = 0;
	}
	~BoMessageDelayer()
	{
		// remaining messages are not stored anywhere else. they are
		// deleted here.
		mDelayedMessages.clear();
	}

	/**
	 * Lock message delivery. All incoming messages are delayed until @ref
	 * unlock is called.
	 **/
	void lock()
	{
		boDebug(300) << k_funcinfo << endl;
		mIsLocked = true;
	}

	/**
	 * Unlock message delivery (see @ref lock) and deliver all delayed
	 * messages.
	 **/
	void unlock()
	{
		boDebug(300) << k_funcinfo << endl;
		mIsLocked = false;
		while (!mDelayedMessages.isEmpty() && !mIsLocked) {
			processDelayed();
		}
	}

	bool isLocked() const
	{
		return mIsLocked;
	}

	unsigned int delayedMessageCount() const
	{
		return mDelayedMessages.count();
	}

	unsigned int delayedAdvanceMessageCount() const
	{
		return mAdvanceMessageWaiting;
	}

	/**
	 * Process a network message. Call this before @ref
	 * KGame::networkTransmission is executed!
	 *
	 * This will test whether the message should get delayed and, if that is
	 * the case, it will do so. @ref KGame::networkTransmission should not
	 * be called then. Otherwise you can simply proceed and call @ref
	 * KGame::networkTransmission for this message.
	 *
	 * The message @p m is stored in this class if the message is delayed.
	 * That object will be used to deliver it later - do NOT delete the
	 * message if it is delayed!
	 *
	 * @return TRUE if the message can be delivered normally, FALSE if it
	 * got delayed.
	 **/
	bool processMessage(BoMessage* m)
	{
		if (!m) {
			return true;
		}
		if (mIsLocked || mDelayedWaiting) {
			boDebug() << k_funcinfo << "delayed " << m->debug(mBoson) << endl;
			mDelayedMessages.enqueue(m);
			mDelayedWaiting = true;
			switch (m->msgid - KGameMessage::IdUser) {
				case BosonMessage::AdvanceN:
					mAdvanceMessageWaiting++;
					boWarning(300) << k_funcinfo << "advance message got delayed @" << mBoson->advanceCallsCount() << endl;
					break;
				default:
					break;
			}

			// message delayed.
			return false;
		}
		return true; // not delayed.
	}

protected:
	/**
	 * Process the first delayed message for delivery. Called by @ref unlock
	 * to deliver delayed messages.
	 *
	 * Note that a delayed message might call @ref lock before all delayed
	 * messages have been deliverd! (this happens e.g. when 2 advance
	 * messages got delayed)
	 **/
	void processDelayed()
	{
		BoMessage* m = mDelayedMessages.dequeue();
		if (!m) {
			boWarning() << k_funcinfo << "no message here" << endl;
			return;
		}
		QDataStream s(m->byteArray, IO_ReadOnly);
		mDelayedWaiting = false;
		switch (m->msgid - KGameMessage::IdUser) {
			case BosonMessage::AdvanceN:
				boWarning(300) << k_funcinfo << "delayed advance msg will be sent!" << endl;
				mAdvanceMessageWaiting--;
				break;
			default:
				break;
		}
		mBoson->networkTransmission(m);
		mDelayedWaiting = !mDelayedMessages.isEmpty();
	}

private:
	Boson* mBoson;
	QPtrQueue<BoMessage> mDelayedMessages;
	bool mIsLocked;
	bool mDelayedWaiting; // FIXME bad name!
	int mAdvanceMessageWaiting;
};


/**
 * @short This class keeps a log of all messages received by now
 **/
// AB: after a certain time / number of messages we should delete old messages,
// in order to save memory
class BoMessageLogger
{
public:
	BoMessageLogger()
	{
		mLoggedMessages.setAutoDelete(true);
	}
	~BoMessageLogger()
	{
		mLoggedMessages.clear();
	}

	/**
	 * Append @p message to the log. Note that ownership is taken, i.e. you
	 * must not delete it after calling this!
	 **/
	void append(BoMessage* message)
	{
		mLoggedMessages.append(message);
	}

	bool saveGameLog(QIODevice* logDevice)
	{
		if (!logDevice) {
			BO_NULL_ERROR(logDevice);
			return false;
		}
		if (!logDevice->isOpen()) {
			boError() << k_funcinfo << "device not open" << endl;
			return false;
		}
		QTextStream log(logDevice);
		QPtrListIterator<BoMessage> it(mLoggedMessages);
		while (it.current()) {
			const BoMessage* m = it.current();
			log << "Msg: " << m->advanceCallsCount << "  "
					<< m->msgid << "  "
					<< m->sender << " "
					<< m->receiver << " "
					<< m->clientId << "  ";
			log.writeRawBytes(m->byteArray.data(), m->byteArray.size());
			log << endl;
			++it;
		}
		return true;
	}

private:
	QPtrList<BoMessage> mLoggedMessages;
};


class Boson::BosonPrivate
{
public:
	BosonPrivate()
	{
		mStartingObject = 0;

		mGameTimer = 0;
		mCanvas = 0;
		mPlayField = 0;
		mPlayer = 0;

		mLoadingStatus = BosonSaveLoad::NotLoaded;

		mAdvance = 0;
		mMessageDelayer = 0;
	}
	BosonStarting* mStartingObject;

	QTimer* mGameTimer;

	BosonCanvas* mCanvas;
	BosonPlayField* mPlayField;
	Player* mPlayer;
	QPtrList<KGameComputerIO> mComputerIOList;

	KGamePropertyInt mGameSpeed;
	KGamePropertyBool mGamePaused;

	BosonSaveLoad::LoadingStatus mLoadingStatus;

	QValueList<QByteArray> mGameLogs;
	BoMessageLogger mMessageLogger;

	BoAdvance* mAdvance;
	BoMessageDelayer* mMessageDelayer;
};

Boson::Boson(QObject* parent) : KGame(BOSON_COOKIE, parent)
{
 setPolicy(PolicyClean);
 d = new BosonPrivate;
 d->mAdvance = new BoAdvance(this);
 d->mMessageDelayer = new BoMessageDelayer(this);

 d->mGameTimer = new QTimer(this);


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
 connect(dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
		this, SLOT(slotPropertyChanged(KGamePropertyBase*)));
 d->mGamePaused.registerData(IdGamePaused, dataHandler(),
		KGamePropertyBase::PolicyClean, "GamePaused");
 d->mGameSpeed.registerData(IdGameSpeed, dataHandler(),
		KGamePropertyBase::PolicyClean, "GameSpeed");
 d->mGamePaused.setLocal(false);
 d->mGameSpeed.setLocal(0);

 setMinPlayers(1);
}

Boson::~Boson()
{
 delete d->mMessageDelayer;
 delete d->mAdvance;
 delete d->mGameTimer;
 delete d->mCanvas;
 delete d;
}

void Boson::initBoson()
{
 mBoson = new Boson(0);
 connect(BoDebug::self(), SIGNAL(notify(const QString&,const char*,int)),
		mBoson, SLOT(slotDebugOutput(const QString&,const char*,int)));
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
}

BosonCanvas* Boson::canvasNonConst() const
{
 return d->mCanvas;
}

const BosonCanvas* Boson::canvas() const
{
 return d->mCanvas;
}

void Boson::setPlayField(BosonPlayField* p)
{
 boDebug() << k_funcinfo << endl;
 d->mPlayField = p;
}

BosonPlayField* Boson::playField() const
{
 return d->mPlayField;
}

Player* Boson::localPlayer() const
{
 return d->mPlayer;
}

void Boson::setLocalPlayer(Player* p)
{
 d->mPlayer = p;
}

void Boson::setStartingObject(BosonStarting* s)
{
 // AB: a NULL starting object unsets the object. it is totally valid
 d->mStartingObject=  s;
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
 Player* player = (Player*)p;
 if (player->isOutOfGame()) {
	boWarning() << k_funcinfo << "Player must not send input anymore!!" << endl;
	return true;
 }
 if (!gameMode()) {
	// editor mode sends an additional entry safety id, just in case we
	// might have constructed a wrong display or so
	Q_UINT32 editor;
	stream >> editor;
	if (editor != BosonMessage::MoveEditor) {
		boError() << k_funcinfo << "Not an editor message, elthough we're in editor mode!" << endl;
		return true;
	}
 }
 Q_UINT32 msgid;
 stream >> msgid;
 switch (msgid) {
	case BosonMessage::MoveMove:
	{
		bool attack;
		Q_UINT8 attackcode;
		QPoint pos;
		Q_UINT32 unitCount;
		stream >> attackcode;
		if (attackcode == 0) {
			attack = false;
		} else {
			attack = true;
		}
		boDebug() << "MOVING: " << k_funcinfo << "attack: " << attack << endl;
		stream >> pos;
		stream >> unitCount;
		QPtrList<Unit> unitsToMove;
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
//			boDebug() << "pos: " << pos.x() << " " << pos.y() << endl;
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->owner() != player) {
				boDebug() << "unit " << unitId << "only owner can move units!" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot move destroyed units" << endl;
				continue;
			}
//			boDebug() << "move " << unitId << endl;
			if (unit->unitProperties()->isMobile()) {
				unitsToMove.append(unit);
			}
		}
		if (unitsToMove.count() == 0) {
			boWarning() << k_lineinfo << "no unit to move" << endl;
			break;
		}
		if (unitsToMove.count() == 1) {
			unitsToMove.first()->moveTo(pos, attack);
		} else {
		QPtrListIterator<Unit> it(unitsToMove);
			it.toFirst();
			while (it.current()) {
				it.current()->moveTo(pos, attack);
				++it;
			}
		}
		break;
	}
	case BosonMessage::MoveAttack:
	{
		Q_ULONG attackedUnitId;
		Q_UINT32 unitCount;
		stream >> attackedUnitId;
		stream >> unitCount;
		Unit* attackedUnit = findUnit(attackedUnitId, 0);
		if (!attackedUnit) {
			boError() << "Cannot attack NULL unit" << endl;
			return true;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			if (unitId == attackedUnitId) {
				// can become possible one day - e.g. when
				// repairing a unit
				boWarning() << "Can not (yet) attack myself" 
						<< endl;
				continue;
			}
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot attack with destroyed units" << endl;
				continue;
			}
			if (attackedUnit->isDestroyed()) {
				boDebug() << "no sense in attacking destroyed units" << endl;
				continue;
			}
			if (unit->unitProperties()->canShoot()) {
				boDebug() << unitId << " attacks " << attackedUnitId << endl;
				unit->setTarget(attackedUnit);
				if (unit->target()) {
					unit->setWork(Unit::WorkAttack);
				}
			}
		}
		break;
	}
	case BosonMessage::MoveStop:
	{
		Q_UINT32 unitCount;
		stream >> unitCount;
		QPtrList<Unit> unitsToStop;
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->owner() != player) {
				boDebug() << "unit " << unitId << "only owner can stop units!" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot stop destroyed units" << endl;
				continue;
			}
			unitsToStop.append(unit);
		}
		if (unitsToStop.count() == 0) {
			boWarning() << k_lineinfo << "no unit to stop" << endl;
			break;
		}
		QPtrListIterator<Unit> it(unitsToStop);
		while (it.current()) {
			it.current()->stopAttacking();  // call stopAttacking() because it also sets unit's work to WorkNone... and it doesn't hurt
			++it;
		}
		break;
	}
	case BosonMessage::MoveMine:
	{
		boDebug() << "MoveMine" << endl;
		Q_ULONG harvesterId;
		Q_ULONG resourceMineId;
		stream >> harvesterId;
		stream >> resourceMineId;
		Unit* u = findUnit(harvesterId, player);
		if (!u) {
			boError() << k_lineinfo << "cannot find harvester unit " << harvesterId << " for player " << player << endl;
			break;
		}
		if (!u->isMobile()) {
			boError() << k_lineinfo << "only mobile units can mine" << endl;
			break;
		}
		HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
		if (!h) {
			boError() << k_lineinfo << "only harvester can mine" << endl;
			break;
		}
		if (u->owner() != player) {
			boDebug() << k_funcinfo << "unit " << harvesterId << "only owner can move units!" << endl;
			break;
		}
		if (u->isDestroyed()) {
			boDebug() << "cannot mine with destroyed units" << endl;
			break;
		}

		u = findUnit(resourceMineId, 0);
		if (!u) {
			boError() << k_lineinfo << "cannot find resourcemine unit " << resourceMineId << " for player " << player << endl;
			break;
		}
		ResourceMinePlugin* r = (ResourceMinePlugin*)u->plugin(UnitPlugin::ResourceMine);
		if (!r) {
			boError() << k_lineinfo << "can mine at resource mine only" << endl;
			break;
		}
		h->mineAt(r);
		break;
	}
	case BosonMessage::MoveRefine:
	{
		boDebug() << "MoveRefine" << endl;
		Q_UINT32 refineryOwnerId;
		Q_ULONG refineryId;
		Q_UINT32 unitCount;
		stream >> refineryOwnerId;
		stream >> refineryId;
		stream >> unitCount;
		Player* refineryOwner = (Player*)findPlayer(refineryOwnerId);
		if (!refineryOwner) {
			boError() << k_lineinfo << "Cannot find player " << refineryOwnerId << endl;
			break;
		}
		if (player->isEnemy(refineryOwner)) {
			boError() << k_lineinfo << "Cannot go to enemy refinery" << endl;
			break;
		}
		Unit* refinery = findUnit(refineryId, refineryOwner);
		if (!refinery) {
			boError() << k_lineinfo << "cannot find refinery " << refineryId << " for player " << refineryOwnerId << endl;
			break;
		}
#warning TODO
//		if (!refinery->plugin(UnitPlugin::Refinery)) {
		if (!refinery->isFacility()) { // FIXME do not depend on facility!
			boWarning() << k_lineinfo << "refinery must be a facility" << endl;
			break;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* u = findUnit(unitId, player);
			if (!u) {
				boError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player->id() << endl;
				continue;
			}
			if (u->isDestroyed()) {
				continue;
			}
			if (u->owner() != player) {
				continue;
			}
			HarvesterPlugin* h = (HarvesterPlugin*)u->plugin(UnitPlugin::Harvester);
			if (!h) {
				boError() << k_lineinfo << "must be a harvester" << endl;
				continue;
			}
			h->refineAt((Facility*)refinery);
		}
		break;
	}
	case BosonMessage::MoveRepair:
	{
		boWarning() << "MoveRepair is a TODO" << endl;
		break;
		// move mobile units to repairyard
		//
		// TODO there are several ways of repairing:
		// 1. move mobile units to repairyard
		// 2. move a mobile repairunit to damaged units
		// 3. repair facilities
		// can we use MoveRepair for all of them?
		// this is currently about 1. only
		Q_UINT32 repairOwnerId;
		Q_ULONG repairId;
		Q_UINT32 unitCount;
		stream >> repairOwnerId;
		stream >> repairId;
		stream >> unitCount;
		Player* repairOwner= (Player*)findPlayer(repairOwnerId);
		if (!repairOwner) {
			boError() << k_lineinfo << "Cannot find player " << repairOwnerId << endl;
			break;
		}
		if (player->isEnemy(repairOwner)) {
			boError() << k_lineinfo << "Cannot move to enemy repairyard" << endl;
			break;
		}
		Unit* repairYard = findUnit(repairId, repairOwner);
		if (!repairYard) {
			boError() << k_lineinfo << "Cannot find " << repairId << " for player " << repairOwnerId << endl;
			break;
		}
		RepairPlugin* repair = (RepairPlugin*)repairYard->plugin(UnitPlugin::Repair);
		if (!repair) {
			boError() << k_lineinfo << "repairyard cannot repair?!" << endl;
			break;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			Unit* u = findUnit(unitId, player);
			if (!u) {
				boError() << k_lineinfo << "cannot find unit " << unitId << " for player " << player->id()  << endl;
				continue;
			}
			if (!u->isMobile()) {
				boError() << k_lineinfo << "must be a mobile unit" << endl;
				continue;
			}
			repair->repair(u);
		}
		break;
	}
	case BosonMessage::MoveProduce:
	{
		Q_UINT32 productionType;
		Q_UINT32 owner;
		Q_ULONG factoryId;
		Q_UINT32 id;

		stream >> productionType;

		stream >> owner;
		stream >> factoryId;
		stream >> id;

		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << factoryId << endl;
			break;
		}

		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// maybe not yet fully constructed
			boWarning() << k_lineinfo << factory->id() << " cannot produce" << endl;
			break;
		}
		if (id <= 0) {
			boError() << k_lineinfo << "Invalid id " << id << endl;
			break;
		}

		unsigned long int mineralCost = 0, oilCost = 0;

		if ((ProductionType)productionType == ProduceUnit) {
			// Produce unit
			const UnitProperties* prop = p->unitProperties(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else if ((ProductionType)productionType == ProduceTech) {
			// Produce upgrade
			const UpgradeProperties* prop = p->speciesTheme()->technology(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
		}

			if (factory->currentPluginType() != UnitPlugin::Production) {
				if ((production->currentProductionId() == id) && (production->currentProductionType() == (ProductionType)productionType)) {
					// production was stopped - continue it now
					factory->setPluginWork(UnitPlugin::Production);
					emit signalUpdateProduction(factory);
					break;
				}
			}

		if (p->minerals() < mineralCost) {
			if (p == localPlayer()) {
				slotAddChatSystemMessage(i18n("You have not enough minerals!"));
			}
			break;
		}
		if (p->oil() < oilCost) {
			if (p == localPlayer()) {
				slotAddChatSystemMessage(i18n("You have not enough oil!"));
			}
			break;
		}
		p->setMinerals(p->minerals() - mineralCost);
		p->setOil(p->oil() - oilCost);
		production->addProduction((ProductionType)productionType, (unsigned long int)id);
		emit signalUpdateProduction(factory);
		break;
	}
	case BosonMessage::MoveProduceStop:
	{
		boDebug() << "MoveProduceStop" << endl;
		Q_UINT32 productionType;
		Q_UINT32 owner;
		Q_ULONG factoryId;
		Q_UINT32 id;

		stream >> productionType;

		stream >> owner;
		stream >> factoryId;
		stream >> id;

		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << factoryId << endl;
			break;
		}

		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// should not happen here!
			boError() << k_lineinfo << factory->id() << "cannot produce?!" << endl;
			break;
		}
		if (!production->contains((ProductionType)productionType, (unsigned long int)id)) {
			boDebug() << k_lineinfo << "Production " << productionType << " with id "
					 << id << " is not in production queue" << endl;
			return true;
		}
		if (id <= 0) {
			boError() << k_lineinfo << "Invalid id " << id << endl;
			break;
		}

		unsigned long int mineralCost = 0, oilCost = 0;

		if ((ProductionType)productionType == ProduceUnit) {
			const UnitProperties* prop = p->unitProperties(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL unit properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else if ((ProductionType)productionType == ProduceTech) {
			const UpgradeProperties* prop = p->speciesTheme()->technology(id);
			if (!prop) {
				boError() << k_lineinfo << "NULL technology properties (EVIL BUG)" << endl;
				break;
			}
			mineralCost = prop->mineralCost();
			oilCost = prop->oilCost();
		} else {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
		}

		if ((production->currentProductionId() == id) && (production->currentProductionType() == (ProductionType)productionType)) {
			if (factory->currentPluginType() == UnitPlugin::Production) {
				// do not abort but just pause
				factory->setWork(Unit::WorkNone);
				emit signalUpdateProduction(factory);
			} else {
				p->setMinerals(p->minerals() + mineralCost);
				p->setOil(p->oil() + oilCost);
				production->removeProduction();
				emit signalUpdateProduction(factory);
			}
		} else {
			// not the current, but a queued production is stopped.

			//FIXME: money should be paid when the production is
			//actually started! (currently it is paid as soon as an
			//item is added to the queue)
			p->setMinerals(p->minerals() + mineralCost);
			p->setOil(p->oil() + oilCost);
			production->removeProduction((ProductionType)productionType, (unsigned long int)id);
			emit signalUpdateProduction(factory);
		}
		break;
	}
	case BosonMessage::MoveBuild:
	{
		boDebug() << "MoveBuild" << endl;
		Q_UINT32 productionType;
		Q_ULONG factoryId;
		Q_UINT32 owner;
		Q_INT32 x;
		Q_INT32 y;

		stream >> productionType;

		stream >> factoryId;
		stream >> owner;
		stream >> x;
		stream >> y;

		// Only units are "built"
		if ((ProductionType)productionType != ProduceUnit) {
			boError() << k_funcinfo << "Invalid productionType: " << productionType << endl;
			break;
		}

		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* factory = findUnit(factoryId, p);
		if (!factory) {
			boError() << "Cannot find unit " << factoryId << endl;
			break;
		}
		ProductionPlugin* production = (ProductionPlugin*)factory->plugin(UnitPlugin::Production);
		if (!production) {
			// should not happen here!
			boError() << k_lineinfo << factory->id() << "cannot produce?!" << endl;
			break;
		}
		if (production->completedProductionType() != ProduceUnit) {
			boError() << k_lineinfo << "not producing unit!" << endl;
			break;
		}
		int unitType = production->completedProductionId();
		boDebug() << k_lineinfo
				<< "factory="
				<< factory->id()
				<< ",unitid="
				<< unitType
				<< endl;
		if (unitType <= 0) {
			// hope this is working...
			boWarning() << k_lineinfo << "not yet completed" << endl;
			break;
		}
		buildProducedUnit(production, unitType, x, y);
		break;
	}
	case BosonMessage::MoveFollow:
	{
		Q_ULONG followUnitId;
		Q_UINT32 unitCount;
		stream >> followUnitId;
		stream >> unitCount;
		Unit* followUnit = findUnit(followUnitId, 0);
		if (!followUnit) {
			boError() << "Cannot follow NULL unit" << endl;
			return true;
		}
		for (unsigned int i = 0; i < unitCount; i++) {
			Q_ULONG unitId;
			stream >> unitId;
			if (unitId == followUnitId) {
				boWarning() << "Cannot follow myself" << endl;
				continue;
			}
			Unit* unit = findUnit(unitId, player);
			if (!unit) {
				boDebug() << "unit " << unitId << " not found for this player" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boDebug() << "cannot follow with destroyed units" << endl;
				continue;
			}
			if (followUnit->isDestroyed()) {
				boDebug() << "Cannot follow destroyed units" << endl;
				continue;
			}
			unit->setTarget(followUnit);
			if (unit->target()) {
				unit->setWork(Unit::WorkFollow);
			}
		}
		break;
	}
	case BosonMessage::MoveLayMine:
	{
		boDebug() << k_funcinfo << "MoveLayMine action" << endl;
		Q_UINT32 unitCount;
		Q_UINT32 unitId, weaponId;

		stream >> unitCount;

		for (unsigned int i = 0; i < unitCount; i++) {
			stream >> unitId;
			stream >> weaponId;
			boDebug() << k_funcinfo << "unit: " << unitId << "; weapon: " << weaponId << endl;

			Unit* unit = findUnit(unitId, 0);
			if (!unit) {
				boWarning() << "unit " << unitId << " not found" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boWarning() << "cannot do anything with destroyed units" << endl;
				continue;
			}
			MiningPlugin* m = (MiningPlugin*)unit->plugin(UnitPlugin::Mining);
			if (!m) {
				boError() << k_lineinfo << "This unit has no mining plugin" << endl;
				break;
			}
			m->mine(weaponId);
		}
		boDebug() << k_funcinfo << "done" << endl;

		break;
	}
	case BosonMessage::MoveDropBomb:
	{
		boDebug() << k_funcinfo << "MoveDropBomb action" << endl;
		Q_UINT32 unitCount;
		Q_INT32 x, y;
		Q_UINT32 unitId, weaponId;

		stream >> x;
		stream >> y;
		stream >> unitCount;

		for (unsigned int i = 0; i < unitCount; i++) {
			stream >> unitId;
			stream >> weaponId;
			boDebug() << k_funcinfo << "unit: " << unitId << "; weapon: " << weaponId << endl;

			Unit* unit = findUnit(unitId, 0);
			if (!unit) {
				boWarning() << "unit " << unitId << " not found" << endl;
				continue;
			}
			if (unit->isDestroyed()) {
				boWarning() << "cannot do anything with destroyed units" << endl;
				continue;
			}
			BombingPlugin* b = (BombingPlugin*)unit->plugin(UnitPlugin::Bombing);
			if (!b) {
				boError() << k_lineinfo << "This unit has no bombing plugin" << endl;
				break;
			}
			b->bomb(weaponId, x, y);
		}
		boDebug() << k_funcinfo << "done" << endl;

		break;
	}
	case BosonMessage::MoveTeleport:
	{
		Q_UINT32 unitId;
		Q_UINT32 owner;
		Q_INT32 x;
		Q_INT32 y;

		stream >> owner;
		stream >> unitId;
		stream >> x;
		stream >> y;

		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* u = findUnit(unitId, p);
		if (!u) {
			boError() << "Cannot find unit " << unitId << endl;
			break;
		}

		u->moveBy(x - u->x(), y - u->y(), 0);

		break;
	}
	case BosonMessage::MoveRotate:
	{
		Q_UINT32 unitId;
		Q_UINT32 owner;
		float rot;

		stream >> owner;
		stream >> unitId;
		stream >> rot;

		Player* p = (Player*)findPlayer(owner);
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}
		Unit* u = findUnit(unitId, p);
		if (!u) {
			boError() << "Cannot find unit " << unitId << endl;
			break;
		}

		u->setRotation(rot);

		break;
	}
	case BosonMessage::MovePlaceUnit:
	{
		Q_UINT32 unitType;
		Q_UINT32 owner;
		Q_INT32 cellX;
		Q_INT32 cellY;

		stream >> owner;
		stream >> unitType;
		stream >> cellX;
		stream >> cellY;

		Player* p = 0;
		if (owner >= 1024) { // a KPlayer ID
			p = (Player*)findPlayer(owner);
		} else {
			p = (Player*)playerList()->at(owner);
		}
		if (!p) {
			boError() << k_lineinfo << "Cannot find player " << owner << endl;
			break;
		}

		BoVector3 pos((float)cellX * BO_TILE_SIZE, (float)cellY * BO_TILE_SIZE, 0.0f);
		d->mCanvas->createNewItem(RTTI::UnitStart + unitType, p, ItemType(unitType), pos);
		break;
	}
	case BosonMessage::MoveChangeTexMap:
	{
		Q_UINT32 count;
		stream >> count;
		for (unsigned int i = 0; i < count; i++) {
			Q_UINT32 x;
			Q_UINT32 y;
			Q_UINT32 texCount;
			stream >> x;
			stream >> y;
			stream >> texCount;
			if (texCount > 200) {
				boError() << k_funcinfo << "more than 200 textures? invalid!" << endl;
				break;
			}
			Q_UINT32* textures = new Q_UINT32[texCount];
			Q_UINT8* alpha = new Q_UINT8[texCount];
			for (unsigned int j = 0; j < texCount; j++) {
				stream >> textures[j];
				stream >> alpha[j];
			}
			emit signalChangeTexMap((int)x, (int)y, texCount, textures, alpha);
			delete[] textures;
			delete[] alpha;
		}
		break;
	}
	case BosonMessage::MoveChangeHeight:
	{
		boDebug() << k_lineinfo << "change height" << endl;
		Q_UINT32 count;
		Q_INT32 cornerX;
		Q_INT32 cornerY;
		float height;
		stream >> count;
		for (unsigned int i = 0; i < count; i++) {
			stream >> cornerX;
			stream >> cornerY;
			stream >> height;
			// note: cornerX == mapWidth() and cornerY == mapHeight()
			// are valid!
			if (cornerX < 0 || (unsigned int)(cornerX - 1) >= d->mCanvas->mapWidth()) {
				boError() << k_funcinfo << "invalid x coordinate " << cornerX << endl;
				continue;
			}
			if (cornerY < 0 || (unsigned int)(cornerY - 1) >= d->mCanvas->mapHeight()) {
				boError() << k_funcinfo << "invalid y coordinate " << cornerY << endl;
				continue;
			}
			boDebug() << k_funcinfo << "new height at " << cornerX << "," << cornerY << " is " << height << endl;
			d->mCanvas->setHeightAtCorner(cornerX, cornerY, height);
		}
		break;
	}
	default:
		boWarning() << k_funcinfo << "unexpected playerInput " << msgid << endl;
		break;
 }
 return true;
}

void Boson::slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 , Q_UINT32 sender)
{
 QDataStream stream(buffer, IO_ReadOnly);
 switch (msgid) {
	case BosonMessage::AdvanceN:
	{
		d->mAdvance->receiveAdvanceMessage(gameSpeed());
		if (delayedMessageCount() > 20) {
			boWarning() << k_funcinfo << "more than 20 messages delayed!!" << endl;
		}
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
		setGameMode(true);
		d->mStartingObject->setNewGameData(buffer);
		QTimer::singleShot(0, this, SIGNAL(signalStartNewGame()));
		break;
	}
	case BosonMessage::IdNewEditor:
	{
		if (isRunning()) {
			boError() << k_funcinfo << "received IdNewEditor, but game is already running" << endl;
			return;
		}
		if (!d->mStartingObject) {
			boError() << k_funcinfo << "received IdNewEditor, but starting a editor/game is not allowed!" << endl;
			return;
		}
		setGameMode(false);
		d->mStartingObject->setNewGameData(buffer);
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
		if (sender != messageClient()->adminId()) {
			boError() << k_funcinfo << "only ADMIN is allowed to send IdGameIsStarted message! sender="
					<< sender << " ADMIN="
					<< messageClient()->adminId() << endl;
			break;
		}

		if (isServer()) {
			connect(d->mGameTimer, SIGNAL(timeout()), this, SLOT(slotSendAdvance()));
		} else {
			boWarning() << "is not server - cannot start the game!" << endl;
		}
		emit signalGameStarted();
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
	default:
		boWarning() << k_funcinfo << "unhandled msgid " << msgid << endl;
		break;
 }
}

void Boson::initFogOfWar(BosonStarting* starting)
{
 if (!starting) { // ensure that this is actually called from BosonStarting
	return;
 }
 if (isRunning()) {
	return;
 }
 QTimer::singleShot(0, this, SIGNAL(signalInitFogOfWar()));
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
							<< ADVANCE_INTERVAL
							<< endl;
					d->mGameTimer->start(ADVANCE_INTERVAL);
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
			d->mGameTimer->start(ADVANCE_INTERVAL);
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
 *remove = false;
 if (!player) {
	boError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (!isAdmin()) {
	boError() << k_funcinfo << "only ADMIN can do this" << endl; 
	return;
 }
// boDebug() << k_funcinfo << endl;
}

bool Boson::buildProducedUnit(ProductionPlugin* factory, unsigned long int unitType, int cellX, int cellY)
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
 if (!(d->mCanvas)->canPlaceUnitAtCell(p->unitProperties(unitType), QPoint(cellX, cellY), 0)) {
	boDebug() << k_funcinfo << "Cannot create unit here" << endl;
	return false;
 }
 BoVector3 pos((float)cellX * BO_TILE_SIZE, (float)cellY * BO_TILE_SIZE, 0.0f);
 Unit* unit = (Unit*)d->mCanvas->createNewItem(RTTI::UnitStart + unitType, p, ItemType(unitType), pos);
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return false;
 }
 if (unit->isFacility()) {
	p->statistics()->addProducedFacility((Facility*)unit, factory);
 } else {
	p->statistics()->addProducedMobileUnit((MobileUnit*)unit, factory);
 }

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
#if KDE_VERSION <= KDE_MAKE_VERSION(3,1,4)
 // this is an UGLY hack to make sure that network is working correctly.
 // we've had a KMessageSocket bug in kdegames < 3.2
 fixKMessageSocketIsRecursive();
#endif
 KGameIO* io = p->findRttiIO(KGameIO::ComputerIO);
 if (io) {
	// note the IO is added on only *one* client!
	d->mComputerIOList.append((KGameComputerIO*)io);
 }
 slotAddChatSystemMessage(i18n("Player %1 - %2 joined").arg(p->id()).arg(p->name()));

 if (!localPlayer() && playerCount() == 1) {
	boWarning() << k_funcinfo << "first player entered the game and no localplayer is set - assume that the localplayer entered" << endl;
	setLocalPlayer((Player*)p);
 }
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
}

void Boson::slotAdvanceComputerPlayers(unsigned int /*advanceCount*/, bool /*advanceFlag*/)
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
 if (advanceCallsCount() % boConfig->gameLogInterval() == 0) {
	makeGameLog();
 }

 d->mAdvance->receiveAdvanceCall();
}

void Boson::networkTransmission(QDataStream& stream, int msgid, Q_UINT32 r, Q_UINT32 s, Q_UINT32 clientId)
{
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
 bool ret = save->saveToFiles(files, d->mPlayer);
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
 bool ret = save->savePlayFieldToFiles(files, d->mPlayer);
 if (!ret) {
	boError() << k_funcinfo << "saving failed" << endl;
	return ret;
 }
 ret = BosonSaveLoad::saveToFile(files, file);
 return ret;
}

bool Boson::save(QDataStream& stream, bool saveplayers)
{
#if HAVE_KGAME_SAVEGAME
 return KGame::save(stream, saveplayers);
#else
// KDE 3.0 didn't have KGame::savegame() - we provide our own savegame()
// version, but the KGame code is in KGame::save(). we need to call that from
// Boson::save(), instead of savegame()
 return savegame(stream, false, saveplayers);
#endif
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
#if !HAVE_KGAME_SAVEGAME
 boWarning() << k_funcinfo << "Saving without KGame::savegame() is untested! (KDE 3.1 has KGame::savegame())" << endl;
 if (!KGame::save(stream, saveplayers)) {
#else
 if (!KGame::savegame(stream, network, saveplayers)) {
#endif
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
 emit signalLoadingType(BosonLoadingWidget::LoadSavedGameHeader);

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

void Boson::slotAddChatSystemMessage(const QString& fromName, const QString& text)
{
 // just forward it to BosonWidgetBase
 emit signalAddChatSystemMessage(fromName, text);
}

void Boson::slotAddChatSystemMessage(const QString& text)
{
 slotAddChatSystemMessage(i18n("Boson"), text);
}

void Boson::slotDebugOutput(const QString& area, const char* data, int level)
{
 slotAddChatSystemMessage(area, data);
}

unsigned int Boson::delayedMessageCount() const
{
 return d->mMessageDelayer->delayedMessageCount();
}

unsigned int Boson::delayedAdvanceMessageCount() const
{
 return d->mMessageDelayer->delayedAdvanceMessageCount();
}

bool Boson::loadXMLDoc(QDomDocument* doc, const QString& xml)
{
 QString errorMsg;
 int lineNo, columnNo;
 if (!doc->setContent(xml, &errorMsg, &lineNo, &columnNo)) {
	boError() << k_funcinfo << "Parse error in line " << lineNo << ",column " << columnNo
			<< " error message: " << errorMsg << endl;
	return false;
 }
 return true;
}

Q_UINT16 Boson::bosonPort()
{
#ifdef KGAME_HAVE_KGAME_PORT
 return KGame::port();
#else
 if (!isNetwork()) {
	return 0;
 }
 boDebug() << k_funcinfo << endl;
 // ugly hack... luckily we *can* do such hacks :)
 const QObjectList* list = QObject::objectTrees();
 QObjectListIt it(*list);
 if (isOfferingConnections()) {
	for (; it.current(); ++it) {
		if (qstrcmp((*it)->className(), "KMessageServerSocket") == 0) {
			QServerSocket* server = (QServerSocket*)(*it);
			return server->port();
		}
	}
	boWarning() << k_funcinfo << "could not find KMessageServerSocket!" << endl;
 } else {
	for (; it.current(); ++it) {
		if (qstrcmp((*it)->className(), "QSocket") == 0) {
			QSocket* socket = (QSocket*)*it;
			return socket->peerPort();

		}
	}
	boWarning() << k_funcinfo << "could not find QSocket!" << endl;
 }
 return 0;
#endif
}

QString Boson::bosonHostName()
{
#ifdef KGAME_HAVE_KGAME_HOSTNAME
 return KGame::hostName();
#else
 if (!isNetwork() || isOfferingConnections()) {
	return QString::fromLatin1("localhost");
 }
 boDebug() << k_funcinfo << endl;
 // ugly hack... luckily we *can* do such hacks :)
 const QObjectList* list = QObject::objectTrees();
 QObjectListIt it(*list);
 for (; it.current(); ++it) {
	if (qstrcmp((*it)->className(), "QSocket") == 0) {
		QSocket* socket = (QSocket*)*it;
		return socket->peerName();
	}
 }
 boWarning() << k_funcinfo << "could not find QSocket!" << endl;
 return QString::fromLatin1("localhost");
#endif
}

void Boson::bosonAddPlayer(KPlayer* player)
{
 if (player->id() != 0) {
	// we don't have a problem.
	KGame::addPlayer(player);
	return;
 }

#ifndef KGAME_HAVE_FIXED_ADDPLAYER_ID
 // this is going to be ugly.
 // in KDE < 3.2 KGame::addPlayer() did not set a player ID, but sent the player
 // out directly. KGame::systemAddPlayer() would set the ID then on all clients,
 // but as they have different client IDs, this ID would be broken.
 // in KDE 3.2 addPlayer() does this correctly.
 //
 // as a workaround we call systemAddPlayer() directly here and revert
 // everything that is done beyond the call to player->setId().
 // note that this is possibe, as we already _do_ know what is done in
 // systemAddPlayer() and that it can _not_ change in the future, as this refers
 // to KDE < 3.2 _only_
 //
 // also note that simply setting the ID on our own is even more dangerous. e.g.
 // the state of the ID counter is saved together with KGame and we can get very
 // undefined results if it is invalid!

 // first prevent signalPlayerJoinedGame() from being emitted.
 bool blocks = signalsBlocked();
 blockSignals(true);

 // save the old KGame pointer of the player. probably NULL.
 KGame* oldGame = player->game();

 // now do the dangerous call.
 systemAddPlayer(player);

 // remove the player from the player list. will be added once systemAddPlayer()
 // is called from a valid point.
 playerList()->removeRef(player);

 // revert to the old KGame pointer
 player->setGame(oldGame);

 // allow signals again, if they were before. we are done now.
 blockSignals(blocks);
#else
 // we can safely call KGame::addPlayer() directly
#endif
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

void Boson::saveGameLogs(const QString& prefix)
{
 BosonProfiler p(BosonProfiling::SaveGameLogs);

 // Write gamelog
 QFile gl(prefix + ".gamelog");
 if (!gl.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << prefix << ".gamelog' for writing gamelog!" << endl;
	return;
 }
 QValueList<QByteArray>::iterator it;
 for (it = d->mGameLogs.begin(); it != d->mGameLogs.end(); it++) {
	gl.writeBlock(qUncompress(*it));
 }
 gl.close();

 // Write network message log
 QFile nl(prefix + ".netlog");
 if (!nl.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Can't open output file '" << prefix << ".netlog' for writing!" << endl;
	return;
 }
 if (!d->mMessageLogger.saveGameLog(&nl)) {
	boError() << k_funcinfo << "unable to write message log" << endl;
	return;
 }
 nl.close();

 boDebug() << k_funcinfo << "Done, elapsed: " << p.stop() << endl;
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

