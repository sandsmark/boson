/***************************************************************************
                          playerUnit.cpp  -  description                              
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

#include <math.h>	//sqrt
#include <assert.h>

#include "playerUnit.h"
#include "speciesTheme.h"

#include "game.h"

#include "../map/map.h"
#include "../common/log.h"

/*
 * playerMobUnit
 */

playerMobUnit::playerMobUnit(mobileMsg_t *msg, QObject* parent=0, const char *name=0L)
	:mobUnit(msg,parent,name)
	, QwSprite(gameProperties.species[msg->who]->getPixmap(msg->type))
	, state(MUS_NONE)
{
z(Z_MOBILE);
moveTo(msg->x, msg->y);

//direction = 10;
direction = 1;
}


/*QRect playerMobUnit::rect(void)
{
QRect r = mobUnit::rect();
r.moveBy(x(), y());
return r;
} */

int playerMobUnit::getWantedMove(int &dx, int &dy)
{

switch(state){
	default:
		logf(LOG_ERROR, "playerMobUnit::getWantedMove : unknown state");
		return 0;
		break;
	case MUS_NONE:
		return 0;
		break;
	case MUS_MOVING:
		dx = dest_x - x() ; dy= dest_y - y();
		assert(dx!=0 || dy!=0);
		double factor = (double) mobileProp[type].speed / sqrt((double)( dx*dx + dy*dy));
		if (factor<1.) {
//printf("\tfactor = %f\n", factor);
			dx =(int) ((double)dx*factor);
			dy =(int) ((double)dy*factor);
			}
//printf("wanted : d(%d.%d) ... ", dx, dy);
		asked_dx = dx; asked_dy = dy;
		return 1;
		break;
	}
}

int playerMobUnit::getWantedAction()
{
return 0; // no action
}




/***** server orders *********/
void playerMobUnit::s_moveBy(int dx, int dy)
{
//orzel : use some kind of fuel
//printf("Moved  : d(%d.%d)\n", dx, dy);
if (MUS_MOVING != state) {
	logf(LOG_ERROR, "playerMobUnit::s_moveBy while not moving, ignored");
	return;
	}

//printf("      dx = %d,       dy = %d\n", dx, dy);
//printf("asked_dx = %d, asked_dy = %d\n", asked_dx, asked_dy);
if (dx != asked_dx || dy != asked_dy)
	logf(LOG_ERROR, "playerMobUnit::s_moveBy : unexpected dx,dy");

moveBy(dx,dy);
if (x()==dest_x && y()==dest_y) {
	state = MUS_NONE;
	logf(LOG_GAME_LOW, "mobile[%p] has stopped\n", this);
	}

asked_dx = asked_dy = 0; // so that willBe returns the good position
}






/***** users orders *********/

void playerMobUnit::u_goto(int mx, int my) // not the same as QwSprite::moveTo
{
dest_x = mx; dest_y = my;
if (x()!=dest_x || y()!=dest_y) state = MUS_MOVING;

//printf("u_goto from %p, x.y = %d.%d, MOVING=%s\n", this, mx, my, (state==MUS_MOVING)?"yes":"no");
///orzel : moving across complicated environment algorithm
}

void playerMobUnit::u_stop(void)
{
state = MUS_NONE;
}

/*
 * playerFacility
 */
playerFacility::playerFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L)
	: Facility(msg,parent,name)
	, QwSprite(gameProperties.species[msg->who]->getPixmap(msg->type))
{
z(Z_FACILITY);
moveTo(BO_TILE_SIZE * msg->x , BO_TILE_SIZE * msg->y);
}


/*QRect playerFacility::rect(void)
{
QRect r = Facility::rect();
r.moveBy(x(), y());
return r;
}*/

void playerFacility::s_setState(int s)
{
boAssert(frame()==s-1);
frame(s);
}
