/***************************************************************************
                          unit.h  -  description                              
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

#ifndef UNIT_H 
#define UNIT_H 

#include <qrect.h>

#include "common/unitType.h"
#include "common/msgData.h"


#define  MAX_POWER 9

enum workType {
	WORK_NONE =0, 
	WORK_TRAINING, 

	WORK_LAST_FACILITY, 
	WORK_WOOD, 
	WORK_MINE, 
	WORK_ 
	};

/*
 * common header for facilityMsg_t and mobileMsg_t  XXX make *Msg_t inherit unitMsg_t...
 */
struct unitMsg_t    { uint who; int key, x, y; };


/** 
  * This class is the base for all boson units
  */
class Unit
{

public:
	Unit(unitMsg_t *m) { who = m->who; key = m->key; countDown = 0; work = WORK_NONE; }

	virtual	uint	getVisibility(void)=0;
	/**
	 * return the place occupied by this unit
	 * UNIT : PIXEL, (not grid)
	 */
	virtual	QRect	rect(void)=0;
  
	/* return the center of this unit, pixel-wise */
	QPoint		center(void) {return rect().center(); }
	/**
	 * the same as rect(), but in the grid system
	 */
	QRect		gridRect(void);

	uint		who;		// who is the owner ?
	int		key;
protected:
	uint		countDown;	// work countDown;
	workType	work;		// work being done
};


/** 
  * This class is the base for all mobile units
  */
class mobUnit : public Unit
{
public:
			mobUnit(mobileMsg_t *msg);

	void		fill(mobileMsg_t &msg);
	mobType		getType(void) {return type;}

	virtual uint	getVisibility(void) {return mobileProp[type].visibility; }
	virtual QRect	rect(void);

	int		goFlag(void) { return mobileProp[type].goFlag;}

protected:
	mobType		type;
	uint		autonomy; 	// carburant ?

};


/** 
  * This class is the base for all fix units
  */
class Facility : public Unit
{
public:
  			Facility(facilityMsg_t *msg);

	void		fill(facilityMsg_t &msg);
	facilityType	getType(void) {return type;}

	virtual uint	getVisibility(void) {return facilityProp[type].visibility; }
	virtual QRect	rect(void) { return QRect(0,0,facilityProp[type].width,facilityProp[type].height); }

protected:
	facilityType	type;
};

#endif // UNIT_H

