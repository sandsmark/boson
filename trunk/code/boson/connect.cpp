/***************************************************************************
                          connect.cpp  -  description                              
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


#include <assert.h>
#include "boson.h"
#include "connect.h"
#include "../common/log.h"
#include "../common/bobuffer.h"

#include "game.h"

void BosonApp::handleSocketMessage(KSocket *s)
{
playerSocketState oldState = socketState;
bosonMsgTag	tag;
bosonMsgData	data;
int		blen;

assert ( s == socket);
recvMsg (buffer, tag, blen, &data);

if ( tag == MSG_END_SOCKET_LAYER) {
	logf(LOG_ERROR, "euh.. received MSG_END_SOCKET_LAYR tag, ignoring\n");
	return;
	}

if ( tag > MSG_END_SOCKET_LAYER)
	if (PSS_CONNECT_OK == socketState) {
		handleDialogMessage(tag,blen, &data);
		return;
		}
	else {
		logf(LOG_ERROR, "Receiving Server message while not in CONNECT_OK state, ignoring\n");
		return;
		}

boAssert(0==blen);

switch(socketState) {
	default : 
		TRANSITION(MSG_SYNC_ASK,  PSS_SYNC_OTHER, MSG_SYNC_ACK1);

		// UNKNOWN_TAG;

		logf(LOG_FATAL, "handleSocketMessage:: GRAVE  unknown state, aborting\n");
		exit(-1);
		break; /* anyway ? */
	
	case PSS_WAIT_CONFIRM_INIT :
		if (MSG_HS_INIT_OK == tag ) {
			state = PS_WAIT_ANSWER;
			sendMsg(buffer, MSG_DLG_ASK, BOSON_NO_DATA);
			}
		TRANSITION(MSG_HS_INIT_OK, PSS_CONNECT_OK, BOSON_NO_TAG);
		UNKNOWN_TAG(socketState);

	case PSS_SYNC_ME :
		TRANSITION(MSG_SYNC_ACK1, PSS_CONNECT_OK, MSG_SYNC_ACK2);
		UNKNOWN_TAG(socketState);

	case PSS_SYNC_OTHER :
		TRANSITION(MSG_SYNC_ACK2, PSS_CONNECT_OK, BOSON_NO_TAG);
		UNKNOWN_TAG(socketState);
		
	}

if (oldState != socketState)
	logf(LOG_LAYER1, "Player : socketState has changed from %d to %d", oldState, socketState);
}



void BosonApp::handleDialogMessage(bosonMsgTag tag, int blen, bosonMsgData *data)
{
playerState oldState = state;

if ( tag>MSG_END_DIALOG_LAYER )
    if ( PS_PLAYING == state) {
	handleGameMessage(tag,blen,data);
	return;
	}
    else {
	logf(LOG_ERROR, "receiving Game-related tag while not in PS_PLAYING state, ignored");
	return;
	}

if ( ! (tag>MSG_END_SOCKET_LAYER && tag<MSG_END_DIALOG_LAYER)) {
	logf(LOG_ERROR, "handleDialogMessage : unexpected tag received(1), ignored");
	return;
	}

switch(state) {
	default:
		UNKNOWN_TAG(state);

	case PS_NO_CONNECT :
		logf(LOG_WARNING, "handleDialogMessage : tag received while in PS_NO_CONNECT, ignored");
		break;

	case PS_WAIT_ANSWER :
		switch(tag) {
		case MSG_DLG_ACCEPTED :
			state = PS_WAIT_BEGIN;
			boAssert(sizeof(data->accepted) == blen);
			logf(LOG_INFO, "Server has accepted our request, map is (%d,%d)",
				data->accepted.sizeX,
				data->accepted.sizeY);
gpp.who_am_i = data->accepted.who_you_are;
gpp.nb_player = data->accepted.total_player;
gpp.myspecies = gpp.species[data->accepted.who_you_are];
			break;
		case MSG_DLG_REFUSED :
			state = PS_NO_CONNECT;
			logf(LOG_INFO, "Connection refused by server");
			break;
		default:
			UNKNOWN_TAG(state);
		};
		break;

	case PS_WAIT_BEGIN :
		TRANSITION2(MSG_DLG_BEGIN, PS_PLAYING, BOSON_NO_TAG);
		UNKNOWN_TAG(state);
	}

if (oldState != state)
	logf(LOG_LAYER2, "Player : state has changed from %d to %d", oldState, state);
}


void BosonApp::handleGameMessage(bosonMsgTag tag, int blen, bosonMsgData *data)
{
playerFacility * f;

//if ( ! tag>MSG_END_DIALOG_LAYER || ! tag<MSG_LAST) {
if ( ! tag>MSG_END_DIALOG_LAYER) { ///orzel why does the previous one not work ?
	logf(LOG_ERROR, "handleGameMessage : unexpected tag received(1), ignored");
	return;
	}

switch(tag) {
	default:
		UNKNOWN_TAG(-1434);

	case MSG_TIME_INCREASE :
		ASSERT_DATA_BLENGHT(sizeof(data->jiffies));
		jiffies ++;
		boAssert(jiffies == data->jiffies);
	// let's each object speaks
		phys->requestAction(buffer);
	// latest message is MSG_TIME_CONFIRM
		sendMsg(buffer, MSG_TIME_CONFIRM, sizeof(data->jiffies), data);
		logf(LOG_GAME_LOW, "flush : jiffies++ : %u", jiffies);
		buffer->flush();
		phys->update();		// QwSpriteField periodical rendering
		break;

	case MSG_MAP_DISCOVERED :
		ASSERT_DATA_BLENGHT(sizeof(data->coo));
		logf(LOG_GAME_LOW, "received MSG_MAP_DISCOVERED : (%d,%d) = %d",
			data->coo.x, data->coo.y, (int)data->coo.g );
		phys->setCell(data->coo.x, data->coo.y, data->coo.g);
		return;
		break;

	case MSG_FACILITY_CREATED :
		ASSERT_DATA_BLENGHT(sizeof(data->facility));
		logf(LOG_GAME_HIGH, "Facility(%d) created at %d,%d, key=%d, state=%d", 
			(int)data->facility.type,
			data->facility.x,
			data->facility.y,
			data->facility.key
			data->facility.state
			);
		phys->createFix(data->facility);
		break;

	case MSG_FACILITY_CHANGED :
		ASSERT_DATA_BLENGHT(sizeof(data->fixChanged));
		logf(LOG_GAME_HIGH, "Facility(key=%d) changed to %d", 
			data->fixChanged.key,
			data->fixChanged.state
			);
		f = phys->getFacility(data->fixChanged.key);
		if (f) {
			f->s_setState(data->fixChanged.state);
			}
		else logf(LOG_ERROR, "MSG_FACILITY_CHANGED : unexpected key:%d",data->fixChanged.key);
		break;


	case MSG_FACILITY_DESTROYED :
		ASSERT_DATA_BLENGHT(sizeof(data->destroyed));
		logf(LOG_GAME_HIGH, "Facility(%d) destroyed", data->destroyed.key);
		phys->destroyFix(data->destroyed);
		break;


	case MSG_MOBILE_CREATED :
		ASSERT_DATA_BLENGHT(sizeof(data->mobile));
		printf("mobile_created : who = %d\n", data->mobile.who);
		fflush(stdout);
		logf(LOG_GAME_HIGH, "mobile(%d) created at %d,%d", 
			(int)data->mobile.type,
			data->mobile.x,
			data->mobile.y
			);
		phys->createMob(data->mobile);
		break;

	case MSG_MOBILE_DESTROYED :
		ASSERT_DATA_BLENGHT(sizeof(data->destroyed));
		logf(LOG_GAME_HIGH, "mobile(%d) destroyed", data->destroyed.key);
		phys->destroyMob(data->destroyed);
		break;

	case MSG_MOBILE_MOVE_C :
		ASSERT_DATA_BLENGHT(sizeof(data->move));
		phys->move(data->move);
		break;

	}


}
