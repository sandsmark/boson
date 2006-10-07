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
#ifndef BOSONAUDIOAL_H
#define BOSONAUDIOAL_H

#include "../bosonaudio.h"

#include <AL/al.h>

class QString;
class BosonMusic;
class BosonSound;
class BoAudioCommand;

class BosonAudioALPrivate;

/**
 * This is the central audio class in boson. Both @ref bosonMusic and
 * @ref bosonSound objects are managed here.
 *
 * The BosonAudio class also takes care of initializing the audio device (i.e.
 * initializing OpenAL).
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonAudioAL : public BosonAudio
{
public:
	~BosonAudioAL();

	/**
	 * Create a BosonAudioAL object. You can call this exactly once only.
	 *
	 * If this is called a 2nd time, it will return NULL (except if the
	 * previously created object has been deleted meanwhile)
	 **/
	static BosonAudioAL* create();

	/**
	 * Execute an audio command.
	 *
	 * Any audio operations should be done through this.
	 **/
	virtual void executeCommand(BoAudioCommand* command);

	/**
	 * @return TRUE if an initialization error occured.
	 **/
	virtual bool isNull() const;

	bool checkALError();

	/**
	 * @internal
	 * Used by @ref BosonMusic and @ref BosonSound
	 **/
	bool loadFileToBuffer(ALuint buffer, const QString& file);

protected:
	/**
	 * This is where the fun happens. @p command contains the command that
	 * is to be executed (e.g. play a music file).
	 *
	 * @param music The music object, if @p command is a music command
	 * (other than creating the music object). Otherwise NULL
	 * @param sound The relevant sound object if @p sound is a sound command
	 * (@ref BoAudioCommand::species is not empty) other than creating a
	 * sound object. Otherwise NULL.
	 **/
	void executeCommand(BoAudioCommand* command, BosonMusic* music, BosonSound* sound);

	/**
	 * @return The @ref BosonMusic object of this application. This can be
	 * NULL e.g. if user started with --nosound. See also @ref bosonSound
	 **/
	BosonMusic* bosonMusic() const;

	BosonSound* bosonSound(const QString& species) const;
	BosonSound* addSounds(const QString& species);

private:
	BosonAudioAL();

private:
	BosonAudioALPrivate* d;
	static bool mCreated;

private:
	// see OpenAL source, linux/src/extenstions/al_ext_mp3.c and al_ext_vorbis.c
	ALboolean (*alutLoadMP3_LOKI)(ALuint, ALvoid*, ALint);
	ALboolean (*alutLoadVorbis_LOKI)(ALuint, ALvoid*, ALint);
};

#endif

