/***************************************************************************
                          boserver.cpp  -  description                    
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

#include <stdio.h>
#include <stdlib.h>	// exit() atoi()

#include <qgroupbox.h>

#include <ksock.h>
#include <kstddirs.h>
#include <kmessagebox.h>

#include "common/bobuffer.h"
#include "common/unitType.h"

#include "boserver.h"
#include "game.h"

FILE *logfile = (FILE *) 0L;
			
// orzel :  prevent a warning, will be removed
extern "C" { extern void usleep(unsigned long); }


BosonServer::BosonServer(int port, const char *mapfile, const char *name)
	: KMainWindow(0l, name)
{
	QLabel		*label;
	QGroupBox	*box;
	QString		buf;

	/* GUI */
	resize( 120+180+10, 160);		// boserver-gui is 100x100

		/* box */
	box	= new QGroupBox(this);
	box->setGeometry( this->rect());
	
		/* pix */
	label = new QLabel(box);
	label->move( 10, 10);		// biglogo is 352x160
	label->setAutoResize(true);
	label->setPixmap( QPixmap( locate("data", "boson/pics/boserver-gui.bmp")) );

		/* port */
	buf.sprintf("listening on port %d", port);
	label	= new QLabel(buf, box);
	//label->setAlignment(AlignCenter);
	label->setGeometry( 120, 20, 180, 30);

  	l_state = new QLabel("Waiting for first connection", box);
	l_state->setGeometry( 120, 50, 180, 30);

  	l_connected = new QLabel("Nobody is connected", box);
	l_connected->setGeometry( 120, 80, 180, 30);

	/* initialisation */

	nbConnected = 0;

	initLog();
	logf(LOG_INFO, "Entering BosonServer constructor");

	initSocket(port);
	if ( SS_DOWN == state) {
 		KMessageBox::error(0l,
				"Network error : server can't bind to socket\n"
				"Check that no other boserver is running",
				"boserver ERROR"
				);
		exit(-1);
		return;
	}
	logf(LOG_INFO, "Socket is initialized");

	initMap(mapfile);
	logf(LOG_INFO, "Map is initialized");
	
	
	/* GUI again */
	buf.sprintf("Scenario : \"%s\" for %d players", worldName->data(), nbPlayer);
	label	= new QLabel(buf, box);
	//label->setAlignment(AlignVCenter | AlignLeft);
	label->setGeometry( 10, 120, 250, 30);

	/* we are ready to handle new connection */
	printf(BOSON_SERVER_LAUNCHED); fflush (stdout);
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


void BosonServer::initSocket(int port)
{
int i;

for(i=0; i<BOSON_MAX_CONNECTION; i++) {
	player[i].socketState = SSS_NO_CONNECT;
	player[i].id = i;
	}

socket = new KServerSocket( port);
state = SS_INIT;

if (-1 == socket->socket()) {
	logf(LOG_FATAL, "socket == -1, no connection");
	state = SS_DOWN;
	return;
	}
else
	logf(LOG_COMM, "KserverSocket ok", socket->socket());
	logf(LOG_COMM, "\tsocket = %d, port = %u, address = %lu",
			socket->socket(), socket->port(), socket->ipv4_addr());

connect(
	socket, SIGNAL(accepted(KSocket *)),
	this, SLOT(handleNewConnection(KSocket *))  );

}




BosonServer::~BosonServer()
{
	logf(LOG_INFO, "Closing logfile normally\n+++\n\n");
	// raise a bug : 
	//     after this line, the server loop on the previous
	//     line forever, filling the logfile
	// if (logfile != stderr) close(logfile);
}



void BosonServer::handleNewConnection(KSocket *newConnection)
{
int i;

for(i=0; i<BOSON_MAX_CONNECTION; i++)
   if (SSS_NO_CONNECT == player[i].socketState) {

	/* Memorize it */
	player[i].socketState = SSS_INIT;
	player[i].socket = newConnection;
	player[i].buffer = new boBuffer(newConnection->socket());
	player[i].lastConfirmedJiffies = 0;

	logf(LOG_INFO, "New incoming connection, put in slot[%d]", i);
	logf(LOG_COMM,"\tsocket = %d, addr = %lu",
			newConnection->socket(), newConnection->ipv4_addr());

	/* Signal handling */
	connect( newConnection, SIGNAL(readEvent(KSocket *)),
		&player[i], SLOT(handleSocketMessage(KSocket*)) );
	connect( newConnection, SIGNAL(closeEvent(KSocket *)),
		&player[i], SLOT(connectionLost(KSocket*)) );
	newConnection->enableRead(TRUE);

	return;
	}
logf(LOG_ERROR, "No place left in player[], connection not even opened");
delete newConnection;
}


void BosonServer::handleDialogMessage(uint playerId, bosonMsgTag tag, int len, bosonMsgData *data)
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
		logf(LOG_ERROR, "handleDialogMessage : unknown state received : %d", state);
		break;

	case SS_INIT :
		if (MSG_DLG_ASK == tag) {
			state = SS_WAITING;
			l_state->setText("waiting for other players");
			l_connected->setText("One player connected");
			nbConnected = 1;

			data->accepted.who_you_are = playerId;
			data->accepted.missing_player = nbPlayer - 1;;
			data->accepted.total_player = nbPlayer;
			data->accepted.sizeX = map_width;;
			data->accepted.sizeY = map_height;
			sendMsg(player[playerId].buffer, MSG_DLG_ACCEPTED, MSG(data->accepted) );
			break;
			}
		UNKNOWN_TAG(state);

	case SS_WAITING :
		if (MSG_DLG_ASK == tag ) {
			QString buf;
			buf.sprintf("%d players connected", ++nbConnected);
			l_connected->setText(buf);
			data->accepted.who_you_are = playerId;
			data->accepted.missing_player = nbPlayer-nbConnected;
			data->accepted.total_player = nbPlayer;
			data->accepted.sizeX = map_height;
			data->accepted.sizeY = map_width;;
			sendMsg(player[playerId].buffer, MSG_DLG_ACCEPTED, MSG(data->accepted) );
			if (nbPlayer == nbConnected) {

				/* first of all : tell 'verybody that the game is beginning*/
				for(i= 0; i < nbPlayer; i++)
					sendMsg(player[i].buffer, MSG_DLG_BEGIN, BOSON_NO_DATA);

				/* then initialize the game */
				state		= SS_PLAYING;
				l_state->setText("Game is begin played");
				loadUnits();
				logf(LOG_INFO, "Game is beginning");

				/* Beginning of synchronization */
				jiffies	= 1;
				confirmedJiffies= 0u;
				data->jiffies	= jiffies;
				boAssert(jiffies == 1); ///orzel : well...
				for(i=0; i<nbPlayer; i++) {
				   sendMsg(player[i].buffer, MSG_TIME_INCREASE, MSG(data->jiffies) );
				   player[i].buffer->flush();	// buffer is flushed, not the player->flush().
				   }
				}
			break;
			}
		UNKNOWN_TAG(state);

	}

if (oldState != state)
	logf(LOG_LAYER2, "State has changed from %d to %d", oldState, state);
}



void BosonServer::handleGameMessage(uint playerId, bosonMsgTag tag, int blen, bosonMsgData *data)
{
	uint i;
	serverMobUnit	*mob;
	serverFacility	*fix;
	/* static to cut off heap allocation overhead */
	static facilityMsg_t	_facility;
	static mobileMsg_t	_mobile;

if ( ! tag>MSG_END_DIALOG_LAYER) {
	logf(LOG_ERROR, "handleGameMessage : unexpected tag received(1), ignored\n");
	return;
	}

switch(tag) {
	default :
		logf(LOG_ERROR, "handleGameMessage : unknown tag received : %d", tag);
		break;

	case MSG_UNIT_HARVEST :
		ASSERT_DATA_BLENGHT(sizeof(data->harvest));
		mob = mobile.find(data->harvest.key);
		if (mob) switch(mob->getType()) {
				case MOB_OIL_HARVESTER:
				case MOB_MINERAL_HARVESTER:
					mob->increaseContain();
					break;
				default:
					logf(LOG_ERROR, "non harvester unit trying to harvest");
					break;
		} else logf(LOG_ERROR, "handleGameMessage : unexpected key in harvestMsg_t(1) : %d", data->harvest.key);
		break;

	case MSG_UNIT_HARVEST_END :
		ASSERT_DATA_BLENGHT(sizeof(data->harvestEnd));
		mob = mobile.find(data->harvestEnd.key);
		if (mob) switch(mob->getType()) {
				case MOB_OIL_HARVESTER:
				case MOB_MINERAL_HARVESTER:
					((serverHarvester*)mob)->emptying();
					break;
				default:
					logf(LOG_ERROR, "non harvester unit trying to harvest");
					break;
		} else logf(LOG_ERROR, "handleGameMessage : unexpected key in harvestMsg_t(2) : %d", data->harvest.key);
		break;

	case MSG_UNIT_SHOOT :
		ASSERT_DATA_BLENGHT(sizeof(data->shoot));
		mob = mobile.find(data->shoot.target_key);
		fix = facility.find(data->shoot.target_key);
		if (mob) {
			/* shooting a mobile */
			mob->sendToKnown(MSG_UNIT_SHOOT, MSG(data->shoot) );
			if (mob->shooted())		// must be last, cause then, mob must be irrelevant (unit has died)
				mobile.remove(data->shoot.target_key);
		} else if (fix) {
			/* shooting a facility */
			fix->sendToKnown(MSG_UNIT_SHOOT, MSG(data->shoot) );
			if (fix->shooted())		// must be last, cause then, fix must be irrelevant (unit has died)
				facility.remove(data->shoot.target_key);
		} else	logf(LOG_ERROR, "handleGameMessage : unexpected target_key in shootMsg_t : %d", data->shoot.target_key);

		break;

	case MSG_MOBILE_CONSTRUCT :
		ASSERT_DATA_BLENGHT(sizeof(data->mobConstruct));
		// found constructor
		fix = facility.find(data->mobConstruct.key_constructor);
		if (!fix) {
			logf(LOG_ERROR, "handleGameMessage : unknown key_constructor in MSG_MOBILE_CONSTRUCT: %d", data->mobConstruct.key_constructor);
			break;
		}
		// check who
		if (fix->who !=playerId) {
			logf(LOG_ERROR, "handleGameMessage : unexpected owner(%d) sending MSG_MOBILE_CONSTRUCT to (owner:%d) facility", playerId, fix->who);
			break;
		}
		// check ressources
		if (!player[playerId].changeRessources(
					- mobileProp[_mobile.type].cost_oil,
					- mobileProp[_mobile.type].cost_mineral))
			break;
		fix->u_createMob(data->mobConstruct.type);
		break;
		
	case MSG_FACILITY_CONSTRUCT :
		ASSERT_DATA_BLENGHT(sizeof(data->fixConstruct));
		_facility.who	= playerId;
		_facility.x	= data->fixConstruct.x;
		_facility.y	= data->fixConstruct.y;
		_facility.type	= data->fixConstruct.type;
		if (player[playerId].changeRessources(
					- facilityProp[_facility.type].cost_oil,
					- facilityProp[_facility.type].cost_mineral))
			createFixUnit(_facility);
		else logf(LOG_ERROR, "client %d has tried to create a %d facility without enough ressources (%d,%d)",
				playerId, _facility.type, player[playerId].oil, player[playerId].mineral);
		break;

	case MSG_MOBILE_MOVE_R :
		ASSERT_DATA_BLENGHT(sizeof(data->move));
		mob = mobile.find(data->move.key);
		if (mob) {
			mob->r_moveBy(data->move, playerId);
  			checkUnitVisibility(mob);
			}
		else logf(LOG_ERROR, "handleGameMessage : unexpected mobile key in moveMsg_t : %d", data->move.key);
		break;

	case MSG_TIME_CONFIRM :
		ASSERT_DATA_BLENGHT(sizeof(data->jiffies));
		boAssert(player[playerId].lastConfirmedJiffies == (jiffies-1));
		boAssert(data->jiffies == jiffies);
		player[playerId].lastConfirmedJiffies = jiffies;
		confirmedJiffies++;
		if (confirmedJiffies == nbPlayer) {
			usleep(50*1000); ///orzel histoire de pas faire peter les jiffies en attendant qu'il y ait un vrai TimeOut

			/* get all wanted action from everybody */
			requestAction();
			/* check knwon state with new mobiles' position distro */
			checkKnownState();
			/* increment jiffies */
			jiffies++;
			confirmedJiffies = 0 ; /* nobody until now has confirmed this new jiffies */

			/* tell everybody, and flush outgoing buffers */
			for(i=0; i<nbPlayer; i++) player[i].flush();

			/* log */
			logf(LOG_COMM, "Jiffies++ : %u", jiffies);
			}
		
		break;
	};

// logf(LOG_INFO, "handleGameMessage : receiving tag %d from player %d, quite normal", tag, playerId);

}

