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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONSOUND_H
#define BOSONSOUND_H

#include "../bosonabstractaudiointerface.h"

#include <qptrlist.h>
#include <qmap.h>

#include <AL/al.h>

class BoPlayObject;
class BosonAudioAL;
class QString;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonSound : public BosonAbstractSoundInterface
{
public:
	// warning: parent does NOT take ownership
	BosonSound(BosonAudioAL* parent);
	virtual ~BosonSound();

	// obsolete. remove.
	void play(const QString& name)
	{
		playSound(name);
	}
	void play(int id)
	{
		playSound(id);
	}

	/**
	 * See @ref BosonAudio::setSound
	 **/
	virtual void setSound(bool s);

	/**
	 * @return BosonAudio::sound
	 **/
	virtual bool sound() const;

	/**
	 * @param Sound name as returned by @ref UnitProperties::sound
	 **/
	virtual void playSound(const QString& name);
	virtual void playSound(int id);

	/**
	 * Note that several files for a single event (i.e. with the same name)
	 * can be added! They are different versions of the same event then.
	 * @param name First part of filename. E.g. "shoot" if "shoot_nn.ogg" is
	 * the filename, where nn is 00-number of available files.
	 * @param file The actual (absolute) filename.
	 **/
	virtual void addEventSound(const QString& name, const QString& file);

	/**
	 * A "general" event, not depending on units. See also @ref
	 * BosonAbstractSoundInterface::addEventSound.
	 **/
	virtual void addEventSound(int id, const QString& file);

	bool loadFileToBuffer(ALuint buffer, const QString& file);
	bool checkALError();

protected:
	/**
	 * @param name First part of filename. E.g. "shoot" if "shoot_nn.ogg" is
	 * the filename, where nn is 00-number of available files.
	 **/
//	void addEvent(const QString& dir, const QString& name);

private:
	typedef QPtrList<BoPlayObject> SoundList;
	typedef QMap<int, SoundList> SoundEvents;
	typedef QMap<unsigned long int, SoundEvents> UnitSounds;
	class BosonSoundPrivate;
	BosonSoundPrivate* d;
};

#endif
