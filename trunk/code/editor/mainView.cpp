/***************************************************************************
                       boson/editor/mainView.cpp -  description 
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

#include "infoWin.h"
#include "editorMap.h"
#include "miniMap.h"
#include "fieldMap.h"
#include "viewMap.h"

#include "mainView.h"		// myself

mainView::mainView(editorMap *phys, QWidget *parent=0, const char *name=0)
	:QWidget(parent, name)
{ 
	QHBoxLayout	*topLayout = new QHBoxLayout(this);
	QVBoxLayout	*leftLayout = new QVBoxLayout();

	topLayout->addLayout(leftLayout,0);

		view = new viewMap(phys); // the view associated with this window
		mini = new miniMap(view, this);
		mini->setFixedSize(200,200);
		leftLayout->addWidget(mini);

		info = new infoWin(this, "infowin");
		leftLayout->addWidget(info, 10);

/* This is the main map, the game area */
	field = new fieldMap(view, this);
	topLayout->addWidget(field,10);

/* finish the stuff */
//	leftLayout->addStretch(10);
	topLayout->activate();
}

