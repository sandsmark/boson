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

#include "infoWin.h"
#include "viewMap.h"
//#include "fieldMap.h"
//#include "speciesTheme.h"
#include "game.h"

#define VIEW_ONE	1
#define VIEW_MANY	2

infoWin::infoWin(QWidget *parent, const char *name)
	:QFrame(parent, name)
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
	printf("infoWin::infoWin : Can't load overview_none \n");

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

