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

#include "common/unit.h"
#include "knownBy.h"


#define BUILDING_SPEED		(40)
#define EMPTYING_DURATION	(40)
#define SEND_TO_KNOWN		(-1)


class boBuffer;

class serverUnit : public knownBy
{
public:
	serverUnit(boBuffer *b, unitMsg_t *msg)
		{ buffer = b; __x=msg->x; __y=msg->y; state = 0; power = MAX_POWER; contain = 0; counter = -1; }

protected:
	boBuffer	*buffer;
	int		__x, __y;
	int		state;
	int		power;
	int		contain;
	int		counter;
};


/*
 *  MOBILE
 */
class serverMobUnit : public mobUnit, public serverUnit
{
public:
		serverMobUnit(boBuffer *, mobileMsg_t *msg);

	void 	reportCreated(int player = SEND_TO_KNOWN);
	void 	reportDestroyed(int player = SEND_TO_KNOWN);
	void 	reportUnHidden(int player = SEND_TO_KNOWN);
	void 	reportHidden(int player = SEND_TO_KNOWN);

/* request */
	void	r_moveBy(moveMsg_t &, uint playerId);

	virtual void	getWantedAction(void) {};
	bool		shooted(void);
	void		increaseContain(void );

	virtual QRect	rect(void);
};
 

class serverHarvester : public serverMobUnit
{
public:
	serverHarvester(boBuffer *b, mobileMsg_t *msg)
		:serverMobUnit( b, msg) { base_x = msg->x; base_y = msg->y; }
	
	void	emptying(void);
	virtual void	getWantedAction(void);

private:
	bool	atHome(void);
	int	base_x, base_y;
};


/*
 *  FACILITY
 */
class serverFacility : public Facility, public serverUnit
{
public:
		serverFacility(boBuffer *, facilityMsg_t *msg);

	void	reportCreated(int player = SEND_TO_KNOWN);
	void	reportDestroyed(int player = SEND_TO_KNOWN);
	void 	reportUnHidden(int player = SEND_TO_KNOWN);
	void 	reportHidden(int player = SEND_TO_KNOWN);

/* request */
	void	getWantedAction(void);
	bool	shooted(void);

	virtual QRect	rect(void);
};
 

#endif // SERVERUNIT_H

