#ifndef __BOSONCOMPUTERIO_H__
#define __BOSONCOMPUTERIO_H__

#include <kgame/kgameio.h>

class Player;

class BosonComputerIO : public KGameComputerIO
{
	enum RTTI {
		IdBosonComputerIO = 500
	};
	Q_OBJECT
public:
	BosonComputerIO();
	BosonComputerIO(KPlayer*);
	~BosonComputerIO();

	virtual int rtti() const { return IdBosonComputerIO; }

protected:
	virtual void reaction();
	Player* boPlayer() const { return (Player*)player(); }

private:
};

#endif
