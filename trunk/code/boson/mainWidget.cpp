/***************************************************************************
                       mainWidget.cpp -  description 
                             -------------------                                         

    version              : $Id$
    begin                : Mon Apr 19 23:56:00 CET 1999
                                           
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

#include <qkeycode.h>
#include <qwidgetstack.h>
#include <qframe.h>
#include <qscrollview.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qvbox.h>

#include "common/log.h"

#include "bosonTopLevel.h"
#include "visualMiniDisplay.h"
#include "bosonBigDisplay.h"
#include "game.h"

#include "mainWidget.h"		// myself


#define VIEW_ONE	1
#define VIEW_MANY	2


mainWidget::mainWidget( bosonTopLevel *parent, const char *name)
	:QHBox(parent, name)
{ 
	btl = parent;

	QVBox *vb = new QVBox(this);
	big = new bosonBigDisplay( parent, this);
	mini = new visualMiniDisplay( parent, vb);
	mainFrame = new QFrame(vb);

	mini->setGeometry (   0,   0, 200,200);
	makeCommandGui();

	/* focus handling */
	setFocusPolicy (StrongFocus);		// accept key event
	setFocus();

}

#define ARROW_KEY_STEP	2

void mainWidget::keyReleaseEvent ( QKeyEvent * e )
{
	switch (e->key()) {
		case Key_Left:
			btl->relativeMoveView(-ARROW_KEY_STEP,0);
			break;
		case Key_Right:
			btl->relativeMoveView(ARROW_KEY_STEP,0);
			break;
		case Key_Up:
			btl->relativeMoveView(0, -ARROW_KEY_STEP);
			break;
		case Key_Down:
			btl->relativeMoveView(0, ARROW_KEY_STEP);
			break;
	}
}


void mainWidget::makeCommandGui(void)
{

	mainFrame->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::MinimumExpanding ) );
	mainFrame->setMinimumSize (220, 200);

	mainFrame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
	mainFrame->setLineWidth(5);
	
	/* stack */
	stack = new QWidgetStack(mainFrame, "qwidgetstack");
	stack->setFrameStyle(QFrame::Raised | QFrame::Panel);
	stack->setLineWidth(5);
	stack->setGeometry(10,23,180,110);

	/* stack/one */
	view_none = new QPixmap();

	view_one = new QLabel(stack,"preview");
	view_one->setPixmap(*view_none);
	stack->addWidget(view_one, VIEW_ONE);

	/* stack/many */
	view_many = new QScrollView(stack,"scrollview");
	stack->addWidget(view_many, VIEW_MANY);

	stack->raiseWidget(VIEW_ONE);
}

