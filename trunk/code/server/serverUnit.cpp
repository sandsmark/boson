/***************************************************************************
                        serverUnit.cpp  -  description                              
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

#include "common/log.h"
#include "common/bomap.h"
#include "common/bobuffer.h"

#include "boserver.h"
#include "player.h"
#include "game.h"


/*
 *  KNOWN_BY
 */
void knownBy::sendToKnown(bosonMsgTag tag, int blen, void *data)
{
	int i;
	ulong	k = known;

	for ( i=0; k; i++,k>>=1) {
		boAssert(i<BOSON_MAX_PLAYERS);
		if (k&1l) sendMsg ( player[i].buffer, tag, blen, data);
	}
}



/*
 * class serverUnit
 */
void serverMobUnit::increaseContain(void)
{
	unitRessMsg_t	m;

	switch( getType() ) {
		case MOB_MINERAL_HARVESTER:
			if (server->groundAt(gridRect().topLeft()) != GROUND_GRASS_MINERAL) {
				logf(LOG_ERROR, "mineral harvester trying to harvest on non mineral ground");
				return;
			}
			break;
		case MOB_OIL_HARVESTER:
			if (server->groundAt(gridRect().topLeft()) != GROUND_GRASS_OIL) {
				logf(LOG_ERROR, "oil harvester trying to harvest on non oil ground");
				return;
			}
			break;
		default:
			logf(LOG_ERROR, "unknown harvester type trying to harvest.aborting");
			return;
	}

	contain ++;

	m.key = key; m.contain = contain;
	sendMsg(buffer, MSG_UNIT_RESS, MSG(m) );

}



/*
 *  MOBILE 
 */
serverMobUnit::serverMobUnit(boBuffer *b, mobileMsg_t *msg)
	:mobUnit(msg)
	,serverUnit( b, (unitMsg_t*)msg)
{
}

bool serverMobUnit::shooted(void)
{
	if (--power <=0) {

		/* tell everybody that we no longer exist */
		reportDestroyed();
		return true;

	} else {
		/* broadcast the info */
		powerMsg_t	_power;
		
		_power.key	= key;
		_power.power	= power;
		sendToKnown( MSG_UNIT_POWER, MSG(_power) );
		return false;
	}

}


void serverMobUnit::r_moveBy(moveMsg_t &msg, uint playerId)
{

	/* owner check */
	if (who!=playerId) {
		logf(LOG_ERROR, "Player %d asking to move player's %d unit, dismissed", playerId, who);
		return;
		}
	

	// XXX some check here ! no cheat allowed...
	// speed check:

	int dx = __x - msg.newx;
	int dy = __y - msg.newy;
	//int speed = mobileProp[type].speed;
	dx *= dx; dy *= dy; // speed *= speed;
	boAssert( dx + dy < 2);

	if (msg.newx<0 || msg.newy<0 || msg.newx>=server->maxX() || msg.newy>=server->maxY()) {
		logf(LOG_ERROR, "client %d asking for (%d,%d) position, refused", playerId, msg.newx, msg.newy);
		return;
	}

	// accepted 
	__x = msg.newx;
	__y = msg.newy;
	sendToKnown( MSG_MOBILE_MOVE_C, MSG(msg) );
}


void serverMobUnit::reportCreated(int i)
{
	mobileMsg_t     mobile;
		
	fill(mobile);
	mobile.key = key;

	if ( SEND_TO_KNOWN == i )
		sendToKnown( MSG_MOBILE_CREATED, MSG(mobile) );
	else
		sendMsg( player[i].buffer, MSG_MOBILE_CREATED, MSG(mobile) );
}


void serverMobUnit::reportUnHidden(int i)
{
	mobileMsg_t     mobile;
		
	fill(mobile);
	mobile.key = key;

	if ( SEND_TO_KNOWN == i )
		sendToKnown( MSG_MOBILE_UNHIDDEN, MSG(mobile) );
	else {

		if ( ! existAt(getPlayerMask(i)) )
			reportCreated(i);
		else	sendMsg ( player[i].buffer, MSG_MOBILE_UNHIDDEN, MSG(mobile) );

		setKnown(getPlayerMask(i));
	}
}


void serverMobUnit::reportDestroyed(int i)
{

	destroyedMsg_t  destroyed;

	destroyed.key = key;
	destroyed.x = __x;
	destroyed.y = __y;

	logf(LOG_WARNING, "serverUnit::shooted, key = %d", key);

	if ( SEND_TO_KNOWN == i )
		sendToKnown( MSG_MOBILE_DESTROYED  , MSG(destroyed) );
	else
		sendMsg( player[i].buffer, MSG_MOBILE_DESTROYED  , MSG(destroyed) );

}


void serverMobUnit::reportHidden(int i)
{

	destroyedMsg_t  destroyed;

	destroyed.key = key;
	destroyed.x = __x;
	destroyed.y = __y;
	
	if ( SEND_TO_KNOWN == i )
		sendToKnown( MSG_MOBILE_HIDDEN, MSG(destroyed) );
	else {
		sendMsg ( player[i].buffer, MSG_MOBILE_HIDDEN, MSG(destroyed) );
		unSetKnown(getPlayerMask(i));
	}

}


QRect serverMobUnit::rect(void)
{
	QRect r = mobUnit::rect();
	r.moveBy( __x*BO_TILE_SIZE, __y*BO_TILE_SIZE );
	return r;
}



/*
 * HARVESTER
 */
void serverHarvester::getWantedAction(void)
{
	if (counter<0) return;

	boAssert(counter<=DURATION_EMPTYING);
	if ( ! --counter ) {

		counter = -1; // not needed anymore
		reportUnHidden(); // re-appear

		/* actual transfer */
		switch(type) {
			case MOB_MINERAL_HARVESTER:
				player[who].changeRessources(0, MINERAL_HARVESTER_CONTENT);
				break;
			case MOB_OIL_HARVESTER:
				player[who].changeRessources(OIL_HARVESTER_CONTENT, 0);
				break;
			default:
				logf(LOG_ERROR, "getWantedAction : unknown harvester type");
				break;
		}
		contain = 0;
	}

}


void serverHarvester::emptying(void)
{
	if (!atHome()) {
		logf(LOG_ERROR, "serverHarvesting::emptying while not at home, refused");
		return;
	}
	/* destroy (hide) the client harvester */
	reportHidden();
	counter = DURATION_EMPTYING;

}


/*
 *  FACILITY
 */
serverFacility::serverFacility(boBuffer *b, facilityMsg_t *msg)
	:Facility(msg)
	,serverUnit( b, (unitMsg_t*)msg)
{
	action = FIX_ACTION_BUILDING;
	counter = DURATION_BUILDING;
}


bool serverFacility::shooted(void)
{

	if (--power <=0) {

		reportDestroyed();
		return true;

	} else {
		/* broadcast the info */
		powerMsg_t	_power;
	
		_power.key	= key;
		_power.power	= power;
		sendToKnown( MSG_UNIT_POWER, MSG(_power) );
		return false;
	}

}


void serverFacility::getWantedAction(void)
{


	if (FIX_ACTION_NONE == action || --counter>0) return; // nothing to do
	switch(action) {
		case FIX_ACTION_NONE:
			break;
		case FIX_ACTION_BUILDING: {
			fixChangedMsg_t msg;
			boAssert(state>=0);
			boAssert(state<CONSTRUCTION_STEPS);
			if (state<CONSTRUCTION_STEPS-1) {
				// next building step
				counter = DURATION_BUILDING;
				state++;
				msg.key   = key;
				msg.state = state;
				sendToKnown (MSG_FACILITY_CHANGED, MSG(msg) );
			} else action = FIX_ACTION_NONE;	// finished
			break;
		}

		case FIX_ACTION_CONSTRUCT: {
			mobileMsg_t data;
			QPoint p = gridRect().center();

			data.who = who;
			data.x = p.x();
			data.y = p.y();
			data.type = action_data.mob;
			server->createMobUnit(data);

			logf(LOG_INFO, "creating a mobile unit");
			action = FIX_ACTION_NONE;	// finished
			break;
		}
	} // action
}


void serverFacility::reportUnHidden(int i)
{

	facilityMsg_t   facility; 

	fill(facility);
	facility.key = key;
	facility.state = state;

	if ( SEND_TO_KNOWN == i )
		sendToKnown (  MSG_FACILITY_UNHIDDEN, MSG(facility) );
	else {

		if ( ! existAt(getPlayerMask(i)) )
			reportCreated(i);
		else	sendMsg ( player[i].buffer, MSG_FACILITY_UNHIDDEN, MSG(facility) );

		setKnown(getPlayerMask(i));
	}

}


void serverFacility::reportCreated(int i)
{

	facilityMsg_t     facility;
		
	fill(facility);
	facility.key	= key;
	facility.state	= state;


	if ( SEND_TO_KNOWN == i )
		sendToKnown( MSG_FACILITY_CREATED, MSG(facility) );
	else
		sendMsg ( player[i].buffer, MSG_FACILITY_CREATED, MSG(facility) );

}


void serverFacility::reportDestroyed(int i)
{

	destroyedMsg_t  destroyed;

	destroyed.key = key;
	destroyed.x = __x;
	destroyed.y = __y;

	logf(LOG_WARNING, "serverUnit::shooted, key = %d", key);


	if ( SEND_TO_KNOWN == i )
		sendToKnown( MSG_FACILITY_DESTROYED, MSG(destroyed) );
	else
		sendMsg ( player[i].buffer, MSG_FACILITY_DESTROYED  , MSG(destroyed) );

}


void serverFacility::reportHidden(int i)
{

	destroyedMsg_t  destroyed;

	destroyed.key = key;
	destroyed.x = __x;
	destroyed.y = __y;


	if ( SEND_TO_KNOWN == i )
		sendToKnown( MSG_FACILITY_HIDDEN, MSG(destroyed) );
	else {
		sendMsg ( player[i].buffer, MSG_FACILITY_HIDDEN, MSG(destroyed) );
		unSetKnown(getPlayerMask(i));
	}
}


QRect serverFacility::rect(void)
{
	QRect r = Facility::rect();
	r.moveBy( __x*BO_TILE_SIZE, __y*BO_TILE_SIZE );
	return r;
}


void serverFacility::u_createMob(mobType t)
{
	if ( FACILITY_WAR_FACTORY != type ) {
		logf(LOG_ERROR, "u_createMob called while not a FACILITY_WAR_FACTORY");
		return;
	}
	if (FIX_ACTION_NONE != action) {
		logf(LOG_ERROR, "u_createMob called whith action already in progress");
		return;
	}
	action = FIX_ACTION_CONSTRUCT;
	counter = DURATION_CONSTRUCT;
	action_data.mob = t;

}


void serverFacility::u_createFix(facilityType )
{
	if ( FACILITY_CMDBUNKER != type ) {
		logf(LOG_ERROR, "u_createFix called while not a FACILITY_CMDBUNKER");
		return;
	}
	logf(LOG_ERROR, "u_createFix not implemented yet");
}


