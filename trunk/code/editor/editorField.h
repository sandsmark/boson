/***************************************************************************
                          editorField.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
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

#ifndef EDITORFIELD_H 
#define EDITORFIELD_H 

#include <qintdict.h>

#include "../common/boFile.h"
//#include "../common/unitType.h"
//#include "../common/unit.h"	// Facility

#include "visualUnit.h"		// visualMobUnit
#include "visualField.h"

class QRect;
class QPainter;
class Cell;
class Unit;


/** 
  * This class encapsulate the "physical" idea of the map : size, contents..
  */
class editorField : public visualField, public boFile
{
	Q_OBJECT

public:
  editorField(uint l, uint h, QObject *parent=0, const char *name=0L);

  void createMobUnit(mobileMsg_t &);
  void destroyMobUnit(destroyedMsg_t &);

  void createFixUnit(facilityMsg_t &);
  void destroyFixUnit(destroyedMsg_t &);

  bool load(QString filename);
  bool save(QString filename);

  bool isModified() {return modified; }

/* concerning contents */
  visualFacility *getFacility(long key) { return facilities.find(key); }
  
  void	deleteCell(int, int);
  void	setCell(int, int, groundType );

public:
//private:
	QIntDict<visualMobUnit>		mobiles;
	QIntDict<visualFacility>	facilities;

private:
	long		key;
	void		freeRessources();
	bool		modified;

public: //needed by visualBigDisplay
	visualCell	**cells;

};

#endif // EDITORFIELD_H


