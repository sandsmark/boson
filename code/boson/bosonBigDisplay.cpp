/***************************************************************************
                       bosonBigDisplay.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Feb 17, 1999
                                           
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

#include "playerUnit.h"
#include "visualBigDisplay.h"
#include "game.h"


void visualBigDisplay::actionClicked(int mx, int my)
{
	QIntDictIterator<visualMobUnit> mobIt(view->mobSelected);

	if (view->mobSelected.isEmpty()) {
		return;
		}

	if (gpp.who_am_i != view->selectionWho)
		return; 		// nothing to do

	for (mobIt.toFirst(); mobIt; ++mobIt) {
		boAssert(mobIt.current()->who == gpp.who_am_i);
		((playerMobUnit *)mobIt.current())->u_goto(mx,my);
		}

} 

