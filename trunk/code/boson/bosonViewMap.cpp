/***************************************************************************
                          bosonViewMap.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
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

#include "../common/log.h"

#include "bosonViewMap.h"
#include "playerUnit.h"
#include "game.h"

bosonViewMap::bosonViewMap(physMap *p, QObject *parent, const char *name=0L)
	:viewMap(p,parent,name)
{
}



void bosonViewMap::leftClicked(int mx, int my)		// selecting, moving...
{
QIntDictIterator<visualMobUnit> mobIt(mobSelected);

/*
if (SELECT_MOVE != getSelectionMode()) {
	logf(LOG_ERROR,"viewMap::leftClicked while not in SELECT_MOVE state");
	emit setOrders(0l);
	return;
	}
*/
if (mobSelected.isEmpty()) {
	logf(LOG_ERROR,"viewMap::leftClicked : unexpected empty mobSelected");
	setSelectionMode(SELECT_NONE);
	emit setOrders(0l);
	return;
	}

/* ACTION */

	if (gpp.who_am_i != selectionWho)
		return; 		// nothing to do

for (mobIt.toFirst(); mobIt; ++mobIt) {
	boAssert(mobIt.current()->who == gpp.who_am_i);
	((playerMobUnit *)mobIt.current())->u_goto(mx,my);
	}

} 

/*
void viewMap::u_goto(void)
{
boAssert( SELECT_NONE == getSelectionMode() );
//boAssert(selectionWho == gpp.who_am_i);
///orzel : should change the cursor over fieldMap
setSelectionMode(SELECT_MOVE);
} */
