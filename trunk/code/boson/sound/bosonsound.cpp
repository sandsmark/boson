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

#include "../defines.h"
#include "../bosonconfig.h"
#include "../bodebug.h"
#include "bosonmusic.h"

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
		boDebug() << k_funcinfo << endl;
		if (!isNull()) {
			object().play();
			mPlayed = true;
		} else {
			boDebug() << k_funcinfo << "NULL playobject" << endl;
		}
	}

	void rewind()
	{
		boDebug() << k_funcinfo << endl;
		if (isNull()) {
			return;
		}
		if (!(object().capabilities() & Arts::capSeek)) {
			boDebug() << "cannot seek" << endl;
			reload();
			return;
		}
		if (position() > 0) {
			boDebug() << "seek" << endl;
			Arts::poTime begin;
			begin.seconds = 0;
			begin.ms = 0;
			object().seek(begin);
			if (position() > 0) {
				boDebug() << "seeking did not work :-(" << endl;
				reload();
			}
		}
	}
	void reload()
	{
		boDebug() << k_funcinfo << endl;
		KMimeType::Ptr mimeType = KMimeType::findByURL(file(), 0, true, true);

		Arts::PlayObject result;
		if (mParent->server().server().isNull()) {
			boWarning() << k_funcinfo << "NULL server" << endl;
			return;
		}
		result = mParent->server().server().createPlayObjectForURL(
				std::string(file()), std::string(mimeType->name()),
				false); // false as we connect it to the soundcard ourselfes
		if (result.isNull()) {
			boError() << k_funcinfo << "NULL playobject - file="
					<< file() << " mimetype="
					<< mimeType->name() << endl;
			return;
		}

		Arts::Synth_BUS_UPLINK uplink = Arts::DynamicCast(
				mParent->server().server().createObject(
				"Arts::Synth_BUS_UPLINK"));
		if (uplink.isNull()) {
			boError() << k_funcinfo << "NULL uplink" << endl;
			return;
		}
		uplink.busname("out_soundcard");
		Arts::connect(result, "left", uplink, "left");
		Arts::connect(result, "right", uplink, "right");
		uplink.start();
		result._node()->start();
		result._addChild(uplink, "uplink");

		mPlayObject = result;
		mPlayed = false;

	}

	void playFromBeginning()
	{
		boDebug() << k_funcinfo << endl;
		if (isNull()) {
			boWarning() << k_funcinfo << "isNull" << endl;
			return;
		}
		if (object().state() != Arts::posIdle) {
			boDebug() << k_funcinfo << "not posIdle" << endl;
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
			boDebug() << "cannot seek!!" << endl;
			return -1;
		}
		Arts::poTime t(object().currentTime());
		boDebug() << k_funcinfo << t.ms + t.seconds * 1000 << endl;
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
		mId = -1;
	}

	QIntDict<BoPlayObject> mSounds;

	QMap<QString, QPtrList<BoPlayObject> > mUnitSounds;

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

#if KDE_VERSION < 301
 d->mPlaySounds = false;
 return;
#endif
/*
 // taken from noatun's code:
 d->mAmanPlay = Arts::DynamicCast(server().server().createObject("Arts::Synth_AMAN_PLAY"));
 if (d->mAmanPlay.isNull()) {
	boError() << k_lineinfo << "Synth_AMAN_Play is NULL" << endl;
	d->mPlaySounds = false; // something evil happened...
	return;
 }
 d->mAmanPlay.title("boson");
 d->mAmanPlay.autoRestoreID("boson"); // whatever this is...
 d->mAmanPlay.start();

 d->mEffectStack = Arts::DynamicCast(server().server().createObject("Arts::StereoEffectStack"));
 if (d->mEffectStack.isNull()) {
	boError() << k_lineinfo << "NULL effect stack" << endl;
	d->mPlaySounds = false; // something evil happened...
	return;
 }
 d->mEffectStack.start();
 Arts::connect(d->mEffectStack, d->mAmanPlay);
*/
 // the volume control
/*
 d->mVolumeControl = Arts::DynamicCast(server().server().createObject("Arts::StereoVolumeControl"));
 if (d->mVolumeControl.isNull()) {
	boError() << k_funcinfo << "NULL volume control" << endl;
	d->mPlaySounds = false; // something evil happened...
 }
 d->mVolumeControl.start();
 d->mId = d->mEffectStack.insertBottom(d->mVolumeControl, "VolumeControl");
*/

 d->mPlaySounds = true;
}

BosonSound::~BosonSound()
{
 boDebug() << k_funcinfo << endl;
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
 return boMusic->server();
}

void BosonSound::addUnitSounds(const QString& speciesPath, const QStringList& sounds)
{
 if (boConfig->disableSound()) {
	return;
 }
 if (speciesPath.isEmpty()) {
	return;
 }
 if (sounds.count() == 0) {
	return;
 }
 QString path = speciesPath;
 if (path.right(1) != QString::fromLatin1("/")) {
	path += QString::fromLatin1("/");
 }
 path += QString::fromLatin1("sounds/");
 QStringList::ConstIterator it = sounds.begin();
 for (; it != sounds.end(); ++it) {
	if (!(*it).isEmpty()) {
		addEvent(path, *it);
	}
 }
}

void BosonSound::addSounds(const QString& speciesPath, QMap<int, QString> sounds)
{
#if KDE_VERSION < 301
 return;
#endif
 if (boConfig->disableSound()) {
	return;
 }
 QString path = speciesPath;
 if (path.right(1) != QString::fromLatin1("/")) {
	path += QString::fromLatin1("/");
 }
 path += QString::fromLatin1("sounds/");

 // TODO: support for different versions
 // currently we support only a single version per sound here. let's do it just
 // like for unit sounds and add _n.ogg to the sound names!
 QDir directory(path);
 directory.setNameFilter(QString("*.ogg;*.wav"));
 QStringList allFiles = directory.entryList();
 if (allFiles.isEmpty()) {
	boWarning() << k_funcinfo << "no sound files found - is the data module installed?" << endl;
	return;
 }
 QMap<int, QString>::Iterator it = sounds.begin();
 for (; it != sounds.end(); ++it) {
	if (d->mSounds.find(it.key())) {
		continue;
	}
//	QStringList list = list.grep(QRegExp(QString("^%1_[0-9]{1,2}\\....").arg(*it))); // support for _n.ogg
	QStringList list = allFiles.grep(QRegExp(QString("^%1\\....").arg(*it))); // support for .ogg only
	if (list.isEmpty()) {
		continue;
	}
	QString file = path + list.first();

	BoPlayObject* playObject = new BoPlayObject(this, file);
	if (!playObject->isNull()) {
		d->mSounds.insert(it.key(), playObject);
	} else {
		boWarning() << k_funcinfo << "NULL sound " << file << endl;
		delete playObject;
	}

 }



}

void BosonSound::addEvent(const QString& dir, const QString& name)
{
 QDir directory(dir);
 directory.setNameFilter(QString("%1_*.ogg;%2_*.wav").arg(name).arg(name));
 QStringList list = directory.entryList();
 // for "oder_select" we'd also get "order_select_cmdbunker_0.wav" which is
 // wrong. so:
 list = list.grep(QRegExp(QString("^%1_[0-9]{1,2}\\....").arg(name)));
 for (unsigned int i = 0; i < list.count(); i++) {
	addEventSound(name, directory.absPath() + QString::fromLatin1("/") + list[i]);
 }
}

void BosonSound::addEventSound(const QString& name, const QString& file)
{
#if KDE_VERSION < 301
 return;
#endif
 if (boConfig->disableSound()) {
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
 boDebug() << k_funcinfo << "loading " << file << endl;
 if (!playObject->isNull()) {
	d->mUnitSounds[name].append(playObject);
	d->mUnitSounds[name].setAutoDelete(true);
 } else {
	boWarning() << k_funcinfo << "NULL sound " << file << endl;
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
 boDebug() << k_funcinfo << "id: " << id << endl;
 BoPlayObject* p = d->mSounds[id];
 if (p && !p->isNull()) {
	p->playFromBeginning();
 }
}

void BosonSound::play(const QString& name)
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
 QPtrList<BoPlayObject>& list = d->mUnitSounds[name];

 BoPlayObject* p = 0;
 if (list.count() > 0) {
	int no = kapp->random() % list.count();
	p = list.at(no);
 } else {
	return;
 }

 if (!p || p->isNull()) {
	boDebug() << k_funcinfo << "NULL sound" << endl;
	return;
 }
 p->playFromBeginning();
}

Arts::StereoEffectStack BosonSound::effectStack()
{
 return d->mEffectStack;
}

