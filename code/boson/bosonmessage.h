/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
		IdStartScenario = 10, // for new game dialog
		AddUnit = 50, // whenever a unit is added
		Advance = 51, // call BosonCanvas::advance()
		IdChat = 52, // a chat message
		IdStopMoving = 53, // a unit shall stop moving. obsolete.

	// Player Moves:
		MoveMove = 100, // Unit(s) is/are moved
		MoveAttack = 101, // a unit is being attacked
		MoveConstruct = 102, // construct a unit


		// the last message ID:
		UnitPropertyHandler = 1000 // + unit id
	};
};

#endif
