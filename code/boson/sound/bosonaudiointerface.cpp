/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonaudiointerface.h"

#include <boaudiocommand.h>

#include "../../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "../bosonconfig.h"
#include "../boglobal.h"

#include <qstringlist.h>
#include <q3dict.h>
#include <q3deepcopy.h>
#include <qdir.h>
#include <qregexp.h>

#include <kdeversion.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// AB: the process was _required_ required when we used arts for sound, as arts
// sucks so badly (often the game froze for a moment until the sound got played)
// for OpenAL this should not be necessary and therefore is _NOT_ recommended.
// most probably we will remove this option in the future.
#define USE_PROCESS 0

#if USE_PROCESS
#include "boaudioprocesscontroller.h"
#else
#include <bosonaudio.h>
#endif

static BoGlobalObject<BosonAudioInterface> globalAudio(BoGlobalObjectBase::BoGlobalAudio);

class BosonAudioInterfacePrivate
{
public:
	BosonAudioInterfacePrivate()
	{
		mMusicInterface = 0;

#if USE_PROCESS
		mProcess = 0;
#else
		mAudio = 0;
#endif
	}

	BosonMusicInterface* mMusicInterface;
	Q3Dict<BosonSoundInterface> mBosonSoundInterfaces;

#if USE_PROCESS
	BoAudioProcessController* mProcess;
#else
	BosonAudio* mAudio;
#endif

	bool mPlayMusic;
	bool mPlaySound;
};

BosonAudioInterface::BosonAudioInterface()
{
 boDebug(200) << k_funcinfo << endl;
 d = new BosonAudioInterfacePrivate;
 d->mPlayMusic = true;
 d->mPlaySound = true;
 d->mBosonSoundInterfaces.setAutoDelete(true);

 d->mMusicInterface = new BosonMusicInterface(this);

 if (boConfig->boolValue("ForceDisableSound")) {
	boWarning(200) << k_funcinfo << "sound disabled permanently!" << endl;
	d->mPlayMusic = false;
	d->mPlaySound = false;
	return;
 }

#if USE_PROCESS
 d->mProcess = new BoAudioProcessController();
 bool ok = d->mProcess->start();
 if (ok) {
	// this shouldn't change anything, but we do it anyway
	ok = d->mProcess->isRunning();
	if (!ok) {
		boError(200) << k_funcinfo << "start() returned true, but process isn't running!" << endl;
	}
 }
 if (!ok) {
	boWarning(200) << k_funcinfo << "Unable to start audio process" << endl;
	d->mPlayMusic = false;
	d->mPlaySound = false;
	boConfig->setBoolValue("ForceDisableSound", true);
	delete d->mProcess;
	d->mProcess = 0;
	return;
 }

 boDebug(200) << k_funcinfo << "audio process started." << endl;
#else
 boDebug(200) << k_funcinfo << "construct BosonAudio object" << endl;
 d->mAudio = BosonAudio::create();
 boDebug(200) << k_funcinfo << "BosonAudio object constructed" << endl;
#endif

 sendCommand(new BoAudioCommand(BoAudioCommand::CreateMusicObject));
}

BosonAudioInterface::~BosonAudioInterface()
{
 d->mBosonSoundInterfaces.clear();
 delete d->mMusicInterface;

#if USE_PROCESS
 delete d->mProcess;
#else
 delete d->mAudio;
#endif

 delete d;
}

BosonAudioInterface* BosonAudioInterface::bosonAudioInterface()
{
 return BoGlobal::boGlobal()->bosonAudio();
}

void BosonAudioInterface::sendCommand(BoAudioCommand* command)
{
#if USE_PROCESS
 if (d->mProcess) {
	d->mProcess->sendCommand(command);
 } else {
	delete command;
 }
#else
 if (d->mAudio) {
	d->mAudio->executeCommand(command);
 } else {
	delete command;
 }
#endif
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
 if (boConfig->boolValue("ForceDisableSound")) {
	m = false;
 }

 if (m == music()) {
	return;
 }
 if (!d->mMusicInterface) {
	return;
 }
 sendCommand(new BoAudioCommand(BoAudioCommand::EnableMusic, (int)m));
 d->mPlayMusic = m;
 if (music()) {
	d->mMusicInterface->playMusic();
 } else {
	d->mMusicInterface->stopMusic();
 }
}

void BosonAudioInterface::setSound(bool s)
{
 if (boConfig->boolValue("ForceDisableSound")) {
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
 Q_UNUSED(species);
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
		"boson/music/*.mp3", KStandardDirs::Recursive);
 list += KGlobal::dirs()->findAllResources("data",
		"boson/music/*.ogg", KStandardDirs::Recursive);
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

void BosonSoundInterface::playSound(const QString& file)
{
 if (!sound()) {
	return;
 }
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::PlaySound, mSpecies, -1, file);
 audioInterface()->sendCommand(c);
}

void BosonSoundInterface::playSound(int id)
{
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::PlaySound, mSpecies, id);
 audioInterface()->sendCommand(c);
}

void BosonSoundInterface::addUnitSounds(const QString& speciesPath, const QStringList& sounds)
{
 if (boConfig->boolValue("ForceDisableSound")) {
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
 path += QString::fromLatin1("sounds");

 QDir directory(path);
 directory.setNameFilter(QString("*.ogg;*.wav"));
 QStringList files = directory.entryList();

 QStringList::const_iterator it;
 for (it = sounds.begin(); it != sounds.end(); ++it) {
	if ((*it).isEmpty()) {
		continue;
	}
	QString name = *it;
	QStringList list = files.grep(QRegExp(QString("^%1_[0-9]{1,2}\\....").arg(name)));
	for (QStringList::const_iterator it2 = list.begin(); it2 != list.end(); ++it2) {
		QString file = directory.absPath() + QString::fromLatin1("/") + *it2;
		addEventSound(name, file);
	}
 }
}

void BosonSoundInterface::addEventSound(const QString& name, const QString& file)
{
 if (boConfig->boolValue("ForceDisableSound")) {
	return;
 }
 if (file.isEmpty()) {
	boWarning(200) << k_funcinfo << "cannot add empty filename for " << name << endl;
	return;
 }
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::AddUnitSound, mSpecies, -1, name, file);
 audioInterface()->sendCommand(c);
}

void BosonSoundInterface::addEventSound(int id, const QString& file)
{
 if (boConfig->boolValue("ForceDisableSound")) {
	return;
 }
 if (file.isEmpty()) {
	boWarning(200) << k_funcinfo << "cannot add empty filename for " << id << endl;
	return;
 }
 BoAudioCommand* c = new BoAudioCommand(BoAudioCommand::AddGeneralSound, mSpecies, id, QString(), file);
 audioInterface()->sendCommand(c);
}

void BosonSoundInterface::addSounds(const QString& speciesPath, QMap<int, QString> sounds)
{
 if (boConfig->boolValue("ForceDisableSound")) {
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

