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
#include "visualView.h"

visualMiniDisplay::visualMiniDisplay(visualView *v, QWidget*parent, const char *name=0L)
	: QWidget(parent, name)
{

	visualField *field = v -> field;

/* the visualView */
	view = v;

/* create the (back)ground pixmap */
	ground = new QPixmap(v->maxX(), v->maxY());
	ground->fill(black);

/* make the connection */
	connect(field, SIGNAL(newCell(int,int, groundType)), this, SLOT(newCell(int,int, groundType)));
	connect(field, SIGNAL(updateMobile(visualMobUnit *)), this, SLOT(drawMobile(visualMobUnit *)));
	connect(field, SIGNAL(updateFix(visualFacility *)), this, SLOT(drawFix(visualFacility *)));

// connect(, SIGNAL(), this, SLOT());
	connect(view, SIGNAL(repaint(bool)), this, SLOT(repaint(bool)));
	connect(this, SIGNAL(reCenterView(int, int)), view, SLOT(reCenterView(int, int)));
	connect(this, SIGNAL(reSizeView(int, int)), view, SLOT(reSizeView(int, int)));
}


void visualMiniDisplay::paintEvent(QPaintEvent *evt)
{
	QPainter p;

	p.begin(this);

	/* map is buffered */
	p.drawPixmap(0,0,*ground);

	/* the little rectangle */
	p.setPen(white);
	p.setRasterOp(XorROP);
	p.drawRect(view->X(), view->Y(), view->L()-1, view->H()-1);

	p.end();

}

void visualMiniDisplay::newCell(int i, int j, groundType g) //, QPainter *p)
{
	QPainter p;
	boAssert(i<view->maxX());
	boAssert(j<view->maxY());

	//printf("visualMiniDisplay::newCell : receiving %d\n", (int)g);


	if (IS_TRANS(g))
		g = groundTransProp[ GET_TRANS_REF(g) ].from;

	p.begin(ground);
	switch(g) {
		default:
			logf(LOG_ERROR, "visualMiniDisplay::drawCell : unexpected groundType");
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


