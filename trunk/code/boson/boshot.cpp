/***************************************************************************
                         boshot  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Dec 16 14:35:00 CET 1999
                                           
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

#include <kapp.h>
//#include "../common/log.h"
#include "boshot.h"


QwSpritePixmapSequence * boShot::shotSequ = 0l;

/*
 *  boshot
 */
boShot::boShot(int _x, int _y, int _z)
{
	if (!shotSequ) {
		QString path(kapp->kde_datadir() + "/boson/pics/explosion/explosion%02d");
		shotSequ = new QwSpritePixmapSequence(path+".ppm", path+".pbm", SHOT_FRAMES);
		}

	setSequence(shotSequ);		// set image set
	counter = 0; frame( 0);		// position the first image of the animation
	moveTo(_x, _y); z( _z + 1);	// position in the field
	startTimer(60);			// begin animation, 60 ms/frame
}

void  boShot::timerEvent( QTimerEvent * )
{
	counter++;
	if (counter<SHOT_FRAMES)
		frame(counter);
	else delete this;
}
