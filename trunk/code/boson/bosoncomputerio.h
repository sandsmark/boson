/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONCOMPUTERIO_H
#define BOSONCOMPUTERIO_H

#include <kgame/kgameio.h>

class Player;
class Unit;
class BosonScript;
class BoComputerPlayerEventListener;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonComputerIO : public KGameComputerIO
{
	Q_OBJECT

public:
	BosonComputerIO();
	~BosonComputerIO();

	virtual int rtti() const { return KGameIO::ComputerIO; }

	/**
	 * Call this once after adding the newly constructed IO to it's player.
	 *
	 * Note that this is NOT called automatically by KGame, in contrast to @ref
	 * initIO. This is required to make use of the return value.
	 **/
	bool initializeIO();

protected:
	virtual void reaction();
	Player* boPlayer() const { return (Player*)player(); }

private:
	BosonScript* mScript;
	BoComputerPlayerEventListener* mEventListener;
};

#endif
