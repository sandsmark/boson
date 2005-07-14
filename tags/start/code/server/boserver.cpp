/***************************************************************************
                          boserver.cpp  -  description                    
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

#include <stdio.h>
#include "boserver.h"

///orzel: temp, pour la creation...
#include "../map/map.h"

FILE *logfile = (FILE *) 0L;

BosonServer::BosonServer(const char *mapfile, const char *name=0L)
	: KTMainWindow(name)
{

//nbPlayer = 2;
nbConnected = 0;

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
	player[i].socketState = SSS_NO_CONNECT;
	player[i].server = this;
	player[i].id = i;
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
   if (SSS_NO_CONNECT == player[i].socketState) {

	/* Memorize it */
	player[i].socketState = SSS_INIT;
	player[i].socket = newConnection;
	player[i].buffer = new boBuffer(newConnection->socket(), BOSON_BUFFER_SIZE );
	player[i].lastConfirmedJiffies = 0;

	logf(LOG_COMM,"new incoming connection, put in slot[%d]", i);


	logf(LOG_COMM,"socket = %d, addr = %lu",
		newConnection->socket(), newConnection->getAddr());

	/* Signal handling */
	connect(
		newConnection, SIGNAL(closeEvent(KSocket *)),
		this, SLOT(clientClose(KSocket*)) );

	connect(
		newConnection, SIGNAL(readEvent(KSocket *)),
		&player[i], SLOT(handleSocketMessage(KSocket*)) );
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
			nbConnected = 1;

			data->accepted.who_you_are = playerId;
			data->accepted.missing_player = nbPlayer - 1;;
			data->accepted.total_player = nbPlayer;
			data->accepted.sizeX = map.height;
			data->accepted.sizeY = map.width;;
			sendMsg(player[playerId].buffer, MSG_DLG_ACCEPTED, sizeof(data->accepted), data);
			break;
			}
		UNKNOWN_TAG(state);

	case SS_WAITING :
		if (MSG_DLG_ASK == tag ) {
			nbConnected++;
			data->accepted.who_you_are = playerId;
			data->accepted.missing_player = nbPlayer-nbConnected;
			data->accepted.total_player = nbPlayer;
			data->accepted.sizeX = map.height;
			data->accepted.sizeY = map.width;;
			sendMsg(player[playerId].buffer, MSG_DLG_ACCEPTED, sizeof(data->accepted), data);
			if (nbPlayer == nbConnected) {

				/* first of all : tell 'verybody that the game is beginning*/
				for(i= 0; i < nbPlayer; i++)
					sendMsg(player[i].buffer, MSG_DLG_BEGIN, BOSON_NO_DATA);

				/* then initialize the game */
				state		= SS_PLAYING;
				initPeople();
				logf(LOG_INFO, "Game is beginning");

				/* Beginning of synchronization */
				jiffies		= 1;
				confirmedJiffies= 0;
				data->jiffies	= jiffies;
				boAssert(jiffies == 1); ///orzel : well...
				for(i=0; i<nbPlayer; i++) {
				   sendMsg(player[i].buffer, MSG_TIME_INCREASE, sizeof(data->jiffies), data);
				   player[i].buffer->flush();
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
			mob->r_moveBy(data->move, playerId, player[playerId].buffer);
  			checkUnitVisibility(mob);
			}
		else logf(LOG_ERROR, "handleGameMessage : unexpected mobile key in moveMsg_t : %d", data->move.key);
		break;
	case MSG_TIME_CONFIRM :
		ASSERT_DATA_BLENGHT(sizeof(data->jiffies));
		boAssert(player[playerId].lastConfirmedJiffies == (jiffies-1));
//if (!(player[playerId].lastConfirmedJiffies == (jiffies-1)))
//	printf("jiffies = %d, lastconfirmed = %d\n", jiffies, player[playerId].lastConfirmedJiffies);
		boAssert(data->jiffies == jiffies);
		player[playerId].lastConfirmedJiffies = jiffies;
		confirmedJiffies++;
		if (confirmedJiffies == nbPlayer) {
			usleep(1000*100); ///orzel histoire de pas faire peter les jiffies en attendant qu'il y ait un vrai TimeOut

			/* get all wanted action from everybody */
			requestAction();
			/* increment jiffies */
			jiffies++;
			confirmedJiffies = 0 ; /* nobody until now has confirmed this new jiffies */

			/* tell everybody, and flush outgoing buffers */
			data->jiffies = jiffies;
			for(i=0; i<nbPlayer; i++) {
				boAssert(player[i].lastConfirmedJiffies == (jiffies-1));
				sendMsg(player[i].buffer, MSG_TIME_INCREASE, sizeof(data->jiffies), data);
				player[i].buffer->flush();
				}
			/* log */
			logf(LOG_GAME_LOW, "Jiffies++ : %u", jiffies);
			}
		
		break;
	};

// logf(LOG_INFO, "handleGameMessage : receiving tag %d from player %d, quite normal", tag, playerId);

}


void BosonServer::initPeople(void)
{
	int i;

	for (i=0; i<people.nbMobiles; i++)
		createMobUnit(people.mobile[i]);
	for (i=0; i<people.nbFacilities; i++)
		createFixUnit(people.facility[i]);
}
