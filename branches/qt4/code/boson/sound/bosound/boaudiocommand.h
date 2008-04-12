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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef BOAUDIOCOMMAND_H
#define BOAUDIOCOMMAND_H

#include <qstring.h>

/**
 * Represents a command for the audio thread. You will probably use @ref
 * BosonAudioInterface (and @ref BosonMusicInterface / @ref BosonSoundInterface
 * that live inside it) to create these commands.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoAudioCommand
{
public:
	/**
	 * Add an entry to listCommands() in main.cpp if you add something here!
	 **/
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
	BoAudioCommand(int command, int dataInt = -1, const QString& dataString1 = QString(), const QString& dataString2 = QString());

	/**
	 * Create a command for sounds. This takes an additional species
	 * parameter.
	 **/
	BoAudioCommand(int command, const QString& species, int dataInt = -1, const QString& dataString = QString(), const QString& dataString2 = QString());

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

#endif

