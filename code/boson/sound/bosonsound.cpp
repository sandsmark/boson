/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "../defines.h"
#include "bodebug.h"
#include "bosonaudio.h"

#include <kapplication.h>
#include <kmimetype.h>
#include <kdeversion.h>

#include <arts/kartsserver.h>
#include <arts/flowsystem.h>
#include <arts/connect.h>
#include <arts/artsflow.h>

#include <qstringlist.h>
#include <qintdict.h>
#include <qdict.h>
#include <qdir.h>
#include <qregexp.h>

class BoPlayObject
{
public:
#if KDE_VERSION >= 301
	BoPlayObject(BosonSound* parent, const QString& file)
	{
		mFile = file;
		mParent = parent;
		mPlayObject = Arts::PlayObject::null();
		reload();
	}
	~BoPlayObject()
	{
	}

	const QString& file() const { return mFile; }
	Arts::PlayObject object() const { return mPlayObject; }

	bool isNull() const
	{
		return object().isNull();
	}

	void play()
	{
		boDebug(200) << k_funcinfo << endl;
		if (!isNull()) {
			object().play();
			mPlayed = true;
		} else {
			boDebug(200) << k_funcinfo << "NULL playobject" << endl;
		}
	}

	void rewind()
	{
		boDebug(200) << k_funcinfo << endl;
		if (isNull()) {
			return;
		}
		if (!(object().capabilities() & Arts::capSeek)) {
			boDebug(200) << "cannot seek" << endl;
			reload();
			return;
		}
		if (position() > 0) {
			boDebug(200) << "seek" << endl;
			Arts::poTime begin;
			begin.seconds = 0;
			begin.ms = 0;
			object().seek(begin);
			if (position() > 0) {
				boDebug(200) << "seeking did not work :-(" << endl;
				reload();
			}
		}
	}
	void reload()
	{
		boDebug(200) << k_funcinfo << endl;
		KMimeType::Ptr mimeType = KMimeType::findByURL(file(), 0, true, true);

		Arts::PlayObject result;
		if (mParent->server().server().isNull()) {
			boWarning(200) << k_funcinfo << "NULL server" << endl;
			return;
		}
		result = mParent->server().server().createPlayObjectForURL(
				std::string(file().latin1()), std::string(mimeType->name().latin1()),
				false); // false as we connect it to the soundcard ourselfes
		if (result.isNull()) {
			boError(200) << k_funcinfo << "NULL playobject - file="
					<< file() << " mimetype="
					<< mimeType->name() << endl;
			return;
		}

		Arts::Synth_BUS_UPLINK uplink = Arts::DynamicCast(
				mParent->server().server().createObject(
				"Arts::Synth_BUS_UPLINK"));
		if (uplink.isNull()) {
			boError(200) << k_funcinfo << "NULL uplink" << endl;
			return;
		}
		uplink.busname("out_soundcard");
		Arts::connect(result, "left", uplink, "left");
		Arts::connect(result, "right", uplink, "right");
		uplink.start();
		result._node()->start();
		result._addChild(uplink, "uplink");

// this seems to be our major speed problem. usually it works just fine, but
// sometimes it takes 1-2 seconds!
// --> the game freezes for that time
// UPDATE (03/06/06): all audio code is in it's own process now, so this isn't
// critical anymore. but still, this is a problem, but boson doesn't freeze
// anymore.
		mPlayObject = result;

		mPlayed = false;
	}

	void playFromBeginning()
	{
		boDebug(200) << k_funcinfo << endl;
		if (isNull()) {
			boWarning(200) << k_funcinfo << "isNull" << endl;
			return;
		}
		if (object().state() != Arts::posIdle) {
			boDebug(200) << k_funcinfo << "not posIdle" << endl;
//			return; // AB: ? ??? 
		}
		if (mPlayed) {
			reload();
		}
		play();
	}

protected:
	int position()
	{
		if (isNull()) {
			return -1;
		}
		if (!(object().capabilities() & Arts::capSeek)) {
			boDebug(200) << "cannot seek!!" << endl;
			return -1;
		}
		Arts::poTime t(object().currentTime());
		boDebug(200) << k_funcinfo << t.ms + t.seconds * 1000 << endl;
		return t.ms + t.seconds * 1000;
	}
private:
	BosonSound* mParent;

	QString mFile;
	Arts::PlayObject mPlayObject;
	bool mPlayed;
#else
#warning KDE versions before 3.0.1 have broken sound support. Sound is disabled.
	BoPlayObject(BosonSound* , const QString& ) {}
	~BoPlayObject() {}
	
	const QString& file() const { return QString::null; }
	Arts::PlayObject object() const { return Arts::PlayObject::null(); }
	bool isNull() const { return true; }
	void play() {}
	void playFromBeginning() {}
	void reload() {}
	void rewind() {}
#endif
};

class BosonSound::BosonSoundPrivate
{
public:
	BosonSoundPrivate()
	{
		mParent = 0;

		mId = -1;
	}

	BosonAudio* mParent;

	QIntDict<BoPlayObject> mSounds;

	QMap<QString, QPtrList<BoPlayObject> > mUnitSounds;

	Arts::StereoEffectStack mEffectStack; // all effects are placed in the stack. we connect a playobject to this and so the playoubject uses all effects.
	Arts::StereoVolumeControl mVolumeControl; // changes the volume.
	Arts::Synth_AMAN_PLAY mAmanPlay; // don't ask me what this is. From natun's code. doesn't work without this.

	long mId;

};

BosonSound::BosonSound(BosonAudio* parent)
{
 d = new BosonSoundPrivate;
 d->mParent = parent;
 d->mSounds.setAutoDelete(true);

#if KDE_VERSION < 301
 setSound(false);
 return;
#endif
/*
 // taken from noatun's code:
 d->mAmanPlay = Arts::DynamicCast(server().server().createObject("Arts::Synth_AMAN_PLAY"));
 if (d->mAmanPlay.isNull()) {
	boError(200) << k_lineinfo << "Synth_AMAN_Play is NULL" << endl;
	return;
 }
 d->mAmanPlay.title("boson");
 d->mAmanPlay.autoRestoreID("boson"); // whatever this is...
 d->mAmanPlay.start();

 d->mEffectStack = Arts::DynamicCast(server().server().createObject("Arts::StereoEffectStack"));
 if (d->mEffectStack.isNull()) {
	boError(200) << k_lineinfo << "NULL effect stack" << endl;
	return;
 }
 d->mEffectStack.start();
 Arts::connect(d->mEffectStack, d->mAmanPlay);
*/
 // the volume control
/*
 d->mVolumeControl = Arts::DynamicCast(server().server().createObject("Arts::StereoVolumeControl"));
 if (d->mVolumeControl.isNull()) {
	boError(200) << k_funcinfo << "NULL volume control" << endl;
 }
 d->mVolumeControl.start();
 d->mId = d->mEffectStack.insertBottom(d->mVolumeControl, "VolumeControl");
*/
}

BosonSound::~BosonSound()
{
 boDebug(200) << k_funcinfo << endl;
 d->mSounds.clear();
 d->mUnitSounds.clear();

 if (d->mId != -1) {
	d->mEffectStack.remove(d->mId);
 }
 d->mVolumeControl = Arts::StereoVolumeControl::null();
 d->mEffectStack = Arts::StereoEffectStack::null();
 delete d;
}

KArtsServer& BosonSound::server() const
{
 return d->mParent->server();
}

void BosonSound::addEventSound(const QString& name, const QString& file)
{
#if KDE_VERSION < 301
 return;
#endif
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
	boWarning(200) << k_funcinfo << "NULL sound " << file << endl;
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
 boDebug(200) << k_funcinfo << "loading " << file << endl;
 BoPlayObject* playObject = new BoPlayObject(this, file);
 if (!playObject->isNull()) {
	d->mSounds.insert(id, playObject);
 } else {
	boWarning(200) << k_funcinfo << "NULL sound " << file << endl;
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

Arts::StereoEffectStack BosonSound::effectStack()
{
 return d->mEffectStack;
}

void BosonSound::setSound(bool s)
{
 d->mParent->setSound(s);
}

bool BosonSound::sound() const
{
 return d->mParent->sound();
}

