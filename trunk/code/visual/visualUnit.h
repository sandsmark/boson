/***************************************************************************
                          visualUnit.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 00:53:00 CET 1999
                                           
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

#ifndef VISUALUNIT_H 
#define VISUALUNIT_H 

#include <qcanvas.h>

#include "common/unit.h"
#include "sprites.h"
#include "visualCanvas.h"
#include "visual.h"


class selectPart;


class visualUnit : public QCanvasSprite
{
public:
	visualUnit(QCanvasPixmapArray* s) : QCanvasSprite(s, vcanvas)
		{ power = MAX_POWER; sp_down = 0l; sp_up = 0l; contain = 0; _destroyed=false; show(); }
	
	void	unSelect();
	void	updateContain(uint c) { contain = c;}
	void	doHide();
	void	doShow();
	bool	isDestroyed(void) {return _destroyed; }

protected:
	int		power;
	/* attachement */
	selectPart	*sp_up;
	selectPart	*sp_down;
	bool		_destroyed;

public:
	uint	contain;

};


class visualMobUnit : public mobUnit, public visualUnit
{

public:
  
	visualMobUnit(mobileMsg_t *);
	~visualMobUnit();

	/* Unit stuff */
	virtual	QRect	rect(void);
	/* QCanvas stuff */
	virtual int rtti() const { return S_MOBILE+type; }
	/* attachement */
	void  select();
	void  destroy(void) { setFrame(PIXMAP_MOBILE_DESTROYED); unSelect(); _destroyed = true;}

};




class visualFacility : public Facility, public visualUnit
{

public:
	visualFacility(facilityMsg_t *msg);
	~visualFacility();

	/* Unit stuff */
	virtual	QRect	rect(void);
	/* QCanvas stuff */
	virtual int	rtti() const { return S_FACILITY+type; }
	/* attachement */
	void  select();
	void  destroy(void) { setFrame(PIXMAP_FIX_DESTROYED); unSelect(); _destroyed = true; }

};

#endif // VISUALUNIT_H

