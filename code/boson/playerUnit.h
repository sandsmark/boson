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


/**
 * common part for all fix/mob units used in
 * the client part.
 *
 * boson units needs connect/disconnect..
 * */
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


/**
 *	this class is used to store the latest moves for
 *	a given mobile
 */

class boPath //: public QValueList<QPoint>
{
public:
	/** constructor, maxloop is the maximum
	 * number of QPoint handled by this path */
	boPath(int maxloop): max(maxloop) { points = new QPoint[maxloop]; reset(); }
	~boPath(void) {delete points; }
	
	/** add a point to the path
	 * returns false if this move close a loop,
	 * true else
	 */
	bool addCheckLoop(QPoint);
	void reset(void) {begin=len=0; }

private:
	QPoint &operator[](int i) { return at(i); }
	QPoint &at(int i) { return points[ (begin+i)%max];} // suppose that i<len !!!!!!!11
	QPoint	*points;
	int	max;
	int	begin, len;

} ;

	
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
	virtual	int	rtti() const { return _destroyed?0:visualMobUnit::rtti(); }

protected:
	int		getLeft(int a=1) {return (direction+DIRECTION_STEPS-a)%DIRECTION_STEPS; }
	int		getRight(int a=1) {return (direction+a)%DIRECTION_STEPS; }
	void		turnTo(int newdir);
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
	boPath		path;


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
	virtual	int rtti() const { return _destroyed?0:visualFacility::rtti(); }

/* Server orders */
	void	s_setState(int );
	void	shooted(int _power);
	void	destroy(void);

};


#endif // PLAYERUNIT_H

