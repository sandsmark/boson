/***************************************************************************
                          visualBigDisplay.cpp  -  description                              
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

#include <assert.h>

#include <kapp.h>

#include "../common/log.h"

#include "visualBigDisplay.h"
#include "visualCell.h"
#include "speciesTheme.h"
#include "groundTheme.h"
#include "visualView.h"
  

visualBigDisplay::visualBigDisplay(/*orderWin *o,*/ visualView *v, QWidget*parent, const char *name, WFlags f)
	: QWidget(parent, name, f)
	, QwAbsSpriteFieldView(v->field)
{

//setBackgroundColor(black);
//setBackgroundMode(fixedColor);

/* related orderWindows */
//order = o;

/* the visualView */
view = v;

// connect(, SIGNAL(), this, SLOT());
connect(view, SIGNAL(repaint(bool)), this, SLOT(repaint(bool)));
connect(this, SIGNAL(relativeReCenterView(int, int)), view, SLOT(relativeReCenterView(int, int)));
connect(this, SIGNAL(reSizeView(int, int)), view, SLOT(reSizeView(int, int)));

}

visualBigDisplay::~visualBigDisplay()
{
	QwAbsSpriteFieldView::view(0);
}


QRect visualBigDisplay::viewArea() const
{
//printf("visualBigDisplay::viewArea = %d.%d, %dx%d\n", BO_TILE_SIZE * view->X(), BO_TILE_SIZE * view->Y(), width(), height());
return QRect(BO_TILE_SIZE * view->X(), BO_TILE_SIZE * view->Y(), width(), height());
boAssert(width() / BO_TILE_SIZE == view->L());
boAssert(height() / BO_TILE_SIZE == view->H());
}

void visualBigDisplay::flush(const  QRect & area)
{
/* nothing special.. */
///orzel : to change is some kind of off-screen buffering is used
}

void visualBigDisplay::beginPainter (QPainter &p)
{
///orzel : to change if some kind of off-screen buffering is used

p.begin(this);
p.translate( - BO_TILE_SIZE * view->X(), - BO_TILE_SIZE * view->Y());
p.setBackgroundColor(black);

boAssert(p.backgroundColor() == black);

//p.setBackgroundMode(OpaqueMode);
}


void visualBigDisplay::paintEvent(QPaintEvent *evt)
{
	if (viewing) {
		QRect r = evt->rect();
///orzel : should be removed :
		r = rect();
//printf("r = %d.%d, %dx%d\n", r.x(), r.y(), r.width(), r.height());
		r.moveBy(view->X() * BO_TILE_SIZE, view->Y() * BO_TILE_SIZE);
//printf("visualBigDisplay::paintEvents, moved : %d.%d, %dx%d\n", r.x(), r.y(), r.width(), r.height());
		viewing->updateInView(this, r);
	}

}


