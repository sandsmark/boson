/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
	// messages for the newgame dialog/widget:
	// (a few might be obsolete or moved to another place)
		InitMap = 0,
		ChangeSpecies = 1,
		ChangePlayField = 2,
		ChangeTeamColor = 4,
		IdNewGame = 5,
		IdNewEditor = 6,

	// once a newgame is started:
		IdInitFogOfWar = 10,
		IdStartScenario = 11,
		AddUnitsXML = 12, // add units from an xml (BosonScenario) file/stream
		IdGameIsStarted = 25, // the game is started as soon as this is received

	// usually withing a game
		AddUnit = 30, // whenever a unit is added

		// advance messages (still the "within a game" section)
		AdvanceN = 50, // call BosonCanvas::advance() N times

		IdChat = 70, // a chat message

	// Player Moves aka Player Input:
		MoveMove = 100, // Unit(s) is/are moved
		MoveAttack = 101, // a unit is being attacked
		MoveBuild = 102, // build a unit - better name: MovePlace. This is used when the unit was produces and is now placed on the ground
		MoveProduce = 103, // start to produce a unit. Sent when a cmd widget is clicked
		MoveProduceStop = 104, // stop a production. Either pause it or abort it completely
		MoveMine = 105,
		MoveRepair = 106,
		MoveRefine = 107,
		MoveStop = 108,  // Stop unit(s) from moving or attacking
		MoveFollow = 109,  // Follow another unit

	// Player Input in Editor mode
		MoveEditor = 200, // all editor moves/inputs are prefixed with this
		MovePlaceUnit = 201,
		MovePlaceCell = 202,


	// the last message ID:
		UnitPropertyHandler = 1000 // + unit id
	};
};

#endif
