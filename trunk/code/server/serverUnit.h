/***************************************************************************
                         serverUnit.h  -  description                              
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

#ifndef SERVERUNIT_H 
#define SERVERUNIT_H 

#include "../common/unit.h"
#include "../common/msgData.h"
#include "knownBy.h"


#define BUILDING_SPEED	40
class boBuffer;


class serverUnit {
 public:
	serverUnit(boBuffer *b, int x, int y) { buffer = b; __x=x; __y=y; state = 0; power = MAX_POWER; }

 protected:
 int		__x, __y;
 int		power;

 boBuffer	*buffer;
 int		state;
 int		counter;
};

/*
 *  MOBILE
 */

class serverMobUnit : public mobUnit, public serverUnit, public knownBy
{
 Q_OBJECT

public:
  serverMobUnit(boBuffer *, mobileMsg_t *msg, QObject* parent = 0L, const char *name=0L);
 virtual	int _x(void) {return __x;}
 virtual	int _y(void) {return __y;}
 void 	reportCreated(int player);
 void 	reportDestroyed(int player);
// void 	reportCreated(boBuffer *);
// void 	reportDestroyed(boBuffer *);

/* request */
 void	r_moveBy(moveMsg_t &, int playerId, boBuffer *);

 bool shooted();

};
 


/*
 *  FACILITY
 */
class serverFacility : public Facility, public serverUnit, public knownBy
{
 Q_OBJECT

 public:
  serverFacility(boBuffer *, facilityMsg_t *msg, QObject* parent = 0L, const char *name=0L);
 virtual	int _x(void) {return __x;}
 virtual	int _y(void) {return __y;}
 void 	reportCreated(int player);
 void 	reportDestroyed(int player);
// void 	reportCreated(int player);
// void 	reportDestroyed(boBuffer *);

/* request */
 void getWantedAction();

 bool shooted();
};
 

#endif // SERVERUNIT_H

