#ifndef __BOSHOT_H__
#define __BOSHOT_H__

#include "rtti.h"

#include <qcanvas.h>

class Unit;

class BoShotPrivate;
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

private:
	BoShotPrivate* d;
};

#endif
