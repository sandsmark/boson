/***************************************************************************
                       bosonBigDisplay.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Feb 17, 1999
                                           
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

#include "common/log.h"

#include "bosonTopLevel.h"
#include "bosonCanvas.h"
#include "playerUnit.h"
#include "bosonBigDisplay.h"
#include "game.h"


bosonBigDisplay::bosonBigDisplay(bosonTopLevel *btl, QWidget *parent, const char *name, WFlags f)
	:visualBigDisplay(btl,parent,name,f)
{
}


void bosonBigDisplay::actionClicked(QPoint mp, int /*state*/)
{

	if (vtl->mobSelected.isEmpty() && !vtl->fixSelected) return;	// nothing to do

	QCanvasItem *sfg = bocanvas->findUnitAt( mp);

	/*
	 * FIX HANDLING
	 */
	puts("actionClicked(2)");
	if (vtl->fixSelected) {
		puts("actionClicked for fixSelected");
		// fix Selected
		if ( IS_MOBILE(sfg->rtti()))
			((playerFacility*)vtl->fixSelected)->u_attack( (playerMobUnit*)sfg );
		else if ( IS_FACILITY(sfg->rtti()))
			((playerFacility*)vtl->fixSelected)->u_attack( (playerFacility*)sfg );
		else return;
	}
		
	/*
	 * MOBILE HANDLING
	 */
	if ((int)who_am_i != vtl->selectionWho) return;	// nothing to do
	QIntDictIterator<visualMobUnit> mobIt(vtl->mobSelected);
	if (!sfg) {
		// nothing has been found : it's a ground-click
		// order all mobiles to go there
		for (mobIt.toFirst(); mobIt; ++mobIt) {
			boAssert(mobIt.current()->who == who_am_i);
			((playerMobUnit *)mobIt.current())->u_goto( mp );
		}
		if (!mobIt.isEmpty())bocanvas->play("mobile_going.wav");
		return;
	}
	

	if ( IS_MOBILE(sfg->rtti())) {
		playerMobUnit *m = (playerMobUnit *) sfg;

		for (mobIt.toFirst(); mobIt; ++mobIt) {
			boAssert(mobIt.current()->who == who_am_i);
			((playerMobUnit *)mobIt.current())->u_attack(m);
		}
		if (!mobIt.isEmpty()) bocanvas->play("mobile_attacking.wav");

		return;
	}

	/* do exactly the same, may change in the future */
	if ( IS_FACILITY(sfg->rtti())) {
		playerFacility *f = (playerFacility *) sfg;

		for (mobIt.toFirst(); mobIt; ++mobIt) {
			boAssert(mobIt.current()->who == who_am_i);
			((playerMobUnit *)mobIt.current())->u_attack(f);
		}
		if (!mobIt.isEmpty()) bocanvas->play("mobile_attacking.wav");

		return;
	}

	// should never be reached !
	logf(LOG_ERROR, "bosonBigDisplay.cpp, unexpected bocanvas->findUnitAt() result");
	

} 

