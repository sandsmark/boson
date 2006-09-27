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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosonsound.h"

#include "bodebug.h"
#include "bosonaudioal.h"

#include <kapplication.h>

#include <qintdict.h>

#include <AL/al.h>

class BoPlayObject
{
public:
	BoPlayObject(BosonSound* parent, const QString& file)
	{
		mFile = file;
		mParent = parent;
		mBuffer = 0;
		mSource = 0;

		reload();
	}
	~BoPlayObject()
	{
		// reset OpenAL data
		resetAL();
	}

	const QString& file() const { return mFile; }

	bool isNull() const
	{
		if (alIsBuffer(mBuffer) == AL_FALSE) {
			return true;
		}
		if (alIsSource(mSource) == AL_FALSE) {
			return true;
		}
		return false;
	}

	void play()
	{
		boDebug(200) << k_funcinfo << file() << endl;
		if (!isNull()) {
			boDebug(200) << "playing " << mSource << endl;
			alSourcePlay(mSource);
			mPlayed = true;
		} else {
			boDebug(200) << k_funcinfo << "NULL playobject" << endl;
		}
	}

	void reload()
	{
		boDebug(200) << k_funcinfo << file() << endl;
		if (checkALError()) {
			boError(200) << k_funcinfo << "an error occured before reloading" << endl;
			// not a critical error here.
		}
		resetAL();
		if (checkALError()) {
			boError(200) << k_funcinfo << "resetAL() failed." << endl;
			return;
		}
		mPlayed = false;
		alGenBuffers(1, &mBuffer);
		if (checkALError()) {
			boError(200) << k_funcinfo << "alGenBuffers() failed." << endl;
			return;
		}
		alGenSources(1, &mSource);
		if (checkALError()) {
			boError(200) << k_funcinfo << "unable to create buffer and source" << endl;
			return;
		}
		if (!mParent->loadFileToBuffer(mBuffer, file())) {
			boError(200) << k_funcinfo << "unable to load file " << file() << endl;
			resetAL();
			return;
		}

		// AB: note that we store the source in this playobject too atm!
		// in the future we would like to move the source out of this,
		// so that multiple sources can use a single playobject (i.e. a
		// single buffer) !
		// -> maybe we will even remove the playobjects, but thats in
		// the future.
		alSourcei(mSource, AL_BUFFER, mBuffer);
	}

	void playFromBeginning()
	{
		boDebug(200) << k_funcinfo << file() << endl;
		if (isNull()) {
			boWarning(200) << k_funcinfo << "isNull" << endl;
			return;
		}
		if (mPlayed) {
			reload();
		}
		play();
	}

protected:
	void resetAL()
	{
		if (alIsSource(mSource) == AL_TRUE) {
			alSourceStop(mSource);
			alDeleteSources(1, &mSource);
		}
		mSource = 0;
		if (alIsBuffer(mBuffer) == AL_TRUE) {
			alDeleteBuffers(1, &mBuffer);
		}
		mBuffer = 0;
	}
	bool checkALError()
	{
		return mParent->checkALError();
	}

private:
	BosonSound* mParent;

	QString mFile;
	bool mPlayed;

	ALuint mBuffer;
	ALuint mSource;
};

class BosonSound::BosonSoundPrivate
{
public:
	BosonSoundPrivate()
	{
		mParent = 0;
	}

	BosonAudioAL* mParent;

	QIntDict<BoPlayObject> mSounds;

	QMap<QString, QPtrList<BoPlayObject> > mUnitSounds;
};

BosonSound::BosonSound(BosonAudioAL* parent)
{
 d = new BosonSoundPrivate;
 d->mParent = parent;
 d->mSounds.setAutoDelete(true);

}

BosonSound::~BosonSound()
{
 boDebug(200) << k_funcinfo << endl;
 d->mSounds.clear();
 d->mUnitSounds.clear();

 delete d;
}

void BosonSound::addEventSound(const QString& name, const QString& file)
{
 if (file.isEmpty()) {
	boWarning(200) << k_funcinfo << "cannot add empty filename for event " << name << endl;
	return;
 }
 QPtrList<BoPlayObject> list = d->mUnitSounds[name];
 QPtrListIterator<BoPlayObject> it(list);
 for (; it.current(); ++it) {
	if (it.current()->file() == file) {
		return;
	}
 }
 BoPlayObject* playObject = new BoPlayObject(this, file);
 boDebug(200) << k_funcinfo << "loading " << file << endl;
 if (!playObject->isNull()) {
	d->mUnitSounds[name].append(playObject);
	d->mUnitSounds[name].setAutoDelete(true);
 } else {
	boWarning(200) << k_funcinfo << "NULL sound " << file << " for " << name << endl;
	delete playObject;
 }
}

void BosonSound::addEventSound(int id, const QString& file)
{
 // we are lacking support for multiple files per id!
 // such as we have for order_move_nn.ogg, where nn is the number of the
 // version.
 if (d->mSounds.find(id)) {
	return;
 }
 if (file.isEmpty()) {
	boWarning(200) << k_funcinfo << "cannot add empty filename for event " << id << endl;
	return;
 }
 boDebug(200) << k_funcinfo << "loading " << file << endl;
 BoPlayObject* playObject = new BoPlayObject(this, file);
 if (!playObject->isNull()) {
	d->mSounds.insert(id, playObject);
 } else {
	boWarning(200) << k_funcinfo << "NULL sound " << file << " for " << id << endl;
	delete playObject;
 }
}

void BosonSound::playSound(int id)
{
 if (!sound()) {
	return;
 }
 BoPlayObject* p = d->mSounds[id];
 if (p && !p->isNull()) {
	p->playFromBeginning();
 }
}

void BosonSound::playSound(const QString& name)
{
 if (!sound()) {
	return;
 }
 QPtrList<BoPlayObject>& list = d->mUnitSounds[name];

 BoPlayObject* p = 0;
 if (list.count() > 0) {
	int no = kapp->random() % list.count();
	p = list.at(no);
 } else {
	boWarning(200) << k_funcinfo << "empty list for " << name << endl;
	return;
 }

 if (!p || p->isNull()) {
	boDebug(200) << k_funcinfo << "NULL sound" << endl;
	return;
 }
 p->playFromBeginning();
}

void BosonSound::setSound(bool s)
{
 d->mParent->setSound(s);
}

bool BosonSound::sound() const
{
 return d->mParent->sound();
}

bool BosonSound::loadFileToBuffer(ALuint buffer, const QString& file)
{
 return d->mParent->loadFileToBuffer(buffer, file);
}

bool BosonSound::checkALError()
{
 return d->mParent->checkALError();
}

