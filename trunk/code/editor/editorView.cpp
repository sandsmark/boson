/***************************************************************************
                          editorView.cpp  -  description                              
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
#include <qcombobox.h>
#include <qcheckbox.h>

#include <QwSpriteField.h>

#include <kapp.h>

#include "../common/log.h"
#include "../visual/groundTheme.h"
#include "../visual/speciesTheme.h"

#include "editorView.h"
//#include "visualView.h"
//#include "visualBigDisplay.h"
//#include "speciesTheme.h"
#include "visual.h"

#define VIEW_ONE	1
#define VIEW_MANY	2

editorView::editorView (visualField *p, QWidget *parent, const char *name=0L)
	:visualView(p,parent,name)
{
	int		i;
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
		logf(LOG_ERROR, "editorView::editorView : Can't load overview_none");

	view_one = new QLabel(stack,"preview");
	view_one->setPixmap(*view_none);
	stack->addWidget(view_one, VIEW_ONE);

	/* stack/many */
	view_many = new QScrollView(stack,"scrollview");
	stack->addWidget(view_many, VIEW_MANY);

	stack->raiseWidget(VIEW_ONE);


/* QCheckBoxes */
	invertBox = new QCheckBox("Invert", this, "checkbox inverted");
	invertBox->setGeometry(130,10,80,30);
	connect(invertBox, SIGNAL(toggled(bool)), this, SLOT(setInverted(bool)));

/* QComboBoxes */
	qcb_which = new QComboBox(this, "qcb_which");
	qcb_which->setGeometry(130,40,80,30);

	qcb_which->insertItem("small",		W_SMALL_GROUND);
	qcb_which->insertItem("big 1",		W_BIG_GROUND_1);
	qcb_which->insertItem("big 2",		W_BIG_GROUND_2);
	qcb_which->insertItem("Facilities",	W_FACILITIES);
	qcb_which->insertItem("Units",		W_UNITS);

	connect(qcb_which, SIGNAL(activated(int)), this, SLOT(setWhich(int)));
	
	qcb_transRef = new QComboBox(this, "qcb_transRef");
	qcb_transRef->setGeometry(130,82,100,30);

	qcb_transRef->insertItem("grass/water", TRANS_GW);
	qcb_transRef->insertItem("grass/desert", TRANS_GD);
	qcb_transRef->insertItem("desert/water", TRANS_DW);
	qcb_transRef->insertItem("deep water", TRANS_DWD);

	connect(qcb_transRef, SIGNAL(activated(int)), this, SLOT(setTransRef(int)));
	
	qcb_who = new QComboBox(this, "qcb_who");
	qcb_who->setGeometry(130,82,100,30);

	qcb_who->insertItem("User 0", 0);
	qcb_who->insertItem("User 1", 1);
//	qcb_who->insertItem("User 2", 2);
//	qcb_who->insertItem("User 3", 3);

	connect(qcb_who, SIGNAL(activated(int)), this, SLOT(_setWho(int)));
	
	
/* QPushButton */
	for (i=0; i<TILES_NB; i++){
		tiles[i] = new QPushButton(this ,"tiles");
		tiles[i]->setGeometry(10+(i%3)*60, 128+(i/3)*60, 55, 55);
	}
	for (i=0; i<BIG_TILES_NB; i++){
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
	connect(tiles[9], SIGNAL(clicked(void)), this, SLOT(bc9(void)));
	connect(tiles[10], SIGNAL(clicked(void)), this, SLOT(bc10(void)));
//	connect(tiles[11], SIGNAL(clicked(void)), this, SLOT(bc11(void)));

	connect(bigTiles[0], SIGNAL(clicked(void)), this, SLOT(bc0(void)));
	connect(bigTiles[1], SIGNAL(clicked(void)), this, SLOT(bc1(void)));
	connect(bigTiles[2], SIGNAL(clicked(void)), this, SLOT(bc2(void)));
	connect(bigTiles[3], SIGNAL(clicked(void)), this, SLOT(bc3(void)));

	inverted	= false;
	trans		= TRANS_GW;
	setOrders	(0);
	otype		= OT_NONE;
	who		= 0;
	qcb_who->hide();
}


void editorView::setSelected(QPixmap *p)
{
	view_one->setPixmap( p?*p:*view_none);
	emit setSelectedObject (OT_NONE, 0);
}


void editorView::_setWho(int w)
{
	who = w;
	emit setWho(w);
	redrawTiles();
}


void editorView::setTransRef(int r)
{
	trans = r;
	redrawTiles();
}

void editorView::redrawTiles(void)
{
	QwSpritePixmapSequence *seq;
	int 	i;

	
	switch(which) {
		case W_BIG_GROUND_1:
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?16:12) + 0) );
			bigTiles[0]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?16:12) + 1) );
			bigTiles[1]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?16:12) + 2) );
			bigTiles[2]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?16:12) + 3) );
			bigTiles[3]->setPixmap(*seq->image(0));
			break;

		case W_BIG_GROUND_2:
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?24:20) + 0) );
			bigTiles[0]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?24:20) + 1) );
			bigTiles[1]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?24:20) + 2) );
			bigTiles[2]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, (inverted?24:20) + 3) );
			bigTiles[3]->setPixmap(*seq->image(0));
			break;

		case W_SMALL_GROUND:
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_ULI:TRANS_UL) );
			tiles[0]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_DOWN:TRANS_UP) );
			tiles[1]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_URI:TRANS_UR) );
			tiles[2]->setPixmap(*seq->image(0));

			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_RIGHT:TRANS_LEFT) );
			tiles[3]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_LEFT:TRANS_RIGHT) );
			tiles[5]->setPixmap(*seq->image(0));

			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_DLI:TRANS_DL) );
			tiles[6]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_UP:TRANS_DOWN) );
			tiles[7]->setPixmap(*seq->image(0));
			seq = ground->getPixmap( GET_TRANS_NUMBER(trans, inverted?TRANS_DRI:TRANS_DR) );
			tiles[8]->setPixmap(*seq->image(0));
			
			// middle one
			seq = ground->getPixmap( inverted?groundTransProp[trans].to:groundTransProp[trans].from);
			tiles[4]->setPixmap(*seq->image(0));
			break;

		case W_FACILITIES:
			for (i=0; i< facilityPropNb; i++)
				tiles[i]->setPixmap( *species[who]->getSmallOverview( (facilityType)i ) );
			break;
		case W_UNITS:
			for (i=0; i< mobilePropNb; i++)
				tiles[i]->setPixmap( *species[who]->getSmallOverview( (mobType)i ) );
			break;
	} // switch()

}

void editorView::setInverted(bool b)
{
	if (b == inverted) return;
	
	inverted = b;
	redrawTiles();
}


void editorView::setOrders(int whatb , int who)
{
	int i, j;
	which_t what;
	
	switch(whatb) {
		case -1:
		case 10:
		case 11:
			return;
	}

	what = (which_t) whatb; // casting needed cause setOrders is virtual from visual
	
	if (what == which) return;
	if (what<W_SMALL_GROUND || what>W_UNITS) {
		logf(LOG_ERROR, "editorView::setOrders : unknown what");
		return;
	}
	
	which = what;

	for (i=0; i<TILES_NB; i++)
			tiles[i]->hide();
	for (i=0; i<BIG_TILES_NB; i++)
			bigTiles[i]->hide();

	i = j = 0;

	switch(which) {
		case W_UNITS:
			i = mobilePropNb;
			qcb_transRef->hide();
			invertBox->hide();
			qcb_who->show();

			break;
		case W_FACILITIES:
			i = facilityPropNb;
			qcb_transRef->hide();
			invertBox->hide();
			qcb_who->show();
			break;
		case W_BIG_GROUND_1:
		case W_BIG_GROUND_2:
			j = 4;
			qcb_who->hide();
			qcb_transRef->show();
			invertBox->show();
			break;
		case W_SMALL_GROUND:
			i = 9;
			qcb_who->hide();
			qcb_transRef->show();
			invertBox->show();
			break;
		default : 
			logf(LOG_ERROR, "editorView::setOrders : unhandled which in switch");
	} // switch(which)
	
	boAssert(i<=TILES_NB);
	boAssert(j<=BIG_TILES_NB);

	for (i--; i>=0; i--)
			tiles[i]->show();
	for (j--; j>=0; j--)
			bigTiles[j]->show();

	redrawTiles();
}


void editorView::handleButton(int but)
{
	static const int m_map[] = {
		TRANS_UL,	TRANS_UP,	TRANS_UR,
		TRANS_LEFT,	0/**/,		TRANS_RIGHT,
		TRANS_DL,	TRANS_DOWN,	TRANS_DR,
		TRANS_ULI,	TRANS_DOWN,	TRANS_URI,
		TRANS_RIGHT,	0/**/,		TRANS_LEFT,
		TRANS_DLI,	TRANS_UP,	TRANS_DRI};
	groundType g;

	boAssert(but>=0);
	switch(which) {
		default:
			logf(LOG_ERROR, "editorView::handleButton, unexpected which..");
			break;

		case W_FACILITIES:
			boAssert(but>=0);
			boAssert(but<facilityPropNb);
			otype = OT_FACILITY;
			setSelected( species[who]->getBigOverview( (facilityType)but ));
			emit setSelectedObject (otype, but);		// need to be after the setSelected
			break;

		case W_UNITS:
			boAssert(but>=0);
			boAssert(but<mobilePropNb);
			otype = OT_UNIT;
			setSelected( species[who]->getBigOverview( (mobType)but ));
			emit setSelectedObject (otype, but);		// need to be after the setSelected
			break;

		case W_BIG_GROUND_1:
			boAssert(but>=0);
			boAssert(but<4);
			g = GET_TRANS_NUMBER(trans, (inverted?16:12) + but);

			otype = OT_GROUND;
			setSelected( ground->getPixmap(g)->image(0));
			emit setSelectedObject (otype, g);		// need to be after the setSelected
			break;

		case W_BIG_GROUND_2:
			boAssert(but>=0);
			boAssert(but<4);
			g = GET_TRANS_NUMBER(trans, (inverted?24:20) + but);

			otype = OT_GROUND;
			setSelected( ground->getPixmap(g)->image(0));
			emit setSelectedObject (otype, g);		// need to be after the setSelected
			break;

		case W_SMALL_GROUND:
			boAssert(but>=0);
			boAssert(but<9);
			if (4 == but)
				g = inverted?groundTransProp[trans].to:groundTransProp[trans].from;
			else	g = GET_TRANS_NUMBER(trans, m_map[ (inverted?9:0) + but ]);

			otype = OT_GROUND;
			setSelected( ground->getPixmap(g)->image(0));
			emit setSelectedObject (otype, g);		// need to be after the setSelected
			break;

	} // switch(which)
}


