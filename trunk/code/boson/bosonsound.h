/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONSOUND_H
#define BOSONSOUND_H

#include <qptrlist.h>
#include <qmap.h>

#include <arts/kartsserver.h>

class Unit;
class UnitProperties;
class QString;
class KArtsServer;
class BoPlayObject;
class QStringList;
class QDir;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short 
 **/
class BosonSound
{
public:
	BosonSound();
	~BosonSound();

	void play(Unit* unit, int event);
	void play(int id);

	/**
	 * Add a unit to BosonSound. All sounds of this unit are added. If the
	 * unit does not have a sound for a special action the default sounds of
	 * the theme are used.
	 **/
	void addUnitSounds(const UnitProperties* prop);

	/**
	 * Add a sound. This is not a unit specific sound but something
	 * independant. Examples may be status reports ("you are under attack")
	 * or a radar sound or something like this.
	 * @param file Absolute (!) filename
	 * @param id unique id for this sound
	 **/
	void addSound(int id, const QString& file);

	/**
	 * @return boMusic->server(); see @ref BosonMusic::server
	 **/
	KArtsServer& server() const;

	void loadDefaultEvent(int event, const QString& eventFilter);

	Arts::StereoEffectStack effectStack();

protected:
	void addEvent(unsigned long int unitType, int unitSound, QDir& dir);
	void addEventSound(unsigned long int unitType, int unitSound, const QString& file);

private:
	typedef QPtrList<BoPlayObject> SoundList;
	typedef QMap<int, SoundList> SoundEvents;
	typedef QMap<unsigned long int, SoundEvents> UnitSounds;
	class BosonSoundPrivate;
	BosonSoundPrivate* d;
};

#endif
