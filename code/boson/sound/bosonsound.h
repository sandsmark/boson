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

	/**
	 * @param Sound name as returned by @ref UnitProperties::sound
	 **/
	void play(const QString& name);
	void play(int id);

	/**
	 * Add a sounds to BosonSound. Calls @ref addEvent for every entry in
	 * sounds.
	 * @param speciesPath Path to the species directory. See @ref
	 * SpeciesTheme::themePath
	 * @param sounds lists of sound names in speciesPath/sounds/. Note that
	 * BosonSound will add a _n.ogg, where n (or nn) is a number.
	 **/
	void addUnitSounds(const QString& speciesPath, const QStringList& sounds);

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

	Arts::StereoEffectStack effectStack();

protected:
	/**
	 * @param name First part of filename. E.g. "shoot" if "shoot_nn.ogg" is
	 * the filename, where nn is 00-number of available files.
	 **/
	void addEvent(const QString& dir, const QString& name);
	void addEventSound(const QString& name, const QString& file);

private:
	typedef QPtrList<BoPlayObject> SoundList;
	typedef QMap<int, SoundList> SoundEvents;
	typedef QMap<unsigned long int, SoundEvents> UnitSounds;
	class BosonSoundPrivate;
	BosonSoundPrivate* d;
};

#endif
