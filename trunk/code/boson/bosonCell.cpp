/***************************************************************************
                       bosonCell.cpp  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Fri Nov 10 20:48:25 CET 2000
                                           
    copyright            : (C) 2000 by Thomas Capricelli                         
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

// Qt includes
#include <qcanvas.h>

// boso includes
#include "common/log.h"
#include "common/bomap.h" // BO_TILE_SIZE

#include "bosonCell.h"
#include "game.h"	// bocanvas
#include "sprites.h"


/*
 * fowSprite
 */
class fowSprite: public QCanvasSprite
{
public:
	// x,y are grid-relative coo of the cell being masked
	fowSprite(int x, int y);
private:
	bool	initData(void);

	static	QCanvasPixmapArray *data;

};

QCanvasPixmapArray *fowSprite::data=0l;

fowSprite::fowSprite(int x, int y)
	: QCanvasSprite (0, vcanvas)
{
	if (!data && !initData() ) // failed 
			delete this;

	// ok
	setSequence(data);
	move ( BO_TILE_SIZE*x , BO_TILE_SIZE*y );
	setZ(Z_FOW);
	show();
}

bool fowSprite::initData(void)
{
	//orzel : XXX should use the theming stuff instead of 'standard'
	//
	QString path = *dataPath + "themes/ui/standard/fow.xpm";
	data = new QCanvasPixmapArray(path);
	if (!data) {
		logf(LOG_ERROR, "can't create QCanvasPixmapArray");
		return false;
	}
	if (!data->image(0) || data->image(0)->width() != (2*BO_TILE_SIZE) || data->image(0)->height() != (2*BO_TILE_SIZE)) {
		logf(LOG_ERROR, "can't load fow.xpm");
		delete data; data = 0l;
		return false;
	}
	QBitmap mask(path);
	if ( mask.width() != (2*BO_TILE_SIZE) || mask.height() != (2*BO_TILE_SIZE)) {
		logf(LOG_ERROR, "can't create fow mask");
		delete data; data = 0l;
		return false;
	}
	data->image(0)->setMask(mask);
	data->image(0)->setOffset(BO_TILE_SIZE/2,BO_TILE_SIZE/2);
	return true;
}


/*
 * bosonCell
 */
void bosonCell::unFog(void)
{

	if (!isFogged()) return;

	delete fow;
	fow = 0l;
}



void bosonCell::fog(int x, int y)
{
	if (isFogged()) return;

	fow = new fowSprite(x,y);
}


