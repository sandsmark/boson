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

#include <kmessagebox.h>
#include <ksock.h>

#include "boson.h"
#include "connect.h"

#include "common/log.h"
#include "common/bobuffer.h"

#include "playerUnit.h"
#include "bosonTopLevel.h"
#include "bosonCanvas.h"
#include "game.h"


void BosonApp::connectionLost(KSocket *s)
{
	boAssert ( s == Socket);
	logf(LOG_ERROR, "connectionLost called");

	delete Socket;

  	KMessageBox::error(0l,
			"Connection with the server has been lost\n"
			"Game is over :-(",
			"Server Connection Error");
	
	socketState	= PSS_CONNECT_DOWN;
	State		= PS_NO_CONNECT;
}



void BosonApp::handleSocketMessage(KSocket *s)
{
playerSocketState oldState = socketState;
bosonMsgTag	tag;
bosonMsgData	data;
int		blen;

boAssert ( s == Socket);
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
			State = PS_WAIT_ANSWER;
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
playerState oldState = State;

if ( tag>MSG_END_DIALOG_LAYER )
	if ( PS_PLAYING == State) {
		bocanvas->handleGameMessage(tag,blen,data);
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


if ( MSG_DLG_END == tag) {
	boAssert(PS_PLAYING == State);
	logf(LOG_ERROR, "peer-client died");
	gameEnd( data->end.endReason);
	return;
}

switch(State) {
	default:
		UNKNOWN_TAG(State);

	case PS_NO_CONNECT :
		logf(LOG_WARNING, "handleDialogMessage : tag received while in PS_NO_CONNECT, ignored");
		break;

	case PS_WAIT_ANSWER :
		switch(tag) {
		case MSG_DLG_ACCEPTED :
			State = PS_WAIT_BEGIN;
			boAssert(sizeof(data->accepted) == blen);
			logf(LOG_INFO, "Server has accepted our request, map is (%d,%d)",
				data->accepted.sizeX,
				data->accepted.sizeY);
			who_am_i	= data->accepted.who_you_are;

			/* actual creation of the bocanvas */
			initCanvas (data->accepted.sizeX, data->accepted.sizeY, data->accepted.total_player);
			myspecy	= species[data->accepted.who_you_are];
			break;
		case MSG_DLG_REFUSED :
			State = PS_NO_CONNECT;
			logf(LOG_INFO, "Connection refused by server");
			break;
		default:
			UNKNOWN_TAG(State);
		};
		break;

	case PS_WAIT_BEGIN :
		TRANSITION2(MSG_DLG_BEGIN, PS_PLAYING, BOSON_NO_TAG);
		UNKNOWN_TAG(State);
	}

if (oldState != State)
	logf(LOG_LAYER2, "Player : State has changed from %d to %d", oldState, State);
}


void BosonApp::gameEnd( endMsg_t::endReasonType reason)
{
	logf(LOG_ERROR, "gameEnd called");
	if ( endMsg_t::playerDiedEnd != reason) {
		logf(LOG_ERROR, "unknown reason for gameEnd from server");
	}

	delete Socket;

  	KMessageBox::error(0l,
			"Connection with one of the other player has been lost\n"
			"Game is over :-(",
			"Some other player has quitted");
	
	socketState	= PSS_CONNECT_DOWN;
	State		= PS_NO_CONNECT;

}


void bosonCanvas::handleGameMessage(bosonMsgTag tag, int blen, bosonMsgData *data)
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
		requestAction();
		// latest message is MSG_TIME_CONFIRM
		sendMsg(buffer, MSG_TIME_CONFIRM, MSG(data->jiffies) );
		logf(LOG_COMM, "flush : jiffies++ : %u", jiffies);
		buffer->flush();
		update();		// QCanvas periodical rendering

		// ping handling 
		{ time_t t = time(NULL);
		ping = t - last_sync;
		last_sync = t; }
		emit pingUpdated((int)ping);
		if (--radar_pulse<0) {
			radar_pulse = RADAR_PULSE_PERIOD;
			play("radar_pulse.wav");
		}
		break;

	case MSG_MAP_DISCOVERED :
		ASSERT_DATA_BLENGHT(sizeof(data->coo));
		logf(LOG_GAME_LOW, "received MSG_MAP_DISCOVERED : (%d,%d) = %d",
			data->coo.x, data->coo.y, (int)data->coo.c );
		setCell(data->coo.x, data->coo.y, data->coo.c);
		return;
		break;

	case MSG_FACILITY_CREATED :
		ASSERT_DATA_BLENGHT(sizeof(data->facility));
		logf(LOG_GAME_HIGH, "Facility(%d) created at %d,%d, key=%d, state=%d", 
			(int)data->facility.type,
			data->facility.x,
			data->facility.y,
			data->facility.key,
			data->facility.state
			);
		createFix(data->facility);

		/* center all TopLevels on CMDBUNKER creation */
		if ( FACILITY_CMDBUNKER == data->facility.type && data->facility.who == who_am_i)
			for ( bosonTopLevel *btl=topLevels.first(); btl != 0; btl=topLevels.next() )
				btl->reCenterView( QPoint(data->facility.x, data->facility.y) );

		break;

	case MSG_FACILITY_UNHIDDEN :
		ASSERT_DATA_BLENGHT(sizeof(data->facility));
		unHideFix(data->facility);
		break;
		
	case MSG_UNIT_POWER:
		ASSERT_DATA_BLENGHT(sizeof(data->power));
		shooted(data->power);
		break;
		
	case MSG_UNIT_RESS:
		ASSERT_DATA_BLENGHT(sizeof(data->unitRess));
		updateRess(data->unitRess);
		break;

	case MSG_FACILITY_CHANGED :
		ASSERT_DATA_BLENGHT(sizeof(data->fixChanged));
		logf(LOG_GAME_HIGH, "Facility(key=%d) changed to %d", 
			data->fixChanged.key,
			data->fixChanged.state
			);
		f = getFacility(data->fixChanged.key);
		if (f) {
			f->s_setState(data->fixChanged.state);
			}
		else logf(LOG_ERROR, "MSG_FACILITY_CHANGED : unexpected key:%d",data->fixChanged.key);
		break;


	case MSG_FACILITY_HIDDEN :
		ASSERT_DATA_BLENGHT(sizeof(data->destroyed));
		logf(LOG_GAME_HIGH, "Facility(%d) destroyed", data->destroyed.key);
		hideFix(data->destroyed);
		break;


	case MSG_FACILITY_DESTROYED :
		ASSERT_DATA_BLENGHT(sizeof(data->destroyed));
		logf(LOG_GAME_HIGH, "Facility(%d) destroyed", data->destroyed.key);
		destroyFix(data->destroyed);
		break;


	case MSG_MOBILE_UNHIDDEN :
		ASSERT_DATA_BLENGHT(sizeof(data->mobile));
		unHideMob(data->mobile);
		break;



	case MSG_MOBILE_CREATED :
		ASSERT_DATA_BLENGHT(sizeof(data->mobile));
		fflush(stdout);
		logf(LOG_GAME_HIGH, "mobile(%d) created at %d,%d", 
			(int)data->mobile.type,
			data->mobile.x,
			data->mobile.y
			);
		createMob(data->mobile);
		break;

	case MSG_MOBILE_HIDDEN :
		ASSERT_DATA_BLENGHT(sizeof(data->destroyed));
		hideMob(data->destroyed);
		break;

	case MSG_MOBILE_DESTROYED :
		ASSERT_DATA_BLENGHT(sizeof(data->destroyed));
		logf(LOG_GAME_HIGH, "mobile(%d) destroyed", data->destroyed.key);
		destroyMob(data->destroyed);
		break;

	case MSG_MOBILE_MOVE_C :
		ASSERT_DATA_BLENGHT(sizeof(data->move));
		move(data->move);
		break;

	case MSG_UNIT_SHOOT :
		ASSERT_DATA_BLENGHT(sizeof(data->shoot));
		shoot(data->shoot);
		break;

	case MSG_PERSO_RESSOURCES :
		ASSERT_DATA_BLENGHT(sizeof(data->ressources));
		/* syncing */
		oil	= data->ressources.oil;
		mineral = data->ressources.mineral;
		emit oilUpdated( oil );
		emit mineralUpdated( mineral );
		break;

	} // switch
}


