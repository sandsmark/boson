/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONMESSAGEIDS_H
#define BOSONMESSAGEIDS_H

// AB: do not include any files here!

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMessageIds
{
public:
	enum BosonMessages {
	// messages for the newgame dialog/widget:
	// (a few might be obsolete or moved to another place)
//		InitMap = 0,
		ChangeSpecies = 1,
		ChangePlayField = 2,
		ChangeTeamColor = 4,
		IdNewGame = 5,
		IdStartGameClicked = 7,
		ChangeSide = 8,

	// once a newgame is started:
//		IdInitFogOfWar = 10,
//		IdStartScenario = 11,
//		AddUnitsXML = 12, // add units from an xml (BosonScenario) file/stream
		IdGameIsStarted = 25, // the game is started as soon as this is received
		IdGameStartingCompleted = 26,

	// usually withing a game
//		AddUnit = 30, // whenever a unit is added
		ChangeMap = 31, // editor only (but comparable to AddUnit)

	// debug/cheat messages
		IdModifyMinerals = 40, // change minerals of a player
		IdModifyOil = 41, // change oil of a player
		IdKillPlayer = 42,

		// advance messages (still the "within a game" section)
		AdvanceN = 50, // call BosonCanvas::advance() N times

		IdStatus = 60, // status messages (events)

		IdChat = 70, // a chat message
		IdNetworkSyncCheck = 80,
		IdNetworkSyncCheckACK = 81,
		IdNetworkRequestSync = 82,
		IdNetworkSync = 83,
		IdNetworkSyncUnlockGame = 84,

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
		MoveEnterUnit = 110,  // Enter a unit (airplane, repairyard, ...)

		MoveLayMine = 120,
		MoveDropBomb = 121,

		MoveTeleport = 150,  // Immediately move unit (set it's position)
		MoveRotate = 151,  // Set unit's rotation

	// Player Input in Editor mode
		MoveEditor = 200, // all editor moves/inputs are prefixed with this
		MovePlaceUnit = 201,
		MoveChangeTexMap = 202,
		MoveChangeHeight = 203,
		MoveDeleteItems = 204,

		MoveUndoPlaceUnit = 241,
		MoveUndoChangeTexMap = 242,
		MoveUndoChangeHeight = 243,
		MoveUndoDeleteItems = 244,


	// the last message ID:
		UnitPropertyHandler = 1000 // + unit id
	};
};

#endif
