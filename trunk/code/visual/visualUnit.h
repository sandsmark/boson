/***************************************************************************
                          visualUnit.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 00:53:00 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : capricel@enst.fr                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef VISUAL_UNIT_H 
#define VISUAL_UNIT_H 

#include <QwSpriteField.h>

#include "../common/unit.h"
#include "sprites.h"


class selectPart_up;
class selectPart_down;


class visualMobUnit : public mobUnit, public QwSprite
{
 Q_OBJECT

 public:
  
  visualMobUnit(mobileMsg_t *, QObject* parent=0, const char *name=0L);
  ~visualMobUnit();

  /** make the connection with <i>non-virtual</i> QwSpriteField functions */
  virtual	int _x(void) {return x();}
  virtual	int _y(void) {return y();}

/* attachement */
  void  select();
  void  unSelect();

/* Qw stuff */
  virtual int rtti() const { return S_MOBILE+type; }

 protected:
/* attachement */
  selectPart_up *sp_up;
  selectPart_down *sp_down;

};




class visualFacility : public Facility, public QwSprite
{

 public:
  visualFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L);
  ~visualFacility();
  /** make the connection with <i>non-virtual</i> QwSpriteField functions */
  virtual int	_x(void) {return x();}
  virtual int	_y(void) {return y();}

/* attachement */
  void  select();
  void  unSelect();

/* Qw stuff */
  virtual int	rtti() const { return S_FACILITY+type; }

 private:
/* attachement */
  selectPart_up		*sp_up;
  selectPart_down	*sp_down;

};

#endif // VISUAL_UNIT_H

