/***************************************************************************
                          visualView.cpp  -  description                              
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

#include "visualView.h"
#include "../common/log.h"
#include "speciesTheme.h"
#include "visual.h"


visualView::visualView(visualField *f, QObject *parent, const char *name=0L)
	: QObject(parent, name)
	,fixSelected( 0L )
	,selectionMode(SELECT_NONE)
{
	/* map geometry */
	viewL = viewH = 5; ///orzel : arbitraire, (doit etre/)sera fixe par un mainMap..
	viewX = viewY = 0;
	field = f;
}


void visualView::reCenterView(int x, int y)
{
	int oldX = viewX, oldY = viewY;

	viewX  = x - viewL/2;
	viewY  = y - viewH/2;

	checkMove();

	if (viewX != oldX || viewY != oldY) {
		emit repaint(FALSE);
		}
}


void visualView::reSizeView(int l, int h)
{
	int	Xcenter = viewX + viewL/2,
		Ycenter = viewY + viewH/2;

	viewL = l;
	viewH = h;

	reCenterView(Xcenter, Ycenter);
}

void visualView::relativeMoveView(int dx, int dy)
{
	int oldX = viewX, oldY = viewY;

	viewX += dx;
	viewY += dy;

	checkMove();

	if (viewX != oldX || viewY != oldY) {
		emit repaint(FALSE);
		}
}

void visualView::checkMove()
{
	viewX = QMIN(viewX, field->maxX - viewL);
	viewY = QMIN(viewY, field->maxY - viewH);

	viewX = QMAX(viewX, 0);
	viewY = QMAX(viewY, 0);
}

visualFacility * visualView::unSelectFix(void)
{
visualFacility *f = fixSelected;

if (!f) return f; // already done
fixSelected	= (visualFacility *) 0l;
f->unSelect();

emit setSelected((QPixmap *)0l);

return f;
}

visualMobUnit *visualView::unSelectMob(long key)
{
visualMobUnit *m = mobSelected[key];
mobSelected.remove(key);
m->unSelect();

if (mobSelected.isEmpty()) {
	emit setSelected((QPixmap *)0l);
	emit setOrders(0);
	/*orderButton[0]->hide();
	orderButton[0]->disconnect(this); */
	}

return m;
}

void visualView::unSelectAll(void)
{
	selectionWho =  -1; ///orzel : should be a WHO_NOBOCY;
	emit setOrders(0);
}


void visualView::selectFix(visualFacility *f)
{
	fixSelected = f;
	fixSelected->select();
	emit setSelected( vpp.species[f->who]->getBigOverview(f));
	
	switch (f->getType()) {
		case FACILITY_CMDBUNKER:
			emit setOrders(1, f->who);
			break;
		case FACILITY_WAR_FACTORY:
			emit setOrders(2, f->who);
			break;
		default:
			break;
	}
	logf(LOG_GAME_LOW, "select facility");
}

void visualView::selectMob(long key, visualMobUnit *m)
{
	if (mobSelected.isEmpty()) {
		boAssert( selectionWho = -1);
		selectionWho = m->who;
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



void visualView::selectArea(int x1, int y1, int x2, int y2)
{
	Pix p;
	visualMobUnit *u;
	int t;
 
	/* ensure that (x1<=x2 && y1<=y2) */
	if (x2<x1) {
		t  = x1; 
		x1 = x2;
		x2 = t ;
        }

	if (y2<y1) {
		t  = y1; 
		y1 = y2;
		y2 = t ;
        }

	/* selection */
	for( p = field->lookIn(x1,y1,x2-x1,y2-y1); p; field->next(p))
		if (IS_MOBILE(field->at(p)->rtti()) )  { //found one
//		if (IS_MOBILE(field->at(p)->rtti()) && field->exact(p))  { //found one
//			puts("hop");
			u =  ((visualMobUnit *) field->at(p));
			if (!mobSelected.find(u->key))			// already selected ?
				selectMob(u->key, u); //, puts("bof");
		}
}


