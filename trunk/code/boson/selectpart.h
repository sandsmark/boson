/***************************************************************************
                         selectPart.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jun 26 16:23:00 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#ifndef __SELECTPART_H__
#define __SELECTPART_H__

#include <qcanvas.h>
#include "rtti.h"

class SelectPart : public QCanvasSprite
{
public:
	enum SelectPartType { 
		PartUp,
		PartDown
	};

	SelectPart(int frame, int z, SelectPartType type, QCanvas* canvas);
	
	virtual int rtti() const 
	{ 
		// unclean!
		return RTTI::SelectPart;
	}

	static int frames();

protected:
	QCanvasPixmapArray* initStatic(SelectPartType type);

private:
	static QCanvasPixmapArray *mPartUp;
	static QCanvasPixmapArray *mPartDown;
};

#endif // __SELECTPART_H__

