/***************************************************************************
                          boconnect.cpp  -  description                    
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
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>

#include "msgData.h"
#include "bobuffer.h"
#include "log.h"

#define RECV_BUFFER_LEN		150

static	int	recv_buffer[RECV_BUFFER_LEN];

#define recvPacket(socket, ilen) read((socket), recv_buffer, (ilen)*sizeof(int))
#define unpackInt(val)  (val) = ntohl(recv_buffer[i++])

static int computeChkSum(int tag, int ilen, bosonMsgData *data)
{
	int sum = 0;
	int i;

	sum ^= tag;	sum <<= 5;
	sum ^= ilen;	sum <<= 5;

	for (i=0; i< ilen; i++)
		sum ^= data->data[i];

	return sum;
}


int sendMsg(boBuffer *buffer, bosonMsgTag tag, int blen, bosonMsgData *data)
{
	int ilen = blen / sizeof(int);

	if (BOSON_NO_TAG == tag) return 0;
 
	assert(blen <= (int)sizeof(bosonMsgData) );
	if ( blen%sizeof(int) )
		logf(LOG_WARNING, "boconnect : sendMsg : lenght % sizeof(int) is not 0");
	assert( 0 == blen % sizeof(int) );

	buffer->packInt(tag);
	buffer->packInt(ilen);
	buffer->packInt(data->data, ilen);
	buffer->packInt(computeChkSum(tag, ilen, data));

	/* for level "socket" and "dialog", there's no buffering */
	if (tag < MSG_END_DIALOG_LAYER) buffer->flush();

	logf ( LOG_LAYER0, "Sent msg tag = %d, ilen = %d", tag, ilen);

	return 0;
}


int recvMsg(boBuffer *buffer, bosonMsgTag &tag, int &blen, bosonMsgData *data)
{
	int i=0, ilen, k, sum;

	logf ( LOG_LAYER0, "Receiving msg");
	assert(buffer->socket>0);

	/* receive tag&len */
	k = recvPacket(buffer->socket,2); i = 0;
	boAssert ( (2*sizeof(int)) == k );

	unpackInt(k);
	tag = (bosonMsgTag)k;
	boAssert(tag >= 0);
	boAssert(tag < MSG_LAST);

	unpackInt(ilen);
	blen = ilen * sizeof(int);

	/* coherency check */
	assert(ilen >= 0);
	assert(blen <= (int)sizeof(bosonMsgData) );
	if (blen > (int)sizeof(bosonMsgData) ) blen = sizeof(bosonMsgData);
	if (ilen > 0 && tag < MSG_END_SOCKET_LAYER)
		logf(LOG_WARNING, "Unexpected data in a socket layer message, ignored");


	/* receive data&checksum */
	k = recvPacket(buffer->socket, ilen+1); /* +1 is checksum */
	i = 0;
	assert(k == ((ilen +1)*(int)sizeof(int)));
	for (k=0; k< ilen; k++)
		unpackInt(data->data[k]);

	unpackInt(sum);


	/* verify checksum */
	if (sum != computeChkSum(tag, ilen, data))
		logf(LOG_FATAL, "Beuh.. uncorrect checksum\n");
	//else	logf(LOG_LAYER0, "recvMsg : correct CheckSum\n");
	
	logf ( LOG_LAYER0, "\tdone -> tag = %d, ilen = %d", tag, ilen);
	return 0;
}
