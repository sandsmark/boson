/***************************************************************************
                          viewSelect.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sun Sep 19 01:01:00 CET 1999
                                           
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

#include "speciesTheme.h"
#include "viewMap.h"
#include "visual.h"


visualFacility * viewMap::unSelectFix(void)
{
visualFacility *f = fixSelected;

if (!f) return f; // already done
fixSelected	= (visualFacility *) 0l;
f->unSelect();

emit setSelected((QPixmap *)0l);

return f;
}

visualMobUnit *viewMap::unSelectMob(long key)
{
visualMobUnit *m = mobSelected[key];
mobSelected.remove(key);
m->unSelect();

if (mobSelected.isEmpty()) {
	emit setSelected((QPixmap *)0l);
	emit setOrders(0l);
	/*orderButton[0]->hide();
	orderButton[0]->disconnect(this); */
	}

return m;
}

void viewMap::unSelectAll(void)
{
	selectionWho =  -1; ///orzel : should be a WHO_NOBOCY;
	emit setOrders(0l);
}


void viewMap::selectFix(visualFacility *f)
{
	fixSelected = f; fixSelected->select();
	emit setSelected( vpp.species[f->who]->getBigOverview(f));
	logf(LOG_GAME_LOW, "select facility");
}

void viewMap::selectMob(long key, visualMobUnit *m)
{
	if (mobSelected.isEmpty()) {
		boAssert( selectionWho = -1);
		selectionWho = m->who;
//		if (selectionWho == gpp.who_am_i) {
			emit setOrders(1l);
			/*connect(orderButton[0], SIGNAL(clicked()), this, SLOT(u_goto()));
			orderButton[0]->show(); */
//			}
		}
	else {
		boAssert( selectionWho>=0 );
		if (m->who != selectionWho)
			return;
		}

	mobSelected.insert(key, m); m->select();
	emit setSelected( vpp.species[m->who]->getBigOverview(m));
	logf(LOG_GAME_LOW, "select mobile");
}



void viewMap::leftClicked(int mx, int my)		// selecting, moving...
{
QIntDictIterator<visualMobUnit> mobIt(mobSelected);

if (SELECT_MOVE != getSelectionMode()) {
	logf(LOG_ERROR,"viewMap::leftClicked while not in SELECT_MOVE state");
	emit setOrders(0l);
	return;
	}
if (mobSelected.isEmpty()) {
	logf(LOG_ERROR,"viewMap::leftClicked : unexpected empty mobSelected");
	setSelectionMode(SELECT_NONE);
	emit setOrders(0l);
	return;
	}

/* ACTION */

/* orzel : hum...
for (mobIt.toFirst(); mobIt; ++mobIt) {
//	boAssert(mobIt.current()->who == gpp.who_am_i);
	mobIt.current()->u_goto(mx,my);
	}
*/

setSelectionMode(SELECT_NONE);
} 


void viewMap::u_goto(void)
{
boAssert( SELECT_NONE == getSelectionMode() );
//boAssert(selectionWho == gpp.who_am_i);
///orzel : should change the cursor over fieldMap
setSelectionMode(SELECT_MOVE);
}
