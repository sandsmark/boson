/***************************************************************************
                          bosonView.h  -  description                              
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

#ifndef BOSONVIEW_H 
#define BOSONVIEW_H 

#include "visualView.h"

class bosonView : public visualView 
{
	Q_OBJECT

public:
	bosonView(visualField *, QObject *parent=0, const char *name=0L);

	/* to handle orderButton 'clicked' event */
	void u_goto(void);
};

#endif // BOSONVIEW_H


