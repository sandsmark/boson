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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOSONABSTRACTAUDIOINTERFACE_H
#define BOSONABSTRACTAUDIOINTERFACE_H

class BosonSound;
class BosonMusic;
class BoAudioCommand;

template<class T1, class T2> class QMap;
class QString;
class QStringList;

/**
 * Abstract class that defines an interface for music playing.
 *
 * We will have two classes that derive this interface: one for the main thread,
 * that is called when a "playMusic()" function should get executed. This one
 * sends out a @ref BoAudioCommand object to the audio thread. The audio thread
 * forwards the class to the audio-thread-implementation of the abstract
 * interface. That class will then actually execute the command.
 *
 * The intention of this "around-three-corners" approach is to ensure that both,
 * the interface that is called by boson and the actual implementation provide
 * the same features. Also we cannot call the functions that actually play the
 * music directly, in order to provide thread-safety as easy as possible.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonAbstractMusicInterface
{
public:
	BosonAbstractMusicInterface()
	{
	}

	virtual ~BosonAbstractMusicInterface()
	{
	}

	/**
	 * Play a previously loaded file. See @ref load
	 **/
	virtual void playMusic() = 0;

	virtual void stopMusic() = 0;

	/**
	 * Start to play files in a loop. If a file from the list could not be
	 * loaded it is removed from the list.
	 *
	 * Note that the files are played in random order!
	 *
	 * Add files to the internal list using @ref addToMusicList
	 * @param files List of absolute file names
	 **/
	virtual void startMusicLoop() = 0;

	/**
	 * Add @p file to the music list. The music list can get looped using
	 * @ref startMusicLoop
	 **/
	virtual void addToMusicList(const QString& file) = 0;

	/**
	 * Clear the music list. See also @ref addToMusicList
	 **/
	virtual void clearMusicList() = 0;

	virtual void setMusic(bool) = 0;
	virtual bool music() const = 0;
};

/**
 * Abstract interface for sound playing. See also @ref
 * BosonAbstractMusicInterface for a detailed explanation.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonAbstractSoundInterface
{
public:
	BosonAbstractSoundInterface()
	{
	}

	virtual ~BosonAbstractSoundInterface()
	{
	}

	/**
	 * @param Sound name as returned by @ref UnitProperties::sound
	 **/
	virtual void playSound(const QString& name) = 0;
	virtual void playSound(int id) = 0;

	/**
	 * Add a sounds to @ref BosonSound. Note that you can (and probably will) add
	 * several files for a single event!
	 *
	 * This is a "unit event", such as move, selected, ...
	 * @param name The name of the event. If the filename of the event is
	 * order_move_*.ogg then this is "order_move".
	 * @param Absolute filename.
	 **/
	virtual void addEventSound(const QString& name, const QString& file) = 0;

	/**
	 * Add a sound to @ref BosonSound. Currently only a single version can
	 * be added per id.
	 *
	 * This is a "general" event, such as "MinimapActivated / Deactivated",
	 * i.e. not specific to certain units.
	 **/
	virtual void addEventSound(int id, const QString& file) = 0;


	virtual void setSound(bool) = 0;
	virtual bool sound() const = 0;

};

#endif

