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


#include <QwSpriteField.h>
#include "sprites.h"		// S_PART
#include "common/unit.h"	// MAX_POWER


#define PART_NB		(MAX_POWER+1)

class selectPart : public QwSprite
{
public:
	selectPart(int frame, int z, bool isDown);
	virtual int rtti() const { return S_PART; } /* Qw stuff */
private:
	static QwSpritePixmapSequence  *qsps_up;
	static QwSpritePixmapSequence  *qsps_down;
	
};

#endif // SELECTPART_H 
