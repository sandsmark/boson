/***************************************************************************
                          orderWin.cpp  -  description                              
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

#include "orderWin.h"
//#include "fieldMap.h"
#include "speciesTheme.h"
#include "game.h"

#define VIEW_ONE	1
#define VIEW_MANY	2

orderWin::orderWin(QWidget *parent, const char *name)
	:QFrame(parent, name)
	,selectionMode(SELECT_NONE)
	,fixSelected( 0L )
{
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
	printf("orderWin::orderWin : Can't load overview_none \n");

view_one = new QLabel(stack,"preview");
view_one->setPixmap(*view_none);
stack->addWidget(view_one, VIEW_ONE);

/* stack/many */
view_many = new QScrollView(stack,"scrollview");
stack->addWidget(view_many, VIEW_MANY);

stack->raiseWidget(VIEW_ONE);

/* buttons */
static char * button_text[ORDER_BUTTONS_NB] = {
	"Move", "Stop",
	"Move", "Stop",
	"Move", "Stop",
	"Move", "Stop",
	};
for (int i=0; i<ORDER_BUTTONS_NB; i++) {
	orderButton[i] = new QPushButton(this);
	orderButton[i]->setText(button_text[i]);
	orderButton[i]->setGeometry( 10+(i%2)*91, 42*(i/2) + 130, 89, 40);
	orderButton[i]->hide();
	}
}


playerFacility * orderWin::unSelectFix(void)
{
playerFacility *f = fixSelected;

if (!f) return f; // already done
fixSelected	= (playerFacility *) 0l;
f->unSelect();

view_one->setPixmap(*view_none);
/**/

return f;
}

playerMobUnit *orderWin::unSelectMob(long key)
{
playerMobUnit *m = mobSelected[key];
mobSelected.remove(key);
m->unSelect();

if (mobSelected.isEmpty()) {
	view_one->setPixmap(*view_none);
	orderButton[0]->hide();
	orderButton[0]->disconnect(this);
	}
/**/

return m;
}

void orderWin::unSelectAll(void)
{
/* */
selectionWho =  -1; ///orzel : should be a WHO_NOBOCY;
for (int i=0; i<ORDER_BUTTONS_NB; i++) {
	orderButton[i]->hide();
	orderButton[i]->disconnect(this);
	}
}


void orderWin::selectFix(playerFacility *f)
{
fixSelected = f; fixSelected->select();
view_one->setPixmap(*vpp.species[f->who]->getBigOverview(f));
logf(LOG_GAME_LOW, "select facility");

}

void orderWin::selectMob(long key, playerMobUnit *m)
{
if (mobSelected.isEmpty()) {
	boAssert( selectionWho = -1);
/*	connect(orderButton[1], SIGNAL(clicked()), this, SLOT(u_stop()));
	orderButton[1]->show(); */
	selectionWho = m->who;
	if (selectionWho == gpp.who_am_i) {
		connect(orderButton[0], SIGNAL(clicked()), this, SLOT(u_goto()));
		orderButton[0]->show();
		}
	}
else {
	boAssert( selectionWho>=0 );
	if (m->who != selectionWho)
		return;
	}

mobSelected.insert(key, m); m->select();
view_one->setPixmap(*vpp.species[m->who]->getBigOverview(m));
logf(LOG_GAME_LOW, "select mobile");
}


void orderWin::u_goto(void)
{
boAssert( SELECT_NONE == getSelectionMode() );
boAssert(selectionWho == gpp.who_am_i);
///orzel : should change the cursor over fieldMap
setSelectionMode(SELECT_MOVE);
}


void orderWin::leftClicked(int mx, int my)		// selecting, moving...
{
QIntDictIterator<playerMobUnit> mobIt(mobSelected);

if (SELECT_MOVE != getSelectionMode()) {
	logf(LOG_ERROR,"orderWin::leftClicked while not in SELECT_MOVE state");
	orderButton[0]->disconnect(this);
	return;
	}
if (mobSelected.isEmpty()) {
	logf(LOG_ERROR,"orderWin::leftClicked : unexpected empty mobSelected");
	setSelectionMode(SELECT_NONE);
	orderButton[0]->disconnect(this);
	return;
	}

for (mobIt.toFirst(); mobIt; ++mobIt) {
	boAssert(mobIt.current()->who == gpp.who_am_i);
	mobIt.current()->u_goto(mx,my);
	}

setSelectionMode(SELECT_NONE);
} 

