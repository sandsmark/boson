/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann <b_mann@gmx.de>

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

#include "bosonaudioal.h"

#include "bodebug.h"
#include "bosonmusic.h"
#include "bosonsound.h"
#include "../boaudiocommand.h"

#include <qdict.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <AL/al.h>
#include <AL/alc.h>

bool BosonAudioAL::mCreated = false;


class BosonAudioALPrivate
{
public:
	BosonAudioALPrivate()
	{
		mBosonMusic = 0;
	}

	BosonMusic* mBosonMusic;
	QDict<BosonSound> mBosonSound;

	bool mIsNull;
};

BosonAudioAL* BosonAudioAL::create()
{
 if (!mCreated) {
	mCreated = true;
	return new BosonAudioAL;
 }
 return 0;
}

BosonAudioAL::BosonAudioAL() : BosonAudio()
{
 d = new BosonAudioALPrivate;
 d->mIsNull = false;
 alutLoadMP3_LOKI = 0;
 alutLoadVorbis_LOKI = 0;
 d->mBosonSound.setAutoDelete(true);

 if (d->mIsNull) {
	return;
 }

 boDebug() << k_funcinfo << "initializing OpenAL" << endl;
 ALCcontext* context;
 ALCdevice* device;
 device = alcOpenDevice(0);
 if (device == 0) {
	boError() << k_funcinfo << "could not open device using alc" << endl;
	d->mIsNull = true;
	return;
 }
 context = alcCreateContext(device, 0);
 alcMakeContextCurrent(context);

 if (checkALError()) {
	boError() << k_funcinfo << "could not initialize OpenAL" << endl;
	d->mIsNull = true;
 } else {
	boDebug() << k_funcinfo << "OpenAL initialized" << endl;
 }
 if (isNull()) {
	return;
 }


#ifndef AL_BYTE_OFFSET // OpenAL 1.0
 alutLoadMP3_LOKI = (ALboolean (*)(ALuint, ALvoid*, ALint))alGetProcAddress((ALubyte*)"alutLoadMP3_LOKI");
 alutLoadVorbis_LOKI = (ALboolean (*)(ALuint, ALvoid*, ALint))alGetProcAddress((ALubyte*)"alutLoadVorbis_LOKI");
#else // OpenAL 1.1
 alutLoadMP3_LOKI = (ALboolean (*)(ALuint, ALvoid*, ALint))alGetProcAddress((ALchar*)"alutLoadMP3_LOKI");
 alutLoadVorbis_LOKI = (ALboolean (*)(ALuint, ALvoid*, ALint))alGetProcAddress((ALchar*)"alutLoadVorbis_LOKI");
#endif


 // warning: NULL if sound disabled (--nosound)!
 d->mBosonMusic = new BosonMusic(this);
}

BosonAudioAL::~BosonAudioAL()
{
 boDebug(200) << k_funcinfo << endl;
 d->mBosonSound.clear();
 delete d->mBosonMusic;
 delete d;
 mCreated = false;
}

bool BosonAudioAL::isNull() const
{
 return d->mIsNull;
}

BosonMusic* BosonAudioAL::bosonMusic() const
{
 return d->mBosonMusic;
}


BosonSound* BosonAudioAL::addSounds(const QString& species)
{
 if (!d->mBosonSound.find(species)) {
	d->mBosonSound.insert(species, new BosonSound(this));
 }
 return d->mBosonSound[species];
}

BosonSound* BosonAudioAL::bosonSound(const QString& species) const
{
 return d->mBosonSound[species];
}

bool BosonAudioAL::checkALError()
{
 ALenum e = alGetError();
 if (e != AL_NO_ERROR) {
	boError() << k_funcinfo << (int)e << endl;
	switch (e) {
		case AL_INVALID_NAME:
			boError() << k_funcinfo << "AL_INVALID_NAME" << endl;
			break;
		case AL_ILLEGAL_ENUM:
			boError() << k_funcinfo << "AL_ILLEGAL_ENUM" << endl;
			break;
		case AL_INVALID_VALUE:
			boError() << k_funcinfo << "AL_INVALID_VALUE" << endl;
			break;
		case AL_ILLEGAL_COMMAND:
			boError() << k_funcinfo << "AL_ILLEGAL_COMMAND" << endl;
			break;
		case AL_OUT_OF_MEMORY:
			boError() << k_funcinfo << "AL_OUT_OF_MEMORY" << endl;
			break;
		default:
			boError() << k_funcinfo << "error not recognized" << endl;
			break;
	}
	return true;
 }
 return false;
}

bool BosonAudioAL::loadFileToBuffer(ALuint buffer, const QString& file)
{
 if (alIsBuffer(buffer) != AL_TRUE) {
	boError(200) << k_funcinfo << "not a valid buffer: " << buffer << endl;
	return false;
 }
 QFile f(file);
 if (!f.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "unable to open file " << file << endl;
	return false;
 }
 QFileInfo info(f);
 if (info.extension(false) == QString::fromLatin1("mp3")) {
	boDebug(200) << k_funcinfo << "loading mp3 file " << file << endl;
	if (alutLoadMP3_LOKI) {
		char* data = new char[f.size()];
		int l = f.readBlock(data, f.size());

		// AB: OpenAL requires MP3 files to be encoded with a frequency
		// of 44100 hz. we check for this first and discard any files
		// that dont fit.
		// AB: the first 4 bytes of a file are a header.
		// bits (11,10) contain the frequency, which depends on the MPEG
		// version. 00 means 44100 hz for MPEG1 (MPEG2 and MPEG2.5 don't
		// have a 44100 frequency).
		// the MPEG version is in bits (20,19), 11 means MPEG1.
		bool valid = true;
		char b = data[1]; // bits 16 to 23
		b = b & 0x18; // look at bits 20 and 19 only
		if (b != 0x18) {
			// at least one of the bits is not set
			valid = false;
		} else {
			b = data[2];
			b = b & 0x0C; // look at bits 11 and 10 only
			if (b != 0) {
				// at least one bit is != 0
				valid = false;
			}
		}
		if (!valid) {
			boError(200) << k_funcinfo << "OpenAL requires MP3 files to be encoded with a frequency of 44100. Cannot load " << file << endl;
			delete[] data;
			return false;
		}
		if (l != (int)f.size()) {
			delete[] data;
			return false;
		}
		if ((*alutLoadMP3_LOKI)(buffer, data, f.size()) != AL_TRUE) {
			boError(200) << k_funcinfo << "alutLoadMP3_LOKI failed for file " << file << endl;
			delete[] data;
			return false;
		}
		delete[] data;
		return true;
	} else {
		return false;
	}
 } else if (info.extension(false) == QString::fromLatin1("ogg")) {
	boDebug(200) << k_funcinfo << "loading ogg vorbis file " << file << endl;
	if (alutLoadVorbis_LOKI) {
		char* data = new char[f.size()];
		int l = f.readBlock(data, f.size());
		if (l != (int)f.size()) {
			boError() << k_funcinfo << "did not read everything. read: " << l << " have: " << f.size() << endl;
			delete[] data;
			return false;
		}
		if ((*alutLoadVorbis_LOKI)(buffer, data, f.size()) != AL_TRUE) {
			boError(200) << k_funcinfo << "alutLoadVorbis_LOKI failed for file " << file << endl;
			delete[] data;
			return false;
		}
		delete[] data;
		return true;
	} else {
		return false;
	}
 }
 boError(200) << k_funcinfo << "dont know how to handle extension " << info.extension() << " of file " << file << endl;
 return false;
}

void BosonAudioAL::executeCommand(BoAudioCommand* command)
{
 BO_CHECK_NULL_RET(command);

 if (isNull()) {
	delete command;
	return;
 }

 BosonSound* sound = 0;
 BosonMusic* music = 0;
 if (command->species().isEmpty()) {
	// most probably this is a music command
	if (command->type() != BoAudioCommand::CreateMusicObject) {
		music = bosonMusic();
		if (!music) {
			// happens e.g. for --nosound
			delete command;
			command = 0;
		}
	}
 } else {
	// this must be a sound command for a species.
	if (command->type() != BoAudioCommand::CreateSoundObject) {
		sound = bosonSound(command->species());
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
 executeCommand(command, music, sound);
 delete command;
}

void BosonAudioAL::executeCommand(BoAudioCommand* command, BosonMusic* music, BosonSound* sound)
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
			addSounds(command->species());
		} else {
			boError(200) << k_funcinfo << "Cannot add a BosonSound object for an empty species string" << endl;
		}
		break;
	case BoAudioCommand::EnableSound:
		boDebug(200) << k_funcinfo << "enable sound: " << command->dataInt() << endl;
		setSound((bool)command->dataInt());
		break;
	case BoAudioCommand::EnableMusic:
		boDebug(200) << k_funcinfo << "enable music: " << command->dataInt() << endl;
		setMusic((bool)command->dataInt());
		if (music) {
			if (this->music()) {
				music->playMusic();
			} else {
				music->stopMusic();
			}
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

