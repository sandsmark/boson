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
#ifndef BOSONAUDIOINTERFACE_H
#define BOSONAUDIOINTERFACE_H

class BosonSound;
class BosonMusic;
class BoAudioThread;

#include <qstring.h>

template<class T1, class T2> class QMap;
class QStringList;

#define boAudio BosonAudioInterface::bosonAudioInterface()
#define boMusic BosonAudioInterface::bosonAudioInterface()->musicInterface()

/**
 * Represents a command for the audio thread. You will probably use @ref
 * BosonAudioInterface (and @ref BosonMusicInterface / @ref BosonSoundInterface
 * that live inside it) to create these commands.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoAudioCommand
{
public:
	enum Command {
		CreateMusicObject = 0,
		CreateSoundObject = 1,
		EnableMusic = 2,
		EnableSound = 3,

		PlayMusic = 10,
		StopMusic = 11,
		ClearMusicList = 12,
		AddToMusicList = 13,
		StartMusicLoop = 14,

		PlaySound = 50,
		AddUnitSound = 51,
		AddGeneralSound = 52
	};
	BoAudioCommand(int command, int dataInt = -1, const QString& dataString1 = QString::null, const QString& dataString2 = QString::null);

	/**
	 * Create a command for sounds. This takes an additional species
	 * parameter.
	 **/
	BoAudioCommand(int command, const QString& species, int dataInt = -1, const QString& dataString = QString::null, const QString& dataString2 = QString::null);

	int type() const { return mCommand; }
	int dataInt() const { return mDataInt; }
	const QString& dataString1() const { return mDataString1; }
	const QString& dataString2() const { return mDataString2; }
	const QString& species() const { return mSpecies; }
private:
	int mCommand;
	int mDataInt;
	QString mDataString1;
	QString mDataString2;
	QString mSpecies;
};

/**
 * Abstract class that defines an interface for music playing.
 *
 * We will have two classes that derive this interface: one for the main thread,
 * that is called when a "playMusic()" function should get executed. This one
 * sends out a @ref BoAudioCommand object to the audio thread. The audio thread
 * forwards the class to the audio-thread-implementation of the abstract
 * interface. That class will then actually execture the command.
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

class BosonMusicInterface;
class BosonSoundInterface;

class BosonAudioInterfacePrivate;
/**
 * This class lives in the main thread (the GUI thread) and provides an
 * interface to the sound/music classes only.
 *
 * It can be called at any time without thinking about threads.
 *
 * Note that when you add a function here that touches the audio thread (most
 * functions do) you must call @ref BoAudioThread::lock first!
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonAudioInterface
{
public:
	BosonAudioInterface();
	virtual ~BosonAudioInterface();

	static BosonAudioInterface* bosonAudioInterface();

	void sendCommand(BoAudioCommand* command);

	bool music() const;
	bool sound() const;
	void setMusic(bool m);
	void setSound(bool s);

	BosonSoundInterface* addSounds(const QString& species);

	BosonMusicInterface* musicInterface() const;
	BosonSoundInterface* soundInterface(const QString& species) const;

protected:
	BoAudioThread* audioThread() const;

private:
	BosonAudioInterfacePrivate* d;
};

/**
 * This class sends commands (see @ref BoAudioCommand) to the audio thread
 * concerning music.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMusicInterface : public BosonAbstractMusicInterface
{
public:
	BosonMusicInterface(BosonAudioInterface* parent);

	BosonAudioInterface* audioInterface() const { return mParent; }

	virtual void setMusic(bool music);
	virtual bool music() const;

	/**
	 * @return All music files that could be found. Note that this start at
	 * the boson/music dir and is recursively. MP3 files are searched as
	 * well as OGG files.
	 **/
	QStringList availableMusic() const;

	/**
	 * Equivlaent to startLoop(availableMusic())
	 **/
	void startLoop();
	/**
	 * Add all files to the loop-list and then start the loop using @ref
	 * startMusicLoop
	 **/
	void startLoop(const QStringList& files);

	virtual void playMusic();
	virtual void stopMusic();
	virtual void startMusicLoop();
	virtual void addToMusicList(const QString&);
	virtual void clearMusicList();
	virtual bool isLoop() const;


private:
	BosonAudioInterface* mParent;
};

/**
 * This class sends commands (see @ref BoAudioCommand) to the audio thread
 * concerning sound effects.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonSoundInterface : public BosonAbstractSoundInterface
{
public:
	BosonSoundInterface(const QString& species, BosonAudioInterface* parent);
	virtual ~BosonSoundInterface()
	{
	}

	BosonAudioInterface* audioInterface() const { return mParent; }

	virtual void setSound(bool sound);
	virtual bool sound() const;

	void addUnitSounds(const QString& speciesPath, const QStringList& sounds);

	/**
	 * Add general sounds. These are not unit specific sounds but something
	 * independant. Examples may be status reports ("you are under attack")
	 * or a radar sound or something like this.
	 * @param speciesPath Path to the species directory. See @ref
	 * SpeciesTheme::themePath
	 * @param sounds A list of id<->sound name pairs.
	 **/
	void addSounds(const QString& speciesPath, QMap<int, QString> sounds);

	virtual void playSound(const QString& name);
	virtual void playSound(int id);
	virtual void addEventSound(const QString& name, const QString& file);
	virtual void addEventSound(int event, const QString& file);


protected:
	/**
	 * @return A filename for @p name in @p soundDir.
	 **/
	QStringList nameToFiles(const QString& soundDir, const QString& name) const;


private:
	BosonAudioInterface* mParent;
	QString mSpecies;
};

#endif

