/***************************************************************************
                        serverUnit.cpp  -  description                              
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

#include "../common/log.h"
#include "../common/map.h"
#include "../common/bobuffer.h"

#include "serverUnit.h"
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
		boAssert(i<3);
		if (k&1l) sendMsg ( player[i].buffer, tag, blen, data);
		}
}



/*
 * class serverUnit
 */

void serverUnit::increaseContain(void)
{
	unitRessMsg_t	m;

	contain ++;

	m.key = key; m.contain = contain;
	sendMsg(buffer, MSG_UNIT_RESS, sizeof(m), &m);

}

/*
 *  MOBILE 
 */

serverMobUnit::serverMobUnit(boBuffer *b, mobileMsg_t *msg, QObject* parent, const char *name)
	:mobUnit(msg,parent,name)
	,serverUnit(msg->key, b,msg->x, msg->y)
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
		sendToKnown( MSG_UNIT_POWER, sizeof(_power), &_power);
		return false;
	}

}



void serverMobUnit::r_moveBy(moveMsg_t &msg, int playerId, boBuffer * buffer)
{

	/* owner check */
	if (who!=playerId) {
		logf(LOG_ERROR, "Player %d asking to move player's %d unit, dismissed", playerId, who);
		return;
		}

	__x += msg.dx;
	__y += msg.dy;

	sendToKnown( MSG_MOBILE_MOVE_C, sizeof(msg), (bosonMsgData*)(&msg));
}

void serverMobUnit::reportCreated(void)
{
	mobileMsg_t     mobile;
		
	mobile.who = who; 
	mobile.key = key;
	mobile.x = __x;
	mobile.y = __y;
	mobile.type = type;

	sendToKnown( MSG_MOBILE_CREATED, sizeof(mobile), &mobile);
}


void serverMobUnit::reportCreated(int i)
{
	mobileMsg_t     mobile;
		
	mobile.who = who; 
	mobile.key = key;
	mobile.x = __x;
	mobile.y = __y;
	mobile.type = type;

	sendMsg ( player[i].buffer, MSG_MOBILE_CREATED, sizeof(mobile), &mobile);
}


void serverMobUnit::reportDestroyed(void)
{
	destroyedMsg_t  destroyed;

	destroyed.key = key;
	destroyed.x = __x;
	destroyed.y = __y;

	logf(LOG_WARNING, "serverUnit::shooted, key = %d", key);

	sendToKnown( MSG_MOBILE_DESTROYED  , sizeof(destroyed), &destroyed);
}


void serverMobUnit::reportDestroyed(int i)
{
	destroyedMsg_t  destroyed;

	destroyed.key = key;
	destroyed.x = __x;
	destroyed.y = __y;
	
	logf(LOG_WARNING, "MSG_MOBILE_ is %d MOBILE_DESTROYED sent : __x is %d", MSG_MOBILE_DESTROYED, __x);

	sendMsg ( player[i].buffer, MSG_MOBILE_DESTROYED, sizeof(destroyed), &destroyed);
}



/*
 * HARVESTER
 */
void serverHarvester::getWantedAction(void)
{
	if (counter<0) return;

	boAssert(counter<=EMPTYING_DURATION);
	if ( ! --counter ) {

		counter = -1; // not needed anymore
		server->placeMob(this); // replace it

		/* actual transfer */
		switch(type) {
			case MOB_MINERAL_HARVESTER:
				player[who].changeRessources(0, contain);
				break;
			case MOB_OIL_HARVESTER:
				player[who].changeRessources(contain, 0);
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
		printf("base is %d,%d, we are at %d,%d\n", base_x, base_y, _x(), _y());
		return;
	}
	/* destroy (hide) the client harvester */
	reportDestroyed();
	counter = EMPTYING_DURATION;

}



/*
 *  FACILITY
 */

serverFacility::serverFacility(boBuffer *b, facilityMsg_t *msg, QObject* parent, const char *name)
	:Facility(msg,parent,name)
	,serverUnit(msg->key, b,msg->x * BO_TILE_SIZE, msg->y * BO_TILE_SIZE)
{
	counter = BUILDING_SPEED;
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
		sendToKnown( MSG_UNIT_POWER, sizeof(_power), &_power);
		return false;
	}

}


void serverFacility::getWantedAction(void)
{
	fixChangedMsg_t msg;

	boAssert(state>=0);
	boAssert(state<CONSTRUCTION_STEP);
	if (state!=(CONSTRUCTION_STEP-1) && --counter <1) {
		counter = BUILDING_SPEED;
		state++;
		msg.key   = key;
		msg.state = state;
		sendToKnown (MSG_FACILITY_CHANGED, sizeof(msg), (bosonMsgData*)(&msg));
		}
}

void serverFacility::reportCreated(int i)
{
	facilityMsg_t   facility; 
		
	facility.who = who; 
	facility.key = key;
	facility.x = __x;
	facility.y = __y;
	facility.type = type;
	facility.state = state;

	sendMsg ( player[i].buffer, MSG_FACILITY_CREATED, sizeof(facility), &facility);
}

void serverFacility::reportDestroyed(void)
{

	destroyedMsg_t  destroyed;

	destroyed.key = key;
	destroyed.x = __x;
	destroyed.y = __y;

	logf(LOG_WARNING, "serverUnit::shooted, key = %d", key);

	sendToKnown( MSG_FACILITY_DESTROYED  , sizeof(destroyed), &destroyed);
}

void serverFacility::reportDestroyed(int i)
{
	destroyedMsg_t  destroyed;

	destroyed.key = key;
	destroyed.x = __x;
	destroyed.y = __y;

	logf(LOG_WARNING, "I'm %d, owned by %d, FACILITY_DESTROYED sent", key, who);
	sendMsg ( player[i].buffer, MSG_FACILITY_DESTROYED, sizeof(destroyed), &destroyed);
}




