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
#include "boshot.h"

#include "speciestheme.h"
#include "unit.h"

#include "defines.h"

#define SHOT_DELAY 0 // number of advance() calls without changing frames

class BoShot::BoShotPrivate
{
public:
	BoShotPrivate()
	{
	
	}

	unsigned int mMaxCounter;
	unsigned int mCounter;
	
	int mDelay;
};

BoShot::BoShot(Unit* target, Unit* attacker, QCanvas* canvas, bool destroyed)
		: QCanvasSprite(0, canvas)
{
 d = new BoShotPrivate;
 d->mCounter = 0;
 d->mDelay = SHOT_DELAY;

 if (!target) {
	kdError() << k_funcinfo << ": NULL target" << endl;
	delete this;
	return;
 }
 if (!attacker) {
	kdError() << k_funcinfo << ": NULL attacker" << endl;
	delete this;
	return;
 }


 SpeciesTheme* theme = attacker->speciesTheme();
 if (!theme) {
	kdError() << k_funcinfo << ": NULL attacker theme" << endl;
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
	kdError() << k_funcinfo << ": NULL sequence" << endl;
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
