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

#include "visualMiniDisplay.h"
#include "bosonBigDisplay.h"
#include "bosonView.h"

#include "mainView.h"		// myself

mainView::mainView(QWidget *parent, const char *name)
	:QWidget(parent, name)
{ 

	/* this code doesn't work, either me or Qt layout sucks,
	 * i've lost too many time trying to use those layout in qt1 and qt2
	 *
	// layout management
	QHBoxLayout	*topLayout = new QHBoxLayout(this);
	QVBoxLayout	*leftLayout = new QVBoxLayout(topLayout);


	// leftLayout
		view = new bosonView(this, "bosonView");
		
		mini = new visualMiniDisplay(view, this);
		mini->setFixedSize(200,200);
		
		leftLayout->addWidget(mini);
		leftLayout->addWidget(view, 10);

		
	// This is the main map, the game area 
	big = new bosonBigDisplay(view, this);
	big->resize(300,300);
	topLayout->addWidget(big, 10);


	printf("size : %d,%d\n", big->width(), big->height() );

	leftLayout->activate();
	topLayout->activate();

	printf("size : %d,%d\n", big->width(), big->height() );
	big->resize(300,300);
	printf("size : %d,%d\n", big->width(), big->height() );

	*/

	view = new bosonView(this, "bosonView");
	mini = new visualMiniDisplay(view, this);
	big = new bosonBigDisplay(view, this);

	mini->setGeometry (   0,   0, 200,200);
	view->setGeometry (   0, 200, 200, height() - 200);
	big->setGeometry  ( 200,   0, width() - 200, height() );


	connect(parent, SIGNAL(ressourcesUpdated(void)), view, SLOT(ressourcesUpdated(void)));

	/* focus handling */
	setFocusPolicy (StrongFocus);		// accept key event
	setFocus();

}

void mainView::resizeEvent ( QResizeEvent * )
{
	big->setGeometry(200,0, width() - 200, height() );
	view->setGeometry(0,200,200, height() - 200);
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



