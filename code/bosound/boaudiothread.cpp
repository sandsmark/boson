/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boaudiothread.h"
#include "boaudiothread.moc"

#include "boaudiocommand.h"
#include "bosonaudio.h"
#include "bosonmusic.h"
#include "bosonsound.h"
#include "bodebug.h"

#include <qptrqueue.h>
#include <qfile.h>


static BoAudioCommand* parseCommand(QString command);
static bool parseInt(QString& command, int* result);
static bool parseString(QString& command, QString* result);

static QString g_buffer;

class BoAudioThreadPrivate
{
public:
	BoAudioThreadPrivate()
	{
		mAudio = 0;
	}

	BosonAudio* mAudio;

	QPtrQueue<BoAudioCommand> mCommandQueue;
};

BoAudioThread::BoAudioThread()
{
 d = new BoAudioThreadPrivate;

 boDebug(200) << k_funcinfo << "construct BosonAudio" << endl;
 d->mAudio = BosonAudio::create();
 if (!d->mAudio) {
	boDebug(200) << k_funcinfo << "NULL BosonAudio constructed" << endl;
 } else {
	boDebug(200) << k_funcinfo << "BosonAudio constructed" << endl;
 }
}

BoAudioThread::~BoAudioThread()
{
 boDebug(200) << k_funcinfo << endl;
 delete d->mAudio;
 delete d;
 boDebug(200) << k_funcinfo << "done" << endl;
}

BosonAudio* BoAudioThread::audio() const
{
 return d->mAudio;
}

bool BoAudioThread::audioStarted() const
{
 bool ret = true;
 if (!audio()) {
	ret = false;
 }
 if (ret && audio()->isNull()) {
	ret = false;
 }
 return ret;
}

void BoAudioThread::processCommand()
{
	if (d->mCommandQueue.isEmpty()) {
		return;
	}
	BoAudioCommand* command = dequeueCommand();
	if (!command) {
		BO_NULL_ERROR(command);
		return;
	}
	BosonSound* sound = 0;
	BosonMusic* music = 0;
	if (command->species().isEmpty()) {
		// most probably this is a music command
		if (command->type() != BoAudioCommand::CreateMusicObject) {
			music = audio()->bosonMusic();
			if (!music) {
				// happens e.g. for --nosound
				delete command;
				command = 0;
			}
		}
	} else {
		// this must be a sound command for a species.
		if (command->type() != BoAudioCommand::CreateSoundObject) {
			sound = audio()->bosonSound(command->species());
			if (!sound) {
				// happens e.g. for --nosound
				delete command;
				command = 0;
			}
		}
	}

	if (!command) {
		// we don't want to continue with this command by any reason
		return;
	}
	executeCommand(command, audio(), music, sound);
	delete command;
}

void BoAudioThread::executeCommand(BoAudioCommand* command, BosonAudio* audio, BosonMusic* music, BosonSound* sound)
{
 if (!command) {
	return;
 }
 switch (command->type()) {
	case BoAudioCommand::CreateMusicObject:
		boDebug(200) << k_funcinfo << "music object is created on startup. nothing to do" << endl;
		break;
	case BoAudioCommand::CreateSoundObject:
		if (!command->species().isEmpty()) {
			boDebug(200) << k_funcinfo << "create sound object for " << command->species() << endl;
			audio->addSounds(command->species());
		} else {
			boError(200) << k_funcinfo << "Cannot add a BosonSound object for an empty species string" << endl;
		}
		break;
	case BoAudioCommand::EnableSound:
		if (audio) {
			audio->setSound((bool)command->dataInt());
		}
		break;
	case BoAudioCommand::EnableMusic:
		if (audio) {
			audio->setMusic((bool)command->dataInt());
		}
		break;
	case BoAudioCommand::PlayMusic:
		if (music) {
			boDebug(200) << k_funcinfo << "start music playing" << endl;
			music->playMusic();
		}
		break;
	case BoAudioCommand::StopMusic:
		if (music) {
			boDebug(200) << k_funcinfo << "stop music playing" << endl;
			music->stopMusic();
		}
		break;
	case BoAudioCommand::ClearMusicList:
		if (music) {
			boDebug(200) << k_funcinfo << "clear music list" << endl;
			music->clearMusicList();
		}
		break;
	case BoAudioCommand::AddToMusicList:
		if (music) {
			QString file = command->dataString1();
//				boDebug(200) << k_funcinfo << "adding to music list: " << file << endl;
			music->addToMusicList(file);
		}
		break;
	case BoAudioCommand::StartMusicLoop:
		if (music) {
			boDebug(200) << k_funcinfo << "looping music list" << endl;
			music->startMusicLoop();
		}
		break;
	case BoAudioCommand::PlaySound:
		if (sound) {
			QString name = command->dataString1();
			int id = command->dataInt();
			if (!name.isEmpty()) {
				sound->playSound(name);
			} else {
				sound->playSound(id);
			}
		}
		break;
	case BoAudioCommand::AddUnitSound:
		if (sound) {
			QString name = command->dataString1();
			QString file = command->dataString2();
//			boDebug(200) << k_funcinfo << "adding sound " << name << "->" << file << endl;
			sound->addEventSound(name, file);
		}
		break;
	case BoAudioCommand::AddGeneralSound:
		if (sound) {
			QString file = command->dataString2();
			int id = command->dataInt();
//			boDebug(200) << k_funcinfo << "adding sound " << id << "->" << file << endl;
			sound->addEventSound(id, file);
		}
		break;
	default:
		boError() << k_funcinfo << "invalid type: " << command->type() << endl;
		break;
 }
}

void BoAudioThread::enqueueCommand(BoAudioCommand* command)
{
 d->mCommandQueue.enqueue(command);
}

BoAudioCommand* BoAudioThread::dequeueCommand()
{
 return d->mCommandQueue.dequeue();
}

void BoAudioThread::slotReceiveStdin(int sock)
{
 if (g_buffer.length() > 2048) {
	// a command of 2 KB? no, I don't believe this!
	fprintf(stderr, "command too long\n");
	g_buffer = QString::null;
	return;
 }
 QFile readFile;
 readFile.open(IO_ReadOnly | IO_Raw, sock);
 int ch = readFile.getch();
 if (ch == -1) {
	return;
 }
 if (ch == '\n') {
	QString command = g_buffer;
	g_buffer = QString::null;
	BoAudioCommand* cmd = parseCommand(command);
	if (!cmd) {
		fprintf(stderr, "parsing error on command %s\n", command.latin1());
	} else {
		enqueueCommand(cmd);
	}
 } else {
	g_buffer.append((char)ch);
 }
 processCommand();
}

BoAudioCommand* parseCommand(QString command)
{
 // first the type
 int type = 0;
 int isSound = 0;
 QString species;
 int dataInt = 0;
 QString dataString1;
 QString dataString2;

 // AB: _all_ commands start with the type (number) - see BoAudioCommand::Command
 bool ok = parseInt(command, &type);
 if (!ok) {
	fprintf(stderr, "Could not parse type\n");
	return 0;
 }

 // the second argument is _always_ a 0 (music message) or a 1 (sound message).
 ok = parseInt(command, &isSound);
 if (!ok) {
	fprintf(stderr, "Could not parse sound/music tag\n");
	return 0;
 }
 if (isSound == 0) {
	// it is a music message.
	isSound = false;
 } else if (isSound == 1) {
	isSound = true;
	ok = parseString(command, &species);
	if (!ok) {
		fprintf(stderr, "Could not parse species\n");
		return 0;
	}
 } else {
	fprintf(stderr, "Invalid sound/music tag %d\n", isSound);
 }

 ok = parseInt(command, &dataInt);
 if (!ok) {
	fprintf(stderr, "Could not parse dataInt\n");
	return 0;
 }
 ok = parseString(command, &dataString1);
 if (!ok) {
	fprintf(stderr, "Could not parse dataString1\n");
	return 0;
 }
 ok = parseString(command, &dataString2);
 if (!ok) {
	fprintf(stderr, "Could not parse dataString2\n");
	return 0;
 }

 BoAudioCommand* cmd = 0;
 if (isSound) {
	cmd = new BoAudioCommand(type, species, dataInt, dataString1, dataString2);
 } else {
	cmd = new BoAudioCommand(type, dataInt, dataString1, dataString2);
 }
 return cmd;
}

bool parseInt(QString& command, int* result)
{
 bool ok = false;
 QString s;
 int index = command.find(' ');
 if (index >= 0) {
	s = command.left(index);
 } else {
	s = command;
 }
 if (s.isEmpty()) {
	fprintf(stderr, "Could not parse integer - command: %s\n", command.latin1());
	return 0;
 }
 *result = s.toInt(&ok);
 if (!ok) {
	fprintf(stderr, "Parsed value not an integer - command: %s\n", command.latin1());
 }
 if (index >= 0) {
	command = command.right(command.length() - index - 1);
 } else {
	command = QString::null;
 }
 return ok;
}

bool parseString(QString& command, QString* result)
{
 QString s;
 int index = command.find(' ');
 if (index >= 0) {
	s = command.left(index);
	command = command.right(command.length() - index - 1);
 } else {
	s = command;
	command = QString::null;
 }
 // note: an empty string is perfectly valid!
 *result = s;
 return true;
}

