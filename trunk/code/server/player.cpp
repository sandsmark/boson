/***************************************************************************
                          player.h  -  description                    
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

#include "../common/msgData.h"
#include "../common/bobuffer.h"
#include "../common/log.h"

#include "player.h"
#include "game.h"


Player::Player(void)
{
	socket = (KSocket *) 0L;
	socketState = SSS_NO_CONNECT;
	name = new QString("Orzel Land");

	lastConfirmedJiffies = 1;

	fixUnitDestroyed = 0;
	mobUnitDestroyed = 0;
	UnitDestroyed = 0;
	
	mineral = BO_INITIAL_MINERAL;
	oil	= BO_INITIAL_OIL;
	
	needFlushing = true;
}



void Player::flush(void)
{
	static bosonMsgData	msg;	// static so that no need to reallocate it at every call

	/* ressources have changed ? */
	if (needFlushing) {
		msg.ressources.mineral	= mineral;
		msg.ressources.oil	= oil;
		sendMsg(buffer, MSG_PERSO_RESSOURCES, sizeof(msg.ressources), &msg);
	}
	

	/* synchro */
	msg.jiffies = jiffies;
	sendMsg(buffer, MSG_TIME_INCREASE, sizeof(msg.jiffies), &msg);
	boAssert(lastConfirmedJiffies == (jiffies-1));

	buffer->flush();	// actually send datas to the player
	needFlushing = false;
}

void Player::changeOil(int delta)
{
	oil += delta;
	needFlushing = true;
}


void Player::changeMineral(int delta)
{
	mineral += delta;
	needFlushing = true;
}


