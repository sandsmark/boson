/***************************************************************************
                          fieldMap.cpp  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include <kapp.h>
#include <assert.h>
#include "../common/log.h"
#include "fieldMap.h"
#include "playerCell.h"
#include "speciesTheme.h"
#include "groundTheme.h"
#include "viewMap.h"
  

fieldMap::fieldMap(orderWin *o, viewMap *v, QWidget*parent, const char *name, WFlags f)
	: QWidget(parent, name, f)
	, QwAbsSpriteFieldView(v->phys)
{

//setBackgroundColor(black);
//setBackgroundMode(fixedColor);

/* related orderWindows */
order = o;

/* the viewMap */
view = v;

/* make the connection */
physMap *phys = v->phys; ///orzel : should be moved

// connect(, SIGNAL(), this, SLOT());
connect(view, SIGNAL(repaint(bool)), this, SLOT(repaint(bool)));
connect(this, SIGNAL(relativeReCenterView(int, int)), view, SLOT(relativeReCenterView(int, int)));
connect(this, SIGNAL(reSizeView(int, int)), view, SLOT(reSizeView(int, int)));

}

fieldMap::~fieldMap()
{
	QwAbsSpriteFieldView::view(0);
}
