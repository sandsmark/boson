/***************************************************************************
                          knownBy.h  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Tue Jun  1 ??:??:?? CET 1999
                                           
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

#ifndef KNOWNBY_H 
#define KNOWNBY_H 

#include "common/msgData.h"	// bosonMsgTag

#ifndef ulong
typedef unsigned long ulong;
#endif


#define getPlayerMask(a)	(1l<<(a))

class knownBy
{

public:
	knownBy() { exist = known = 0l; } 

	bool existAt(ulong mask) { return (exist & mask); }
	bool isKnownBy(ulong mask) { return (known & mask); }
	void setKnown(ulong mask) { known |= mask; exist |=mask ;}
	void unSetKnown(ulong mask) { known &= ~mask; }

// protected:
	/* implemented in serverUnit.cpp */
	void sendToKnown(bosonMsgTag tag, int blen, void *data);
// private :
 	ulong	known;
	ulong	exist;

};

#endif // KNOWNBY_H
