/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONAUDIO_H
#define BOSONAUDIO_H

class QStringList;
class QString;
class BoAudioCommand;

class BosonAudioPrivate;

/**
 * This is the central audio class in boson. When boson is compiled without
 * OpenAL support, this is simply a dummy class that is ignoring all audio
 * commands. If OpenAL is compiled in, the @ref BosonAudioAL class is used,
 * which is derived from this class. It manages both, the music and the sound
 * objects.
 *
 * The BosonAudio class also takes care of initializing the audio device (e.g.
 * initializing OpenAL).
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonAudio
{
public:
	virtual ~BosonAudio();

	/**
	 * Create a BosonAudio object. You can call this exactly once only.
	 *
	 * If this is called a 2nd time, it will return NULL (except if the
	 * previously created object has been deleted meanwhile)
	 **/
	static BosonAudio* create();

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

	bool music() const;
	bool sound() const;
	void setMusic(bool m);
	void setSound(bool s);

protected:
	BosonAudio();

private:
	BosonAudioPrivate* d;
	static bool mCreated;
};

#endif

