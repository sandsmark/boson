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

#include "bosonaudiointerface.h"
#include "boaudiothread.h"

#include "bodebug.h"
#include "../bosonconfig.h"
#include "../boglobal.h"
#include "../bosonprofiling.h"

#include <qstringlist.h>
#include <qdict.h>
#include <qdeepcopy.h>
#include <qdir.h>
#include <qregexp.h>

#include <kdeversion.h>
#include <kglobal.h>
#include <kstandarddirs.h>

static BoGlobalObject<BosonAudioInterface> globalAudio(BoGlobalObjectBase::BoGlobalAudio);

BoAudioCommand::BoAudioCommand(int command, int dataInt, const QString& dataString1, const QString& dataString2)
{
 mCommand = command;
 mDataInt = dataInt;
 mDataString1 = QDeepCopy<QString>(dataString1);
 mDataString2 = QDeepCopy<QString>(dataString2);
}

BoAudioCommand::BoAudioCommand(int command, const QString& species, int dataInt, const QString& dataString1, const QString& dataString2)
{
 mCommand = command;
 mDataInt = dataInt;
 mDataString1 = QDeepCopy<QString>(dataString1);
 mDataString2 = QDeepCopy<QString>(dataString2);
 mSpecies = QDeepCopy<QString>(species);
}

class BosonAudioInterfacePrivate
{
public:
	BosonAudioInterfacePrivate()
	{
		mMusicInterface = 0;

		mAudioThread = 0;
	}

	BosonMusicInterface* mMusicInterface;
	QDict<BosonSoundInterface> mBosonSoundInterfaces;

	BoAudioThread* mAudioThread;

	bool mPlayMusic;
	bool mPlaySound;
};

BosonAudioInterface::BosonAudioInterface()
{
 d = new BosonAudioInterfacePrivate;
 d->mPlayMusic = true;
 d->mPlaySound = true;
 d->mBosonSoundInterfaces.setAutoDelete(true);

 d->mMusicInterface = new BosonMusicInterface(this);

 if (boConfig->disableSound()) {
	d->mPlayMusic = false;
	d->mPlaySound = false;
	return;
 }
#if KDE_VERSION < 301
 boWarning(200) << k_funcinfo << "Won't play audio on KDE < 3.0.1" << endl;
 d->mPlayMusic = false;
 d->mPlaySound = false;
 boConfig->setDisableSound(true);
 return;
#endif

 d->mAudioThread = new BoAudioThread;

 if (!d->mAudioThread->audioStarted()) {
	boWarning(200) << k_funcinfo << "Unable to start audio thread" << endl;
	d->mPlayMusic = false;
	d->mPlaySound = false;
	boConfig->setDisableSound(true);
	delete d->mAudioThread;
	d->mAudioThread = 0;
	return;
 }

 boDebug(200) << k_funcinfo << "starting audio thread" << endl;
 d->mAudioThread->start();

 sendCommand(new BoAudioCommand(BoAudioCommand::CreateMusicObject));
}

BosonAudioInterface::~BosonAudioInterface()
{
 d->mBosonSoundInterfaces.clear();
 delete d->mMusicInterface;

#warning TODO
// d->mAudioThread->stop();
 delete d->mAudioThread;

 delete d;
}

BoAudioThread* BosonAudioInterface::audioThread() const
{
 return d->mAudioThread;
}

BosonAudioInterface* BosonAudioInterface::bosonAudioInterface()
{
 return BoGlobal::boGlobal()->bosonAudio();
}

void BosonAudioInterface::sendCommand(BoAudioCommand* command)
{
 if (boProfiling) {
	boProfiling->start(1112);
 }
 if (audioThread()) {
	audioThread()->enqueueCommand(command);
 }
 if(boProfiling) {
	boProfiling->stop(1112);
 }
}

bool BosonAudioInterface::music() const
{
 return d->mPlayMusic;
}

bool BosonAudioInterface::sound() const
{
 return d->mPlaySound;
}

void BosonAudioInterface::setMusic(bool m)
{
 if (boConfig->disableSound()) {
	m = false;
 }
 d->mPlayMusic = m;
 sendCommand(new BoAudioCommand(BoAudioCommand::EnableMusic, (int)m));

 if (m == music()) {
	return;
 }
 if (!d->mMusicInterface) {
	return;
 }
 if (music()) {
	d->mMusicInterface->playMusic();
 } else {
	d->mMusicInterface->stopMusic();
 }
}

void BosonAudioInterface::setSound(bool s)
{
 if (boConfig->disableSound()) {
	s = false;
 }
 d->mPlaySound = s;
 sendCommand(new BoAudioCommand(BoAudioCommand::EnableSound, (int)s));
}

BosonMusicInterface* BosonAudioInterface::musicInterface() const
{
 return d->mMusicInterface;
}

BosonSoundInterface* BosonAudioInterface::addSounds(const QString& species)
{
 BosonSoundInterface* interface = 0;
 if (!d->mBosonSoundInterfaces.find(species)) {
	interface = new BosonSoundInterface(species, this);
	d->mBosonSoundInterfaces.insert(species, interface);
	sendCommand(new BoAudioCommand(BoAudioCommand::CreateSoundObject, species));
 }
 interface = d->mBosonSoundInterfaces[species];
 return interface;
}

BosonSoundInterface* BosonAudioInterface::soundInterface(const QString& species) const
{
 return 0;
}

BosonMusicInterface::BosonMusicInterface(BosonAudioInterface* parent) : BosonAbstractMusicInterface()
{
 mParent = parent;
}

void BosonMusicInterface::setMusic(bool m)
{
 audioInterface()->setMusic(m);
}

bool BosonMusicInterface::music() const
{
 return audioInterface()->music();
}

void BosonMusicInterface::playMusic()
{
 if (!music()) {
	return;
 }
 boDebug(200) << k_funcinfo << "sending command" << endl;
 audioInterface()->sendCommand(new BoAudioCommand(BoAudioCommand::PlayMusic));
}

void BosonMusicInterface::stopMusic()
{
 boDebug(200) << k_funcinfo << "sending command" << endl;
 audioInterface()->sendCommand(new BoAudioCommand(BoAudioCommand::StopMusic));
}

bool BosonMusicInterface::isLoop() const
{
#if 0
 if (!mMusic) {
	return false;
 }
 AudioThreadMutexLocker lockMutex(audioInterface()->audioThread());
 return mMusic->isLoop();
#endif
 return false;
}

void BosonMusicInterface::startLoop(const QStringList& list)
{
 clearMusicList();
 if (list.count() == 0) {
	return;
 }
 QStringList::ConstIterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	addToMusicList(*it);
 }
 startMusicLoop();
}

void BosonMusicInterface::startLoop()
{
 startLoop(availableMusic());
}

void BosonMusicInterface::startMusicLoop()
{
 audioInterface()->sendCommand(new BoAudioCommand(BoAudioCommand::StartMusicLoop));
}

void BosonMusicInterface::clearMusicList()
{
 audioInterface()->sendCommand(new BoAudioCommand(BoAudioCommand::ClearMusicList));
}

void BosonMusicInterface::addToMusicList(const QString& file)
{
 if (file.isEmpty()) {
	return;
 }
 if (!KStandardDirs::exists(file)) {
	return;
 }
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::AddToMusicList, -1, file);
 audioInterface()->sendCommand(c);
}

QStringList BosonMusicInterface::availableMusic() const
{
 QStringList list = KGlobal::dirs()->findAllResources("data",
		"boson/music/*.mp3", true);
 list += KGlobal::dirs()->findAllResources("data",
		"boson/music/*.ogg", true);
 if (list.isEmpty()) {
	boDebug(200) << "no music found" << endl;
	return list;
 }
 return list;
}


BosonSoundInterface::BosonSoundInterface(const QString& species, BosonAudioInterface* parent)
		: BosonAbstractSoundInterface()
{
 mParent = parent;
 mSpecies = species;
}

void BosonSoundInterface::setSound(bool s)
{
 audioInterface()->setSound(s);
}

bool BosonSoundInterface::sound() const
{
 return audioInterface()->sound();
}

QStringList BosonSoundInterface::nameToFiles(const QString& dir, const QString& name) const
{
 QDir directory(dir);
 directory.setNameFilter(QString("%1_*.ogg;%2_*.wav").arg(name).arg(name));
 QStringList list = directory.entryList();
 // for "oder_select" we'd also get "order_select_cmdbunker_0.wav" which is
 // wrong. so:
 list = list.grep(QRegExp(QString("^%1_[0-9]{1,2}\\....").arg(name)));
 QStringList ret;
 for (unsigned int i = 0; i < list.count(); i++) {
	QString file = directory.absPath() + QString::fromLatin1("/") + list[i];
	ret.append(file);
 }
 return ret;
}

void BosonSoundInterface::playSound(const QString& file)
{
 if (!sound()) {
	return;
 }
 boProfiling->start(1111);
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::PlaySound, mSpecies, -1, file);
 audioInterface()->sendCommand(c);
 boProfiling->stop(1111);
}

void BosonSoundInterface::playSound(int id)
{
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::PlaySound, mSpecies, id);
 audioInterface()->sendCommand(c);
}

void BosonSoundInterface::addUnitSounds(const QString& speciesPath, const QStringList& sounds)
{
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
 path += QString::fromLatin1("sounds");
 QStringList::ConstIterator it;
 for (it = sounds.begin(); it != sounds.end(); ++it) {
	if ((*it).isEmpty()) {
		continue;
	}
	QString name = *it;
	QStringList list = nameToFiles(path, name);
	QStringList::Iterator it2;
	for (it2 = list.begin(); it2 != list.end(); ++it2) {
		addEventSound(name, *it2);
	}
 }
}

void BosonSoundInterface::addEventSound(const QString& name, const QString& file)
{
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::AddUnitSound, mSpecies, -1, name, file);
 audioInterface()->sendCommand(c);
}

void BosonSoundInterface::addEventSound(int id, const QString& file)
{
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::AddGeneralSound, mSpecies, id, QString::null, file);
 audioInterface()->sendCommand(c);
}

void BosonSoundInterface::addSounds(const QString& speciesPath, QMap<int, QString> sounds)
{
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
	boWarning(200) << k_funcinfo << "no sound files found - is the data module installed?" << endl;
	return;
 }
 QMap<int, QString>::Iterator it = sounds.begin();
 for (; it != sounds.end(); ++it) {
//	QStringList list = list.grep(QRegExp(QString("^%1_[0-9]{1,2}\\....").arg(*it))); // support for _n.ogg
	QStringList list = allFiles.grep(QRegExp(QString("^%1\\....").arg(*it))); // support for .ogg only
	if (list.isEmpty()) {
		continue;
	}
	QString file = path + list.first();
	addEventSound(it.key(), file);
 }
}

