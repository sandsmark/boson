/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __BOSHOT_H__
#define __BOSHOT_H__

#include "rtti.h"

#include <qcanvas.h>

class Unit;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BoShot : public QCanvasSprite
{
public:
	/**
	 * Construct a BoShot in the middle of target. 
	 * @param target The unit on which the shot is being displayed
	 * @param attacker The unit that is attacking the target. Used for the
	 * shot animation - different species may have different shot
	 * animations.
	 * @param canvas Guess what?
	 **/
	BoShot(Unit* target, Unit* attacker, QCanvas* canvas, bool destroyed = false);
	virtual ~BoShot();

	virtual int rtti() const { return RTTI::BoShot; }

	virtual void advance(int phase);

protected:
	void deleteMe();

private:
	class BoShotPrivate;
	BoShotPrivate* d;
};

#endif
