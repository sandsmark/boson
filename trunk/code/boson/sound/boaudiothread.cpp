/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boaudiothread.h"

#include "boaudiocommand.h"
#include "bosonaudio.h"
#include "bosonmusic.h"
#include "bosonsound.h"
#include "bosonaudiointerface.h"
#include "bodebug.h"

#include <qptrqueue.h>


/**
 * Shamelessy stolen from Rik Hemsley: http://rikkus.info/kde_mt.html
 **/
template<class T> class MTQueue
{
public:

	MTQueue()
	{
	}
	bool isEmpty()
	{
		mMutex.lock();
		bool empty = mQueue.isEmpty();
		mMutex.unlock();
		return empty;
	}

	void flush()
	{
		mMutex.lock();
		while (!mQueue.isEmpty()) {
			delete (mQueue.dequeue());
		}
		mMutex_.unlock();
	}

	void enqueue(T * t)
	{
		mMutex.lock();
		mQueue.enqueue(t);
		mMutex.unlock();
	}

	T* dequeue()
	{
		mMutex.lock();
		T* i = mQueue.dequeue();
		mMutex.unlock();
		return i;
	}

private:
	QPtrQueue<T> mQueue;
	QMutex mMutex;
};

class BoAudioThreadPrivate
{
public:
	BoAudioThreadPrivate()
	{
		mAudio = 0;
	}

	QMutex mMutex;

	BosonAudio* mAudio;

	MTQueue<BoAudioCommand> mCommandQueue;
};

BoAudioThread::BoAudioThread() : QThread()
{
 d = new BoAudioThreadPrivate;

 boDebug(200) << k_funcinfo << "construct BosonAudio" << endl;
 d->mAudio = BosonAudio::create();
 if (!d->mAudio) {
	boDebug(200) << k_funcinfo << "NULL BosonAudio constructed" << endl;
 } else {
	boDebug(200) << k_funcinfo << "BosonAudio constructed" << endl;
 }
}

BoAudioThread::~BoAudioThread()
{
 boDebug(200) << k_funcinfo << endl;
 delete d->mAudio;
 delete d;
 boDebug(200) << k_funcinfo << "done" << endl;
}

BosonAudio* BoAudioThread::audio() const
{
 return d->mAudio;
}

bool BoAudioThread::audioStarted() const
{
 // do we have to lock the mutex here?
// lock();
 bool ret = true;
 if (!audio()) {
	ret = false;
 }
 if (ret && audio()->isNull()) {
	ret = false;
 }
// unlock();
 return ret;
}

void BoAudioThread::run()
{
 boDebug(200) << k_funcinfo << endl;
 while (true) {
	while (d->mCommandQueue.isEmpty()) {
		msleep(100); // TODO: increase?

		// maybe we can implement BosonMusic::slotUpdateTicker() at this
		// point!
	}
	BoAudioCommand* command = dequeueCommand();
	if (!command) {
		BO_NULL_ERROR(command);
		continue;
	}
	BosonSound* sound = 0;
	BosonMusic* music = 0;
	if (command->species().isEmpty()) {
		// most probably this is a music command
		if (command->type() != BoAudioCommand::CreateMusicObject) {
			music = audio()->bosonMusic();
			if (!music) {
				// happens e.g. for --nosound
				delete command;
				command = 0;
			}
		}
	} else {
		// this must be a sound command for a species.
		if (command->type() != BoAudioCommand::CreateSoundObject) {
			sound = audio()->bosonSound(command->species());
			if (!sound) {
				// happens e.g. for --nosound
				delete command;
				command = 0;
			}
		}
	}

	if (!command) {
		// we don't want to continue with this command by any reason
		continue;
	}
	executeCommand(command, audio(), music, sound);
	delete command;
 }
}

void BoAudioThread::executeCommand(BoAudioCommand* command, BosonAudio* audio, BosonMusic* music, BosonSound* sound)
{
 if (!command) {
	return;
 }
 switch (command->type()) {
	case BoAudioCommand::CreateMusicObject:
		boDebug(200) << k_funcinfo << "music object is created on startup. nothing to do" << endl;
		break;
	case BoAudioCommand::CreateSoundObject:
		if (!command->species().isEmpty()) {
			boDebug(200) << k_funcinfo << "create sound object for " << command->species() << endl;
			audio->addSounds(command->species());
		} else {
			boError(200) << k_funcinfo << "Cannot add a BosonSound object for an empty species string" << endl;
		}
		break;
	case BoAudioCommand::EnableSound:
		if (audio) {
			audio->setSound((bool)command->dataInt());
		}
		break;
	case BoAudioCommand::EnableMusic:
		if (audio) {
			audio->setMusic((bool)command->dataInt());
		}
		break;
	case BoAudioCommand::PlayMusic:
		if (music) {
			boDebug(200) << k_funcinfo << "start music playing" << endl;
			music->playMusic();
		}
		break;
	case BoAudioCommand::StopMusic:
		if (music) {
			boDebug(200) << k_funcinfo << "stop music playing" << endl;
			music->stopMusic();
		}
		break;
	case BoAudioCommand::ClearMusicList:
		if (music) {
			boDebug(200) << k_funcinfo << "clear music list" << endl;
			music->clearMusicList();
		}
		break;
	case BoAudioCommand::AddToMusicList:
		if (music) {
			QString file = command->dataString1();
//				boDebug(200) << k_funcinfo << "adding to music list: " << file << endl;
			music->addToMusicList(file);
		}
		break;
	case BoAudioCommand::StartMusicLoop:
		if (music) {
			boDebug(200) << k_funcinfo << "looping music list" << endl;
			music->startMusicLoop();
		}
		break;
	case BoAudioCommand::PlaySound:
		if (sound) {
			QString name = command->dataString1();
			int id = command->dataInt();
			if (!name.isEmpty()) {
				sound->playSound(name);
			} else {
				sound->playSound(id);
			}
		}
		break;
	case BoAudioCommand::AddUnitSound:
		if (sound) {
			QString name = command->dataString1();
			QString file = command->dataString2();
//			boDebug(200) << k_funcinfo << "adding sound " << name << "->" << file << endl;
			sound->addEventSound(name, file);
		}
		break;
	case BoAudioCommand::AddGeneralSound:
		if (sound) {
			QString file = command->dataString2();
			int id = command->dataInt();
//			boDebug(200) << k_funcinfo << "adding sound " << id << "->" << file << endl;
			sound->addEventSound(id, file);
		}
		break;
	default:
		boError() << k_funcinfo << "invalid type: " << command->type() << endl;
		break;
 }
}

void BoAudioThread::lock()
{
 d->mMutex.lock();
}

void BoAudioThread::unlock()
{
 d->mMutex.unlock();
}

void BoAudioThread::enqueueCommand(BoAudioCommand* command)
{
 d->mCommandQueue.enqueue(command);
}

BoAudioCommand* BoAudioThread::dequeueCommand()
{
 return d->mCommandQueue.dequeue();
}

