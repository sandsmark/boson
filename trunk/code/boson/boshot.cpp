#include "boshot.h"

#include "speciestheme.h"
#include "visualunit.h"

#include "defines.h"

#define SHOT_DELAY 0 // number of advance() calls without changing frames

class BoShotPrivate
{
public:
	BoShotPrivate()
	{
	
	}

	unsigned int mMaxCounter;
	unsigned int mCounter;
	
	int mDelay;
};

BoShot::BoShot(VisualUnit* target, VisualUnit* attacker, QCanvas* canvas, bool destroyed)
		: QCanvasSprite(0, canvas)
{
 d = new BoShotPrivate;
 d->mCounter = 0;
 d->mDelay = SHOT_DELAY;

 if (!target) {
	kdError() << "BoShot::BoShot(): NULL target" << endl;
	delete this;
	return;
 }
 if (!attacker) {
	kdError() << "BoShot::BoShot(): NULL attacker" << endl;
	delete this;
	return;
 }


 SpeciesTheme* theme = attacker->speciesTheme();
 if (!theme) {
	kdError() << "BoShot::BoShot(): NULL attacker theme" << endl;
	delete this;
	return;
 }

 QCanvasPixmapArray* sequence = 0;
 if (destroyed) {
	int version = 0; // TODO: random (see KGame::random())
	theme->loadBigShot(target->isFacility(), version);
	sequence = theme->bigShot(target->isFacility(), version);
	d->mMaxCounter = (target->isFacility() ? FACILITY_SHOT_FRAMES : MOBILE_SHOT_FRAMES);

	QPoint center = target->boundingRect().center();
	setX(center.x());
	setY(center.y());
	setZ(target->z() + 1);
 } else {
	theme->loadShot();
	sequence = theme->shot();
	d->mMaxCounter = SHOT_FRAMES;

	// x and y should be kind of random (note: use KGame::random()) ! -> see
	// original boShot
	setX(target->x());
	setY(target->y());
	setZ(target->z() + 1);
 }
 if (!sequence) {
	kdError() << "BoShot::BoShot(): NULL sequence" << endl;
		delete this;
		return;
 }
 setSequence(sequence);
 
 setFrame(0);
 show();
 setAnimated(true);
}

BoShot::~BoShot()
{
 delete d;
}

void BoShot::advance(int phase)
{
 if (phase == 1) {
	if (d->mDelay > 0) {
		d->mDelay--;
		return;
	}
	d->mDelay = SHOT_DELAY;
	d->mCounter++;
	if (d->mCounter < d->mMaxCounter) {
		setFrame(d->mCounter);
		return;
	}
	delete this;
 }
}
