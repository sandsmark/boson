/***************************************************************************
                          bobuffer.h  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Sun Jun  6 17:35:00 CET 1999
                                           
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

#ifndef BOBUFFER_H 
#define BOBUFFER_H 

#define BOSON_BUFFER_SIZE	(5*1024)

class boBuffer {

public:
	boBuffer(int socket, unsigned int size=BOSON_BUFFER_SIZE);
	~boBuffer();

	void reset(void) { pos = 0; };
	void flush(void);
	void packInt(int value);
	void packInt(int *tab, unsigned int len);

	int	socket;
private :
	int	*data;
	unsigned int pos;
	unsigned int posMax;
};

#endif // BOBUFFER_H
