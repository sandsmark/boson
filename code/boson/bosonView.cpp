/***************************************************************************
                          bosonView.cpp  -  description                              
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

#include "../common/log.h"

#include "bosonView.h"
#include "playerUnit.h"
#include "game.h"

bosonView::bosonView(visualField *p, QObject *parent, const char *name=0L)
	:visualView(p,parent,name)
{
	connect(p, SIGNAL(reCenterView(int,int)), SLOT(reCenterView(int,int)));
}


