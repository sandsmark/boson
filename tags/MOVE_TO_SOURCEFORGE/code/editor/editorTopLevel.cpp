/***************************************************************************
                          editorTopLevel.cpp  -  description                              
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


#include <qlayout.h>
#include <qwidgetstack.h>
#include <qframe.h>
#include <qscrollview.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <klocale.h>		// i18n()
#include <kmessagebox.h>	// KMessageBox::information()
#include <kstdaction.h>

#include "common/log.h"
#include "common/bomap.h"

#include "editorTopLevel.h"
#include "editorCanvas.h"
#include "speciesTheme.h"
#include "boeditor.h"

#include "editorBigDisplay.h"
#include "visualMiniDisplay.h"
#include "game.h"

#define VIEW_ONE	1
#define VIEW_MANY	2


#define BITBLT(_x,_y) bitBlt(p, (_x), (_y) , bigBackground, GET_BIG_X(g), GET_BIG_Y(g), BO_TILE_SIZE, BO_TILE_SIZE)
static void fillGroundPixmap( QPixmap *p, int g)
{
	p->fill(); // clear widget

	g<<=2;

	BITBLT( 0 , 0);

	if (IS_BIG_TRANS(g>>2)) {
		g+=4; BITBLT( BO_TILE_SIZE, 0);
		g+=4; BITBLT( 0 , BO_TILE_SIZE);
		g+=4; BITBLT( BO_TILE_SIZE, BO_TILE_SIZE);
	}
}
#undef BITBLT


#define ADD_ACTION(name) KStdAction::##name(this, SLOT(slot_##name()), actionCollection() );
editorTopLevel::editorTopLevel( BoEditorApp *app,  const char *name, WFlags f)
	: visualTopLevel(name,f)
	, mw(this)
	, beapp(app)
{

	/* init the statusBar
	 * remember that ecanvas has been initialised in main.cpp before any editorTopLevel
	 */
	QLabel *label;
	QHBox *qhb;
	KStatusBar *ksb = statusBar();

	qhb   = new QHBox(ksb, "unitsInfoBox");
	label = new QLabel(" Mobiles : ", qhb);
	label = new QLabel("?", qhb);
	connect(ecanvas , SIGNAL(mobileNbUpdated(int)), label, SLOT(setNum(int)));
	label = new QLabel("  Facilities : ", qhb);
	label = new QLabel("?", qhb);
	connect(ecanvas , SIGNAL(facilityNbUpdated(int)), label, SLOT(setNum(int)));
	ksb->addWidget(qhb);

	/* toplevelwindow-specific actions */
	(void) new KAction(
		i18n("&Destroy objects"), Qt::CTRL + Qt::Key_E,
		this, SLOT(slot_editDestroy()),
		actionCollection(), "edit_destroy");
	(void) new KAction(
		i18n("&Add a new player"), Qt::CTRL + Qt::Key_A,
		this, SLOT(slot_addPlayer()),
		actionCollection(), "edit_add_player");
	ADD_ACTION(close);
//	ADD_ACTION(showToolbar);

	/* application wide actions */
	*actionCollection() +=  app->actions();

	/* widgets */
	makeCommandGui();
	setCentralWidget(&mw);

	/* actual building */
	createGUI("boeditorui.rc");

	resize (790, 590);

}
#undef ADD_ACTION


void editorTopLevel::setSelected(QPixmap *p)
{
	view_one->setPixmap( p?*p:*view_none);
	emit setSelectedObject (OT_NONE, 0);
}


void editorTopLevel::_setWho(int w)
{
	who = (uint)w;
	emit setWho(who);
	redrawTiles();
}


void editorTopLevel::setTransRef(int r)
{
	trans = r;
	redrawTiles();
}

void editorTopLevel::redrawTiles(void)
{
	int 	i;
	QPixmap p (BO_TILE_SIZE, BO_TILE_SIZE);
	QPixmap p2 (2*BO_TILE_SIZE, 2*BO_TILE_SIZE);

	
	switch(which) {
		case W_BIG_GROUND_1:
			for(i=0; i<4; i++) {
				fillGroundPixmap( &p2, GET_BIG_TRANS_NUMBER(trans, (inverted?4:0) + i));
				bigTiles[i]->setPixmap(p2);
			}
			break;

		case W_BIG_GROUND_2:
			for(i=0; i<4; i++) {
				fillGroundPixmap( &p2, GET_BIG_TRANS_NUMBER(trans, (inverted?12:8) + i));
				bigTiles[i]->setPixmap(p2);
			}
			break;

		case W_SMALL_GROUND:
			fillGroundPixmap ( &p, GET_TRANS_NUMBER(trans, inverted?TRANS_ULI:TRANS_UL) );
			tiles[0]->setPixmap(p);
			fillGroundPixmap ( &p, GET_TRANS_NUMBER(trans, inverted?TRANS_DOWN:TRANS_UP) );
			tiles[1]->setPixmap(p);
			fillGroundPixmap ( &p, GET_TRANS_NUMBER(trans, inverted?TRANS_URI:TRANS_UR) );
			tiles[2]->setPixmap(p);

			fillGroundPixmap ( &p, GET_TRANS_NUMBER(trans, inverted?TRANS_RIGHT:TRANS_LEFT) );
			tiles[3]->setPixmap(p);
			fillGroundPixmap ( &p, GET_TRANS_NUMBER(trans, inverted?TRANS_LEFT:TRANS_RIGHT) );
			tiles[5]->setPixmap(p);

			fillGroundPixmap ( &p, GET_TRANS_NUMBER(trans, inverted?TRANS_DLI:TRANS_DL) );
			tiles[6]->setPixmap(p);
			fillGroundPixmap ( &p, GET_TRANS_NUMBER(trans, inverted?TRANS_UP:TRANS_DOWN) );
			tiles[7]->setPixmap(p);
			fillGroundPixmap ( &p, GET_TRANS_NUMBER(trans, inverted?TRANS_DRI:TRANS_DR) );
			tiles[8]->setPixmap(p);
			
			// middle one
			fillGroundPixmap ( &p, inverted?groundTransProp[trans].to:groundTransProp[trans].from);
			tiles[4]->setPixmap(p);
			break;
			
		case W_SMALL_PLAIN:
			for (i=1; i<GROUND_LAST; i++) {
				fillGroundPixmap( &p, i );
				tiles[i-1]->setPixmap(p);
			}
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

void editorTopLevel::setInverted(bool b)
{
	if (b == inverted) return;
	
	inverted = b;
	redrawTiles();
}


void editorTopLevel::setOrders(int whatb , int )
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
	if (what<W_SMALL_PLAIN || what>W_UNITS) {
		logf(LOG_ERROR, "editorTopLevel::setOrders : unknown what");
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
		case W_SMALL_PLAIN:
			i = GROUND_LAST-1; // -1 caused by GROUND_UNKNOWN
			qcb_who->hide();
			qcb_transRef->hide();
			invertBox->hide();
			break;
		default : 
			logf(LOG_ERROR, "editorTopLevel::setOrders : unhandled which in switch");
	} // switch(which)
	
	boAssert(i<=TILES_NB);
	boAssert(j<=BIG_TILES_NB);

	for (i--; i>=0; i--)
			tiles[i]->show();
	for (j--; j>=0; j--)
			bigTiles[j]->show();

	redrawTiles();
}


void editorTopLevel::handleButton(int but)
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

	setSelectionMode( visualTopLevel::SELECT_NONE);

	switch(which) {
		default:
			logf(LOG_ERROR, "editorTopLevel::handleButton, unexpected which..");
			break;

		case W_FACILITIES:
			boAssert(but<facilityPropNb);
			otype = OT_FACILITY;
			setSelected( species[who]->getBigOverview( (facilityType)but ));
			emit setSelectedObject (otype, but);		// need to be after the setSelected
			mw.bigDisplay()->ready4put( QSize (facilityProp[but].width, facilityProp[but].height) );
			setSelectionMode( visualTopLevel::SELECT_PUT);
			break;

		case W_UNITS:
			boAssert(but<mobilePropNb);
			otype = OT_UNIT;
			setSelected( species[who]->getBigOverview( (mobType)but ));
			emit setSelectedObject (otype, but);		// need to be after the setSelected
			mw.bigDisplay()->ready4put( QSize (mobileProp[but].width, mobileProp[but].height) );
			setSelectionMode( visualTopLevel::SELECT_PUT);
			break;

		case W_BIG_GROUND_1:
			boAssert(but<4);
			g = GET_BIG_TRANS_NUMBER (trans,  (inverted?4:0) + but );
			otype = OT_GROUND;
			setSelected ( & QPixmap( * bigTiles[but]->pixmap()) );
			emit setSelectedObject (otype, g);		// need to be after the setSelected
			break;

		case W_BIG_GROUND_2:
			boAssert(but<4);
			g = GET_BIG_TRANS_NUMBER (trans,  (inverted?12:8) + but );
			otype = OT_GROUND;
			setSelected ( & QPixmap( * bigTiles[but]->pixmap()) );
			emit setSelectedObject (otype, g);		// need to be after the setSelected
			break;

		case W_SMALL_GROUND:
			boAssert(but<9);

			if (4 == but)
				g = inverted?groundTransProp[trans].to:groundTransProp[trans].from;
			else	g = GET_TRANS_NUMBER(trans, m_map[ (inverted?9:0) + but ]);
			otype = OT_GROUND;
			setSelected( & QPixmap ( * tiles[but]->pixmap()) );
			emit setSelectedObject (otype, g);		// need to be after the setSelected
			if (g<GROUND_GRASS_MINERAL) setSelectionMode( visualTopLevel::SELECT_FILL);
			break;

		case W_SMALL_PLAIN:
			boAssert(but<GROUND_LAST-1);
			g = (groundType) (but+1); // +1 cause GROUND_UNKNOWN
			otype = OT_GROUND;
			setSelected( & QPixmap ( * tiles[but]->pixmap()) );
			emit setSelectedObject (otype, g);		// need to be after the setSelected
			if (g<GROUND_GRASS_MINERAL) setSelectionMode( visualTopLevel::SELECT_FILL);
			break;

	} // switch(which)
}


void editorTopLevel::slot_playerAdded(uint np)
{
	QString pl;
	qcb_who->clear();
	for (uint i = 0; i<np; i++) {
		pl = QString("Player %1").arg(i+1);	// let's print [1..n] instead of [0..n-1], users aren't developpers
		qcb_who->insertItem(pl, i);
	}
}


void editorTopLevel::makeCommandGui(void)
{
	int		i;

	mainFrame = mw.mainFrame;

	mainFrame->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::MinimumExpanding ) );
	mainFrame->setMinimumSize (220, 200);


	mainFrame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
	mainFrame->setLineWidth(5);

	/* stack */
	stack = new QWidgetStack(mainFrame, "qwidgetstack");
	stack->setFrameStyle(QFrame::Raised | QFrame::Panel);
	stack->setLineWidth(5);
	stack->setGeometry(10,10,110,110);

	/* stack/one */
	view_none   = new QPixmap();

	view_one = new QLabel(stack,"preview");
	view_one->setPixmap(*view_none);
	stack->addWidget(view_one, VIEW_ONE);

	/* stack/many */
	view_many = new QScrollView(stack,"scrollview");
	stack->addWidget(view_many, VIEW_MANY);

	stack->raiseWidget(VIEW_ONE);


/* QCheckBoxes */
	invertBox = new QCheckBox("Invert", mainFrame, "checkbox inverted");
	invertBox->setGeometry(130,10,80,30);
	connect(invertBox, SIGNAL(toggled(bool)), this, SLOT(setInverted(bool)));

/* QComboBoxes */
	qcb_which = new QComboBox(mainFrame, "qcb_which");
	qcb_which->setGeometry(130,40,90,30);

	qcb_which->insertItem("Plain tiles",	W_SMALL_PLAIN);
	qcb_which->insertItem("small",		W_SMALL_GROUND);
	qcb_which->insertItem("big 1",		W_BIG_GROUND_1);
	qcb_which->insertItem("big 2",		W_BIG_GROUND_2);
	qcb_which->insertItem("Facilities",	W_FACILITIES);
	qcb_which->insertItem("Units",		W_UNITS);

	connect(qcb_which, SIGNAL(activated(int)), this, SLOT(setWhich(int)));
	
	qcb_transRef = new QComboBox(mainFrame, "qcb_transRef");
	qcb_transRef->setGeometry(130,82,90,30);

	qcb_transRef->insertItem("grass/water",		TRANS_GW);
	qcb_transRef->insertItem("grass/desert",	TRANS_GD);
	qcb_transRef->insertItem("desert/water",	TRANS_DW);
	qcb_transRef->insertItem("deep water",		TRANS_DWD);

	connect(qcb_transRef, SIGNAL(activated(int)), this, SLOT(setTransRef(int)));
	

/* the who box */
	qcb_who = new QComboBox(mainFrame, "qcb_who");
	qcb_who->setGeometry(130,82,90,30);
	connect(qcb_who, SIGNAL(activated(int)), this, SLOT(_setWho(int)));
	connect (ecanvas, SIGNAL(nbPlayerChanged(uint)), this, SLOT(slot_playerAdded(uint)) );

/* QPushButton */
	for (i=0; i<TILES_NB; i++){
		tiles[i] = new QPushButton(mainFrame ,"tiles");
		tiles[i]->setGeometry(10+(i%3)*60, 128+(i/3)*60, 55, 55);
	}
	for (i=0; i<BIG_TILES_NB; i++){
		bigTiles[i] = new QPushButton(mainFrame ,"bigTiles");
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
	otype		= OT_GROUND;
	who		= 0;
	which		= W_BIG_GROUND_1;
	setOrders	(W_SMALL_PLAIN);

	
}


void editorTopLevel::slot_addPlayer(void)
{
	uint np = ecanvas->nb_player();

	if (np>=BOSON_MAX_PLAYERS-1) {
  		KMessageBox::sorry(0, i18n("You can't have more than 10 players in this version"), i18n("too much player") );
		logf(LOG_ERROR, "too much player asked, should not be possible (grayed menu)");
		return;
	}
	who = np; ecanvas->addPlayer();
	if (np>=BOSON_MAX_PLAYERS-1) {
		// XXX todo : gray the menu item
	}
	otype = OT_FACILITY;
	setSelected( species[who]->getBigOverview(FACILITY_CMDBUNKER) );
	emit setSelectedObject (otype, FACILITY_CMDBUNKER);	// need to be after the setSelected
	mw.bigDisplay()->ready4put( QSize (facilityProp[FACILITY_CMDBUNKER].width, facilityProp[FACILITY_CMDBUNKER].height) );
	setSelectionMode( editorTopLevel::SELECT_PUT);
}


void editorTopLevel::slot_editDestroy(void)
{
	int mkey;
	editorCanvas  *_canvas = (editorCanvas*)vcanvas;

	if (fixSelected) {
		/* destroy fix */
		mkey = fixSelected->key;
		unSelectFix();
		_canvas->destroyFixUnit(mkey);
		_canvas->facilities.remove(mkey);
	} else {
		/* destroy mobiles */
		QIntDictIterator<visualMobUnit> selIt(mobSelected);
		for (selIt.toFirst(); selIt;) {			// ++ not needed, selIt should be increased
			mkey = selIt.currentKey(); 		// by the .remove() in unselect
			unSelectMob(mkey);
			_canvas->destroyMobUnit(mkey);
		}
	}
	_canvas->update();
}

void editorTopLevel::slot_close(void)
{
	logf(LOG_INFO, "editorTopLevel::slot_close(void) called");
}

void editorTopLevel::updateViews(void)
{
	mw.big->setContentsPos( viewPos.x() * BO_TILE_SIZE, viewPos.y() * BO_TILE_SIZE );
	mw.big->update();
	mw.mini->repaint(FALSE);
}


bool editorTopLevel::queryExit()
{
	logf(LOG_INFO, "queryExit called");
	return beapp->slot_close();
}


void editorTopLevel::setSelectionMode(selectionMode_t t)
{
	visualTopLevel::setSelectionMode(t);

	switch(t) {
		case SELECT_PUT:
			mw.bigDisplay()->viewport()->setMouseTracking(true);
			break;
		default:
			mw.bigDisplay()->viewport()->setMouseTracking(false);
			break;
	}
}
