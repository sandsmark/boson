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

#include "bosonaudio.h"

#include "bodebug.h"
#include "bosonmusic.h"
#include "bosonsound.h"

#include <arts/kartsserver.h>
#include <arts/kartsdispatcher.h>

#include <qtimer.h>
#include <qstringlist.h>
#include <qdict.h>

bool BosonAudio::mCreated = false;


class BosonAudioPrivate
{
public:
	BosonAudioPrivate()
	{
		mDispatcher = 0;
		mServer = 0;

		mBosonMusic = 0;
	}

	KArtsDispatcher* mDispatcher;
	KArtsServer* mServer;

	// add the soundeffectstack here

	BosonMusic* mBosonMusic;
	QDict<BosonSound> mBosonSound;

	bool mIsNull;

	bool mPlayMusic;
	bool mPlaySound;
};

BosonAudio* BosonAudio::create()
{
 if (!mCreated) {
	mCreated = true;
	return new BosonAudio;
 }
 return 0;
}

BosonAudio::BosonAudio()
{
 d = new BosonAudioPrivate;
 d->mIsNull = false;
 d->mPlayMusic = true;
 d->mPlaySound = true;
 d->mBosonSound.setAutoDelete(true);
 boDebug(200) << k_funcinfo << "Starting arts dispatcher" << endl;
 d->mDispatcher = new KArtsDispatcher(0);
 d->mServer = new KArtsServer(0);
 if (d->mServer->server().isNull()) {
	boWarning(200) << "Cannot access KArtsServer - sound disabled" << endl;
	// TODO: message box in GUI thread
	d->mIsNull = true;
	d->mPlayMusic = false;
	d->mPlaySound = false;
 } else {
	Arts::TraderQuery query;
	query.supports("Interface", "Arts::PlayObject");
	query.supports("Extension", "ogg");
	std::vector<Arts::TraderOffer>* offers = query.query();
	if (offers->empty()) {
		boWarning(200) << "Your arts installation does not support .ogg files! Disabling sounds now..." << endl;
		// TODO: message box in GUI thread
		d->mIsNull = true;
	} else {
		boDebug(200) << k_funcinfo << "ogg support seems to be ok" << endl;
		std::vector<Arts::TraderOffer>::iterator it;
		for (it = offers->begin(); it != offers->end(); it++) {
			boDebug(200) << "ogg offer: " << it->interfaceName().c_str() << endl;
		}
	}
	delete offers;
 }

 if (d->mIsNull) {
	return;
 }

 // warning: NULL if sound disabled (--nosound)!
 d->mBosonMusic = new BosonMusic(this);
}

BosonAudio::~BosonAudio()
{
 boDebug(200) << k_funcinfo << endl;
 d->mBosonSound.clear();
 delete d->mBosonMusic;
 delete d->mServer;
 delete d->mDispatcher;
 delete d;
 mCreated = false;
}

bool BosonAudio::isNull() const
{
 return d->mIsNull;
}

BosonMusic* BosonAudio::bosonMusic() const
{
 return d->mBosonMusic;
}

KArtsServer& BosonAudio::server() const
{
 return *d->mServer;
}

BosonSound* BosonAudio::addSounds(const QString& species)
{
 if (!d->mBosonSound.find(species)) {
	d->mBosonSound.insert(species, new BosonSound(this));
 }
 return d->mBosonSound[species];
}

BosonSound* BosonAudio::bosonSound(const QString& species) const
{
 return d->mBosonSound[species];
}

void BosonAudio::setMusic(bool m)
{
 d->mPlayMusic = m;
}

void BosonAudio::setSound(bool s)
{
 d->mPlaySound = s;
}

bool BosonAudio::music() const
{
 if (isNull()) {
	return false;
 }
 return d->mPlayMusic;
}

bool BosonAudio::sound() const
{
 if (isNull()) {
	return false;
 }
 return d->mPlaySound;
}

