/***************************************************************************
                          playerUnit.h  -  description                              
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

#ifndef PLAYERUNIT_H 
#define PLAYERUNIT_H 

#include <qobject.h>

#include "visualUnit.h"
#include "sprites.h"

enum mobUnitState {
	MUS_NONE,
	MUS_TURNING,
	MUS_MOVING,
	MUS_MOVING_WAIT,
	MUS_
	};


/* boson units needs connect/disconnect.. */
class bosonUnit : public QObject
{
Q_OBJECT

public:
	bosonUnit() : QObject() { target = 0l; }
	~bosonUnit() { emit dying(this); }

	virtual void	u_attack(bosonUnit *);
public slots:
//  	void	targetMoveTo(int, int);
  	void	targetDying(bosonUnit *);
signals:
	void	sig_moveTo(int newx, int newy); // never emitted for a facility !
	void	dying(bosonUnit *);
protected:
	/* attack */
	bosonUnit 	*target;
	int		shoot_timer;

};
	
	
class playerMobUnit : public bosonUnit, public visualMobUnit
{
Q_OBJECT

public:
	struct	state_t {
		int x;
		int y;
		int dir;
	};

			playerMobUnit(mobileMsg_t *);

	void		getWantedAction();
 
	void		shooted(int _power);
/* Server orders */
	void		doMoveTo(state_t nstate);
	void		s_moveTo(state_t nstate);

protected:
	int		getLeft(int a=1) {return (direction+12-a)%12; }
	int		getRight(int a=1) {return (direction+a)%12; }
	void		turnTo(int newdir);
	bool		checkMove(state_t nstate);
   	bool		near(int distance);

	virtual bool	getWantedMove(state_t &);
	virtual bool	getWantedShoot(bosonMsgData *);
	
public slots:
	/* orders from user */
	virtual void	u_goto(int, int); // not the same as QCanvasSprite::moveTo
	void		u_stop(void);	
	virtual void	u_attack(bosonUnit *); // reimplemented from bosonUnit
  	void		targetMoveTo(int, int);

private :
	void		do_goto(int, int);
	int		direction;	// [0-11] is the angle ...
	mobUnitState	state;

/* moving */
	int 		dest_x, dest_y;
	state_t		asked;
	int		present_dx, present_dy;
	mobUnitState	asked_state;

};


class harvesterUnit : public playerMobUnit
{
Q_OBJECT
		
public:
	harvesterUnit(mobileMsg_t *m) : playerMobUnit(m) 
		{ hstate = standBy; base_x = m->x; base_y = m->y; }

	virtual bool	getWantedMove(state_t &);
	virtual bool	getWantedShoot(bosonMsgData *);
	virtual void	u_goto(int, int);
	
	enum		harvestState { standBy, goingTo, comingBack, harvesting };
	
private:
	harvestState	hstate;
	int		base_x, base_y;
	int		harvest_x, harvest_y;
};


/*
 *  Facility
 */
class playerFacility : public bosonUnit, public visualFacility
{
Q_OBJECT

public:
	playerFacility(facilityMsg_t *msg);
	~playerFacility();

	void	getWantedAction();

/* Server orders */
	void	s_setState(int );
	void	shooted(int _power);


};


#endif // PLAYERUNIT_H

