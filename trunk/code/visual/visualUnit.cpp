/***************************************************************************
                       visualUnit.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 00:53:00 CET 1999
                                           
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

//#include <stdlib.h>	// int abs(int);
//#include <assert.h>

#include "../map/map.h"
#include "../common/log.h"

#include "visualUnit.h"
#include "speciesTheme.h"
#include "visual.h"
#include "selectPart.h"

#define PF_DELTA	5   // facilities selection box are DELTA pixels more inside rect()

/*
 * visualMobUnit
 */

visualMobUnit::visualMobUnit(mobileMsg_t *msg, QObject* parent=0, const char *name=0L)
	: mobUnit(msg,parent,name)
	, QwSprite(vpp.species[msg->who]->getPixmap(msg->type))
{

	z(Z_MOBILE + 3 * type);
	moveTo(msg->x, msg->y);

	sp_down = 0l; sp_up = 0l;
}

visualMobUnit::~visualMobUnit()
{
	unSelect();
}


/***** selection *********/
void visualMobUnit::select()
{
	QRect	r = rect();

	boAssert(!sp_up);
	boAssert(!sp_down);

	sp_up = new selectPart_up(5, z());
	sp_up->moveTo(r.right(), r.top());
	sp_down = new selectPart_down(4, z());
	sp_down->moveTo(r.left(), r.bottom());
}


void visualMobUnit::unSelect()
{
	if (sp_up) delete sp_up;
	if (sp_down) delete sp_down;
	sp_down = 0l;
	sp_up = 0l;
}


/*
 * visualFacility
 */
visualFacility::visualFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L)
	: Facility(msg,parent,name)
	, QwSprite(vpp.species[msg->who]->getPixmap(msg->type))
{
	z(Z_FACILITY);
	sp_down = 0l; sp_up = 0l;
}


visualFacility::~visualFacility()
{
	unSelect();
}



/***** selection *********/
void visualFacility::select()
{
	QRect	r = rect();

	boAssert(!sp_up);
	boAssert(!sp_down);

	sp_up = new selectPart_up(3, z());
	sp_up->moveTo(r.right() - PF_DELTA, r.top() + PF_DELTA);
	sp_down = new selectPart_down(2, z());
	sp_down->moveTo(r.left() + PF_DELTA, r.bottom() - PF_DELTA);
}


void visualFacility::unSelect()
{
	if (sp_up) delete sp_up;
	if (sp_down) delete sp_down;
	sp_down = 0l;
	sp_up = 0l;
}


