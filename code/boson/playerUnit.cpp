/***************************************************************************
                          playerUnit.cpp  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : capricel@enst.fr                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>	// int abs(int);
#include <assert.h>

#include "playerUnit.h"
#include "speciesTheme.h"
#include "game.h"
#include "selectPart.h"

#include "../map/map.h"
#include "../common/log.h"


/*
 * playerMobUnit
 */

const static int pos_x[12] = 
	{  34,  77,  98,  94,  64,  17, -34, -77, -98, -94, -64, -17};
const static int pos_y[12] = 
	{ -94, -64, -17, +34, +77, +98, +94, +64, +17, -34, -77, -98};

playerMobUnit::playerMobUnit(mobileMsg_t *msg, QObject* parent=0, const char *name=0L)
	:mobUnit(msg,parent,name)
	, QwSprite(gpp.species[msg->who]->getPixmap(msg->type))
	, state(MUS_NONE)
{
	z(Z_MOBILE);
	moveTo(msg->x, msg->y);

	asked_dx = asked_dy = 0;

	sp_down = 0l; sp_up = 0l;
	
	turnTo(4); ///orzel : should be random
}

playerMobUnit::~playerMobUnit()
{
unSelect();
}


#define VECT_PRODUCT(dir) (pos_x[dir]*(ldy) - pos_y[dir]*(ldx))

int playerMobUnit::getWantedMove(int &dx, int &dy, int &dir)
{
int ldx, ldy;
int vp1, vp2, vp3;
int newdir;

switch(state){
	default:
		logf(LOG_ERROR, "playerMobUnit::getWantedMove : unknown state");
		return 0;
		break;

	case MUS_NONE:
		return 0;
		break;

	case MUS_TURNING:
		assert(direction>=0); assert(direction<12);
		ldx = dest_x - x(); ldy = dest_y - y();
		vp1 = VECT_PRODUCT(direction);
		// turning 
		newdir =  (vp1<0)?getLeft():getRight();
		vp2 = VECT_PRODUCT( newdir);
		printf("vp1 = %d, vp2 = %d \n", vp1, vp2);
		if ( (vp1<0 && vp2>0) || (vp1>0 && vp2<0) ) { // it's the end
			newdir =   (abs(vp1) > abs(vp2))? newdir:direction;
			state = MUS_MOVING;
			//puts("going to MUS_MOVING");
			}
		if (newdir != direction) {
			turnTo(newdir); ///orzel : is this really useful
			dx = dy = 0; dir = direction;
			return 1;
			}
		turnTo(newdir); // turning anyway
		return 0; // no move asked
		break;

	case MUS_MOVING:
		ldx = dest_x - x() ; ldy= dest_y - y();
		vp1 = VECT_PRODUCT(getLeft(2));
		vp2 = VECT_PRODUCT(direction);
		vp3 = VECT_PRODUCT(getRight(2));
//		printf("vp1 = %d, ", vp1); printf("vp2 = %d, ", vp2); printf("vp3 = %d\n", vp3);
		if ( abs(vp2) > abs(vp1) || abs(vp2) > abs(vp3)) // direction isn't optimal
			turnTo ( ( abs(vp1) < abs(vp3) )? getLeft():getRight() ); // change it

		if ( ( abs(ldx) + abs(ldy) ) < abs (mobileProp[type].speed) ) {
			asked_dx = ldx; asked_dy = ldy;
			}
		dx = asked_dx ; dy = asked_dy; dir = direction;
		return 1;
		break;
	}
}


void playerMobUnit::turnTo(int newdir)
{
//printf("turning from %d to %d\n", direction, newdir);
	assert(newdir>=0); assert(newdir<12);
	//if (direction==newdir) return;
	direction = newdir;
	asked_dx =  pos_x[direction];
	asked_dy =  pos_y[direction];
	double factor = (double) mobileProp[type].speed / 100.;
	if (factor<1.) {
		asked_dx =(int) ((double)asked_dx*factor);
		asked_dy =(int) ((double)asked_dy*factor);
		}
	else logf(LOG_ERROR, "turnTo : unexpected mobileProp.speed..."); ///orzel : test should be removed

	frame(direction);
}


int playerMobUnit::getWantedAction()
{
	return 0; // no action
}




/***** selection *********/
void playerMobUnit::select()
{
	QRect	r = rect();

	sp_up = new selectPart_up(5);
	sp_up->moveTo(r.right(), r.top());
	sp_down = new selectPart_down(4);
	sp_down->moveTo(r.left(), r.bottom());
}


void playerMobUnit::unSelect()
{
	delete  sp_up;
	delete	sp_down;
	sp_down = 0l;
	sp_up = 0l;
}


/***** server orders *********/
void playerMobUnit::doMoveBy(int dx, int dy)
{
	moveBy(dx,dy);
	if (sp_up) sp_up->moveBy(dx,dy);
	if (sp_down) sp_down->moveBy(dx,dy);
}

void playerMobUnit::s_moveBy(int dx, int dy, int dir)
{
//orzel : use some kind of fuel
//printf("Moved  : d(%d.%d)\n", dx, dy);
if ( who!=gpp.who_am_i) {
	/* this not my unit */
	doMoveBy(dx,dy);
	direction = dir;
	frame(direction);
	return;
	}

/* else */

if ( MUS_MOVING != state && (dx!=0 || dy!=0) ) {
	logf(LOG_ERROR, "playerMobUnit::s_moveBy while not moving, ignored");
	return;
	}

//printf("      dx = %d,       dy = %d\n", dx, dy);
//printf("asked_dx = %d, asked_dy = %d\n", asked_dx, asked_dy);
if (dx != asked_dx || dy != asked_dy)
	logf(LOG_ERROR, "playerMobUnit::s_moveBy : unexpected dx,dy");
if (dir != direction)
	logf(LOG_ERROR, "playerMobUnit::s_moveBy : unexpected direction");

doMoveBy(dx,dy);

if (x()==dest_x && y()==dest_y) {
	//puts("going to MUS_NONE");
	state = MUS_NONE;
	asked_dx = asked_dy = 0; // so that willBe returns the good position
	logf(LOG_GAME_LOW, "mobile[%p] has stopped\n", this);
	}

}






/***** users orders *********/

void playerMobUnit::u_goto(int mx, int my) // not the same as QwSprite::moveTo
{
	dest_x = mx; dest_y = my;
	if (x()==dest_x || y()==dest_y) return;

	state = MUS_TURNING;
	//puts("going to MUS_TURNING");

///orzel : moving across complicated environment algorithm
}


void playerMobUnit::u_stop(void)
{
	state = MUS_NONE;
}


/*
 * playerFacility
 */
playerFacility::playerFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L)
	: Facility(msg,parent,name)
	, QwSprite(gpp.species[msg->who]->getPixmap(msg->type))
{
	z(Z_FACILITY);
	moveTo(BO_TILE_SIZE * msg->x , BO_TILE_SIZE * msg->y);
	frame(msg->state);
}


void playerFacility::s_setState(int s)
{
	boAssert(frame()==s-1);
	frame(s);
}
