/***************************************************************************
                       editor/mainView.cpp -  description 
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

#include "visual/visual.h"

#include "editorField.h"
#include "visualMiniDisplay.h"
#include "editorBigDisplay.h"
#include "editorView.h"

#include "mainView.h"		// myself

mainView::mainView(editorField *field, QWidget *parent, const char *name)
	:QWidget(parent, name)
{ 
	QHBoxLayout	*topLayout = new QHBoxLayout(this);
	QVBoxLayout	*leftLayout = new QVBoxLayout();

	setFocusPolicy (StrongFocus);		// accept key event
	setFocus();

	topLayout->addLayout(leftLayout,0);

		view = new editorView(field, this, "editorView"); // the view associated with this window

		mini = new visualMiniDisplay(view, this);
		mini->setFixedSize(250,200);

		leftLayout->addWidget(mini);
		leftLayout->addWidget(view, 10);

/* This is the main map, the game area */
	big = new editorBigDisplay(view, this);
	topLayout->addWidget(big,10);

	connect (view, SIGNAL(setSelectedObject(object_type, int)), big, SLOT(setSelectedObject(object_type, int)));
	connect (view, SIGNAL(setWho(int)), big, SLOT(setWho(int)));

/* finish the stuff */
//	leftLayout->addStretch(10);
	topLayout->activate();
}


#define ARROW_KEY_STEP	1

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


void mainView::slotEditDestroy(void)
{
	int mkey;
	editorField  *field = (editorField*)vfield;

	if (view->fixSelected) {
		/* destroy fix */
		mkey = view->fixSelected->key;
		view->unSelectFix();
		field->facilities.remove(mkey);
	} else {
		/* destroy mobiles */
		QIntDictIterator<visualMobUnit> selIt(view->mobSelected);
		for (selIt.toFirst(); selIt;) {			// ++ not needed, selIt should be increased
			mkey = selIt.currentKey(); 		// by the .remove() in unselect
			view->unSelectMob(mkey);
			field->mobiles.remove(mkey);
		}
	}
	field->update();
}


