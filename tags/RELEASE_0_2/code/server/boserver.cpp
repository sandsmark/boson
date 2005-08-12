/***************************************************************************
                          boserver.cpp  -  description                    
                             -------------------                                         

    version              : $Id$
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

#include <stdio.h>

#include "../common/bobuffer.h"
#include "../common/map.h"	 ///orzel: temp, pour la creation...

#include "boserver.h"
#include "game.h"

FILE *logfile = (FILE *) 0L;

BosonServer::BosonServer(const char *mapfile, const char *name=0L)
	: KTMainWindow(name)
{

//gpp.nbPlayer = 2;
gpp.nbConnected = 0;

initLog();
logf(LOG_INFO, "Entering BosonServer constructor");

initSocket();
logf(LOG_INFO, "Socket is initialized");

initMap(mapfile);
logf(LOG_INFO, "Map is initialized");

}


void BosonServer::initLog(void)
{

logfile = fopen(BOSON_LOGFILE_SERVER, "a+b");
if (!logfile) {
	logfile = stderr;
	logf(LOG_ERROR, "Can't open logfile, using stderr");
	}

logf(LOG_INFO,"============New Log File===============");
}


void BosonServer::initSocket()
{
int i;

for(i=0; i<BOSON_MAX_CONNECTION; i++) {
	gpp.player[i].socketState = SSS_NO_CONNECT;
	gpp.player[i].server = this;
	gpp.player[i].id = i;
	}

socket = new KServerSocket(BOSON_DEFAULT_PORT);
state = SS_INIT;

if (-1 == socket->socket()) {
	logf(LOG_FATAL, "socket == -1, no connection");
	return;
	}
else
	logf(LOG_COMM, "KserverSocket ok", socket->socket());
	logf(LOG_COMM, "\tsocket is %d, port = %u, address = %lu"
		, socket->socket(), socket->getPort(), socket->getAddr());

connect(
	socket, SIGNAL(accepted(KSocket *)),
	this, SLOT(handleNewConnection(KSocket *))  );
}




BosonServer::~BosonServer()
{
logf(LOG_INFO, "Closing logfile normally\n+++\n\n");
//if (logfile != stderr) close(logfile);
}



void BosonServer::handleNewConnection(KSocket *newConnection)
{
int i;

for(i=0; i<BOSON_MAX_CONNECTION; i++)
   if (SSS_NO_CONNECT == gpp.player[i].socketState) {

	/* Memorize it */
	gpp.player[i].socketState = SSS_INIT;
	gpp.player[i].socket = newConnection;
	gpp.player[i].buffer = new boBuffer(newConnection->socket(), BOSON_BUFFER_SIZE );
	gpp.player[i].lastConfirmedJiffies = 0;

	logf(LOG_COMM,"new incoming connection, put in slot[%d]", i);


	logf(LOG_COMM,"socket = %d, addr = %lu",
		newConnection->socket(), newConnection->getAddr());

	/* Signal handling */
	connect(
		newConnection, SIGNAL(closeEvent(KSocket *)),
		this, SLOT(clientClose(KSocket*)) );

	connect(
		newConnection, SIGNAL(readEvent(KSocket *)),
		&gpp.player[i], SLOT(handleSocketMessage(KSocket*)) );
	newConnection->enableRead(TRUE);

	return;
	}
logf(LOG_ERROR, "No place left in player[], connection not even opened");
delete newConnection;
}


void BosonServer::handleDialogMessage(int playerId, bosonMsgTag tag, int len, bosonMsgData *data)
{
serverState oldState = state;
uint i;

if ( tag>MSG_END_DIALOG_LAYER )
    if ( SS_PLAYING == state) {
	handleGameMessage(playerId,tag,len,data);
	return;
	}
    else {
	logf(LOG_ERROR, "receiving Game-related tag while not in SS_PLAYING state, ignored\n");
	return;
	}

if ( ! (tag>MSG_END_SOCKET_LAYER && tag<MSG_END_DIALOG_LAYER)) {
	logf(LOG_ERROR, "handleDialogMessage : unexpected tag received(1), ignored\n");
	return;
	}

switch(state) {
	default:
		UNKNOWN_TAG(state);

	case SS_INIT :
		if (MSG_DLG_ASK == tag) {
			state = SS_WAITING;
			gpp.nbConnected = 1;

			data->accepted.who_you_are = playerId;
			data->accepted.missing_player = gpp.nbPlayer - 1;;
			data->accepted.total_player = gpp.nbPlayer;
			data->accepted.sizeX = map_height;
			data->accepted.sizeY = map_width;;
			sendMsg(gpp.player[playerId].buffer, MSG_DLG_ACCEPTED, sizeof(data->accepted), data);
			break;
			}
		UNKNOWN_TAG(state);

	case SS_WAITING :
		if (MSG_DLG_ASK == tag ) {
			gpp.nbConnected++;
			data->accepted.who_you_are = playerId;
			data->accepted.missing_player = gpp.nbPlayer-gpp.nbConnected;
			data->accepted.total_player = gpp.nbPlayer;
			data->accepted.sizeX = map_height;
			data->accepted.sizeY = map_width;;
			sendMsg(gpp.player[playerId].buffer, MSG_DLG_ACCEPTED, sizeof(data->accepted), data);
			if (gpp.nbPlayer == gpp.nbConnected) {

				/* first of all : tell 'verybody that the game is beginning*/
				for(i= 0; i < gpp.nbPlayer; i++)
					sendMsg(gpp.player[i].buffer, MSG_DLG_BEGIN, BOSON_NO_DATA);

				/* then initialize the game */
				state		= SS_PLAYING;
				loadUnits();
				logf(LOG_INFO, "Game is beginning");

				/* Beginning of synchronization */
				gpp.jiffies	= 1;
				confirmedJiffies= 0;
				data->jiffies	= gpp.jiffies;
				boAssert(gpp.jiffies == 1); ///orzel : well...
				for(i=0; i<gpp.nbPlayer; i++) {
				   sendMsg(gpp.player[i].buffer, MSG_TIME_INCREASE, sizeof(data->jiffies), data);
				   gpp.player[i].buffer->flush();
				   }
				}
			break;
			}
		UNKNOWN_TAG(state);

	}

if (oldState != state)
	logf(LOG_LAYER2, "State has changed from %d to %d", oldState, state);
}



void BosonServer::handleGameMessage(int playerId, bosonMsgTag tag, int blen, bosonMsgData *data)
{
uint i;
serverMobUnit *mob;

if ( ! tag>MSG_END_DIALOG_LAYER) {
	logf(LOG_ERROR, "handleGameMessage : unexpected tag received(1), ignored\n");
	return;
	}

switch(tag) {
	default :
		UNKNOWN_TAG(-3535);
	case MSG_MOBILE_MOVE_R :
		ASSERT_DATA_BLENGHT(sizeof(data->move));
		mob = mobile.find(data->move.key);
		if (mob) {
			mob->r_moveBy(data->move, playerId, gpp.player[playerId].buffer);
  			checkUnitVisibility(mob);
			}
		else logf(LOG_ERROR, "handleGameMessage : unexpected mobile key in moveMsg_t : %d", data->move.key);
		break;
	case MSG_TIME_CONFIRM :
		ASSERT_DATA_BLENGHT(sizeof(data->jiffies));
		boAssert(gpp.player[playerId].lastConfirmedJiffies == (gpp.jiffies-1));
		boAssert(data->jiffies == gpp.jiffies);
		gpp.player[playerId].lastConfirmedJiffies = gpp.jiffies;
		confirmedJiffies++;
		if (confirmedJiffies == gpp.nbPlayer) {
			usleep(50*1000); ///orzel histoire de pas faire peter les jiffies en attendant qu'il y ait un vrai TimeOut

			/* get all wanted action from everybody */
			requestAction();
			/* check knwon state with new mobiles' position distro */
			checkKnownState();
			/* increment jiffies */
			gpp.jiffies++;
			confirmedJiffies = 0 ; /* nobody until now has confirmed this new jiffies */

			/* tell everybody, and flush outgoing buffers */
			data->jiffies = gpp.jiffies;
			for(i=0; i<gpp.nbPlayer; i++) {
				boAssert(gpp.player[i].lastConfirmedJiffies == (gpp.jiffies-1));
				sendMsg(gpp.player[i].buffer, MSG_TIME_INCREASE, sizeof(data->jiffies), data);
				gpp.player[i].buffer->flush();
				}
			/* log */
			logf(LOG_COMM, "Jiffies++ : %u", gpp.jiffies);
			}
		
		break;
	};

// logf(LOG_INFO, "handleGameMessage : receiving tag %d from player %d, quite normal", tag, playerId);

}
