/***************************************************************************
                          connect.cpp  -  description                    
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

#include <assert.h>
#include <stdlib.h> // exit
#include <unistd.h> // sleep

#include <ksock.h>

#include "boserver.h"
#include "connect.h"
#include "game.h"

#include "common/bobuffer.h"
#include "common/msgData.h"


void Player::connectionLost(KSocket *s)
{
	assert ( s == socket);
	socketState = SSS_CONNECT_DOWN;
	server->playerHasDied(id);
}


void Player::handleSocketMessage(KSocket *s)
{
serverSocketState oldState = socketState;
bosonMsgTag	tag;
bosonMsgData	data;
int		len;


assert ( s == socket);

logf(LOG_LAYER1, "[%d] Receiving new msg", id);
recvMsg (buffer, tag, len, &data);

if ( tag == MSG_END_SOCKET_LAYER) {
	logf(LOG_ERROR, "[%d] Received MSG_END_SOCKET_LAYER tag, ignoring", id);
	return;
	}

if ( tag > MSG_END_SOCKET_LAYER)
	if (SSS_CONNECT_OK == socketState) {
		server->handleDialogMessage(id,tag,len, &data);
		return;
		}
	else {
		logf(LOG_ERROR, "[%d] Receiving client message while not in CONNECT_OK state, ignoring", id);
		return;
		}

assert(0==len);

switch(socketState) {
	default : 
		if ( MSG_SYNC_ASK == tag)
			logf(LOG_INFO, "[%d] Sync asked by peer", id);
		TRANSITION(MSG_SYNC_ASK,  SSS_SYNC_OTHER, MSG_SYNC_ACK1);

		// UNKNOWN_TAG;

		logf(LOG_FATAL, "[%d] handleSocketMessage : unknown state, aborting", id);
		exit(-1);
		break; /* anyway ? */
	
	case SSS_INIT :
		TRANSITION(MSG_HS_INIT, SSS_CONNECT_OK, MSG_HS_INIT_OK);
		UNKNOWN_TAG(socketState);

	case SSS_SYNC_ME :
		if ( MSG_SYNC_ACK1 == tag)
			logf(LOG_INFO, "[%d] Sync(me) ok", id);
		TRANSITION(MSG_SYNC_ACK1, SSS_CONNECT_OK, MSG_SYNC_ACK2);
		UNKNOWN_TAG(socketState);

	case SSS_SYNC_OTHER :
		if ( MSG_SYNC_ACK2 == tag)
			logf(LOG_INFO, "[%d] Sync(peer) ok", id);
		TRANSITION(MSG_SYNC_ACK2, SSS_CONNECT_OK, BOSON_NO_TAG);
		UNKNOWN_TAG(socketState);
		
	}

if (oldState != socketState)
	logf(LOG_LAYER1, "[%d] socketState has changed from %d to %d", id, oldState, socketState);
}

void BosonServer::playerHasDied(uint playerId)
{
	uint i;
	endMsg_t end;
	QString buf;

	end.endReason = endMsg_t::playerDiedEnd;
	logf( LOG_INFO, "Connection[%u] has closed", playerId);

	// closing connections
	for(i= 0; i < nbPlayer; i++) {
		if (i!=playerId) sendMsg(player[i].buffer, MSG_DLG_END,  MSG(end) );
	}
	sleep(1);	// let the messages reach clients
	for(i= 0; i < nbPlayer; i++) {
		player[i].socketState = SSS_CONNECT_DOWN;
		delete player[i].socket;
		delete player[i].buffer;
	}

	// gui
	assert(nbConnected>0);
	buf.sprintf("%d players connected", --nbConnected);
	l_connected->setText(buf);

	if (nbConnected<1) {
		state = SS_INIT ;
  		l_state->setText("Waiting for first connection");
	} else {
		state = SS_DOWN ;
  		l_state->setText("Server is down. Stopped");
	}

}

