/***************************************************************************
                       bosonBigDisplay.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Feb 17, 1999
                                           
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

#include "../common/log.h"

#include "playerUnit.h"
#include "bosonBigDisplay.h"
#include "game.h"


bosonBigDisplay::bosonBigDisplay(bosonView *v, QWidget *parent=0, const char *name=0L, WFlags f=0)
	:visualBigDisplay(v,parent,name,f)
{

	bview = v;
}


void bosonBigDisplay::actionClicked(int mx, int my)
{
	QwSpriteFieldGraphic *sfg;

	/* is there any mobiles of my own selected ? */
	if (view->mobSelected.isEmpty()) return;	// nothing to do
	if (who_am_i != view->selectionWho) return;	// nothing to do

	QIntDictIterator<visualMobUnit> mobIt(view->mobSelected);


	sfg = view->field->findUnitAt( mx, my);
	if (!sfg) {
		// nothing has been found : it's a ground-click
		// order all mobiles to go there
		for (mobIt.toFirst(); mobIt; ++mobIt) {
			boAssert(mobIt.current()->who == who_am_i);
			((playerMobUnit *)mobIt.current())->u_goto(mx,my);
		}
		return;
	}
	

	if ( IS_MOBILE(sfg->rtti())) {
		playerMobUnit *m = (playerMobUnit *) sfg;

		for (mobIt.toFirst(); mobIt; ++mobIt) {
			boAssert(mobIt.current()->who == who_am_i);
			((playerMobUnit *)mobIt.current())->u_attack(m);
		}

		return;
	}

	/* do exactly the same, may change in the future */
	if ( IS_FACILITY(sfg->rtti())) {
		playerFacility *f = (playerFacility *) sfg;

		for (mobIt.toFirst(); mobIt; ++mobIt) {
			boAssert(mobIt.current()->who == who_am_i);
			((playerMobUnit *)mobIt.current())->u_attack(f);
		}

		return;
	}

	// should never be reached !
	logf(LOG_ERROR, "bosonBigDisplay.cpp, unexpeted field->findUnitAt() result");
	

} 

