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
#include "defines.h"
#include "unitproperties.h"
#include "unit.h"

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
		delete mPlayObject;
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
		if (mPlayObject) {
			delete mPlayObject;
		}
		KPlayObjectFactory factory(mParent->server().server());
		mPlayObject = factory.createPlayObject(file(), true);
		mPlayed = false;
	}

	void playFromBeginning()
	{
		kdDebug() << k_funcinfo << endl;
		if (isNull()) {
			return;
		}
		if (object()->state() != Arts::posIdle) {
			kdDebug() << k_funcinfo << "not posIdle" << endl;
			return;
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
	}

	QIntDict<BoPlayObject> mSounds;

	UnitSounds mUnitSounds; // whoaa... QMap< int, QMap< int, QPtrList< BoPlayObject> > > // <-- probably a design problem!! (probably ??? )
	SoundEvents mDefaultSounds;
};

BosonSound::BosonSound()
{
 d = new BosonSoundPrivate;
 d->mSounds.setAutoDelete(true);
}

BosonSound::~BosonSound()
{
 d->mSounds.clear();
 delete d;
}

KArtsServer& BosonSound::server() const
{
 return boMusic->server();
}

void BosonSound::addUnitSounds(const UnitProperties* prop)
{
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
 if (d->mDefaultSounds[event].count() > 0) {
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
	if (playObject->object()) {
		if (playObject->isNull()) {
			kdWarning() << k_funcinfo << "NULL sound " << list[i] << endl;
			delete playObject;
		} else {
			d->mDefaultSounds[event].append(playObject);
		}
	} else {
		kdWarning() << k_funcinfo << "NULL sound " << list[i] << endl;
	}
 }
 d->mDefaultSounds[event].setAutoDelete(true);
}


void BosonSound::addSound(int id, const QString& file)
{
 if (d->mSounds.find(id)) {
	return;
 }
 BoPlayObject* playObject = new BoPlayObject(this, file);
 if (playObject->isNull()) {
	kdWarning() << k_funcinfo << "NULL sound " << file << endl;
	delete playObject;
 } else {
	d->mSounds.insert(id, playObject);
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
 kdDebug() << k_funcinfo << "adding: " << unitType << "->" << event << " = " << file << endl;
 KPlayObjectFactory factory(server().server());
 BoPlayObject* playObject = new BoPlayObject(this, file);
 if (playObject->isNull()) {
	kdWarning() << k_funcinfo << "NULL sound " << file << endl;
 } else {
	// that's really ugly code:
	(d->mUnitSounds[unitType])[event].append(playObject);
 }
}

void BosonSound::play(int id)
{
 kdDebug() << k_funcinfo << "id: " << id << endl;
 BoPlayObject* p = d->mSounds[id];
 if (p && !p->isNull()) {
	p->playFromBeginning();
 }
}

void BosonSound::play(Unit* unit, int event)
{
 kdDebug() << k_funcinfo << "event: " << event << endl;
 // that's really ugly code:
 QPtrList<BoPlayObject>& list =  (d->mUnitSounds[unit->unitProperties()->typeId()])[event];

 BoPlayObject* p = 0;
 if (list.count() > 0) {
	int no = kapp->random() % list.count();
	p = list.at(no);
 } else if (d->mDefaultSounds[event].count() > 0) {
	int no = kapp->random() % d->mDefaultSounds[event].count();
	p = d->mDefaultSounds[event].at(no);
 }

 if (!p || p->isNull()) {
	return;
 }

 p->playFromBeginning();
}
