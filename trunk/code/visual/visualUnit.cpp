/***************************************************************************
                       visualUnit.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 00:53:00 CET 1999
                                           
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

#include "common/map.h"
#include "common/log.h"

#include "visualUnit.h"
#include "speciesTheme.h"
#include "visual.h"
#include "selectPart.h"

#define PF_DELTA	5   // facilities selection box are DELTA pixels more inside rect()


/*
 * visualUnit
 */

void visualUnit::unSelect()
{
	if (sp_up) delete sp_up;
	if (sp_down) delete sp_down;
	sp_down = 0l;
	sp_up = 0l;
}


void visualUnit::doHide()
{
	if (sp_up) sp_up->hide();
	if (sp_down) sp_down->hide();
	hide();
}


void visualUnit::doShow()
{
	if (sp_up) sp_up->show();
	if (sp_down) sp_down->show();
	show();
}


/*
 * visualMobUnit
 */

visualMobUnit::visualMobUnit(mobileMsg_t *msg, QObject* parent=0, const char *name=0L)
	: mobUnit(msg,parent,name)
	, visualUnit(msg->key, species[msg->who]->getPixmap(msg->type))
{

	z(Z_MOBILE + 3 * type);
	moveTo(msg->x, msg->y);
}

visualMobUnit::~visualMobUnit()
{
	unSelect(); // destroy selectPart
}


/***** selection *********/
void visualMobUnit::select()
{
	QRect	r = rect();

	boAssert(!sp_up);
	boAssert(!sp_down);

	sp_up = new selectPart(power, z(), selectPart::PART_UP);
	sp_up->moveTo(r.right(), r.top());
	sp_down = new selectPart(9, z(), selectPart::PART_DOWN);
	sp_down->moveTo(r.left(), r.bottom());
}


/*
 * visualFacility
 */
visualFacility::visualFacility(facilityMsg_t *msg, QObject* parent=0L, const char *name=0L)
	: Facility(msg,parent,name)
	, visualUnit(msg->key, species[msg->who]->getPixmap(msg->type))
{
	z(Z_FACILITY);
	moveTo(BO_TILE_SIZE * msg->x , BO_TILE_SIZE * msg->y);

	frame(msg->state);
}


visualFacility::~visualFacility()
{
	unSelect(); // destroy selectPart
}



/***** selection *********/
void visualFacility::select()
{
	QRect	r = rect();

	boAssert(!sp_up);
	boAssert(!sp_down);

	sp_up = new selectPart(power, z(), selectPart::PART_UP);
	sp_up->moveTo(r.right() - PF_DELTA, r.top() + PF_DELTA);
	sp_down = new selectPart(9, z(), selectPart::PART_DOWN);
	sp_down->moveTo(r.left() + PF_DELTA, r.bottom() - PF_DELTA);
}


