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
#ifndef __BOSONMESSAGE_H__
#define __BOSONMESSAGE_H__

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMessage
{
public:
	enum BosonMessages {
		InitMap = 0, // for new game dialog
		ChangeSpecies = 1, // for new game dialog
		ChangeMap = 2, // for new game dialog
		ChangeScenario = 3, // for new game dialog
		IdInitFogOfWar = 10,
		IdStartScenario = 11,
		AddUnit = 50, // whenever a unit is added
		Advance = 51, // call BosonCanvas::advance()
		IdChat = 52, // a chat message
		IdGameIsStarted = 55, // the game is started as soon as this is received

	// Player Moves:
		MoveMove = 100, // Unit(s) is/are moved
		MoveAttack = 101, // a unit is being attacked
		MoveBuild = 102, // build a unit - better name: MovePlace. This is used when the unit was produces and is now placed on the ground
		MoveProduce = 103, // start to produce a unit. Sent when a cmd widget is clicked


		// the last message ID:
		UnitPropertyHandler = 1000 // + unit id
	};
};

#endif
