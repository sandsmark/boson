/***************************************************************************
                          playerUnit.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : orzel@yalbi.com                                     
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

#include "../common/map.h"
#include "../common/log.h"

#include "selectPart.h"
#include "playerUnit.h"
//#include "speciesTheme.h"
#include "game.h"



/*
 * playerMobUnit
 */

const static int pos_x[12] = 
	{  34,  77,  98,  94,  64,  17, -34, -77, -98, -94, -64, -17};
const static int pos_y[12] = 
	{ -94, -64, -17, +34, +77, +98, +94, +64, +17, -34, -77, -98};

playerMobUnit::playerMobUnit(mobileMsg_t *msg, QObject* parent=0, const char *name=0L)
	: visualMobUnit(msg,parent,name)
	, state(MUS_NONE)
{
	asked_dx = asked_dy = 0;
	turnTo(4); ///orzel : should be random
	target = 0l;
}

playerMobUnit::~playerMobUnit()
{
	emit dying(this);
}

#define VECT_PRODUCT(dir) (pos_x[dir]*(ldy) - pos_y[dir]*(ldx))

bool playerMobUnit::getWantedMove(bosonMsgData *msg)
{
	int ldx, ldy;
	int vp1, vp2, vp3;
	int newdir;
	int range;
	
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
			//printf("vp1 = %d, vp2 = %d \n", vp1, vp2);
			if ( (vp1<0 && vp2>0) || (vp1>0 && vp2<0) ) { // it's the end
				newdir =   (abs(vp1) > abs(vp2))? newdir:direction;
				state = MUS_MOVING;
				//puts("going to MUS_MOVING");
				}
			if (newdir != direction) {
				turnTo(newdir); ///orzel : is this really useful
				msg->move.dx = msg->move.dy = 0; msg->move.direction = direction;
				return 1;
				}
			turnTo(newdir); // turning anyway
			if (state != MUS_MOVING) return 0;

		case MUS_MOVING:
			ldx = dest_x - x() ; ldy= dest_y - y();

			range = mobileProp[type].range;
			if (target && range*range > ldx*ldx + ldy*ldy) // we are near enough to shoot at the target
				return false;

			vp1 = VECT_PRODUCT(getLeft(2));
			vp2 = VECT_PRODUCT(direction);
			vp3 = VECT_PRODUCT(getRight(2));
//			printf("vp1 = %d, ", vp1); printf("vp2 = %d, ", vp2); printf("vp3 = %d\n", vp3);
			if ( abs(vp2) > abs(vp1) || abs(vp2) > abs(vp3)) // direction isn't optimal
				turnTo ( ( abs(vp1) < abs(vp3) )? getLeft():getRight() ); // change it

			if ( ( abs(ldx) + abs(ldy) ) < abs (mobileProp[type].speed) ) { ///orzel should be square
				asked_dx = ldx; asked_dy = ldy;
				}
			msg->move.dx = asked_dx ; msg->move.dy = asked_dy; msg->move.direction = direction;

			if (checkMove( asked_dx, asked_dy)) return 1;

			if (asked_dy && checkMove( 0, asked_dy)) {
				msg->move.dx = asked_dx = 0;
				return 1;
				}
			if (asked_dx && checkMove( asked_dx, 0)) {
				msg->move.dy = asked_dy = 0;
				return 1;
				}

			/* failed : can't move any more */
			state = MUS_NONE;
			asked_dx = asked_dy = 0; // so that willBe returns the good position
			//logf(LOG_GAME_LOW, "ckeckMove failed stopping: mobile[%p] has stopped\n", this);
			logf(LOG_WARNING, "ckeckMove failed : mobile[%p] has stopped\n", this);
			return 0; 

			break;
		}
}


bool playerMobUnit::checkMove(int dx, int dy)
{
	int ty;
	int g;

	if (goFlag() == BO_GO_AIR) return true; // flyers can go everywhere...

	Pix p = neighbourhood( x()+dx, y()+dy);

//	printf("%p would hit :", this);
	for(; p; next(p) )
		if (exact(p)) {     // ugly, should be find-tuned
			// p is what would be hit if ....
			ty = at(p)->rtti();
			if (ty < S_GROUND) continue;	// S_PART
			if (ty < S_MOBILE) {		// S_GROUND
				g = ty - S_GROUND;
//printf("\ng = %d\n", g); fflush(stdout);
				if (IS_PLAIN(g))
					if ( !(GET_BIT(g) & goFlag()) ) {
						end(p);
						return false;
						}
					else continue;
//puts("e"); fflush(stdout);
//printf("\ntrans = %d\n", GET_TRANS_REF(g)); fflush(stdout);
//printf("\ngetbit 1 = %d\n", GET_BIT( groundTransProp[ GET_TRANS_REF(g) ].from ) );
//printf("\ngetbit 2 = %d\n", GET_BIT( groundTransProp[ GET_TRANS_REF(g) ].to ) );
//printf("\ngoFlag = %d\n", goFlag() );
				// is TRANS
					if ( !(GET_BIT( groundTransProp[ GET_TRANS_REF(g) ].from ) & goFlag()) ||
					     !(GET_BIT( groundTransProp[ GET_TRANS_REF(g) ].to ) & goFlag()) )  {
						end(p);
						return false;
						}
					else continue;
				/*printf("%s, ", (IS_PLAIN(g))?
					groundProp[g].name:
					groundTransProp[ GET_TRANS_REF(g) ].name); */
				continue;// unreachable ?
				}
			if (ty < S_FACILITY) {		// S_MOBILE
				if (BO_GO_AIR == mobileProp[ty-S_MOBILE].goFlag)// smthg that can fly
					continue;
				end(p);					// anything else
				return false;
				//printf("%s, ", mobileProp[ty-S_MOBILE].name);
				}
			if (ty < S_FACILITY+FACILITY_LAST) { // S_FACILITY
				return false;
				//printf("%s, ", facilityProp[ty-S_FACILITY].name);
				continue;// unreachable ?
				}
			logf(LOG_ERROR, "playerMobUnit::checkMove : type is %d, ", ty);
			}
	//end(p); 
	// not needed because we have reached the end of the list

	return true;
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


void playerMobUnit::getWantedAction()
{
	bosonMsgData	data;

	if (who != who_am_i) return;

	/* movve ?*/
	if (getWantedMove(&data)) {
		data.move.key		= key;
		sendMsg(buffer, MSG_MOBILE_MOVE_R, sizeof(data.move), &data);
	}

	if (getWantedShoot(&data)) {
		data.shoot.key		= key;
		sendMsg(buffer, MSG_UNIT_SHOOT, sizeof(data.shoot), &data);
	}
}


bool playerMobUnit::getWantedShoot(bosonMsgData *msg)
{
	int dx, dy;
	int range = mobileProp[type].range;

	if (!target) return false;		// no target
	if (range<=0) return false;		// Unit can't shoot

	dx = x() - target->_x(); dy = y() - target->_y();
	if (range*range < dx*dx + dy*dy) return false; // too far

	shoot_timer--;
	if (shoot_timer<=0) shoot_timer = 30;
		else return false;		// not yet

	// ok, let's shoot it
	msg->shoot.target_key = target->key;
	return true;
}


/***** server orders *********/
void playerMobUnit::doMoveBy(int dx, int dy)
{
	moveBy(dx,dy);

	emit sig_move(dx, dy);
	if (sp_up) sp_up->moveBy(dx,dy);
	if (sp_down) sp_down->moveBy(dx,dy);
}

void playerMobUnit::s_moveBy(int dx, int dy, int dir)
{
//orzel : use some kind of fuel
//printf("Moved  : d(%d.%d)\n", dx, dy);
if ( who!=who_am_i) {
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

/*
if (asked_dx !=0 && asked_dy !=0 && (dx != asked_dx || dy != asked_dy) )
	logf(LOG_ERROR, "playerMobUnit::s_moveBy : unexpected dx,dy");
*/
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
	if (target) {
		disconnect(target, 0, this, 0); // target isn't connected to 'this' anymore
		target = 0l;
		//puts("u_goto disconnecting target");
	}
	do_goto(mx, my);
}
	
	
void playerMobUnit::do_goto(int mx, int my)
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


void playerMobUnit::u_attack(Unit *u)
{
	if (target) {
		disconnect(target, 0, this, 0); // target isn't connected to 'this' anymore
	}

	target		= u;
	shoot_timer	= -1;
	//puts("assigning target");

	connect( u, SIGNAL(dying(Unit*)), this, SLOT(targetDying(Unit*)) );
	connect( u, SIGNAL(sig_move(int,int)), this, SLOT(targetMoveBy(int,int)) );

	do_goto(u->_x(), u->_y());

}


void playerMobUnit::targetDying(Unit *t)
{
	boAssert (target == t);
	//puts("target = 0l");
	target = 0l;
	// disconnection is handled by target destructor (in Qt)
}


void playerMobUnit::targetMoveBy(int dx, int dy)
{
	do_goto(dest_x+dx, dest_y+dy);
}


/*
 * playerFacility
 */
playerFacility::playerFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L)
	: visualFacility(msg,parent,name)
{
}

playerFacility::~playerFacility()
{
	//puts("~playerFacility");
	emit dying(this);
}


void playerFacility::s_setState(int s)
{
	boAssert(frame()==s-1);
	frame(s);
}


void playerFacility::getWantedAction()
{

	if (who != who_am_i) return;
/*
	bosonMsgData	data;

	if (getWantedShoot(&data) {
		data.shoot.key		= key;
		sendMsg(buffer, MSG_UNIT_SHOOT, sizeof(data.shoot), &data);
	}
*/
}


void playerFacility::targetDying(Unit*)
{
//	target = 0l;
}


void playerFacility::targetMoveBy(int dx, int dy)
{
}


