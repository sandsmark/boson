/***************************************************************************
                          orderWin.cpp  -  description                              
                             -------------------                                         

    version              :                                   
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
#include "playerUnit.h"
#include "orderWin.h"
#include "fieldMap.h"
#include "game.h"

#define VIEW_ONE	1
#define VIEW_MANY	2

orderWin::orderWin(fieldMap *f, QWidget *parent, const char *name)
	:QFrame(parent, name)
	,selectionMode(SELECT_NONE)
	,fixSelected( 0L )
	,field(f)
{
QString path(kapp->kde_datadir() + "/boson/pics/misc/overview_none.xpm" );

setFrameStyle(QFrame::Sunken | QFrame::Panel);
setLineWidth(5);


/* stack */
stack = new QWidgetStack(this, "qwidgetstack");
//stack->setFixedSize(200,200);
stack->setFrameStyle(QFrame::Raised | QFrame::Panel);
stack->setLineWidth(5);
stack->setGeometry(10,10,180,110);

/* stack/one */
view_none = new QPixmap(path);
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
	//orderButton[i]->setFixedSize(100,40);
	orderButton[i]->setGeometry( 10+(i%2)*91, 42*(i/2) + 130, 89, 40);
	orderButton[i]->hide();
	}
}


playerFacility * orderWin::unSelectFix(void)
{
playerFacility *f = fixSelected;

if (!f) return f; // already done
fixSelected	= (playerFacility *) 0l;

view_one->setPixmap(*view_none);
/**/

return f;
}

playerMobUnit *orderWin::unSelectMob(long key)
{
playerMobUnit *m = mobSelected[key];
mobSelected.remove(key);

if (mobSelected.isEmpty()) {
	view_one->setPixmap(*view_none);
	orderButton[0]->hide();
printf("disconnect button 0\n");
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
fixSelected = f;
view_one->setPixmap(*gameProperties.myspecies->getBigOverview(f)); ///orzel 0 is not 0, it's my_number_in_the_game
logf(LOG_GAME_LOW, "select facility");

}

void orderWin::selectMob(long key, playerMobUnit *m)
{
if (mobSelected.isEmpty()) {
	boAssert( selectionWho = -1);
/*	connect(orderButton[1], SIGNAL(clicked()), this, SLOT(u_stop()));
	orderButton[1]->show(); */
	selectionWho = m->who;
	if (selectionWho == gameProperties.who_am_i) {
printf("connect button 0\n");
		connect(orderButton[0], SIGNAL(clicked()), this, SLOT(u_goto()));
		orderButton[0]->show();
		}
	}
else {
	boAssert( selectionWho>=0 );
	if (m->who != selectionWho)
		return;
	}

//setSelectionMode(SELECT_MOVE);

mobSelected.insert(key, m);
view_one->setPixmap(*gameProperties.myspecies->getBigOverview(m)); ///orzel 0 is not 0, it's my_number_in_the_game
logf(LOG_GAME_LOW, "select mobile");
}


void orderWin::u_goto(void)
{
printf("selection is : %d\n", getSelectionMode());
boAssert( SELECT_NONE == getSelectionMode() );
boAssert(selectionWho == gameProperties.who_am_i);
///orzel : should change the cursor over fieldMap
setSelectionMode(SELECT_MOVE);
}

void orderWin::leftClicked(int mx, int my)		// selecting, moving...
{
QIntDictIterator<playerMobUnit> mobIt(mobSelected);

//printf(" to %d.%d\n", mx, my);

if (SELECT_MOVE != getSelectionMode()) {
	logf(LOG_ERROR,"orderWin::leftClicked while not in SELECT_MOVE state");
	return;
	}
if (mobSelected.isEmpty()) {
	logf(LOG_ERROR,"orderWin::leftClicked : unexpected empty mobSelected");
	setSelectionMode(SELECT_NONE);
	return;
	}

for (mobIt.toFirst(); mobIt; ++mobIt) {
	boAssert(mobIt.current()->who == gameProperties.who_am_i);
	mobIt.current()->u_goto(mx,my);
	}

printf("disconnect button 0\n");
orderButton[0]->disconnect(this);
setSelectionMode(SELECT_NONE);
} 

