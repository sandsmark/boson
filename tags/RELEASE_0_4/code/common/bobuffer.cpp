/***************************************************************************
                          bobuffer.cpp  -  description                    
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

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>

#include "bobuffer.h"
#include "log.h"


boBuffer::boBuffer(int s, unsigned int size)
{
	posMax = size;
	socket = s;
	data = new int[size];
	assert(data!=0);

	reset();

}


boBuffer::~boBuffer()
{
	delete [] data;
}


void boBuffer::flush(void)
{
	int i;

	assert(socket>0);
	assert(pos< posMax);

	if (0 == pos) {
		logf(LOG_WARNING, "boBuffer::flush : flushing empty buffer");
		return;
		}

	i = write(socket, data, pos*sizeof(int));
	assert(i == (int)(sizeof(int)*(pos)));

	logf(LOG_LAYER1, "[socket %2d ] Flushing %d out of %d (%f %%)", socket, pos, posMax, (float)(100.*pos/posMax));

	reset();
}

void boBuffer::packInt(int val)
{
	data[pos++] = htonl(val);
	boAssert(pos < posMax);
}

void boBuffer::packInt(int *tab, unsigned int len)
{
	unsigned int i;
	for(i=0; i<len; i++) packInt(tab[i]);
}


