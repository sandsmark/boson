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
#ifndef BOAUDIOTHREAD_H
#define BOAUDIOTHREAD_H

#include <qthread.h>

class BosonAudio;
class BosonSound;
class BosonMusic;
class BoAudioCommand;

class BoAudioThreadPrivate;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoAudioThread : public QThread
{
public:
	BoAudioThread();
	~BoAudioThread();

	bool audioStarted() const;

	/**
	 * Lock the thread. This calls @ref QMutex::lock on the @ref QMutex
	 * object maintained inside this class.
	 **/
	void lock();

	/**
	 * Unlock the @ref QMutex object that was locked by @ref lock.
	 **/
	void unlock();

	void enqueueCommand(BoAudioCommand* command);

	// AB: if you add public functions you MUST make them thread safe!
	// if they are accessed from the audio thread only then make them
	// protected and use friends.

protected:
	virtual void run();

	/**
	 * @param music The music object, if @p command is a music command
	 * (other than creating the music object). Otherwise NULL
	 * @param sound The relevant sound object if @p sound is a sound command
	 * (@ref BoAudioCommand::species is not empty) other than creating a
	 * sound object. Otherwise NULL.
	 **/
	void executeCommand(BoAudioCommand* command, BosonMusic* music, BosonSound* sound);

	BosonAudio* audio() const;
	BoAudioCommand* dequeueCommand();


private:
	BoAudioThreadPrivate* d;
};

#endif
