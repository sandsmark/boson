/***************************************************************************
                          visualMiniDisplay.cpp  -  description                              
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

#include <qpixmap.h>

#include "common/log.h"

#include "visualMiniDisplay.h"
#include "visualTopLevel.h"

visualMiniDisplay::visualMiniDisplay(visualTopLevel *v, QWidget*parent, const char *name)
	: QWidget(parent, name)
	, vtl(v)
{

/* create the (back)ground pixmap */
	_ground = new QPixmap(v->maxX(), v->maxY());
	_ground->fill(black);
	sync();

/* make the connection */
	connect(vcanvas, SIGNAL(newCell(int,int, groundType)), this, SLOT(newCell(int,int, groundType)));
	connect(vcanvas, SIGNAL(updateMobile(visualMobUnit *)), this, SLOT(drawMobile(visualMobUnit *)));
	connect(vcanvas, SIGNAL(updateFix(visualFacility *)), this, SLOT(drawFix(visualFacility *)));
	connect(vcanvas, SIGNAL(syncMini(void)), this, SLOT(sync(void)));

// connect(, SIGNAL(), this, SLOT());
	connect(this, SIGNAL(reCenterView(int, int)), vtl, SLOT(reCenterView(int, int)));
	connect(this, SIGNAL(reSizeView(int, int)), vtl, SLOT(reSizeView(int, int)));
}


void visualMiniDisplay::sync(void)
{

	int i,j;
	groundType g;
	QPainter p;

	p.begin(_ground);

	for (i=0; i< vtl->maxX(); i++)
		for (j=0; j< vtl->maxY(); j++) {

			g = ground(vcanvas->tile(i,j));

			if (IS_TRANS(g))
				g = groundTransProp[ GET_TRANS_REF(g) ].from;

			switch(g) {
				default:
					logf(LOG_ERROR, "visualMiniDisplay::sync : unexpected groundType : %d", g);
				case GROUND_WATER :
				case GROUND_WATER_OIL:
					setPoint( i, j, blue, &p);
					break;
				case GROUND_GRASS :
				case GROUND_GRASS_OIL :
					setPoint( i, j, green, &p);
					break;
				case GROUND_DESERT :
					setPoint( i, j, darkYellow, &p);
					break;
			}
		}

	p.end();

	// update 
	repaint(FALSE);
}


void visualMiniDisplay::paintEvent(QPaintEvent *evt)
{
	QPainter p;

	p.begin(this);

	/* map is buffered */
	p.drawPixmap(0,0,*_ground);

	/* the little rectangle */
	p.setPen(white);
	p.setRasterOp(XorROP);
	p.drawRect(vtl->X(), vtl->Y(), vtl->L()-1, vtl->H()-1);

	p.end();

}

void visualMiniDisplay::newCell(int i, int j, groundType g) //, QPainter *p)
{
	QPainter p;
	boAssert(i<vtl->maxX());
	boAssert(j<vtl->maxY());

	//printf("visualMiniDisplay::newCell : receiving %d\n", (int)g);


	if (IS_TRANS(g))
		g = groundTransProp[ GET_TRANS_REF(g) ].from;

	p.begin(_ground);
	switch(g) {
		default:
			logf(LOG_ERROR, "visualMiniDisplay::newCell : unexpected groundType : %d", g);
		case GROUND_WATER :
		case GROUND_WATER_OIL:
			setPoint( i, j, blue, &p);
			break;
		case GROUND_GRASS :
		case GROUND_GRASS_OIL :
			setPoint( i, j, green, &p);
			break;
		case GROUND_DESERT :
			setPoint( i, j, darkYellow, &p);
			break;
		}
	p.end();
	repaint(FALSE);
}

void visualMiniDisplay::setPoint(int x, int y, const QColor &color, QPainter *p)
{
	if (!p) {
		logf(LOG_ERROR, "setPoint: p == 0...");
		return;
		}
	p->setPen(color);
	p->drawPoint(x,y);
}



void visualMiniDisplay::mousePressEvent(QMouseEvent *e)
{
	int x, y;

	x = e->x();
	y = e->y();

	if (e->button() & LeftButton) {
		emit reCenterView(x,y);
		return;
		}

}


