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
#ifndef BOSONMESSAGE_H
#define BOSONMESSAGE_H

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMessage
{
public:
	enum BosonMessages {
		InitMap = 0, // for new game dialog
		ChangeSpecies = 1, // for new game dialog
		ChangePlayField = 2, // for new game dialog
		ChangeTeamColor = 4, // for new game dialog
		IdNewGame = 5, // for new game (no, not dialog anymore ;)) widget
		IdInitFogOfWar = 10,
		IdStartScenario = 11,
		AddUnit = 50, // whenever a unit is added
		AddUnitsXML = 49, // whenever a unit is added (for BosonScenario)
		Advance10 = 51, // call BosonCanvas::advance() 10 times
		IdChat = 52, // a chat message
		IdGameIsStarted = 55, // the game is started as soon as this is received

	// Player Moves:
		MoveMove = 100, // Unit(s) is/are moved
		MoveAttack = 101, // a unit is being attacked
		MoveBuild = 102, // build a unit - better name: MovePlace. This is used when the unit was produces and is now placed on the ground
		MoveProduce = 103, // start to produce a unit. Sent when a cmd widget is clicked
		MoveProduceStop = 104, // stop a production. Either pause it or abort it completely
		MoveMine = 105,
		MoveRepair = 106,
		MoveRefine = 107,


		// the last message ID:
		UnitPropertyHandler = 1000 // + unit id
	};
};

#endif
