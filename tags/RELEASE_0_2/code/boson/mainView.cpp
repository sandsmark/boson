/***************************************************************************
                       boson/boson/mainView.cpp -  description 
                             -------------------                                         

    version              : $Id$
    begin                : Mon Apr 19 23:56:00 CET 1999
                                           
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
#include <qkeycode.h>

#include "infoWin.h"
#include "bosonField.h"
#include "visualMiniDisplay.h"
#include "visualBigDisplay.h"
#include "bosonView.h"

#include "mainView.h"		// myself

mainView::mainView(bosonField *field, QWidget *parent=0, const char *name=0)
	:QWidget(parent, name)
{ 
	QHBoxLayout	*topLayout = new QHBoxLayout(this);
	QVBoxLayout	*leftLayout = new QVBoxLayout();

	setFocusPolicy (StrongFocus);		// accept key event
	setFocus();

	topLayout->addLayout(leftLayout,0);

		view = new bosonView(field); // the view associated with this window
		mini = new visualMiniDisplay(view, this);
		mini->setFixedSize(200,200);
		leftLayout->addWidget(mini);

		info = new infoWin(this, "infowin");
		connect (view, SIGNAL(setSelected(QPixmap*)), info, SLOT(setSelected(QPixmap*)));
		leftLayout->addWidget(info, 10);

/* This is the main map, the game area */
	big = new visualBigDisplay(view, this);
	topLayout->addWidget(big,10);

/* finish the stuff */
//	leftLayout->addStretch(10);
	topLayout->activate();
}

#define ARROW_KEY_STEP	2

void mainView::keyReleaseEvent ( QKeyEvent * e )
{
	switch (e->key()) {
		case Key_Left:
			view->relativeMoveView(-ARROW_KEY_STEP,0);
			break;
		case Key_Right:
			view->relativeMoveView(ARROW_KEY_STEP,0);
			break;
		case Key_Up:
			view->relativeMoveView(0, -ARROW_KEY_STEP);
			break;
		case Key_Down:
			view->relativeMoveView(0, ARROW_KEY_STEP);
			break;
	}
}



