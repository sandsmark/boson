/***************************************************************************
                          infoWin.cpp  -  description                              
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

#include <qlayout.h>
#include <qwidgetstack.h>
#include <qframe.h>
#include <qscrollview.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <kapp.h>

#include "../common/log.h"
#include "../common/unitType.h"

#include "speciesTheme.h"

#include "infoWin.h"
//#include "bosonView.h"
//#include "visualBigDisplay.h"
#include "game.h"

#define VIEW_ONE	1
#define VIEW_MANY	2

infoWin::infoWin(QWidget *parent, const char *name)
	:QFrame(parent, name)
{
	int i;
QString path(kapp->kde_datadir() + "/boson/themes/panels/standard/overview_none.xpm" );

setFrameStyle(QFrame::Sunken | QFrame::Panel);
setLineWidth(5);


/* stack */
stack = new QWidgetStack(this, "qwidgetstack");
stack->setFrameStyle(QFrame::Raised | QFrame::Panel);
stack->setLineWidth(5);
stack->setGeometry(10,10,180,110);

/* stack/one */
view_none = new QPixmap(path);
if (view_none->isNull())
	printf("infoWin::infoWin : Can't load overview_none \n");

view_one = new QLabel(stack,"preview");
view_one->setPixmap(*view_none);
stack->addWidget(view_one, VIEW_ONE);

/* stack/many */
view_many = new QScrollView(stack,"scrollview");
stack->addWidget(view_many, VIEW_MANY);

stack->raiseWidget(VIEW_ONE);

/* orders buttons */
	for (i=0; i< 11; i++) {
		orderButton[i] = new QPushButton(this, "orderButtons");
		orderButton[i]->setGeometry( 10+(i%3)*60, 128+(i/3)*60, 55, 55);
		orderButton[i]->hide();
		}
	connect(orderButton[0], SIGNAL(clicked(void)), this, SLOT(bc0(void)));
	connect(orderButton[1], SIGNAL(clicked(void)), this, SLOT(bc1(void)));
	connect(orderButton[2], SIGNAL(clicked(void)), this, SLOT(bc2(void)));
	connect(orderButton[3], SIGNAL(clicked(void)), this, SLOT(bc3(void)));
	connect(orderButton[4], SIGNAL(clicked(void)), this, SLOT(bc4(void)));
	connect(orderButton[5], SIGNAL(clicked(void)), this, SLOT(bc5(void)));
	connect(orderButton[6], SIGNAL(clicked(void)), this, SLOT(bc6(void)));
	connect(orderButton[7], SIGNAL(clicked(void)), this, SLOT(bc7(void)));
	connect(orderButton[8], SIGNAL(clicked(void)), this, SLOT(bc8(void)));
	connect(orderButton[9], SIGNAL(clicked(void)), this, SLOT(bc9(void)));
	connect(orderButton[10], SIGNAL(clicked(void)), this, SLOT(bc10(void)));
	
	orderType = OT_NONE;
}

void infoWin::setSelected(QPixmap *p)
{
	view_one->setPixmap( p?*p:*view_none);
	setOrderType( OT_NONE, -1);
}


void infoWin::handleOrder(int order)
{
	switch(orderType) {
		default:
		case OT_NONE:
			logf(LOG_ERROR, "unexpected infoWin::handleOrder");
			return;
			break;
		case OT_FACILITY:
			break;
		case OT_MOBILE:
			break;
	}
}


void infoWin::setOrderType( int what, int who)
{
	int i;


	switch(what) {
		case OT_NONE:
			orderType = OT_NONE;
			for (i=0; i<11; i++) orderButton[i]->hide();
			break;
		case OT_FACILITY:
			if ( who!=who_am_i) return;
			orderType = OT_FACILITY;
			for (i=0; i<FACILITY_LAST; i++) {
				orderButton[i]->setPixmap( *myspecy->getSmallOverview((facilityType)i) );
				orderButton[i]->show();
			}
			for (i=FACILITY_LAST; i<11; i++) orderButton[i]->hide();
			break;
		case OT_MOBILE:
			if ( who!=who_am_i) return;
			orderType = OT_MOBILE;
			for (i=0; i<MOB_LAST; i++) {
				orderButton[i]->setPixmap( *myspecy->getSmallOverview( (mobType)i) );
				orderButton[i]->show();
			}
			for (i=MOB_LAST; i<11; i++) orderButton[i]->hide();
			break;
		default:
			logf(LOG_ERROR, "infoWin::setOrderType : unexpected 'what' argument");
	}
}
