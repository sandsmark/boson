/***************************************************************************
                          editorFieldMap.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Tue Sep 21 01:18:00 CET 1999
                                           
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

#ifndef EDITOR_FIELD_MAP_H 
#define EDITOR_FIELD_MAP_H 

#include "fieldMap.h"

class QPopupMenu;
class Cell;

/** 
  * Add all editor-specific 'bells and whistles' to the visual/fieldMap
  */
class editorFieldMap : public fieldMap
{

	Q_OBJECT

public:
	editorFieldMap(viewMap *v, QWidget *parent=0, const char *name=0L, WFlags f=0);

protected:
	virtual void mousePressEvent(QMouseEvent *e);

private slots:
	void setCell(int);
	void setTransTile(int);
	void setTransType(int);
	void setTransItem(int);

private:
	QPopupMenu	*popup;
	Cell		*selectedCell;

};

#endif // EDITOR_FIELD_MAP_H


