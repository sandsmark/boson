/***************************************************************************
                          infoWin.cpp  -  description                              
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

#include <qlayout.h>
#include <qwidgetstack.h>
#include <qframe.h>
#include <qscrollview.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <QwSpriteField.h>

#include <kapp.h>

#include "../common/log.h"
#include "../visual/groundTheme.h"

#include "infoWin.h"
//#include "visualView.h"
//#include "visualBigDisplay.h"
//#include "speciesTheme.h"
#include "visual.h"

#define VIEW_ONE	1
#define VIEW_MANY	2

infoWin::infoWin(QWidget *parent, const char *name)
	:QFrame(parent, name)
{
	int		i;
	QCheckBox	*qcheck;
	QString path(kapp->kde_datadir() + "/boson/themes/panels/standard/overview_none.xpm" );

	setFrameStyle(QFrame::Sunken | QFrame::Panel);
	setLineWidth(5);


/* stack */
stack = new QWidgetStack(this, "qwidgetstack");
stack->setFrameStyle(QFrame::Raised | QFrame::Panel);
stack->setLineWidth(5);
stack->setGeometry(10,10,110,110);

/* stack/one */
view_none = new QPixmap(path);
if (view_none->isNull())
	logf(LOG_ERROR, "infoWin::infoWin : Can't load overview_none");

view_one = new QLabel(stack,"preview");
view_one->setPixmap(*view_none);
stack->addWidget(view_one, VIEW_ONE);

/* stack/many */
view_many = new QScrollView(stack,"scrollview");
stack->addWidget(view_many, VIEW_MANY);

stack->raiseWidget(VIEW_ONE);


/* QCheckBoxes */
	qcheck = new QCheckBox("Invert", this, "checkbox inverted");
	qcheck->setGeometry(130,10,80,30);
	connect(qcheck, SIGNAL(toggled(bool)), this, SLOT(setInverted(bool)));
//	qcheck = new QCheckBox("Big tiles", this, "checkbox bigtiles");
//	qcheck->setGeometry(130,40,80,30);
//	connect(qcheck, SIGNAL(toggled(bool)), this, SLOT(setBigSize(bool)));

/* QComboBoxes */
	qcb_which = new QComboBox(this, "qcb_which");
	qcb_which->setGeometry(130,40,80,30);

	qcb_which->insertItem("small", 0);
	qcb_which->insertItem("big 1", 1);
	qcb_which->insertItem("big 2", 2);

	connect(qcb_which, SIGNAL(activated(int)), this, SLOT(setWhich(int)));
	
	qcb_transRef = new QComboBox(this, "qcb_transRef");
	qcb_transRef->setGeometry(130,82,100,30);

	qcb_transRef->insertItem("grass/water", TRANS_GW);
	qcb_transRef->insertItem("grass/desert", TRANS_GD);
	qcb_transRef->insertItem("desert/water", TRANS_DW);
	qcb_transRef->insertItem("deep water", TRANS_DWD);

	connect(qcb_transRef, SIGNAL(activated(int)), this, SLOT(setTransRef(int)));
	
/* QPushButton */
	for (i=0; i<9; i++){
		tiles[i] = new QPushButton(this ,"tiles");
		tiles[i]->setGeometry(10+(i%3)*60, 128+(i/3)*60, 55, 55);
	}
	for (i=0; i<4; i++){
		bigTiles[i] = new QPushButton(this ,"bigTiles");
		bigTiles[i]->setGeometry(10+(i%2)*110, 128+(i/2)*110, 104, 104);
	}

	connect(tiles[0], SIGNAL(clicked(void)), this, SLOT(bc0(void)));
	connect(tiles[1], SIGNAL(clicked(void)), this, SLOT(bc1(void)));
	connect(tiles[2], SIGNAL(clicked(void)), this, SLOT(bc2(void)));
	connect(tiles[3], SIGNAL(clicked(void)), this, SLOT(bc3(void)));
	connect(tiles[4], SIGNAL(clicked(void)), this, SLOT(bc4(void)));
	connect(tiles[5], SIGNAL(clicked(void)), this, SLOT(bc5(void)));
	connect(tiles[6], SIGNAL(clicked(void)), this, SLOT(bc6(void)));
	connect(tiles[7], SIGNAL(clicked(void)), this, SLOT(bc7(void)));
	connect(tiles[8], SIGNAL(clicked(void)), this, SLOT(bc8(void)));

	connect(bigTiles[0], SIGNAL(clicked(void)), this, SLOT(bc0(void)));
	connect(bigTiles[1], SIGNAL(clicked(void)), this, SLOT(bc1(void)));
	connect(bigTiles[2], SIGNAL(clicked(void)), this, SLOT(bc2(void)));
	connect(bigTiles[3], SIGNAL(clicked(void)), this, SLOT(bc3(void)));

	inverted	= false;
	trans		= TRANS_GW;
	setWhich	(0);
}


void infoWin::setSelected(QPixmap *p)
{
	view_one->setPixmap( p?*p:*view_none);
	emit setSelectedTile (GROUND_UNKNOWN);
}

void infoWin::setTransRef(int r)
{
	trans = r;
	redrawTiles();
}

void infoWin::redrawTiles(void)
{
	QwSpritePixmapSequence *seq;

	
	switch(which) {
		case 1:
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?16:12) + 0) );
			bigTiles[0]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?16:12) + 1) );
			bigTiles[1]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?16:12) + 2) );
			bigTiles[2]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?16:12) + 3) );
			bigTiles[3]->setPixmap(*seq->image(0));
			break;

		case 2:
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?24:20) + 0) );
			bigTiles[0]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?24:20) + 1) );
			bigTiles[1]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?24:20) + 2) );
			bigTiles[2]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?24:20) + 3) );
			bigTiles[3]->setPixmap(*seq->image(0));
			break;
		case 0:
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_ULI:TRANS_UL) );
			tiles[0]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_DOWN:TRANS_UP) );
			tiles[1]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_URI:TRANS_UR) );
			tiles[2]->setPixmap(*seq->image(0));

			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_RIGHT:TRANS_LEFT) );
			tiles[3]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_LEFT:TRANS_RIGHT) );
			tiles[5]->setPixmap(*seq->image(0));

			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_DLI:TRANS_DL) );
			tiles[6]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_UP:TRANS_DOWN) );
			tiles[7]->setPixmap(*seq->image(0));
			seq = vpp.ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_DRI:TRANS_DR) );
			tiles[8]->setPixmap(*seq->image(0));
			
			// middle one
			seq = vpp.ground->getPixmap( inverted?groundTransProp[trans].to:groundTransProp[trans].from);
			tiles[4]->setPixmap(*seq->image(0));
			break;
	} // switch()

}

void infoWin::setInverted(bool b)
{
	if (b == inverted) return;
	
	inverted = b;
	redrawTiles();
}


void infoWin::setWhich(int w)
{
	int i;
	
	if (w == which) return;
	if (w<0 || w>2) {
		logf(LOG_ERROR, "infoWin::setWhich : unknown w");
		return;
	}
	
	which = w;

	switch(which) {
		case 1:
		case 2:  // big tiles 1 or 2
			for (i=0; i<9; i++)
					tiles[i]->hide();
			for (i=0; i<4; i++)
					bigTiles[i]->show();
			break;

		case 0: // small tiles
			for (i=0; i<4; i++)
					bigTiles[i]->hide();
			for (i=0; i<9; i++)
					tiles[i]->show();
			break;
	} // switch(which)

	redrawTiles();
}


void infoWin::handleButton(int but)
{
	static const int m_map[] = {
		TRANS_UL,	TRANS_UP,	TRANS_UR,
		TRANS_LEFT,	0/**/,		TRANS_RIGHT,
		TRANS_DL,	TRANS_DOWN,	TRANS_DR,
		TRANS_ULI,	TRANS_DOWN,	TRANS_URI,
		TRANS_RIGHT,	0/**/,		TRANS_LEFT,
		TRANS_DLI,	TRANS_UP,	TRANS_DRI};
	groundType	g;

	boAssert(but>=0);
	switch(which) {
		default:
			logf(LOG_ERROR, "infoWin::handleButton, unexpected which..");
			g = GROUND_WATER;
			break;
		case 1:
			boAssert(but<4);
			g = GET_TRANS_NUMBER(trans, (inverted?16:12) + but);
			break;
		case 2:
			boAssert(but<4);
			g = GET_TRANS_NUMBER(trans, (inverted?24:20) + but);
			break;
		case 0: // small tiles
			boAssert(but<9);
			if (4 == but)
				g = inverted?groundTransProp[trans].to:groundTransProp[trans].from;
			else	g = GET_TRANS_NUMBER(trans, m_map[ (inverted?9:0) + but ]);
			break;
	} // switch(which)
	
	setSelected( vpp.ground->getPixmap(g)->image(0));
	emit setSelectedTile (g);		// need to be after the setSelected
}

