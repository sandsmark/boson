/***************************************************************************
                          msgData.h  -  description                    
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

#ifndef MSGDATA_H 
#define MSGDATA_H 

#include "../common/groundType.h"
#include "../common/unitType.h"
#include "../common/refused.h"

#define BOSON_NO_DATA	0, ((bosonMsgData *) 0L)

class boBuffer;

/* MSG_DLG_ASK */ ///orzel still unused
struct askMsg_t		{ int major, minor, patch; };
/* MSG_DLG_ACCEPTED */ /// orzel still unused
struct acceptedMsg_t	{ int who_you_are, missing_player, total_player, sizeX, sizeY; };
/* MSG_DLG_REFUSED */ /// orzel still unused
struct refusedMsg_t	{ refusedType why_not; };
/* MSG_MAP_ */
struct cooMsg_t		{ int x, y; groundType g; };
/* MSG_FACILITY_CREATED */
struct facilityMsg_t	{ int who, key, x, y, state; facilityType type; };
/* MSG_FACILITY_CHANGED */
struct fixChangedMsg_t	{ int key, state; };
/* MSG_MOBILE_CREATED */
struct mobileMsg_t	{ int who, key, x, y; mobType type; };
/* MSG_MOBILE_MOVE_*  */
struct moveMsg_t	{ int key, dx, dy ;};
/* MSG_*_DESTROYED */
struct destroyedMsg_t	{ int key, x, y; }; // x and y are for checking 

typedef union {
/* Dialog layer */
	askMsg_t	ask;
	acceptedMsg_t	accepted;
	refusedMsg_t	refused;
/* game layer */
	cooMsg_t	coo;
	facilityMsg_t	facility;
	fixChangedMsg_t	fixChanged;
	mobileMsg_t	mobile;
	moveMsg_t	move;
	destroyedMsg_t  destroyed;
/* MSG_TIME */
	unsigned int	jiffies;
/* used by  {send,recv}Msg */
	int	data[1];
	} bosonMsgData;

/*
test made on LinuxPPC on february 3rd 1999 : 
   sizeof(bosonMsgData) = 40
   sizeof(bosonMsgData.coo) = 12
*/

/* layer 1 transition */
#define TRANSITION(signal,newState,emit)	\
	if (signal == tag) {			\
	socketState = newState;			\
	if ( BOSON_NO_TAG != emit )		\
		sendMsg(buffer, emit, BOSON_NO_DATA);  \
	break;  }

/* layer 2 transition */

#define TRANSITION2(signal,newState,emit)	\
	if (signal == tag) {			\
	state = newState;			\
	if ( BOSON_NO_TAG != emit )		\
		sendMsg(buffer, emit, BOSON_NO_DATA);  \
	break;  }

/* Misc. */

#define UNKNOWN_TAG(State)	{		\
	logf(LOG_COMM, "Unknown tag(%d) while in " #State  " %d (file %s, line %d) -> ignored", \
	tag, State, __FILE__, __LINE__);	\
	break;					\
	}

#define ASSERT_DATA_BLENGHT(wanted_blen)	\
	if (blen != wanted_blen)		\
	logf(LOG_ERROR, "unexpected data blengh : %d, wanted %d, line %d, file %s", \
	blen, wanted_blen, __LINE__, __FILE__ );

enum bosonMsgTag {

	BOSON_NO_TAG = -1,

/** Here are the message tags used for the underlying socket communiation */

/* General */
	MSG_NOK=0,
	MSG_OK=1,


/* Handshake */
	MSG_HS_INIT,
	MSG_HS_INIT_OK,
	MSG_HS_,

/* Socket Synchronization */
	MSG_SYNC_ASK,
	MSG_SYNC_ACK1,
	MSG_SYNC_ACK2,
	MSG_SYNC,

	MSG_END_SOCKET_LAYER,

/** Here begins message tags used for the client/server communication */

/* Initial dialog */

	MSG_DLG_ASK,
	MSG_DLG_ACCEPTED,
	MSG_DLG_REFUSED,
	MSG_DLG_BEGIN,
	MSG_DLG_OK,
	MSG_DLG_,

	MSG_END_DIALOG_LAYER,

/** Those messages are used by the game engine */

/* "time" synchronization */
	MSG_TIME_INCREASE,
	MSG_TIME_CONFIRM,
	MSG_TIME_,

/* Map management */

	MSG_MAP_DISCOVERED,
	MSG_MAP_,

/* Facility management */

	MSG_FACILITY_CREATED,
	MSG_FACILITY_DESTROYED,
	MSG_FACILITY_CHANGED,
	MSG_FACILITY_,

/* General units management */
	MSG_MOBILE_MOVE_R,	// Request
	MSG_MOBILE_MOVE_C,	// confirm
	MSG_MOBILE_CREATED,
	MSG_MOBILE_DESTROYED,

/* Player's units management */
	MSG_UNIT_POWER,
	MSG_UNIT_DAMAGE,

/* Player's facilities management */
	MSG_FACILITY_POWER,
	MSG_FACILITY_DAMAGE,

/* Player's global management */
	MSG_PERSO_MONEY,
	MSG_PERSO_PETROL,
	MSG_PERSO_WOOD,
	MSG_PERSO_KNOWELDGE,
	MSG_PERSO_,

	MSG_LAST,
	};

/* ilen is for integer-lenght */
/* blen is for byte-lenght */

int	sendMsg  (boBuffer *, bosonMsgTag tag, int blen, bosonMsgData *data);
int	recvMsg  (boBuffer *, bosonMsgTag &tag, int &blen, bosonMsgData *data);

#endif // MSGDATA_H
