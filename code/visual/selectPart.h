/***************************************************************************
                         selectPart.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jun 26 16:23:00 CET 1999
                                           
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

#ifndef SELECTPART_H 
#define SELECTPART_H 


#include <qcanvas.h>
#include "sprites.h"		// S_PART
#include "common/unit.h"	// POWER_LEVELS


#define PART_NB		(POWER_LEVELS)

class selectPart : public QCanvasSprite
{
public:
	enum sp_type { PART_UP, PART_DOWN};

	selectPart(int frame, int z, sp_type type);
	
	virtual int rtti() const { return S_PART; } /* Qcanvas stuff */
private:
	static QCanvasPixmapArray	*qsps_up;
	static QCanvasPixmapArray	*qsps_down;
	
};

#endif // SELECTPART_H 
