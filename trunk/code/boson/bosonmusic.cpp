/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonmusic.moc"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kartsserver.h>
#include <kplayobject.h>
#include <kplayobjectfactory.h>

#include <qtimer.h>
#include <qstringlist.h>

#define TICKER_VALUE 500 // same as in kaboodle

class BosonMusic::BosonMusicPrivate
{
public:
	BosonMusicPrivate()
	{
		mPlayObject = 0;

		mTicker = 0;
	}

	KArtsServer mServer;
	KPlayObject* mPlayObject;

	QTimer* mTicker;

	QStringList mFiles;
	bool mLoop;
};

//AB: interesting question: we have sounds and we have music. is it better to
//use 2 KArtsServer and 2 KArtsDispatcher objects or is it better to use a
//single one??

BosonMusic::BosonMusic(QObject* parent) : QObject(parent)
{
 d = new BosonMusicPrivate;
 d->mTicker = new QTimer(this);
 connect(d->mTicker, SIGNAL(timeout()), this, SLOT(slotUpdateTicker()));
 d->mLoop = false;
}

BosonMusic::~BosonMusic()
{
 delete d;
}

void BosonMusic::play()
{
 if (!d->mPlayObject) {
	return;
 }
 d->mPlayObject->play();
 d->mTicker->start(TICKER_VALUE);
}

bool BosonMusic::load(const QString& file)
{
 kdDebug() << k_funcinfo << file << endl;
 if (d->mPlayObject) {
	delete d->mPlayObject;
 }
 KPlayObjectFactory factory(d->mServer.server());
 d->mPlayObject = factory.createPlayObject(file, true);
 if (d->mPlayObject->isNull()) {
	delete d->mPlayObject;
	d->mPlayObject = 0;
	return false;
 }
 play();
 return true;
}

QStringList BosonMusic::availableMusic() const
{
 QStringList list = KGlobal::dirs()->findAllResources("data",
		"boson/music/*.mp3", true);
 list += KGlobal::dirs()->findAllResources("data",
		"boson/music/*.ogg", true);
 if (list.isEmpty()) {
	kdDebug() << "no music found" << endl;
	return list;
 }
 return list;
}

void BosonMusic::startLoop()
{
 startLoop(availableMusic());
}

void BosonMusic::startLoop(const QStringList& files)
{
 // the loop is played in *random order*!
 d->mFiles = files;
 if (d->mFiles.count() == 0) {
	d->mLoop = false;
	return;
 }
 d->mLoop = true;
 int pos = kapp->random() %d->mFiles.count();
 if (!load(d->mFiles[pos])) {
	d->mFiles.remove(d->mFiles.at(pos));
 }
}

void BosonMusic::slotUpdateTicker()
{
 if (!d->mPlayObject || d->mPlayObject->isNull()) {
	return;
 }
 if (d->mPlayObject->state() == Arts::posIdle) {
	if (isLoop()) {
		// continue the loop
		startLoop(d->mFiles);
	} else if (d->mFiles.count() != 0) {
		// play the next file that can be loaded
		while (d->mFiles.count() > 0 && !load(d->mFiles[0])) {
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
