/*
    This file is part of the Boson game
    Copyright (C) 2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "../defines.h"
#include "../bosonconfig.h"
#include "bosonsound.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kstaticdeleter.h>
#include <kdebug.h>

#include <arts/kplayobject.h>
#include <arts/kplayobjectfactory.h>
#include <arts/kartsserver.h>
#include <arts/kartsdispatcher.h>

#include <qtimer.h>
#include <qstringlist.h>
#include <qdict.h>

#include "bosonmusic.moc"


#define TICKER_VALUE 500 // same as in kaboodle

static KStaticDeleter<BosonMusic> sd;
BosonMusic* BosonMusic::mBosonMusic = 0;


class BosonMusic::BosonMusicPrivate
{
public:
	BosonMusicPrivate()
	{
		mPlayObject = 0;

		mTicker = 0;
	}

	KArtsDispatcher mDispatcher;
	KArtsServer mServer;
	KPlayObject* mPlayObject;

	QDict<BosonSound> mBosonSound;

	QTimer* mTicker;

	QStringList mFiles;
	bool mLoop;

	bool mPlayMusic;
	bool mPlaySound;
};

BosonMusic::BosonMusic(QObject* parent) : QObject(parent)
{
 d = new BosonMusicPrivate;
 d->mPlayMusic = true;
 d->mPlaySound = true;
 d->mTicker = new QTimer(this);
 connect(d->mTicker, SIGNAL(timeout()), this, SLOT(slotUpdateTicker()));
 d->mLoop = false;
 d->mBosonSound.setAutoDelete(true);
 if (d->mServer.server().isNull()) {
	kdWarning() << "Cannot access KArtsServer - sound disabled" << endl;
	// TODO: message box
	d->mPlayMusic = false;
	d->mPlaySound = false;
	boConfig->setDisableSound(true);
 } else {
	Arts::TraderQuery query;
	query.supports("Interface", "Arts::PlayObject");
	query.supports("Extension", "ogg");
	vector<Arts::TraderOffer>* offers = query.query();
	if (offers->empty()) {
		kdWarning() << "Your arts installation does not support .ogg files! Disabling sounds now..." << endl;
		// TODO: message box
		d->mPlayMusic = false;
		d->mPlaySound = false;
		boConfig->setDisableSound(true);
	} else {
		kdDebug() << k_funcinfo << "ogg support seems to be ok" << endl;
		vector<Arts::TraderOffer>::iterator it;
		for (it = offers->begin(); it != offers->end(); it++) {
			kdDebug() << "ogg offer: " << it->interfaceName().c_str() << endl;
		}
	}
	delete offers;
 }
}

BosonMusic::~BosonMusic()
{
kdDebug() << k_funcinfo << endl;
 d->mBosonSound.clear();
 delete d->mTicker;
 if (d->mPlayObject) {
	if (!d->mPlayObject->isNull()) {
		kdDebug() << "halting music file" << endl;
		d->mPlayObject->halt();
	}
	delete d->mPlayObject;
 }
 delete d;
}

void BosonMusic::initBosonMusic()
{
 if (mBosonMusic) {
	return;
 }
 sd.setObject(mBosonMusic, new BosonMusic(0));
}

void BosonMusic::play()
{
 if (boConfig->disableSound()) {
	return;
 }
 if (!d->mPlayObject || d->mPlayObject->isNull()) {
	return;
 }
 d->mPlayObject->play();
 d->mTicker->start(TICKER_VALUE);
}

void BosonMusic::stop()
{
 if (d->mPlayObject && !d->mPlayObject->isNull()) {
	d->mPlayObject->pause(); // AB: or halt() ??
 }
 d->mTicker->stop();
}

bool BosonMusic::load(const QString& file)
{
 if (boConfig->disableSound()) {
	return false;
 }
 kdDebug() << k_funcinfo << file << endl;
 delete d->mPlayObject;
 KPlayObjectFactory factory(server().server());
 d->mPlayObject = factory.createPlayObject(file, true);
 if (d->mPlayObject->isNull()) {
	delete d->mPlayObject;
	d->mPlayObject = 0;
	return false;
 }
 if (music()) {
	play();
 }
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

bool BosonMusic::sound() const
{
 return d->mPlaySound;
}

bool BosonMusic::music() const
{
 return d->mPlayMusic;
}

void BosonMusic::setMusic(bool music_)
{
 if (music() == music_) {
	return;
 }
 d->mPlayMusic = music_;
 if (music()) {
	play();
 } else {
	stop();
 }
}

void BosonMusic::setSound(bool sound_)
{
 if (sound() == sound_) {
	return;
 }
 d->mPlaySound = sound_;
}

BosonSound* BosonMusic::addSounds(const QString& species)
{
 if (!d->mBosonSound.find(species)) {
	d->mBosonSound.insert(species, new BosonSound);
 }
 return d->mBosonSound[species];
}

BosonSound* BosonMusic::bosonSound(const QString& species) const
{
 return d->mBosonSound[species];
}

KArtsServer& BosonMusic::server() const
{
 return d->mServer;
}

