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
	void	sig_moveTo(QPoint npos); // never emitted for a facility !
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
			playerMobUnit(mobileMsg_t *);

	void		getWantedAction();
	void		shooted(int _power);
	void		destroy(void);
/* Server orders */
	void		s_moveTo(QPoint nstate);

protected:
	int		getLeft(int a=1) {return (direction+DIRECTION_STEPS-a)%DIRECTION_STEPS; }
	int		getRight(int a=1) {return (direction+a)%DIRECTION_STEPS; }
	void		turnTo(int newdir);
	bool		checkMove(QPoint pos);
   	bool		near(int distance);

	virtual bool	getWantedMove(QPoint &);
	virtual bool	getWantedShoot(bosonMsgData *);

public slots:
	/** user asked for a given destination */
	virtual void	u_goto(QPoint); // not the same as QCanvasSprite::moveTo
	/** user asked to stop */
	void		u_stop(void);	
	/** user asked to attack the given unit */
	virtual void	u_attack(bosonUnit *); // reimplemented from bosonUnit

	/** this slots receives message when the attacked unit moves */
  	void		targetMoveTo(QPoint);

private :
	/** actually do the job of moving the unit, from server order */
	void		do_moveTo(QPoint nstate);
	/** actually do the job of configuring the unit with given destination */
	void		do_goto(QPoint);
	int		direction;	// [0-11] is the angle ...
	mobUnitState	state;

/* moving */
	QPoint		dest;
	QPoint		asked;
	mobUnitState	asked_state;
	uint		failed_move;

};


class harvesterUnit : public playerMobUnit
{
Q_OBJECT
		
public:
	harvesterUnit(mobileMsg_t *m) : playerMobUnit(m) , base(m->x,m->y)
		{ hstate = standBy;}

	virtual bool	getWantedMove(QPoint &);
	virtual bool	getWantedShoot(bosonMsgData *);
	virtual void	u_goto(QPoint);
	
	enum		harvestState { standBy, goingTo, comingBack, harvesting };
	
private:
	harvestState	hstate;
	QPoint		base;
	QPoint		harvest;
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
	void	destroy(void);

};


#endif // PLAYERUNIT_H

