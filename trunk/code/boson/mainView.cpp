/***************************************************************************
                       boson/boson/mainView.cpp -  description 
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

#include <qlayout.h>
#include <qkeycode.h>

#include "bosonField.h"
#include "visualMiniDisplay.h"
#include "bosonBigDisplay.h"
#include "bosonView.h"

#include "mainView.h"		// myself

mainView::mainView(QWidget *parent, const char *name)
	:QWidget(parent, name)
{ 
	QHBoxLayout	*topLayout = new QHBoxLayout(this);
	QVBoxLayout	*leftLayout = new QVBoxLayout();

	setFocusPolicy (StrongFocus);		// accept key event
	setFocus();

	topLayout->addLayout(leftLayout,0);

		view = new bosonView(this, "bosonView");
		
		mini = new visualMiniDisplay(view, this);
		mini->setFixedSize(200,200);
		
		leftLayout->addWidget(mini);
		leftLayout->addWidget(view, 10);

		
	connect(parent, SIGNAL(ressourcesUpdated(void)), view, SLOT(ressourcesUpdated(void)));
/* This is the main map, the game area */
	big = new bosonBigDisplay(view, this);
	topLayout->addWidget(big,10);

/* finish the stuff */
//	leftLayout->addStretch(10);
	topLayout->activate();
//	setMinimumSize(800, 624);
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



