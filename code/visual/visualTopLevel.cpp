/***************************************************************************
                          visualTopLevel.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include "visualTopLevel.h"
#include "common/log.h"
#include "speciesTheme.h"


visualTopLevel::visualTopLevel( const char *name, WFlags f)
	: KTMainWindow(name,f)
	,fixSelected( 0L )
	,selectionMode(SELECT_NONE)
{
	/* map geometry */
	viewL = viewH = 5; ///orzel : senseless, will be set by mainMap later
	viewX = viewY = 0;

//	connect(vcanvas, SIGNAL(mobileDestroyed(int)), this, SLOT(mobileDestroyed(int)));
//	connect(vcanvas, SIGNAL(fixDestroyed(int)), this, SLOT(fixDestroyed(int)));

}


void visualTopLevel::reCenterView(int x, int y)
{
	int oldX = viewX, oldY = viewY;

	viewX  = x - viewL/2;
	viewY  = y - viewH/2;

	checkMove();

	if (viewX != oldX || viewY != oldY) {
		emit updateViews();
		}
}


void visualTopLevel::reSizeView(int l, int h)
{
	int	Xcenter = viewX + viewL/2,
		Ycenter = viewY + viewH/2;

	viewL = l;
	viewH = h;

	reCenterView(Xcenter, Ycenter);
}

void visualTopLevel::relativeMoveView(int dx, int dy)
{
	int oldX = viewX, oldY = viewY;

	viewX += dx;
	viewY += dy;

	checkMove();

	if (viewX != oldX || viewY != oldY) {
		updateViews();
		}
}

void visualTopLevel::checkMove()
{
	viewX = QMIN(viewX, vcanvas->maxX - viewL);
	viewY = QMIN(viewY, vcanvas->maxY - viewH);

	viewX = QMAX(viewX, 0);
	viewY = QMAX(viewY, 0);
}

/*
void visualTopLevel::fixDestroyed(int k)
{
	if (fixSelected && fixSelected->key == k) unSelectFix();
}
*/


visualFacility * visualTopLevel::unSelectFix(void)
{
	visualFacility *f = fixSelected;

	if (!f) return f; // already done
	fixSelected	= (visualFacility *) 0l;
	f->unSelect();

	emit setSelected((QPixmap *)0l);

	return f;
}


/*
void visualTopLevel::mobileDestroyed(int k)
{
	unSelectMob(k);
}
*/

visualMobUnit *visualTopLevel::unSelectMob(long key)
{
	visualMobUnit *m = mobSelected[key];
	if (!m) {
		logf(LOG_WARNING, "unSelectMob unknown mobile..");
		return 0l;
	}
	mobSelected.remove(key);
	m->unSelect();

	if (mobSelected.isEmpty()) {
		emit setSelected((QPixmap *)0l);
		emit setOrders(-1);
		}

	return m;
}

void visualTopLevel::unSelectAll(void)
{
	QIntDictIterator<visualMobUnit> selIt(mobSelected);

	/* deal with fix */
	unSelectFix();

	/* deal with mobiles */
	for (selIt.toFirst(); selIt;) { 		// ++ not needed, selIt should be increased
		unSelectMob(selIt.currentKey());	// by the .remove() in unselect
	}
	boAssert(mobSelected.isEmpty());
	if (!mobSelected.isEmpty()) mobSelected.clear();

	selectionWho =  -1; ///orzel : should be a WHO_NOBOCY;
	emit setOrders(-1);
}


void visualTopLevel::selectFix(visualFacility *f)
{
	fixSelected = f;
	fixSelected->select();
	emit setSelected( species[f->who]->getBigOverview(f));
	
	switch (f->getType()) {
		case FACILITY_CMDBUNKER:
			emit setOrders(10, f->who);
			break;
		case FACILITY_WAR_FACTORY:
			emit setOrders(11, f->who);
			break;
		default:
			break;
	}
	logf(LOG_GAME_LOW, "select facility");
}

void visualTopLevel::selectMob(long key, visualMobUnit *m)
{
	if (mobSelected.isEmpty()) {
		boAssert( selectionWho = -1);
		selectionWho = m->who;
		}
	else {
		boAssert( selectionWho>=0 );
		if ((int)m->who != selectionWho)
			return;
		}

	mobSelected.insert(key, m); m->select();
	emit setSelected( species[m->who]->getBigOverview(m));
	logf(LOG_GAME_LOW, "select mobile");
}



void visualTopLevel::selectArea(int x1, int y1, int x2, int y2)
{
	QCanvasItemList list;
	QCanvasItemList::Iterator it;
	visualMobUnit *u;
 
	// XXX : still neede with QCanvas collisions ? (QRect ?)
	/* ensure that (x1<=x2 && y1<=y2) */
	int t;
	if (x2<x1) { t  = x1; x1 = x2; x2 = t ; }
	if (y2<y1) { t  = y1; y1 = y2; y2 = t ; }

	/* selection */
	list = vcanvas->collisions( QRect(x1,y1,x2-x1,y2-y1) );

	for( it = list.begin(); it != list.end(); ++it )
		if ( IS_MOBILE( (*it)->rtti() ) ) {
			u =  (visualMobUnit *) (*it);
			if (!mobSelected.find(u->key))		// already selected ?
				selectMob(u->key, u);
		}
}

