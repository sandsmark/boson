/*
    This file is part of the Boson game
    Copyright (C) 2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonmusic.h"

#include "bodebug.h"
#include "bosonaudio.h"

// FIXME: we use kapp->random(). is that thread safe? probably not...
#include <kapplication.h>

#include <arts/kplayobject.h>
#include <arts/kplayobjectfactory.h>
#include <arts/kartsserver.h>

// FIXME we should not use a QTimer here!
#include <qtimer.h>

#include <qstringlist.h>
#include <qdict.h>

#include "bosonmusic.moc"


#define TICKER_VALUE 500 // same as in kaboodle

class BosonMusic::BosonMusicPrivate
{
public:
	BosonMusicPrivate()
	{
		mParent = 0;

		mPlayObject = 0;

		mTicker = 0;
	}

	BosonAudio* mParent;
	KPlayObject* mPlayObject;

	QTimer* mTicker;

	QStringList mFiles;
	bool mLoop;
};

BosonMusic::BosonMusic(BosonAudio* parent) : QObject(0, "bosonmusic")
{
 d = new BosonMusicPrivate;
 d->mParent = parent;
 d->mTicker = new QTimer(this);
 connect(d->mTicker, SIGNAL(timeout()), this, SLOT(slotUpdateTicker()));
 d->mLoop = false;
 if (!d->mParent) {
	boError(200) << k_funcinfo << "NULL parent" << endl;
	return;
 }
}

BosonMusic::~BosonMusic()
{
 boDebug(200) << k_funcinfo << endl;
 delete d->mTicker;
 if (d->mPlayObject) {
	if (!d->mPlayObject->isNull()) {
		boDebug(200) << "halting music file" << endl;
		d->mPlayObject->halt();
	}
	delete d->mPlayObject;
 }
 delete d;
}

void BosonMusic::playMusic()
{
 if (!d->mPlayObject || d->mPlayObject->isNull()) {
	boDebug(200) << k_funcinfo << "no playobject" << endl;
	return;
 }
 boDebug(200) << k_funcinfo << "playing now" << endl;
 d->mPlayObject->play();
 d->mTicker->start(TICKER_VALUE);
}

void BosonMusic::stopMusic()
{
 if (d->mPlayObject && !d->mPlayObject->isNull()) {
	d->mPlayObject->pause(); // AB: or halt() ??
 }
 d->mTicker->stop();
}

bool BosonMusic::loadMusic(const QString& file)
{
 boDebug(200) << k_funcinfo << file << endl;
 delete d->mPlayObject;
 KPlayObjectFactory factory(d->mParent->server().server());
 d->mPlayObject = factory.createPlayObject(file, true);
 if (d->mPlayObject->isNull()) {
	delete d->mPlayObject;
	d->mPlayObject = 0;
	return false;
 }
 play();
 return true;
}

void BosonMusic::startMusicLoop()
{
 d->mLoop = true;
#warning FIXME thread safe?
 int pos = kapp->random() % d->mFiles.count();
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
#warning we should avoid slots in this thread
// slots should be possible, but dangerous. what e.g. about QTimer, which gets
// used here? does it use an event for the timer? probably! events are not
// allowed!
 if (!d->mPlayObject || d->mPlayObject->isNull()) {
	return;
 }
 if (d->mPlayObject->state() == Arts::posIdle) {
	if (isLoop()) {
		// continue the loop
		startMusicLoop();
	} else if (d->mFiles.count() != 0) {
		// play the next file that can be loaded
		while (d->mFiles.count() > 0 && !loadMusic(d->mFiles[0])) {
			d->mFiles.pop_front();
		}
	}
 } else {
	if (d->mPlayObject && !d->mPlayObject->isNull()) {
		d->mTicker->start(TICKER_VALUE);
	}
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

