/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "boshot.h"

#include "bosoncanvas.h"
#include "speciestheme.h"
#include "unit.h"

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

BoShot::BoShot(Unit* target, Unit* attacker, BosonCanvas* c, bool destroyed)
		: BosonSprite(0, c)
{
 d = new BoShotPrivate;
 d->mCounter = 0;
 d->mDelay = SHOT_DELAY;

 if (!target) {
	kdError() << k_funcinfo << "NULL target" << endl;
	deleteMe();
	return;
 }

 SpeciesTheme* theme;
 if (!destroyed) {
	if (!attacker) {
		kdError() << k_funcinfo << "NULL attacker" << endl;
		deleteMe();
		return;
	} else {
		theme = attacker->speciesTheme();
	}
 } else {
		theme = target->speciesTheme();
 }
 if (!theme) {
	kdError() << k_funcinfo << "NULL attacker theme" << endl;
	deleteMe();
	return;
 }

// TODO: fix for OpenGL!!
/*
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
	setX(target->x() + target->width() / 2);
	setY(target->y() + target->height() / 2);
	setZ(target->z() + 1);
 }
 if (!sequence) {
	kdError() << k_funcinfo << "NULL sequence" << endl;
	deleteMe();
	return;
 }
 setSequence(sequence);
 */

 setFrame(0);
 setVisible(true);
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
	deleteMe();
 }
}

void BoShot::deleteMe()
{
 kdDebug() << k_funcinfo << endl;
 ((BosonCanvas*)canvas())->deleteShot(this);
 setAnimated(false);
 setVisible(false);
}

