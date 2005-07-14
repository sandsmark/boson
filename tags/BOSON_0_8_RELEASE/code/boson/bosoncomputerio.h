/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonComputerIO : public KGameComputerIO
{
	Q_OBJECT

public:
	BosonComputerIO();
	BosonComputerIO(KPlayer*);
	~BosonComputerIO();

	virtual int rtti() const { return KGameIO::ComputerIO; }

protected:
	virtual void reaction();
	Player* boPlayer() const { return (Player*)player(); }
	Unit* findTarget();

private:
	int mUnit;
	Unit* mTarget;
};

#endif