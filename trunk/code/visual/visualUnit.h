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

#include <QwSpriteField.h>

#include "../common/unit.h"
#include "sprites.h"


class selectPart;


class visualUnit : public QwSprite
{
public:
	visualUnit(int k, QwSpritePixmapSequence* s) : QwSprite(s), key(k)
		{ power = MAX_POWER; sp_down = 0l; sp_up = 0l; contain = 0; }
	
	void	unSelect();
	void	updateContain(uint c) { contain = c;}
	void	doHide();
	void	doShow();

protected:
	int		power;
	/* attachement */
	selectPart	*sp_up;
	selectPart	*sp_down;
	
public:
	int	key;
	uint	contain;
};

class visualMobUnit : public mobUnit, public visualUnit
{

	Q_OBJECT

 public:
  
  visualMobUnit(mobileMsg_t *, QObject* parent=0, const char *name=0L);
  ~visualMobUnit();

	/** make the connection with <i>non-virtual</i> QwSpriteField functions */
	virtual	int	_x(void) {return x();}
	virtual	int	_y(void) {return y();}

/* attachement */
  void  select();

/* Qw stuff */
  virtual int rtti() const { return S_MOBILE+type; }

};




class visualFacility : public Facility, public visualUnit
{

	Q_OBJECT

 public:
  visualFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L);
  ~visualFacility();

	/** make the connection with <i>non-virtual</i> QwSpriteField functions */
	virtual	int	_x(void) {return x();}
	virtual	int	_y(void) {return y();}

/* attachement */
  void  select();

/* Qw stuff */
  virtual int	rtti() const { return S_FACILITY+type; }

};

#endif // VISUALUNIT_H

