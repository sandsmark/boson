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

	// Player Moves:
		MoveMove = 100, 
		MoveAttack = 101, 


		// the last message ID:
		UnitPropertyHandler = 1000 // + unit id
	};
};

#endif
