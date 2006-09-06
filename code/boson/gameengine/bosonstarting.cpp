/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonstarting.h"
#include "bosonstarting.moc"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "boson.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosonmessageids.h"
#include "speciestheme.h"
#include "bosonprofiling.h"
#include "bodebug.h"
#include "bosonsaveload.h"

#include <klocale.h>
#include <kgame/kmessageclient.h>

#include <qtimer.h>
#include <qmap.h>

#define DO_GUI_INIT_ON_DATA_INIT 1

/**
 * Calls @ref Boson::lock on construction and @ref Boson::unlock on destruction,
 * or on @ref unlock.
 **/
class BosonStartingBosonLocker
{
public:
	BosonStartingBosonLocker(Boson* b)
	{
		mBoson = b;
		mBoson->lock();
		mIsLocked = true;
	}
	~BosonStartingBosonLocker()
	{
		unlock();
	}

	void unlock()
	{
		if (mIsLocked) {
			mBoson->unlock();
			mIsLocked = false;
		}
	}

private:
	Boson* mBoson;
	bool mIsLocked;
};

class BosonStartingPrivate
{
public:
	BosonStartingPrivate()
	{
	}
	QPtrList<BosonStartingTaskCreator> mTaskCreators;

	QValueList<Q_UINT32> mStartingCompleted; // clients that completed starting
	QMap<unsigned int, QByteArray> mStartingCompletedMessage;

	QString mLoadFromLogFile;
};



BosonStarting::BosonStarting(QObject* parent) : QObject(parent, "bosonstarting")
{
 d = new BosonStartingPrivate;
 mDestPlayField = 0;
 d->mTaskCreators.setAutoDelete(true);
}

BosonStarting::~BosonStarting()
{
 d->mTaskCreators.setAutoDelete(true);
 d->mTaskCreators.clear();
 delete mDestPlayField;
 delete d;
}

void BosonStarting::addTaskCreator(BosonStartingTaskCreator* c)
{
 d->mTaskCreators.append(c);
}

void BosonStarting::slotSetNewGameData(const QByteArray& data, bool* taken)
{
 if (taken) {
	if (*taken) {
		boError() << k_funcinfo << "data has been taken before already! only the starting object should take it!" << endl;
		// don't return
	}
 }
 mNewGameData = data;
 if (taken) {
	*taken = true;
 }
}

void BosonStarting::setEditorMap(const QByteArray& buffer)
{
}

void BosonStarting::setLoadFromLogFile(const QString& file)
{
 // TODO check if thats a valid file
 d->mLoadFromLogFile = file;
}

QString BosonStarting::logFile() const
{
 return d->mLoadFromLogFile;
}

void BosonStarting::slotStartNewGameWithTimer()
{
 boDebug(270) << k_funcinfo << endl;

 // we need to do this with timer because we can't add checkEvents() here. there is a
 // (more or less) KGame bug in KDE < 3.1 that causes KGame to go into an
 // infinite loop when calling checkEvents() from a slot that gets called from a
 // network message (exact: from KMessageDirect::received())
 QTimer::singleShot(0, this, SLOT(slotStart()));
}

bool BosonStarting::executeTasks(const QPtrList<BosonStartingTask>& tasks)
{
 unsigned long int duration = 0;
 for (QPtrListIterator<BosonStartingTask> it(tasks); it.current(); ++it) {
	disconnect(it.current(), SIGNAL(signalStartSubTask(const QString&)), this, 0);
	connect(it.current(), SIGNAL(signalStartSubTask(const QString&)),
			this, SIGNAL(signalLoadingStartSubTask(const QString&)));
	disconnect(it.current(), SIGNAL(signalCompleteSubTask(unsigned int)), this, 0);
	connect(it.current(), SIGNAL(signalCompleteSubTask(unsigned int)),
			this, SIGNAL(signalLoadingTaskCompleted(unsigned int)));

	duration += it.current()->taskDuration();
 }
 emit signalLoadingMaxDuration(duration);

 duration = 0;
 emit signalLoadingTaskCompleted(duration);
 for (QPtrListIterator<BosonStartingTask> it(tasks); it.current(); ++it) {
	boDebug(270) << k_funcinfo << "starting task: " << it.current()->text() << endl;
	emit signalLoadingStartTask(it.current()->text());
	emit signalLoadingStartSubTask("");

	boProfiling->push(QString("StartingTask: %1").arg(it.current()->text()));
	bool completed = it.current()->start(duration);
	boProfiling->pop();
	if (!completed) {
		boError(270) << k_funcinfo << "could not complete task " << it.current()->text() << endl;

		return false;
	}
	boDebug(270) << k_funcinfo << "completed task: " << it.current()->text() << endl;

	duration += it.current()->taskDuration();
	emit signalLoadingTaskCompleted(duration);
 }
 return true;
}

void BosonStarting::slotStart()
{
 boDebug(270) << k_funcinfo << "STARTING" << endl;
 if (!start()) {
	boError(270) << k_funcinfo << "STARTING: game starting failed" << endl;
	emit signalStartingFailed();
	return;
 }
 boDebug(270) << k_funcinfo << "STARTING: game starting succeeded" << endl;
}

void BosonStarting::slotPlayFieldCreated(BosonPlayField* playField, bool* ownerChanged)
{
 if (!ownerChanged) {
	BO_NULL_ERROR(ownerChanged);
	return;
 }
 if (!playField) {
	BO_NULL_ERROR(playField);
	// don't return
 }
 mDestPlayField = playField;
 *ownerChanged = true;

 emit signalDestPlayField(mDestPlayField);
}

bool BosonStarting::start()
{
 BosonProfiler profiler("BosonStarting::start()", "Starting");
 d->mStartingCompleted.clear();

 boDebug(270) << k_funcinfo << endl;
 if (!boGame) {
	boError(270) << k_funcinfo << "NULL boson object" << endl;
	return false;
 }
 if (boGame->gameStatus() != KGame::Init) {
	boError(270) << k_funcinfo
		<< "Boson must be in init status to receive map!" << endl
		<< "Current status: " << boGame->gameStatus() << endl;
	return false;
 }
 BosonStartingBosonLocker lock(boGame); // calls Boson::lock() and unlocks it again on destruction

 QMap<QString, QByteArray> files;
 if (!BosonPlayField::unstreamFiles(files, mNewGameData)) {
	boError(270) << k_funcinfo << "invalid newgame stream" << endl;
	return false;
 }


 QPtrList<BosonStartingTask> tasks;
 tasks.setAutoDelete(true);

 for (QPtrListIterator<BosonStartingTaskCreator> it(d->mTaskCreators); it.current(); ++it) {
	it.current()->setFiles(&files);
	if (!it.current()->createTasks(&tasks)) {
		boError(270) << k_funcinfo << "tasks of " << it.current()->creatorName() << " cannot be created" << endl;
		return false;
	}
 }


#if DO_GUI_INIT_ON_DATA_INIT
#warning TODO: split up?
 // AB: this should be split up. there are eventlisteners for both, for the game
 // engine (canvas eventlistener) and the GUI (localplayer eventlistener)
 // -> we should split them into two different tasks

 BosonStartingLoadEventListeners* eventListeners = new BosonStartingLoadEventListeners(i18n("Load event listeners"));
 eventListeners->setFiles(&files);
 tasks.append(eventListeners);
#endif

 bool ret = executeTasks(tasks);

 tasks.setAutoDelete(true);
 tasks.clear();

 if (!ret) {
	return false;
 }


 sendStartingCompleted(true);
 return true;
}

void BosonStarting::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(100);
 }
}

void BosonStarting::slotStartingCompletedReceived(const QByteArray& buffer, Q_UINT32 sender)
{
 if (!d->mStartingCompleted.contains(sender)) {
	d->mStartingCompleted.append(sender);
	d->mStartingCompletedMessage.insert(sender, buffer);
 }

 if (!boGame->isAdmin()) {
	return;
 }
 QValueList<Q_UINT32> clients = boGame->messageClient()->clientList();
 if (clients.count() > d->mStartingCompleted.count()) {
	return;
 }
 QValueList<Q_UINT32>::Iterator it;
 for (it = clients.begin(); it != clients.end(); ++it) {
	if (!d->mStartingCompleted.contains(*it)) {
		return;
	}
 }

 boDebug(270) << k_funcinfo << "received IdGameStartingCompleted from all clients." << endl;
 if (!checkStartingCompletedMessages()) {
	#warning TODO
	// TODO: abort game starting.
	// AB: we cannot use return, as then boson would be in a unusable state
	// (cannot leave loading widget)
	boError(270) << k_funcinfo << "starting messages broken." << endl;
	boError(270) << k_funcinfo << "TODO: abort game starting" << endl;
 } else {
	boDebug(270) << k_funcinfo << "all IdGameStartingCompleted valid. starting the game." << endl;
 }

 // AB: _first_ set the new game status.
 // note: Boson is in PolicyClean, so the game status does *not* change
 // immediately. but it will change before IdGameIsStarted is received.
 boGame->setGameStatus(KGame::Run);

 // AB: d->mLoadFromLogFile is null usually. non-null makes sense only for
 // non-network games, we will not start a normal game, but reproduce from a log
 // file then.
 boGame->sendMessage(d->mLoadFromLogFile, BosonMessageIds::IdGameIsStarted);
}

void BosonStarting::sendStartingCompleted(bool success)
{
 // AB: note: in normal games this is not required. however sending this message
 // means that we have this message in the log, which in turn makes the
 // "loadfromlog" mode much easier.
 boGame->sendMessageSyncRandom();

 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_INT8)success;
 QCString themeMD5;
 for (unsigned int i = 0; i < boGame->allPlayerCount(); i++) {
	Player* p = (Player*)boGame->allPlayerList()->at(i);
	SpeciesTheme* theme = p->speciesTheme();
	if (!theme) {
		// make an invalid string.
		themeMD5 = QCString();
		break;
	}
	QCString num;
	num.setNum(p->bosonId());
	themeMD5 += QCString("Player ") + num + ":\n";
	themeMD5 += "UnitProperties:\n" + theme->unitPropertiesMD5();
	themeMD5 += "\n";
 }
 stream << themeMD5;

 boGame->sendMessage(b, BosonMessageIds::IdGameStartingCompleted);
}

bool BosonStarting::checkStartingCompletedMessages() const
{
 if (!boGame->isAdmin()) {
	boError(270) << k_funcinfo << "only ADMIN can do this" << endl;
	return false;
 }
 QByteArray admin = d->mStartingCompletedMessage[boGame->gameId()];
 if (admin.size() == 0) {
	boError(270) << k_funcinfo << "have not StartingCompleted message from ADMIN" << endl;
	return false;
 }
 QDataStream adminStream(admin, IO_ReadOnly);
 Q_INT8 adminSuccess;
 adminStream >> adminSuccess;
 if (!adminSuccess) {
	boError(270) << k_funcinfo << "ADMIN failed in game starting" << endl;
	return false;
 }
 QCString adminThemeMD5;
 adminStream >> adminThemeMD5;
 if (adminThemeMD5.isNull()) {
	boError(270) << k_funcinfo << "no MD5 string for themes by ADMIN" << endl;
	return false;
 }
 QMap<unsigned int, QByteArray>::Iterator it = d->mStartingCompletedMessage.begin();
 for (; it != d->mStartingCompletedMessage.end(); ++it) {
	if (it.key() == boGame->gameId()) {
		continue;
	}
	QDataStream stream(it.data(), IO_ReadOnly);
	Q_INT8 success;
	stream >> success;
	if (!success) {
		boError(270) << k_funcinfo << "client " << it.key() << " failed on game starting" << endl;
		return false;
	}
	QCString themeMD5;
	stream >> themeMD5;
	if (themeMD5 != adminThemeMD5) {
		boError(270) << k_funcinfo << "theme MD5 sums of client "
				<< it.key()
				<< " and ADMIN differ." << endl
				<< "ADMIN has: " << adminThemeMD5 << endl
				<< "client " << it.key() << " has: " << themeMD5 << endl;
		return false;
	}
 }
 return true;
}


BosonStartingTaskCreator::BosonStartingTaskCreator(QObject* parent)
	: QObject(parent, "bosonstartingtaskcreator")
{
}

BosonStartingTaskCreator::~BosonStartingTaskCreator()
{
}


BosonStartingTask::BosonStartingTask(const QString& text)
	: QObject(0)
{
 mText = text;
 mTimePassed = 0;
}

BosonStartingTask::~BosonStartingTask()
{
}

bool BosonStartingTask::start(unsigned int timePassed)
{
 mTimePassed = timePassed;
 return startTask();
}

void BosonStartingTask::startSubTask(const QString& text)
{
 emit signalStartSubTask(text);
}

void BosonStartingTask::completeSubTask(unsigned int duration)
{
 if (duration > taskDuration()) {
	boError(270) << k_funcinfo << "a sub task must not take more time than the whole task! (" << duration << " > " << taskDuration() << endl;
	duration = taskDuration();
 }
 emit signalCompleteSubTask(mTimePassed + duration);
}

void BosonStartingTask::checkEvents()
{
 if (qApp->hasPendingEvents()) {
	qApp->processEvents(100);
 }
}


bool BosonStartingLoadEventListeners::startTask()
{
 PROFILE_METHOD
 if (!boGame) {
	BO_NULL_ERROR(boGame);
	return false;
 }

 if (!boGame->eventManager()) {
	return false;
 }
 if (!mFiles) {
	return false;
 }
 startSubTask(i18n("Scripts..."));
 BosonSaveLoad load(boGame);
 if (!load.loadEventListenerScripts(*mFiles)) {
	boError(270) << k_funcinfo << "unable to load eventlistener scripts" << endl;
	return false;
 }
 completeSubTask(25);

 startSubTask(i18n("XML..."));
 if (!load.loadEventListenersXML(*mFiles)) {
	boError(270) << k_funcinfo << "unable to load eventlistener scripts" << endl;
	return false;
 }
 completeSubTask(25);


#warning TODO: clear variable data
 // AB: after loading all scripts, we should clear the script data.
 //     it has already been loaded into the scripts, so it is not required
 //     anymore (WARNING: at THIS point this may NOT be the case! IOs are added
 //     later!)
 //     but when removing/adding IOs on the fly, we would always use the script
 //     data of the moment when we saved the game, which is not intended.
 //     instead the script should simply be reloaded in that case.
 //
 //     but to be able to clear the script data, we need to make sure that _ALL_
 //     scripts have been loaded...
 //
 //     UPDATE: at this point all scripts should be loaded.

 return true;
}


unsigned int BosonStartingLoadEventListeners::taskDuration() const
{
 return 50;
}


