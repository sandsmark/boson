/***************************************************************************
                          bosonTopLevel.cpp  -  description                              
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

#include <qpushbutton.h>
#include <qhbox.h>

#include "common/log.h"
#include "common/bomap.h"

#include "bosonTopLevel.h"
#include "visualMiniDisplay.h"
#include "bosonBigDisplay.h"
#include "bosonCanvas.h"
#include "speciesTheme.h"
#include "boson.h"

#include "game.h"


bosonTopLevel::bosonTopLevel(BosonApp * /*parent*/, const char *name, WFlags f)
	: visualTopLevel(name,f)
	, mw(this)
{

	/* init the statusBar
	 * remember that bocanvas has been initialised before any bosonTopLevel is created
	 */
	QLabel *label;
	QHBox *qhb;
	KStatusBar *ksb = statusBar();

	qhb   = new QHBox(ksb, "unitsInfoBox");
	label = new QLabel(" Mobiles : ", qhb);
	label = new QLabel("?", qhb);
	label = new QLabel("  Facilities : ", qhb);
	label = new QLabel("?", qhb);
	ksb->addWidget(qhb);

	qhb   = new QHBox(ksb, "ressourcesInfoBox");
	label = new QLabel(" Oil : ", qhb);
	label = new QLabel("?", qhb);
	connect(bocanvas , SIGNAL(oilUpdated(int)), label, SLOT(setNum(int)));
	label = new QLabel("  Minerals : ", qhb);
	label = new QLabel("?", qhb);
	connect(bocanvas , SIGNAL(mineralUpdated(int)), label, SLOT(setNum(int)));
	ksb->addWidget(qhb);

	ksb->insertItem( "Boson beginning", 0, 100);

	qhb   = new QHBox(ksb, "pingInfoBox");
	label = new QLabel(" Server Ping : ", qhb);
	label = new QLabel("?", qhb);
	ksb->addWidget(qhb);



	/* .. */
	orderType = OT_NONE;
	setView(&mw, false);

	/* orders buttons */
	for (int i=0; i< 11; i++) { ///orzel : why 11, use *Nb
		orderButton[i] = new QPushButton(mw.mainFrame, "orderButtons");
		orderButton[i]->setGeometry( 10+(i%3)*60, 141+(i/3)*60, 55, 55);
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

}

void bosonTopLevel::setOrders( int what, int who)
{
	int i;

	switch(what) {
		case OT_NONE:
			orderType = OT_NONE;
			for (i=0; i<11; i++) orderButton[i]->hide();
			break;
		case OT_FACILITY:
			if ( (uint) who!=who_am_i) return;
			orderType = OT_FACILITY;
			for (i=0; i<FACILITY_LAST; i++) {
				orderButton[i]->setPixmap( *myspecy->getSmallOverview((facilityType)i) );
				orderButton[i]->show();
			}
			for (i=FACILITY_LAST; i<11; i++) orderButton[i]->hide();
			break;
		case OT_MOBILE:
			if ( (uint) who!=who_am_i) return;
			orderType = OT_MOBILE;
			for (i=0; i<MOB_LAST; i++) {
				orderButton[i]->setPixmap( *myspecy->getSmallOverview( (mobType)i) );
				orderButton[i]->show();
			}
			for (i=MOB_LAST; i<11; i++) orderButton[i]->hide();
			break;
		default:
			logf(LOG_ERROR, "bosonTopLevel::setOrders : unexpected 'what' argument");
	}
}


void bosonTopLevel::object_put(int x, int y)
{
	construct.x = X() + (x / BO_TILE_SIZE) ;
	construct.y = Y() + (y / BO_TILE_SIZE) ;
	switch(orderType) {
		case OT_FACILITY:
			sendMsg(buffer, MSG_FACILITY_CONSTRUCT, MSG(construct) );
			break;
		case OT_MOBILE:
			sendMsg(buffer, MSG_MOBILE_CONSTRUCT, MSG(construct) );
			break;
		default:
			logf(LOG_ERROR, "object_put : unexpected \"orderType\" value");
	}
}


void bosonTopLevel::handleOrder(int order)
{
	switch(orderType) {
		default:
		case OT_NONE:
			logf(LOG_ERROR, "unexpected mainWidget::handleOrder");
			return;
			break;
		case OT_FACILITY:
			construct.type.fix = (facilityType) order;
			setSelectionMode( SELECT_PUT);
			break;
		case OT_MOBILE:
			construct.type.mob = (mobType) order;
			setSelectionMode( SELECT_PUT);
			break;
	}
}


void bosonTopLevel::setSelected(QPixmap *p)
{
	mw.view_one->setPixmap( p?*p:*mw.view_none);
}


void bosonTopLevel::updateViews(void)
{
	mw.big->setContentsPos( X() * BO_TILE_SIZE, Y() * BO_TILE_SIZE );
	mw.big->update();
	mw.mini->repaint(FALSE);
}


