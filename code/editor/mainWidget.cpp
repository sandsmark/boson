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

#include "common/log.h"

#include "editorTopLevel.h"
#include "visualMiniDisplay.h"
#include "editorBigDisplay.h"
#include "mainWidget.h"		// myself


mainWidget::mainWidget( editorTopLevel *parent, const char *name)
	:QWidget(parent, name)
{ 
	etl = parent;

	mini = new visualMiniDisplay( parent, this);
	big = new editorBigDisplay( parent, this);

	mini->setGeometry (   0,   0, 200,200);

	// big->setGeometry  (200,  0, width() - 200, height() );
	// etl->mainFrame->setGeometry (  0,200, 200, height() - 200);

	connect (etl, SIGNAL(setSelectedObject(object_type, int)), big, SLOT(setSelectedObject(object_type, int)));
	connect (etl, SIGNAL(setWho(int)), big, SLOT(setWho(int)));

	/* focus handling */
	setFocusPolicy (StrongFocus);		// accept key event
	setFocus();


}

void mainWidget::resizeEvent ( QResizeEvent * )
{
	big->setGeometry  (200,  0, width() - 200, height() );
	etl->mainFrame->setGeometry (  0,200, 200, height() - 200);
}

#define ARROW_KEY_STEP	2

void mainWidget::keyReleaseEvent ( QKeyEvent * e )
{
	switch (e->key()) {
		case Key_Left:
			etl->relativeMoveView(-ARROW_KEY_STEP,0);
			break;
		case Key_Right:
			etl->relativeMoveView(ARROW_KEY_STEP,0);
			break;
		case Key_Up:
			etl->relativeMoveView(0, -ARROW_KEY_STEP);
			break;
		case Key_Down:
			etl->relativeMoveView(0, ARROW_KEY_STEP);
			break;
	}
}

