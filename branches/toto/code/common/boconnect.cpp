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
#include "log.h"

#define RECV_BUFFER_LEN		150

static	int	recv_buffer[RECV_BUFFER_LEN];

#define recvPacket(socket, ilen) read((socket), recv_buffer, (ilen)*sizeof(int))

#if 1
#define unpackInt(val)  (val) = ntohl(recv_buffer[i++])
#else
/* occhio, out of date concerning variable names */
#define unpackInt(val)  {(val) = ntohl(socket_buffer[i++]); logf(LOG_LAYER0, "\t[unpacked(%d) %d ]", i-1, val); }
#endif

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
 
/*** pack tag and len ***/
buffer->packInt(tag);
buffer->packInt(ilen);

assert(blen <= sizeof(bosonMsgData) );
if ( blen%sizeof(int) )
	logf(LOG_WARNING, "boconnect : sendMsg : lenght % sizeof(int) is not 0");
assert( 0 == blen % sizeof(int) );

/*** pack data ***/
buffer->packInt(data->data, ilen);

/*** pack checksum ***/
buffer->packInt(computeChkSum(tag, ilen, data));

/* for level "socket" and "dialog", there's no buffering */
if (tag < MSG_END_DIALOG_LAYER) buffer->flush();

logf ( LOG_LAYER0, "Sent msg tag = %d, ilen = %d", tag, ilen);

return 0;
}


int recvMsg(boBuffer *buffer, bosonMsgTag &tag, int &blen, bosonMsgData *data)
{
int i=0;
int ilen;
int k ;
int sum;

logf ( LOG_LAYER0, "Receiving msg");

assert(buffer->socket>0);

i = 0;
k = recvPacket(buffer->socket,2);
assert ( (2*sizeof(int)) == k );

unpackInt(tag);
assert(tag >= 0);
assert(tag < MSG_LAST);

unpackInt(ilen);
assert(ilen >= 0);
blen = ilen * sizeof(int);

assert(blen <= sizeof(bosonMsgData) );
if (blen > sizeof(bosonMsgData) ) blen = sizeof(bosonMsgData);

if (ilen > 0 && tag < MSG_END_SOCKET_LAYER)
	logf(LOG_WARNING, "Unexpected data in a socket layer message, ignored");


k = recvPacket(buffer->socket, ilen+1); /* +1 is checksum */
//printf("BOBOBOBOBOBO k = %d, ilen+1 = %d\n", k, ilen+1);
i = 0; assert(k == ((ilen +1)*(int)sizeof(int)));

for (k=0; k< ilen; k++)
	unpackInt(data->data[k]);

unpackInt(sum);

if (sum != computeChkSum(tag, ilen, data))
	logf(LOG_FATAL, "Beuh.. uncorrect checksum\n");
//else	logf(LOG_LAYER0, "recvMsg : correct CheckSum\n");

logf ( LOG_LAYER0, "\tdone -> tag = %d, ilen = %d", tag, ilen);

return 0;
}
