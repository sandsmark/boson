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


class selectPart_up;
class selectPart_down;


class visualUnit : public QwSprite
{
public:
	visualUnit(QwSpritePixmapSequence* s) : QwSprite(s)
		{ power = MAX_POWER; sp_down = 0l; sp_up = 0l; }
	
	/** make the connection with <i>non-virtual</i> QwSpriteField functions */
	virtual	int	_x(void) {return x();}
	virtual	int	_y(void) {return y();}

	void  unSelect();

protected:
	int		power;
	/* attachement */
	selectPart_up	*sp_up;
	selectPart_down	*sp_down;
};

class visualMobUnit : public mobUnit, public visualUnit
{

	Q_OBJECT

 public:
  
  visualMobUnit(mobileMsg_t *, QObject* parent=0, const char *name=0L);
  ~visualMobUnit();

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

/* attachement */
  void  select();

/* Qw stuff */
  virtual int	rtti() const { return S_FACILITY+type; }

};

#endif // VISUALUNIT_H

