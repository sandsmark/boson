/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann <b_mann@gmx.de>

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

#include "bosonmusic.h"
#include "bosonmusic.moc"

#include "bodebug.h"
#include "bosonaudioal.h"

#include <kapplication.h>

#include <AL/al.h>

#include <qtimer.h>

#include <qstringlist.h>
#include <krandom.h>

#define TICKER_VALUE 500 // same as in kaboodle

class BosonMusic::BosonMusicPrivate
{
public:
	BosonMusicPrivate()
	{
		mParent = 0;

		mTicker = 0;
	}

	BosonAudioAL* mParent;

	QTimer* mTicker;

	QStringList mFiles;
	bool mLoop;


	ALuint mMusicBuffer;
	ALuint mMusicSource;
};

BosonMusic::BosonMusic(BosonAudioAL* parent) : QObject(0, "bosonmusic")
{
 d = new BosonMusicPrivate;
 d->mParent = parent;
 d->mTicker = new QTimer(this);
 connect(d->mTicker, SIGNAL(timeout()), this, SLOT(slotUpdateTicker()));
 d->mLoop = false;
 d->mMusicSource = 0;
 d->mMusicBuffer = 0;
 if (!d->mParent) {
	boError(200) << k_funcinfo << "NULL parent" << endl;
	return;
 }

 // AB: remove after this was run once
 if (alIsSource(0) == AL_TRUE || alIsBuffer(0) == AL_TRUE) {
	boError(200) << k_funcinfo << "oops - should be invalid" << endl;
 }

 initMusicSource();
}

BosonMusic::~BosonMusic()
{
 boDebug(200) << k_funcinfo << endl;
 delete d->mTicker;
 alDeleteSources(1, &d->mMusicSource);
 alDeleteBuffers(1, &d->mMusicBuffer);
 delete d;
}

void BosonMusic::playMusic()
{
 if (alIsBuffer(d->mMusicBuffer) != AL_TRUE) {
	boDebug(200) << k_funcinfo << "no music buffer" << endl;
	return;
 }
 if (alIsSource(d->mMusicSource) != AL_TRUE) {
	boDebug(200) << k_funcinfo << "no music source" << endl;
	return;
 }
 boDebug(200) << k_funcinfo << "playing now" << endl;
 alSourcePlay(d->mMusicSource);
 d->mTicker->start(TICKER_VALUE);
}

void BosonMusic::stopMusic()
{
 alSourcePause(d->mMusicSource);
 d->mTicker->stop();
}

bool BosonMusic::loadMusic(const QString& file)
{
 boDebug(200) << k_funcinfo << file << endl;
 if (checkALError()) {
	boError(200) << k_funcinfo << "OpenAL error before loading a file" << endl;
	// this was an old error. we can continue loading the file
 }
 if (alIsBuffer(d->mMusicBuffer) == AL_TRUE) {
	alDeleteBuffers(1, &d->mMusicBuffer);
 }
 alGenBuffers(1, &d->mMusicBuffer);
 if (checkALError()) {
	boError(200) << k_funcinfo << "error creating buffer for file " << file << endl;
	return false;
 }
 if (!d->mParent->loadFileToBuffer(d->mMusicBuffer, file)) {
	boError(200) << k_funcinfo << "error loading file " << file << endl;
	return false;
 }

 alDeleteSources(1, &d->mMusicSource);
 d->mMusicSource = 0;
 initMusicSource();

 alSourcei(d->mMusicSource, AL_BUFFER, d->mMusicBuffer);

 play();
 return true;
}

void BosonMusic::startMusicLoop()
{
 d->mLoop = true;
 int pos = KRandom::random() % d->mFiles.count();
 if (!loadMusic(d->mFiles[pos])) {
	d->mFiles.remove(d->mFiles.at(pos));
 }
}

void BosonMusic::addToMusicList(const QString& file)
{
 d->mFiles.append(file);
}

void BosonMusic::clearMusicList()
{
 d->mFiles.clear();
}

void BosonMusic::slotUpdateTicker()
{
// slots should be possible, but dangerous. what e.g. about QTimer, which gets
// used here? does it use an event for the timer? probably! events are not
// allowed!
 if (alIsBuffer(d->mMusicBuffer) != AL_TRUE || alIsSource(d->mMusicSource) != AL_TRUE) {
	boDebug(200) << k_funcinfo << "no music playing" << endl;
	return;
 }
 ALint v;
 alGetSourcei(d->mMusicSource, AL_SOURCE_STATE, &v);
 switch (v) {
	case AL_PAUSED:
	case AL_PLAYING:
		d->mTicker->stop();
		d->mTicker->start(TICKER_VALUE);
		break;
	case AL_STOPPED:
	{
		if (isLoop()) {
			// continue the loop
			startMusicLoop();
		} else if (d->mFiles.count() != 0) {
			// play the next file that can be loaded
			while (d->mFiles.count() > 0 && !loadMusic(d->mFiles[0])) {
				d->mFiles.pop_front();
			}
		}
		break;
	}
	case AL_INITIAL:
		break;
	default:
		boDebug(200) << k_funcinfo << "unknown state " << v << endl;
		break;
 }
}

bool BosonMusic::isLoop() const
{
 return d->mLoop;
}

void BosonMusic::setMusic(bool m)
{
 d->mParent->setMusic(m);
}

bool BosonMusic::music() const
{
 return d->mParent->music();
}

void BosonMusic::initMusicSource()
{
 if (d->mMusicSource != 0) {
	return;
 }
 alGenSources(1, &d->mMusicSource);
 if (checkALError()) {
	boError() << k_funcinfo << "unable to create music source" << endl;
	return;
 }
}

bool BosonMusic::checkALError()
{
 return d->mParent->checkALError();
}

