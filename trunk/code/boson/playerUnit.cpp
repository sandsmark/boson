/***************************************************************************
                          playerUnit.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#include <stdlib.h> 	// random
#include <assert.h>

#include <math.h>

#include "common/bomap.h"
#include "common/log.h"

#include "bosonCanvas.h"
#include "selectPart.h"
#include "playerUnit.h"
#include "game.h"


/*
 *  boPath
 */
bool boPath::addCheckLoop(QPoint p) 
{
	int i;
	bool ret = true;

	// check for previous step
	for(i=0; i<len; i++)
		if ( at(i) == p ) { ret=false; break; }
	// add the latest one
	at(len) = p;
	if (len == max) {
		begin++;
		begin %= max;
	} else len++;

	boAssert(len<=max);
	return ret;
}
	



/*
 *  bosonUnit
 */

void bosonUnit::targetDying(bosonUnit *t)
{
	boAssert (target == t);
	stop_attacking();
}


void bosonUnit::u_attack(bosonUnit *u)
{

	if (u == this) {
		/* attacking myself */
		logf(LOG_WARNING, "(bosonUnit::u_attack()) %p attacking itself, aborting", this);
		return;
	}

	if (target) {
		disconnect(target, 0, this, 0); // target isn't connected to 'this' anymore
	}

	target		= u;
	shoot_timer	= -1;

	connect( u, SIGNAL(dying(bosonUnit*)), this, SLOT(targetDying(bosonUnit*)) );
}

bool bosonUnit::_getWantedShoot(Unit *&_target)
{
	if (!target) return false;		// no target
	shoot_timer--;
	if (shoot_timer<=0) shoot_timer = 30;
		else return false;		// not yet

	if (target->inherits("playerMobUnit"))
		_target = ((playerMobUnit*)target);
	else if (target->inherits("playerFacility"))
		_target = ((playerFacility*)target);
	else {
		logf(LOG_ERROR, "_getWantedShoot, what's this bosonUnit ???");
		return false;
	}
	return true;
}


/*
 * playerMobUnit
 */

playerMobUnit::playerMobUnit(mobileMsg_t *msg)
	: visualMobUnit(msg)
	, state(MUS_NONE)
	, path(5)
{
	turnTo(random()%DIRECTION_STEPS);

	bocanvas->setCellFlag ( gridRect(), (BO_GO_AIR==goFlag())? Cell::flying_unit_f:Cell::field_unit_f );
}


#define VECT_PRODUCT(dir)	(pos_x[dir]*(ldy) - pos_y[dir]*(ldx))
#define SQ(x)			( (x) * (x) )


bool playerMobUnit::getWantedMove(QPoint &wmove)
{


	QRect	r = gridRect();
	QRect	nr = r;			// temporary variable
	asked = r.topLeft();		// destinaton asked, let's begin where we already are
	QPoint	dv = dest - asked;	// delta do the destination
	uint	gf = goFlag();		// caching

	int	range = mobileProp[type].range;
	bool	ret;

	if ( dv == QPoint(0,0) ) state = MUS_NONE;

	switch(state){
		default:
			logf(LOG_ERROR, "playerMobUnit::getWantedMove : unknown state");
			return false;

		case MUS_NONE:
			return false;

		case MUS_TURNING:
		case MUS_MOVING:
			if (target && boGridDist(dv)<=range) {// near enough the target to shot it
				state = MUS_NONE;
				return false;
			}
			//
			// "Raw" move algorithm
			//
			ret = true;
			// so that we do not prevent ourselves to move : 
			bocanvas->unsetCellFlag ( r, (BO_GO_AIR==gf)? Cell::flying_unit_f:Cell::field_unit_f );
			if ( abs(dv.x()) > abs(dv.y()) ) {
				// x is greater
				nr.moveBy( (dv.x()>0)?1:-1, 0); // try first along the x axis
				if (!bocanvas->checkMove(nr, gf ))  {
					nr.moveBy( (dv.x()>0)?-1:1,  (dv.y()>0)?1:-1); // then along the y axis
					if (!bocanvas->checkMove(nr, gf ))
						ret = false;
				}

			} else {
				// y is greater
				nr.moveBy( 0,  (dv.y()>0)?1:-1); // try first along the y axis
				if (!bocanvas->checkMove(nr, gf )) {
					nr.moveBy( (dv.x()>0)?1:-1,  (dv.y()>0)?-1:1); // try first along the x axis
					if (!bocanvas->checkMove(nr, gf ))
						ret = false;
				}
			}
			// restore the state for ourselves
			bocanvas->setCellFlag ( r, (BO_GO_AIR==gf)? Cell::flying_unit_f:Cell::field_unit_f );
			if (!ret)
			{
 				failed_move++;
 				if (failed_move>4) state = MUS_NONE; // prevent 'keep on trying when it can obviously not go further'
				return false;
			}
			asked = wmove = nr.topLeft();	
			asked_state = MUS_MOVING; 	///orzel:  asked_state still useful here ?
			if (failed_move>3) failed_move = 0; // prevent 3-timeunit loop
			if (!path.addCheckLoop(asked)) {
				// loop
//				logf(LOG_INFO, "loop detected, stopping");
				state = MUS_NONE;
				return false;
			}
			// it's ok, let's request the move
			///orzel : XXX probaly a but if the SERVER refuses the move because another player has already moved on this tile
			bocanvas->setCellFlag ( nr, (BO_GO_AIR==gf)? Cell::request_flying_f:Cell::request_f );
			return true;
	}

	// dead code : should not be reached :
	return false;

/*
			assert(direction>=0); assert(direction<DIRECTION_STEPS);
			ldx = dest_x - x(); ldy = dest_y - y();
			vp1 = VECT_PRODUCT(direction);
			// turning 
			asked.dir =  (vp1<0)?getLeft():getRight();
			vp2 = VECT_PRODUCT( asked.dir);
			//printf("vp1 = %d, vp2 = %d \n", vp1, vp2);
			if ( (vp1<0 && vp2>0) || (vp1>0 && vp2<0) ) { // it's the end
				 // different sign : this is the end of TURNING, go to MOVING

				asked.dir =   (abs(vp1) > abs(vp2))? asked.dir:direction; // choose the more accurate
				state = MUS_MOVING;
				//puts("going to MUS_MOVING");
			} else if (asked.dir != direction) {
				wstate = asked;
				return true;
			} else {
				logf(LOG_ERROR, "moving algorithm error #1");
				return false;
			}
		case MUS_MOVING_WAIT:
			if ( 0 >= --countDown ) {
				state = MUS_MOVING;	// and keep on the next case
//				logf(LOG_INFO, "MUS_MOVING_WAIT finished, [%p] try again now", this);
			}
			else return false;		// nothing, return

		case MUS_MOVING:
			countDown = 0 ;
			ldx = dest_x - x() ; ldy = dest_y - y();

			range = mobileProp[type].range;
			if (target && SQ(range) > SQ(ldx) + SQ(ldy) ) // we are near enough to shoot at the target
				return false;

		// choose direction 
			vp1 = VECT_PRODUCT(getLeft(2));
			vp2 = VECT_PRODUCT(direction);
			vp3 = VECT_PRODUCT(getRight(2));
//			printf("vp1 = %d, ", vp1); printf("vp2 = %d, ", vp2); printf("vp3 = %d\n", vp3);
			if ( abs(vp2) > abs(vp1) || abs(vp2) > abs(vp3)) // direction isn't optimal
				asked.dir =  ( ( abs(vp1) < abs(vp3) )? getLeft():getRight() ); // change it

		// choose dx/dy 
			if ( ( SQ(ldx) + SQ(ldy) ) < SQ(mobileProp[type].speed) ) { ///orzel should be square
				setXVelocity( ldx); setYVelocity( ldy);
				}
			asked.x = x() + xVelocity();
			asked.y = y() + yVelocity();

			wstate = asked;

		// try and try again ... 
			asked_state = MUS_MOVING;
			if (checkMove( asked)) return true;

			if ( fabs(xVelocity() ) > fabs(yVelocity() ) )  {
				// we are going mainly along x axis, so try that first
				wstate.y = y();
				if (checkMove(wstate)) return true;
				wstate = asked;

				wstate.x = x();
				if (checkMove(wstate)) return true;
				wstate = asked;

			} else {
				// we are going mainly along y axis, so try that first
				wstate.x = x();
				if (checkMove(wstate)) return true;
				wstate = asked;

				wstate.y = y();
				if (checkMove(wstate)) return true;
			}

			// failed : can't move any more
			asked_state = state = MUS_NONE;
//			logf(LOG_INFO, "ckeckMove failed : mobile[%p] will try later", this);
			state = MUS_MOVING_WAIT;
			countDown = 5;
			return false; 

			break;
		}
	*/
}

void playerMobUnit::turnTo(int newdir)
{
//printf("turning from %d to %d\n", direction, newdir);
	assert(newdir>=0); assert(newdir<DIRECTION_STEPS);
	//if (direction==newdir) return;
	direction = newdir;
	/*
	setXVelocity( pos_x[direction]);
	setYVelocity( pos_y[direction]);
	double factor = (double) mobileProp[type].speed / 100.;
	if (factor<1.) {
		setXVelocity((double)xVelocity()*factor);
		setYVelocity((double)yVelocity()*factor);
		}
	else logf(LOG_ERROR, "turnTo : unexpected mobileProp.speed..."); ///orzel : test should be removed
	*/

	setFrame(direction);
}


void playerMobUnit::getWantedAction()
{
	bosonMsgData	data;
	QPoint		ns;

	if (who != who_am_i) return;
	if ( !visible() ) return;

	/* move ?*/
	if (getWantedMove(ns)) {
		data.move.newx		= ns.x();
		data.move.newy		= ns.y();
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
	int	range = mobileProp[type].range;

	if (range<=0) return false;		// Unit can't shoot


	// how far are we ? 
	Unit *_target;
	if (!_getWantedShoot(_target)) return false;

	QPoint p = _target->center()  - QPoint( x(), y());
	if ( boDist(p) > range) return false; // too far

	// ok, let's shoot it
	msg->shoot.target_key = _target->key;
	return true;
}


/***** server orders *********/

/* actually do the job, used by different functions */
void playerMobUnit::do_moveTo(QPoint npos)
{
	QPoint	dv = npos - gridRect().topLeft();
	dv *= BO_TILE_SIZE;	// pixelwise
	QRect r = gridRect();

	boAssert(int(x())%BO_TILE_SIZE==0);
	boAssert(int(y())%BO_TILE_SIZE==0);

	// actually move the mobile, updating flags in cells
	bocanvas->unsetCellFlag ( r, (BO_GO_AIR==goFlag())? Cell::flying_unit_f:Cell::field_unit_f );
	move( BO_TILE_SIZE*npos.x(), BO_TILE_SIZE*npos.y() );


	// update cells
	r = gridRect();
	bocanvas->setCellFlag ( r, (BO_GO_AIR==goFlag())? Cell::flying_unit_f:Cell::field_unit_f );
	bocanvas->unsetCellFlag ( r, (BO_GO_AIR==goFlag())? Cell::request_flying_f:Cell::request_f );

	emit sig_moveTo(npos);

	if (sp_up) sp_up->moveBy(dv.x() ,dv.y() );
	if (sp_down) sp_down->moveBy(dv.x() ,dv.y() );
}


/* server order */
void playerMobUnit::s_moveTo(QPoint nz)
{

	//orzel : use some kind of fuel
	if ( who!=who_am_i) { /* this not my unit */
		do_moveTo(nz);
		return;
		}

	/* else */

	if ( MUS_MOVING != asked_state && (nz != asked )) {
		logf(LOG_ERROR, "playerMobUnit::s_moveTo while not moving, ignored");
		return;
		}

	if (nz != asked)
		logf(LOG_ERROR, "playerMobUnit::s_moveTo : unexpected dx,dy");

	do_moveTo(nz);

	if (nz == dest) {
		//puts("going to MUS_NONE");
		state = MUS_NONE;
		setXVelocity(0.); setYVelocity(0.);
		logf(LOG_GAME_LOW, "mobile[%p] has stopped\n", this);
		}
}






/***** users orders *********/

void playerMobUnit::u_goto(QPoint mpos) // not the same as QCanvasSprite::moveTo
{
	if (target) {
		disconnect(target, 0, this, 0); // target isn't connected to 'this' anymore
		target = 0l;
		//puts("u_goto disconnecting target");
	}
	failed_move = 0; path.reset();
	do_goto(mpos/BO_TILE_SIZE);
}
	
	
void playerMobUnit::do_goto(QPoint _dest)
{
	dest = _dest;
	if ( gridRect().topLeft() == dest )
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
	QPoint	p;

	if (mobileProp[type].range<=0) return;		// Unit can't shoot

	bosonUnit *that = this;
	if (u == that) { /* attacking myself */
//		logf(LOG_WARNING, "(playerMobUnit::u_attack()) %p attacking itself, aborting", this);
		return;
	}

	bosonUnit::u_attack(u);

	connect( u, SIGNAL(sig_moveTo(QPoint)), this, SLOT(targetMoveTo(QPoint)) );

	if (u->inherits("playerMobUnit"))
		p = ((playerMobUnit*)u)->center();
	else if (u->inherits("playerFacility"))
		p = ((playerFacility*)u)->center();
	else {
		logf(LOG_ERROR, "u_attack, what's this bosonUnit ???");
		return;
	}

	failed_move = 0; path.reset();
	do_goto(p/BO_TILE_SIZE);
}


void playerMobUnit::targetMoveTo(QPoint npos)
{
	do_goto(npos);
}


void playerMobUnit::s_shooted(int _power)
{
	power = _power;
	bocanvas->play("shoot.wav");
	if (sp_up) sp_up->setFrame(_power);
}
  
void playerMobUnit::s_destroy(void)
{
	setFrame(PIXMAP_MOBILE_DESTROYED);
	setZ( Z_DESTROYED_MOBILE );
	bocanvas->unsetCellFlag ( gridRect() , (BO_GO_AIR==goFlag())? Cell::flying_unit_f:Cell::field_unit_f );
	_destroyed = true;
	bocanvas->play("mobile_destroyed.wav");
	unSelect();
	emit dying(this);
}

/*
 * playerFacility
 */
playerFacility::playerFacility(facilityMsg_t *msg)
	: visualFacility(msg)
{
	bocanvas->setCellFlag ( gridRect(), Cell::building_f );

}

playerFacility::~playerFacility()
{
	//puts("~playerFacility");
	emit dying(this);
}


void playerFacility::s_setState(int s)
{
	boAssert(frame()==s-1);
	setFrame(s);
}


void playerFacility::getWantedAction()
{
	bosonMsgData	data;

	if (who != who_am_i) return;
	if ( !visible() ) return;

	if (getWantedShoot(&data)) {
		data.shoot.key		= key;
		sendMsg(buffer, MSG_UNIT_SHOOT, MSG(data.shoot) );
	}
}


void playerFacility::s_shooted(int _power)
{
	power = _power;
	bocanvas->play("shoot.wav");
	if (sp_up) sp_up->setFrame(_power);
}
  
void playerFacility::s_destroy(void)
{
	setFrame(PIXMAP_FIX_DESTROYED);
	setZ( Z_DESTROYED_FACILITY );
	bocanvas->unsetCellFlag ( gridRect(), Cell::building_f );
	_destroyed = true;
	bocanvas->play("fix_destroyed.wav");
	unSelect();
	emit dying(this);
}


void playerFacility::u_attack(bosonUnit *u)
{
	if (facilityProp[type].range<=0) return;	// Unit can't shoot

	bosonUnit *that = this;
	if (u == that) { /* attacking myself */
//		logf(LOG_WARNING, "(playerFacility::u_attack()) %p attacking itself, aborting", this);
		return;
	}

	bosonUnit::u_attack(u);

	connect( u, SIGNAL(sig_moveTo(QPoint)), this, SLOT(targetMoveTo(QPoint)) );
}


void playerFacility::u_stop(void)
{
	stop_attacking();
}


bool playerFacility::getWantedShoot(bosonMsgData *msg)
{
	int	range = facilityProp[type].range;

	if (range<=0) return false;		// Unit can't shoot

	// how far are we ? 
	Unit *_target;
	if (!_getWantedShoot(_target)) return false;

	QPoint p = _target->center()  - QPoint( x(), y());
	if ( boDist(p) > range) return false; // too far

	// ok, let's shoot it
	msg->shoot.target_key = _target->key;
	return true;
}

#define underlyingGround() bocanvas->groundAt( gridRect().topLeft() )

/*
 * harvester 
 */
bool harvesterUnit::getWantedMove(QPoint &wstate)
{
	bool ret = false;

	switch(hstate) {
		case standBy:
			return false;
			break;
		case comingBack:
			if ( atHome() ) {
				/* we are back : empty the harvester */ 
//				puts("harvester : arrived home");
				harvestEndMsg_t    he;
				he.key = key;
				sendMsg(buffer, MSG_UNIT_HARVEST_END, MSG(he) );
				hstate = goingTo;
				playerMobUnit::u_goto(harvest*BO_TILE_SIZE); // go to harvest point
				contain = 0 ;		// emptying

			}
			return playerMobUnit::getWantedMove(wstate);
			break;
		case goingTo:
			ret = playerMobUnit::getWantedMove(wstate);
			if ( dest == gridRect().topLeft() && underlyingGround() == myHarvestGround() ) {
				hstate = harvesting;
//				puts("harvester : change to \"harvesting\" state");
			} else // nothing to harvest
			if ( dest == gridRect().topLeft() ) { 
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
				playerMobUnit::u_goto(home*BO_TILE_SIZE); // go to home station
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

void harvesterUnit::u_goto(QPoint npos)
{
	hstate = goingTo;
	harvest = npos/BO_TILE_SIZE;
//	puts("harvester : change to \"goingTo\" state");
	playerMobUnit::u_goto(npos);
}

