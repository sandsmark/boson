/***************************************************************************
                          miniMap.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
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

//#include <assert.h>

#include <qpixmap.h>

#include "miniMap.h"
#include "viewMap.h"
  

miniMap::miniMap(viewMap *v, QWidget*parent, const char *name=0L)
	: QWidget(parent, name)
{

	physMap *phys = v->phys;

/* the viewMap */
	view = v;

/* create the (back)ground pixmap */
	ground = new QPixmap(v->maxX(), v->maxY());
	ground->fill(black);

/* make the connection */
	connect(phys, SIGNAL(newCell(int,int, groundType)), this, SLOT(newCell(int,int, groundType)));
	connect(phys, SIGNAL(updateMobile(playerMobUnit *)), this, SLOT(drawMobile(playerMobUnit *)));
	connect(phys, SIGNAL(updateFix(playerFacility *)), this, SLOT(drawFix(playerFacility *)));

// connect(, SIGNAL(), this, SLOT());
	connect(view, SIGNAL(repaint(bool)), this, SLOT(repaint(bool)));
	connect(this, SIGNAL(reCenterView(int, int)), view, SLOT(reCenterView(int, int)));
	connect(this, SIGNAL(reSizeView(int, int)), view, SLOT(reSizeView(int, int)));
}

