#ifndef __BOSONMESSAGE_H__
#define __BOSONMESSAGE_H__

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
