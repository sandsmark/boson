/***************************************************************************
                          visualTopLevel.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include "visualTopLevel.h"
#include "common/log.h"
#include "common/bomap.h" 	// BO_TILE_SIZE
#include "speciesTheme.h"


visualTopLevel::visualTopLevel( const char *name, WFlags f)
	: KMainWindow(0l, name,f)
	,fixSelected( 0L )
	,viewPos(0,0)
	,viewSize(6,6) ///orzel : senseless, will be set by mainMap later
	,selectionMode(SELECT_NONE)
{

//	connect(vcanvas, SIGNAL(mobileDestroyed(int)), this, SLOT(mobileDestroyed(int)));
//	connect(vcanvas, SIGNAL(fixDestroyed(int)), this, SLOT(fixDestroyed(int)));

}


void visualTopLevel::reCenterView(QPoint p)
{
	QPoint old = viewPos;

	viewPos = p - QPoint(viewSize.width(), viewSize.height())/2;

	checkMove();

	if (old != viewPos)
		emit updateViews();
}


void visualTopLevel::reSizeView(QSize s)
{
	QPoint center = viewPos + QPoint(viewSize.width(), viewSize.height())/2;
	viewSize = s;

	reCenterView(center);
}

void visualTopLevel::relativeMoveView(QPoint dpos)
{
	QPoint old = viewPos;

	viewPos += dpos;

	checkMove();

	if (old != viewPos)
		emit updateViews();
}

void visualTopLevel::checkMove()
{
	int minx = vcanvas->maxX - viewSize.width();
	int miny = vcanvas->maxY - viewSize.height();

	if (viewPos.x() > minx) viewPos.setX(minx);
	if (viewPos.y() > miny) viewPos.setY(miny);

	if (viewPos.x() < 0) viewPos.setX(0);
	if (viewPos.y() < 0) viewPos.setY(0);
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
	if (f->isDestroyed()) return;

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
	if (m->isDestroyed()) return;

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



void visualTopLevel::selectArea(QRect r)
{
	QCanvasItemList list;
	QCanvasItemList::Iterator it;
	visualMobUnit *u;
 
	/* selection */
	QPoint _do = viewPos*BO_TILE_SIZE;
	r.moveBy( _do.x(), _do.y() );

	list = vcanvas->collisions( r.normalize() );

	for( it = list.begin(); it != list.end(); ++it )
		if ( IS_MOBILE( (*it)->rtti() ) ) {
			u =  (visualMobUnit *) (*it);
			if (!mobSelected.find(u->key))		// already selected ?
				selectMob(u->key, u);
		}
}

