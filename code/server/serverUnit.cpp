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
#include "serverUnit.h"

#include "../common/bobuffer.h"

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

sendMsg(buffer, MSG_MOBILE_MOVE_C, sizeof(msg), (bosonMsgData*)(&msg));
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
	sendMsg ( buffer, MSG_FACILITY_CHANGED, sizeof(msg), (bosonMsgData*)(&msg));
	}
}
