/***************************************************************************
                          bobuffer.h  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Sun Jun  6 17:35:00 CET 1999
                                           
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

#ifndef BOBUFFER_H 
#define BOBUFFER_H 

class boBuffer {

public:
	boBuffer(int socket, unsigned int size);

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
