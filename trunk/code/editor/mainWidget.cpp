/***************************************************************************
                       mainWidget.cpp -  description 
                             -------------------                                         

    version              : $Id$
    begin                : Mon Apr 19 23:56:00 CET 1999
                                           
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

#include <qkeycode.h>
#include <qwidgetstack.h>
#include <qframe.h>
#include <qscrollview.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qvbox.h>

#include "common/log.h"

#include "editorTopLevel.h"
#include "visualMiniDisplay.h"
#include "editorBigDisplay.h"
#include "mainWidget.h"		// myself


mainWidget::mainWidget( editorTopLevel *parent, const char *name)
	:QHBox(parent, name)
{ 
	etl = parent;

	QVBox *vb = new QVBox(this);
	big = new editorBigDisplay( parent, this);
	mini = new visualMiniDisplay( parent, vb);
	mainFrame = new QFrame(vb);

	connect (etl, SIGNAL(setSelectedObject(object_type, int)), big, SLOT(setSelectedObject(object_type, int)));
	connect (etl, SIGNAL(setWho(uint)), big, SLOT(setWho(uint)));

	/* focus handling */
	setFocusPolicy (StrongFocus);		// accept key event
	setFocus();


}

#define ARROW_KEY_STEP	2

void mainWidget::keyReleaseEvent ( QKeyEvent * e )
{
	switch (e->key()) {
		case Key_Left:
			etl->relativeMoveView( QPoint(-ARROW_KEY_STEP,0) );
			break;
		case Key_Right:
			etl->relativeMoveView( QPoint(ARROW_KEY_STEP,0) );
			break;
		case Key_Up:
			etl->relativeMoveView( QPoint(0, -ARROW_KEY_STEP) );
			break;
		case Key_Down:
			etl->relativeMoveView( QPoint(0, ARROW_KEY_STEP) );
			break;
	}
}

