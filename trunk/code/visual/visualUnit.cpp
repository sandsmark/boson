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

#include "common/bomap.h"
#include "common/log.h"

#include "visualUnit.h"
#include "speciesTheme.h"
#include "visual.h"
#include "selectPart.h"

#define PF_DELTA	(+10)   // facilities selection boxes are DELTA pixels more inside rect()
#define PM_DELTA_H	(+4)   // mobiles selection boxes are DELTA pixels more inside rect()
#define PM_DELTA_V	(+10)   // mobiles selection boxes are DELTA pixels more inside rect()


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

visualMobUnit::visualMobUnit(mobileMsg_t *msg)
	: mobUnit(msg)
	, visualUnit(species[msg->who]->getPixmap(msg->type))
{

	setZ	(Z_MOBILE + 3 * type);
	move	(BO_TILE_SIZE * msg->x , BO_TILE_SIZE * msg->y);
}

visualMobUnit::~visualMobUnit()
{
	unSelect(); // destroy selectPart
}


/***** selection *********/
void visualMobUnit::select()
{
	QRect	r = rect();

	if (_destroyed) return;

	boAssert(!sp_up);
	boAssert(!sp_down);

	sp_up = new selectPart(power, z(), selectPart::PART_UP);
	sp_up->move(r.right() - PM_DELTA_H, r.top() + PM_DELTA_V);
	sp_down = new selectPart(POWER_LEVELS-1, z(), selectPart::PART_DOWN);
	sp_down->move(r.left() + PM_DELTA_H, r.bottom() - PM_DELTA_V);

}


QRect visualMobUnit::rect(void)
{
	QRect r = mobUnit::rect();
	r.moveBy( x(), y() );
	return r;
}



/*
 * visualFacility
 */
visualFacility::visualFacility(facilityMsg_t *msg)
	: Facility(msg)
	, visualUnit(species[msg->who]->getPixmap(msg->type))
{
	setZ	(Z_FACILITY);
	move	(BO_TILE_SIZE * msg->x , BO_TILE_SIZE * msg->y);

	setFrame(msg->state);
}


visualFacility::~visualFacility()
{
	unSelect(); // destroy selectPart
}



/***** selection *********/
void visualFacility::select()
{
	QRect	r = rect();

	if (_destroyed) return;

	boAssert(!sp_up);
	boAssert(!sp_down);

	sp_up = new selectPart(power, z(), selectPart::PART_UP);
	sp_up->move(r.right() - PF_DELTA, r.top() + PF_DELTA);
	sp_down = new selectPart(POWER_LEVELS-1, z(), selectPart::PART_DOWN);
	sp_down->move(r.left() + PF_DELTA, r.bottom() - PF_DELTA);
}


QRect visualFacility::rect(void)
{
	QRect r = Facility::rect();
	r.moveBy( x(), y() );
	return r;
}

