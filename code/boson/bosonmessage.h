#ifndef __BOSONMESSAGE_H__
#define __BOSONMESSAGE_H__

class BosonMessage
{
public:
	enum BosonMessages {
		InitMap = 0,
		AddUnit = 3,
		Advance = 4,
		IdChat = 5,
		IdStopMoving = 6,
		IdStartScenario = 7,

	// Player Moves:
		MoveMove = 100, // Unit(s) is/are moved
		MoveAttack = 101, // a unit is being attacked
		MoveConstruct = 102, // construct a unit


		// the last message ID:
		UnitPropertyHandler = 1000 // + unit id
	};
};

#endif
