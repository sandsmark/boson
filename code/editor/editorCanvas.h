/***************************************************************************
                          editorCanvas.h  -  description                              
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

#include "common/boFile.h"

#include "visualUnit.h"		// visualMobUnit
#include "visualCanvas.h"

class QRect;
class QPainter;
class Cell;
class visualCell;
class Unit;


/** 
  * This class encapsulate the "physical" idea of the map : size, contents..
  */
class editorCanvas : public visualCanvas, public boFile
{
	Q_OBJECT

public:
  editorCanvas();
  ~editorCanvas() { freeRessources(); }

  void createMobUnit(mobileMsg_t &);
  void destroyMobUnit(destroyedMsg_t &);

  void createFixUnit(facilityMsg_t &);
  void destroyFixUnit(destroyedMsg_t &);

  bool Load(QString filename);
  bool Save(QString filename);
  bool New(groundType fill_ground, uint w, uint h, const QString &name);

  bool isModified() {return modified; }

	/* concerning contents */
	visualFacility	*getFacility(long key) { return facilities.find(key); }
	void		changeCell(int x, int y, groundType g) { deleteCell(x,y); setCell(x,y,g); }

public:
//private:
	QIntDict<visualMobUnit>		mobiles;
	QIntDict<visualFacility>	facilities;

private:
	void		deleteCell(int, int);
	void		setCell(int, int, groundType );

	long		key;
	void		freeRessources();
	bool		modified;
	bool		cells_allocated;

public: //needed by visualBigDisplay
	visualCell	**cells;

};

#endif // EDITORFIELD_H


