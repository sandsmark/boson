/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonmusic.h"
#include "unitproperties.h"
#include "unit.h"
#include "bosonconfig.h"
#include "defines.h"

#include <kglobal.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <arts/kplayobject.h>
#include <arts/kplayobjectfactory.h>
#include <arts/kartsserver.h>
#include <arts/flowsystem.h>

#include <qstringlist.h>
#include <qintdict.h>

class BoPlayObject
{
public:
	BoPlayObject(BosonSound* parent, const QString& file)
	{
		mFile = file;
		mParent = parent;
		mPlayObject = 0;
		reload();
	}
	~BoPlayObject()
	{
		killObject();
	}
	const QString& file() const { return mFile; }
	KPlayObject* object() const { return mPlayObject; }

	bool isNull() const
	{
		if (object()) {
			return object()->isNull();
		}
		return true;
	}

	void killObject()
	{
		if (mPlayObject) {
			Arts::disconnect(object()->object(), "left",
					mParent->effectStack(), "inleft");
			Arts::disconnect(object()->object(), "right",
					mParent->effectStack(), "inright");
			delete mPlayObject;
			mPlayObject = 0;
		}

	}

	void play()
	{
		kdDebug() << k_funcinfo << endl;
		if (!isNull()) {
			object()->play();
			mPlayed = true;
		}
	}

	void rewind()
	{
		kdDebug() << k_funcinfo << endl;
		if (isNull()) {
			return;
		}
		if (!(object()->capabilities() & Arts::capSeek)) {
			kdDebug() << "cannot seek" << endl;
			reload();
			return;
		}
		if (position() > 0) {
			kdDebug() << "seek" << endl;
			Arts::poTime begin;
			begin.seconds = 0;
			begin.ms = 0;
			object()->seek(begin);
			if (position() > 0) {
				kdDebug() << "seeking did not work :-(" << endl;
				reload();
			}
		}
	}
	void reload()
	{
		kdDebug() << k_funcinfo << endl;
		killObject();

		KPlayObjectFactory factory(mParent->server().server());
		mPlayObject = factory.createPlayObject(file(), false);
		if (mPlayObject && !mPlayObject->isNull()) {
			mPlayed = false;

			// code taken from noatun's engine.cpp
			// we connect the playobject to the effect stack of the
			// parent.
			// There is defined 'how' the object should be played -
			// e.g. which volume.

			object()->object()._node()->start(); // I really don't know what this means...
			Arts::connect(object()->object(), "left", 
					mParent->effectStack(), "inleft");
			Arts::connect(object()->object(), "right", 
					mParent->effectStack(), "inright");
		} else {
			delete mPlayObject;
			mPlayObject = 0;
		}
	}

	void playFromBeginning()
	{
		kdDebug() << k_funcinfo << endl;
		if (isNull()) {
			return;
		}
		if (object()->state() != Arts::posIdle) {
			kdDebug() << k_funcinfo << "not posIdle" << endl;
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
		if (!(object()->capabilities() & Arts::capSeek)) {
			kdDebug() << "cannot seek!!" << endl;
			return -1;
		}
		Arts::poTime t(object()->currentTime());
		kdDebug() << k_funcinfo << t.ms + t.seconds * 1000 << endl;
		return t.ms + t.seconds * 1000;
	}
private:
	BosonSound* mParent;

	QString mFile;
	KPlayObject* mPlayObject;
	bool mPlayed;
};

class BosonSound::BosonSoundPrivate
{
public:
	BosonSoundPrivate()
	{
		mId = -1;
	}

	QIntDict<BoPlayObject> mSounds;

	UnitSounds mUnitSounds; // whoaa... QMap< int, QMap< int, QPtrList< BoPlayObject> > > // <-- probably a design problem!! (probably ??? )
	SoundEvents mDefaultSounds;

	Arts::StereoEffectStack mEffectStack; // all effects are placed in the stack. we connect a playobject to this and so the playoubject uses all effects.
	Arts::StereoVolumeControl mVolumeControl; // changes the volume.
	Arts::Synth_AMAN_PLAY mAmanPlay; // don't ask me what this is. From natun's code. doesn't work without this.

	long mId;

	bool mPlaySounds;

};

BosonSound::BosonSound()
{
 d = new BosonSoundPrivate;
 d->mSounds.setAutoDelete(true);

 // taken from noatun's code:
 d->mAmanPlay = Arts::DynamicCast(server().server().createObject("Arts::Synth_AMAN_PLAY"));
 if (d->mAmanPlay.isNull()) {
	kdError() << k_lineinfo << "Synth_AMAN_Play is NULL" << endl;
	d->mPlaySounds = false; // something evil happened...
	return;
 }
 d->mAmanPlay.title("boson");
 d->mAmanPlay.autoRestoreID("boson"); // whatever this is...
 d->mAmanPlay.start();

 d->mEffectStack = Arts::DynamicCast(server().server().createObject("Arts::StereoEffectStack"));
 if (d->mEffectStack.isNull()) {
	kdError() << k_lineinfo << "NULL effect stack" << endl;
	d->mPlaySounds = false; // something evil happened...
	return;
 }
 d->mEffectStack.start();
 Arts::connect(d->mEffectStack, d->mAmanPlay);

 // the volume control
/*
 d->mVolumeControl = Arts::DynamicCast(server().server().createObject("Arts::StereoVolumeControl"));
 if (d->mVolumeControl.isNull()) {
	kdError() << k_funcinfo << "NULL volume control" << endl;
	d->mPlaySounds = false; // something evil happened...
 }
 d->mVolumeControl.start();
 d->mId = d->mEffectStack.insertBottom(d->mVolumeControl, "VolumeControl");
*/

 d->mPlaySounds = true;
}

BosonSound::~BosonSound()
{
 kdDebug() << k_funcinfo << endl;
 d->mSounds.clear();
 UnitSounds::Iterator unitsIt;
 unitsIt = d->mUnitSounds.begin();
 for (; unitsIt != d->mUnitSounds.end(); ++unitsIt) {
	SoundEvents::Iterator eventsIt;
	for (eventsIt = (*unitsIt).begin(); eventsIt != (*unitsIt).end(); ++eventsIt) {
		QPtrListIterator<BoPlayObject> it = (*eventsIt);
		while ((*eventsIt).count() > 0) {
			BoPlayObject* p = (*eventsIt).take(0);
			delete p;
		}
	}
 }
 SoundEvents::Iterator eventsIt;
 for (eventsIt = d->mDefaultSounds.begin(); eventsIt != d->mDefaultSounds.end(); ++eventsIt) {
	QPtrListIterator<BoPlayObject> it = (*eventsIt);
	while ((*eventsIt).count() > 0) {
		BoPlayObject* p = (*eventsIt).take(0);
		delete p;
	}
 }
 
 if (d->mId != -1) {
	d->mEffectStack.remove(d->mId);
 }
 d->mVolumeControl = Arts::StereoVolumeControl::null();
 d->mEffectStack = Arts::StereoEffectStack::null();
 delete d;
}

KArtsServer& BosonSound::server() const
{
 return boMusic->server();
}

void BosonSound::addUnitSounds(const UnitProperties* prop)
{
 if (boConfig->disableSound()) {
	return;
 }
 QStringList sounds = KGlobal::dirs()->findAllResources("data", "boson/themes/species/*/units/*/sounds/*.ogg");
 sounds += KGlobal::dirs()->findAllResources("data", "boson/themes/species/*/units/*/sounds/*.wav");

 addEvent(prop->typeId(), Unit::SoundShoot, sounds);
 addEvent(prop->typeId(), Unit::SoundOrderMove, sounds);
 addEvent(prop->typeId(), Unit::SoundOrderAttack, sounds);
 addEvent(prop->typeId(), Unit::SoundOrderSelect, sounds);
 addEvent(prop->typeId(), Unit::SoundReportProduced, sounds);
 addEvent(prop->typeId(), Unit::SoundReportDestroyed, sounds);
 addEvent(prop->typeId(), Unit::SoundReportUnderAttack, sounds);
}

void BosonSound::loadDefaultEvent(int event, const QString& filter)
{
 if (boConfig->disableSound()) {
	return;
 }
 if (d->mDefaultSounds.contains(event)) {
	return;
 }
 if (server().server().isNull()) {
	return;
 }
 QStringList defaultSounds = KGlobal::dirs()->findAllResources("data", "boson/themes/species/*/sounds/*.ogg");
 defaultSounds += KGlobal::dirs()->findAllResources("data", "boson/themes/species/*/sounds/*.wav");
 QStringList list = defaultSounds.grep(filter);
 for (unsigned int i = 0; i < list.count(); i++) {
	kdDebug() << k_funcinfo << "adding " << event << " = " << list[i] << endl;
	BoPlayObject* playObject = new BoPlayObject(this, list[i]);
	if (!playObject->isNull()) {
		d->mDefaultSounds[event].append(playObject);
	} else {
		kdWarning() << k_funcinfo << "NULL sound " << list[i] << endl;
		delete playObject;
	}
 }
 d->mDefaultSounds[event].setAutoDelete(true);
}


void BosonSound::addSound(int id, const QString& file)
{
 if (boConfig->disableSound()) {
	return;
 }
 if (d->mSounds.find(id)) {
	return;
 }
 BoPlayObject* playObject = new BoPlayObject(this, file);
 if (!playObject->isNull()) {
	d->mSounds.insert(id, playObject);
 } else {
	kdWarning() << k_funcinfo << "NULL sound " << file << endl;
	delete playObject;
 }
}

void BosonSound::addEvent(int unitType, int event, const QStringList& sounds)
{
 QString filter;
 switch (event) {
	case Unit::SoundShoot:
		filter = "shoot";
		break;
	case Unit::SoundOrderMove:
		filter = "order_move";
		break;
	case Unit::SoundOrderAttack:
		filter = "order_attack";
		break;
	case Unit::SoundOrderSelect:
		filter = "order_select";
		break;
	case Unit::SoundReportProduced:
		filter = "report_produced";
		break;
	case Unit::SoundReportDestroyed:
		filter = "report_destroyed";
		break;
	case Unit::SoundReportUnderAttack:
		filter = "report_underattack";
		break;
 }
 loadDefaultEvent(event, filter);
 QStringList list = sounds.grep(filter);
 for (unsigned int i = 0; i < list.count(); i++) {
	addEventSound(unitType, event, list[i]);
 }
 (d->mUnitSounds[unitType])[event].setAutoDelete(true);
}

void BosonSound::addEventSound(int unitType, int event, const QString& file)
{
 if (boConfig->disableSound()) {
	return;
 }
 kdDebug() << k_funcinfo << "adding: " << unitType << "->" << event << " = " << file << endl;
 KPlayObjectFactory factory(server().server());
 BoPlayObject* playObject = new BoPlayObject(this, file);
 if (!playObject->isNull()) {
	// that's really ugly code:
	(d->mUnitSounds[unitType])[event].append(playObject);
 } else {
	kdWarning() << k_funcinfo << "NULL sound " << file << endl;
	delete playObject;
 }
}

void BosonSound::play(int id)
{
 if (boConfig->disableSound()) {
	return;
 }
 if (!d->mPlaySounds) { 
	return; // something evil happened to our sound server...
 }
 if (!boConfig->sound()) {
	return;
 }
 kdDebug() << k_funcinfo << "id: " << id << endl;
 BoPlayObject* p = d->mSounds[id];
 if (p && !p->isNull()) {
	p->playFromBeginning();
 }
}

void BosonSound::play(Unit* unit, int event)
{
 if (boConfig->disableSound()) {
	return;
 }
 if (!d->mPlaySounds) { 
	return; // something evil happened to our sound server...
 }
 if (!boConfig->sound()) {
	return;
 }
 kdDebug() << k_funcinfo << "event: " << event << endl;
 // that's really ugly code:
 QPtrList<BoPlayObject>& list =  (d->mUnitSounds[unit->unitProperties()->typeId()])[event];

 BoPlayObject* p = 0;
 if (list.count() > 0) {
	int no = kapp->random() % list.count();
	p = list.at(no);
 } else if (d->mDefaultSounds.contains(event) && d->mDefaultSounds[event].count() > 0) {
	int no = kapp->random() % d->mDefaultSounds[event].count();
	p = d->mDefaultSounds[event].at(no);
 }

 if (!p || p->isNull()) {
	kdDebug() << k_funcinfo << "NULL sound" << endl;
	return;
 }
 p->playFromBeginning();
}

Arts::StereoEffectStack BosonSound::effectStack() 
{ 
 return d->mEffectStack; 
}
