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

#include "common/map.h"
#include "common/log.h"

#include "bosonCanvas.h"
#include "selectPart.h"
#include "playerUnit.h"
#include "game.h"



/*
 *  bosonUnit
 */

void bosonUnit::targetDying(bosonUnit *t)
{
	boAssert (target == t);
	target = 0l;
}


void bosonUnit::u_attack(bosonUnit *u)
{
	if (target) {
		disconnect(target, 0, this, 0); // target isn't connected to 'this' anymore
	}

	if (u == this) {
		/* attacking myself */
		logf(LOG_WARNING, "%p attacking itself, aborting", this);
		target = 0;
		return;
	}
	target		= u;
	shoot_timer	= -1;

	connect( u, SIGNAL(dying(bosonUnit*)), this, SLOT(targetDying(bosonUnit*)) );
}


/*
 * playerMobUnit
 */

static const int pos_x[12] = 
	{  34,  77,  98,  94,  64,  17, -34, -77, -98, -94, -64, -17};
static const int pos_y[12] = 
	{ -94, -64, -17, +34, +77, +98, +94, +64, +17, -34, -77, -98};

playerMobUnit::playerMobUnit(mobileMsg_t *msg)
	: visualMobUnit(msg)
	, state(MUS_NONE)
{
	asked_x = asked_y = 0;
	turnTo(4); ///orzel : should be random
}


#define VECT_PRODUCT(dir)	(pos_x[dir]*(ldy) - pos_y[dir]*(ldx))
#define SQ(x)			( (x) * (x) )
bool playerMobUnit::getWantedMove(bosonMsgData *msg)
{
	int ldx, ldy;
	int vp1, vp2, vp3;
	int newdir;
	int range;
	
	switch(state){
		default:
			logf(LOG_ERROR, "playerMobUnit::getWantedMove : unknown state");
			return false;
			break;

		case MUS_NONE:
			return false;
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
				msg->move.newx = x(); msg->move.newy = y(); msg->move.direction = direction;
				asked_state = MUS_TURNING;
				return true;
				}
			turnTo(newdir); // turning anyway
			if (state != MUS_MOVING) return false;

		case MUS_MOVING_WAIT:
			if ( 0 == --countDown ) {
				state = MUS_MOVING;	// and keep on the next case
//				logf(LOG_INFO, "MUS_MOVING_WAIT finished, [%p] try again now", this);
			}
			else return false;		// nothing, return

		case MUS_MOVING:
			ldx = dest_x - x() ; ldy = dest_y - y();

			range = mobileProp[type].range;
			if (target && SQ(range) > SQ(ldx) + SQ(ldy) ) // we are near enough to shoot at the target
				return false;

			vp1 = VECT_PRODUCT(getLeft(2));
			vp2 = VECT_PRODUCT(direction);
			vp3 = VECT_PRODUCT(getRight(2));
//			printf("vp1 = %d, ", vp1); printf("vp2 = %d, ", vp2); printf("vp3 = %d\n", vp3);
			if ( abs(vp2) > abs(vp1) || abs(vp2) > abs(vp3)) // direction isn't optimal
				turnTo ( ( abs(vp1) < abs(vp3) )? getLeft():getRight() ); // change it

			if ( ( SQ(ldx) + SQ(ldy) ) < SQ(mobileProp[type].speed) ) { ///orzel should be square
				present_dx = ldx; present_dy = ldy;
				}
			msg->move.newx = asked_x = x() + present_dx;
			msg->move.newy = asked_y = y() + present_dy;
			msg->move.direction = direction;

			asked_state = MUS_MOVING;
			if (checkMove( asked_x, asked_y)) return true;

			if ( abs(present_dy) > abs(present_dx) )  {
				/* we are going mainly along y axis, so try that first*/
				if (asked_y != y() && checkMove( x(), asked_y)) {
					msg->move.newx = asked_x = x();
					return true;
					}
				if (asked_x != x() && checkMove( asked_x, y())) {
					msg->move.newy = asked_y = y();
					return true;
					}

			} else {
				/* we are going mainly along x axis, so try that first*/
				if (asked_x != x() && checkMove( asked_x, y())) {
					msg->move.newy = asked_y = y();
					return true;
					}
				if (asked_y != y() && checkMove( x(), asked_y)) {
					msg->move.newx = asked_x = x();
					return true;
					}
			}

			/* failed : can't move any more */
			asked_state = state = MUS_NONE;
//			logf(LOG_INFO, "ckeckMove failed : mobile[%p] will try later", this);
			state = MUS_MOVING_WAIT;
			countDown = 5;
			return false; 

			break;
		}

	logf(LOG_ERROR, "unhandled state in getWantedMove()");
	return false;
}


bool playerMobUnit::checkMove(int newx, int newy)
{
	int ty;
	int g;

	/* XXXX  
	Pix p = neighbourhood( newx, newy);
	
	if (goFlag() == BO_GO_AIR) { // we are a flyer
		for(; p; next(p) ) {
			ty = at(p)->rtti();
			if ( (ty < S_FACILITY) && !(ty < S_MOBILE) && (BO_GO_AIR == mobileProp[ty-S_MOBILE].goFlag) && exact(p))
				return false; // another flyer is around
		}
		return true; // nothing else in the aire here
	}

//	printf("%p would hit :", this);
	for(; p; next(p) )
		if (exact(p)) {     // ugly, should be find-tuned
			// p is what would be hit if ....
			ty = at(p)->rtti();
			if (ty < S_GROUND) continue;	// S_PART
			if (ty >=  S_SHOT) continue;	// S_PART
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
			//	printf("%s, ", (IS_PLAIN(g))?
			//		groundProp[g].name:
			//		groundTransProp[ GET_TRANS_REF(g) ].name); 
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
	*/	

	return true;
}

void playerMobUnit::turnTo(int newdir)
{
//printf("turning from %d to %d\n", direction, newdir);
	assert(newdir>=0); assert(newdir<12);
	//if (direction==newdir) return;
	direction = newdir;
	present_dx =  pos_x[direction];
	present_dy =  pos_y[direction];
	double factor = (double) mobileProp[type].speed / 100.;
	if (factor<1.) {
		present_dx =(int) ((double)present_dx*factor);
		present_dy =(int) ((double)present_dy*factor);
		}
	else logf(LOG_ERROR, "turnTo : unexpected mobileProp.speed..."); ///orzel : test should be removed

	setFrame(direction);
}


void playerMobUnit::getWantedAction()
{
	bosonMsgData	data;

	if (who != who_am_i) return;

	if ( !visible() ) return;

	/* movve ?*/
	if (getWantedMove(&data)) {
		data.move.key		= key;
		sendMsg(buffer, MSG_MOBILE_MOVE_R, MSG(data.move) );
	}

	if (getWantedShoot(&data)) {
		data.shoot.key		= key;
		sendMsg(buffer, MSG_UNIT_SHOOT, MSG(data.shoot) );
	}
}


bool playerMobUnit::getWantedShoot(bosonMsgData *msg)
{
	int	dx, dy;
	int	range = mobileProp[type].range;
	QRect	r;

	if (!target) return false;		// no target
	if (range<=0) return false;		// Unit can't shoot

	Unit * _target;

	if (target->inherits("playerMobUnit"))
		_target = ((playerMobUnit*)target);
	else if (target->inherits("playerFacility"))
		_target = ((playerFacility*)target);
	else {
		logf(LOG_ERROR, "getWantedShoot, what's this bosonUnit ???");
		return false;
	}

	r = _target->rect();
	dx = x() - r.x(); dy = y() - r.y();
	if (range*range < dx*dx + dy*dy) return false; // too far

	shoot_timer--;
	if (shoot_timer<=0) shoot_timer = 30;
		else return false;		// not yet

	// ok, let's shoot it
	msg->shoot.target_key = _target->key;
	return true;
}


/***** server orders *********/
void playerMobUnit::doMoveTo(int newx, int newy)
{
	int dx = newx - x();
	int dy = newy - y();

	move(newx,newy);
	emit sig_moveTo(newx, newy);

	if (sp_up) sp_up->moveBy(dx,dy);
	if (sp_down) sp_down->moveBy(dx,dy);
}

void playerMobUnit::s_moveTo(int newx, int newy, int dir)
{
//orzel : use some kind of fuel
if ( who!=who_am_i) {
	/* this not my unit */
	doMoveTo(newx, newy);
	direction = dir;
	setFrame(dir);
	return;
	}

/* else */

if ( MUS_MOVING != asked_state && (newx!=x() || newy!=y()) ) {
	logf(LOG_ERROR, "playerMobUnit::s_moveTo while not moving, ignored");
	return;
	}

if (dir != direction)
	logf(LOG_ERROR, "playerMobUnit::s_moveTo : unexpected direction");

doMoveTo(newx, newy);

if (x()==dest_x && y()==dest_y) {
	//puts("going to MUS_NONE");
	state = MUS_NONE;
	present_dx = present_dy = 0; // so that willBe returns the good position
	logf(LOG_GAME_LOW, "mobile[%p] has stopped\n", this);
	}

}






/***** users orders *********/

void playerMobUnit::u_goto(int mx, int my) // not the same as QCanvasSprite::moveTo
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
	if (x()==dest_x && y()==dest_y)
		state = MUS_NONE;
	else	state = MUS_TURNING;
	//puts("going to MUS_TURNING");

///orzel : moving across complicated environment algorithm
}


void playerMobUnit::u_stop(void)
{
	state = MUS_NONE;
}


void playerMobUnit::u_attack(bosonUnit *u)
{
	QRect	r;

	bosonUnit::u_attack(u);

	connect( u, SIGNAL(sig_moveTo(int,int)), this, SLOT(targetMoveTo(int,int)) );

	if (u->inherits("playerMobUnit"))
		r = ((playerMobUnit*)u)->rect();
	else if (u->inherits("playerFacility"))
		r = ((playerFacility*)u)->rect();
	else {
		logf(LOG_ERROR, "u_attack, what's this bosonUnit ???");
		return;
	}


	do_goto(r.x(), r.y());
}


void playerMobUnit::targetMoveTo(int newx, int newy)
{
	do_goto(newx, newy);
}


void playerMobUnit::shooted(int _power)
{
	if (sp_up) sp_up->setFrame(_power);
}
  

bool playerMobUnit::near(int d)
{
	int a = x() - dest_x;
	int b = y() - dest_y;
	
	return (a*a + b*b) < (d*d);
}


/*
 * playerFacility
 */
playerFacility::playerFacility(facilityMsg_t *msg)
	: visualFacility(msg)
{
}


void playerFacility::s_setState(int s)
{
	boAssert(frame()==s-1);
	setFrame(s);
}


void playerFacility::getWantedAction()
{
	if (who != who_am_i) return;
}


void playerFacility::shooted(int _power)
{
	if (sp_up) sp_up->setFrame(_power);
}
  


#define underlyingGround() vcanvas->groundAt( x()+10, y()+10)

/*
 * harvester 
 */
bool harvesterUnit::getWantedMove(bosonMsgData *msg)
{
	bool ret = false;

	switch(hstate) {
		case standBy:
			return false;
			break;
		case comingBack:
			if ( x() == base_x && y() == base_y) {
				/* we are back : empty the harvester */ 
//				puts("harvester : arrived home");
				harvestEndMsg_t    he;
				he.key = key;
				sendMsg(buffer, MSG_UNIT_HARVEST_END, MSG(he) );
				hstate = goingTo;
				playerMobUnit::u_goto(harvest_x, harvest_y); // go to base station
				contain = 0 ;		// emptying

			}
			return playerMobUnit::getWantedMove(msg);
			break;
		case goingTo:
			ret = playerMobUnit::getWantedMove(msg);
			if (near (100) && underlyingGround() == GROUND_GRASS_OIL ) {
				hstate = harvesting;
//				puts("harvester : change to \"harvesting\" state");
			} else // nothing to harvest
			if ( near (5) ) { 
				// orzel : look around for another cell to harvest
				hstate = standBy;
//				printf("harvester : change to \"standby\" state, underlying is %d, not %d\n", underlyingGround(), GROUND_GRASS_OIL);
				return false;
			}
			else return ret; // continue
			// no break; 
		case harvesting:
			if ( contain < 200 ) {
				harvestMsg_t    harvest;
				harvest.key = key;
				sendMsg(buffer, MSG_UNIT_HARVEST, MSG(harvest) );
			} else {
				hstate = comingBack;
//				puts("harvester : change to \"comingBack\" state");
				playerMobUnit::u_goto(base_x, base_y); // go to base station
			}
			// send a message "i'm harvesting there"
			// check "contain" ...
			break;
	};
	return  ret;
}

bool harvesterUnit::getWantedShoot(bosonMsgData *)
{
	return false;
}

void harvesterUnit::u_goto(int mx, int my)
{
	harvest_x = mx; harvest_y = my;
	hstate = goingTo;
//	puts("harvester : change to \"goingTo\" state");
	playerMobUnit::u_goto(mx, my);
}

