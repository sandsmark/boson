/***************************************************************************
                          connect.cpp  -  description                    
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

#include <assert.h>
#include <stdlib.h> //exit

#include <ksock.h>

#include "boserver.h"
#include "connect.h"
#include "game.h"

#include "common/bobuffer.h"

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

void BosonServer::clientClose(KSocket *s)
{
int i;
QString buf;

for(i=0; i<BOSON_MAX_CONNECTION; i++)
   if (player[i].socket == s) {

	player[i].socketState = SSS_NO_CONNECT;
	delete player[i].buffer;
	delete s;
	assert(nbConnected>0);

	buf.sprintf("%d players connected", --nbConnected);
	l_connected->setText(buf);
	if (nbConnected<1) {
		state = SS_INIT ;
  		l_state->setText("Waiting for first connection");
	}
	logf( LOG_INFO, "Connection[%d] has closed", i);
	return;

	}
logf(LOG_FATAL, "Unknown client has closed.");
delete s;
}
