/***************************************************************************
                        serverUnit.cpp  -  description                              
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

#include "../common/log.h"
#include "../common/bobuffer.h"
#include "serverUnit.h"
#include "game.h"


/*
 *  KNOWN_BY
 */

void knownBy::sendToKnown(bosonMsgTag tag, int blen, bosonMsgData *data)
{
	int i;
	ulong	k = known;

	for ( i=0; k; i++,k>>=1) {
		boAssert(i<3);
		if (k&1l) sendMsg (
			gpp.player[i].buffer, tag, blen, data);
		}
}


/*
 *  MOBILE 
 */

serverMobUnit::serverMobUnit(boBuffer *b, mobileMsg_t *msg, QObject* parent, const char *name)
	:mobUnit(msg,parent,name)
	,serverUnit(b,msg->x, msg->y)
{
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


void serverMobUnit::reportCreated(boBuffer * b)
{
	bosonMsgData	data;
		
	data.mobile.who = who; 
	data.mobile.key = key;
	data.mobile.x = __x;
	data.mobile.y = __y;
	data.mobile.type = type;

	sendMsg ( b, MSG_MOBILE_CREATED, sizeof(data.mobile),	&data);
}

void serverMobUnit::reportDestroyed(boBuffer * b)
{
	bosonMsgData	data;

	data.destroyed.key = key;
	data.destroyed.x = __x;
	data.destroyed.y = __y;

	sendMsg ( b, MSG_MOBILE_DESTROYED, sizeof(data.destroyed), &data);
}





/*
 *  FACILITY
 */

serverFacility::serverFacility(boBuffer *b, facilityMsg_t *msg, QObject* parent, const char *name)
	:Facility(msg,parent,name)
	,serverUnit(b,msg->x, msg->y)
{
	counter = BUILDING_SPEED;
}


void serverFacility::getWantedAction()
{
	fixChangedMsg_t msg;

	boAssert(state>=0);
	boAssert(state<6);
	if (state!=5 && --counter <1) {
		counter = BUILDING_SPEED;
		state++;
		msg.key   = key;
		msg.state = state;
		sendToKnown (MSG_FACILITY_CHANGED, sizeof(msg), (bosonMsgData*)(&msg));
		}
}

void serverFacility::reportCreated(boBuffer * b)
{
	bosonMsgData	data;
		
	data.facility.who = who; 
	data.facility.key = key;
	data.facility.x = __x;
	data.facility.y = __y;
	data.facility.type = type;
	data.facility.state = state;

	sendMsg ( b, MSG_FACILITY_CREATED, sizeof(data.facility), &data);
}


void serverFacility::reportDestroyed(boBuffer * b)
{
	bosonMsgData	data;

	data.destroyed.key = key;
	data.destroyed.x = __x;
	data.destroyed.y = __y;

	sendMsg ( b, MSG_FACILITY_DESTROYED, sizeof(data.destroyed), &data);
}




