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
#ifndef BOSONAUDIOINTERFACE_H
#define BOSONAUDIOINTERFACE_H

class BosonSound;
class BosonMusic;
class BoAudioCommand;

#include <qstring.h>

#include "bosound/bosonabstractaudiointerface.h"

template<class T1, class T2> class QMap;
class QStringList;

#define boAudio BosonAudioInterface::bosonAudioInterface()
#define boMusic BosonAudioInterface::bosonAudioInterface()->musicInterface()

class BosonMusicInterface;
class BosonSoundInterface;

class BosonAudioInterfacePrivate;
/**
 * This class lives in the main thread (the GUI thread) and provides an
 * interface to the sound/music classes only.
 *
 * It can be called at any time without thinking about threads.
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

