/***************************************************************************
                          editorField.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
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

#ifndef EDITORFIELD_H 
#define EDITORFIELD_H 

#include <qintdict.h>

#include "../common/msgData.h"
#include "../common/playField.h" // origPeople
//#include "../common/unitType.h"
#include "../common/unit.h"	// Facility

#include "visualUnit.h"		// visualMobUnit
#include "visualField.h"

class QRect;
class QPainter;
class Cell;
class Unit;


struct visualMap {
	visualCell	**cells;
	int		width;
	int		height;
};


/** 
  * This class encapsulate the "physical" idea of the map : size, contents..
  */
class editorField : public visualField 
{
	Q_OBJECT

public:
  editorField(uint l, uint h, QObject *parent=0, const char *name=0L);

  void createMob(mobileMsg_t &);
  void destroyMob(destroyedMsg_t &);

  void createFix(facilityMsg_t &);
  void destroyFix(destroyedMsg_t &);

  bool load(QString filename);
  bool save(QString filename);

/* concerning contents */
  visualFacility *getFacility(long key) { return facility.find(key); }

public: ///orzel : temp

  visualMap	map;
  origPeople	people;

  bool		isModified;

//private:
	QIntDict<visualMobUnit>		mobile;
	QIntDict<visualFacility>	facility;

};

#endif // EDITORFIELD_H


