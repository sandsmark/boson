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
#ifndef BOSONAUDIO_H
#define BOSONAUDIO_H

#include <qobject.h>

class QStringList;
class KArtsServer;
class BosonMusic;
class BosonSound;

class BosonAudioPrivate;

/**
 * This class lives inside the audio thread only. Nothing outside of the audio
 * thread can access it.
 *
 * Here the sound server is managed (@ref server), as well as the @ref
 * bosonMusic object and all @ref bosonSound objects.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonAudio
{
public:
	~BosonAudio();

	/**
	 * Create a BosonAudio object. You can call this exactly once only -
	 * this is the instance for the audio thread. You must not call this
	 * outside of the audio thread.
	 *
	 * If this is called a 2nd time, it will return NULL (except if the
	 * previously created object has been deleted meanwhile)
	 **/
	static BosonAudio* create();

	/**
	 * @return The @ref BosonMusic object of this application. This can be
	 * NULL e.g. if user started with --nosound. See also @ref bosonSound
	 **/
	BosonMusic* bosonMusic() const;

	BosonSound* bosonSound(const QString& species) const;
	BosonSound* addSounds(const QString& species);


	KArtsServer& server() const;

	/**
	 * @return TRUE if an initialization error occured. E.g. if artsd can't
	 * be reached or if it lacks support for .ogg files.
	 **/
	bool isNull() const;

	bool music() const;
	bool sound() const;
	void setMusic(bool m);
	void setSound(bool s);

private:
	BosonAudio();

private:
	BosonAudioPrivate* d;
	static bool mCreated;
};

#endif

