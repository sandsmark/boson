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

#include <kmimetype.h>

#include <qtimer.h>
#include <qstringlist.h>
#include <qdict.h>
#include <qfile.h>

#include <AL/al.h>
#include <AL/alc.h>

bool BosonAudio::mCreated = false;


class BosonAudioPrivate
{
public:
	BosonAudioPrivate()
	{
		mBosonMusic = 0;
	}

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
	return;
 }
 context = alcCreateContext(device, 0);
 alcMakeContextCurrent(context);

 if (checkALError()) {
	boError() << k_funcinfo << "could not initialize OpenAL" << endl;
 } else {
	boDebug() << k_funcinfo << "OpenAL initialized" << endl;
 }


 alutLoadMP3_LOKI = (ALboolean (*)(ALuint, ALvoid*, ALint))alGetProcAddress((ALubyte*)"alutLoadMP3_LOKI");
 alutLoadVorbis_LOKI = (ALboolean (*)(ALuint, ALvoid*, ALint))alGetProcAddress((ALubyte*)"alutLoadVorbis_LOKI");


 // warning: NULL if sound disabled (--nosound)!
 d->mBosonMusic = new BosonMusic(this);
}

BosonAudio::~BosonAudio()
{
 boDebug(200) << k_funcinfo << endl;
 d->mBosonSound.clear();
 delete d->mBosonMusic;
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

bool BosonAudio::checkALError()
{
 ALenum e = alGetError();
 if (e != AL_NO_ERROR) {
	boError() << k_funcinfo << (int)e << endl;
	return true;
 }
 return false;
}

bool BosonAudio::loadFileToBuffer(ALuint buffer, const QString& file)
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
 KMimeType::Ptr mimeType = KMimeType::findByURL(file, 0, true, true);
 if (mimeType->name() == QString::fromLatin1("audio/x-mp3")) {
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
 } else if (mimeType->name() == QString::fromLatin1("audio/x-vorbis") ||
		mimeType->name() == QString::fromLatin1("application/ogg")) {
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
 boError(200) << k_funcinfo << "dont know how to handle mimetpye " << mimeType->name() << " of file " << file << endl;
 return false;
}

