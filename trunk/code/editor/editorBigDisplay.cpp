/***************************************************************************
                          editorBigDisplay.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Tue Sep 21 01:18:00 CET 1999
                                           
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

#include <qpopupmenu.h>

#include "editorBigDisplay.h"
#include "visualView.h"
#include "visualCell.h"

editorBigDisplay::editorBigDisplay(visualView *v, QWidget *p, const char *n, WFlags f)
	:visualBigDisplay(v,p,n,f)
{

	QPopupMenu *qpm;
	selectedCell = (visualCell*) 0l;

	// global menu
	popup = new QPopupMenu();

	popup->insertItem("grass",	GROUND_GRASS);
	popup->insertItem("water",	GROUND_WATER);
	popup->insertItem("desert",	GROUND_DESERT);
	popup->insertItem("deep water",	GROUND_DEEP_WATER);

	connect(popup, SIGNAL(activated(int)), this, SLOT(setCell(int)));

	popup->insertSeparator();

	// transitions type
	qpm = new QPopupMenu();

	qpm->insertItem("grass-water");
	qpm->insertItem("desert-grass");
	qpm->insertItem("desert-water");

	connect(qpm, SIGNAL(activated(int)), this, SLOT(setTransType(int)));
	popup->insertItem("Transition type", qpm);

	// transitions tiles
	qpm = new QPopupMenu();

	qpm->insertItem("up");
	qpm->insertItem("down");
	qpm->insertItem("left");
	qpm->insertItem("right");
	qpm->insertItem("upper-left");
	qpm->insertItem("upper-right");
	qpm->insertItem("lower-left");
	qpm->insertItem("lower-right");

	connect(qpm, SIGNAL(activated(int)), this, SLOT(setTransTile(int)));
	popup->insertItem("Transition sprite", qpm);

	// transitions type
	qpm = new QPopupMenu();

	qpm->insertItem("# 1");
	qpm->insertItem("# 2");
	qpm->insertItem("# 3");
	qpm->insertItem("# 4");

	connect(qpm, SIGNAL(activated(int)), this, SLOT(setTransItem(int)));
	popup->insertItem("Transition Item", qpm);

}


void editorBigDisplay::setCell(int g)
{
	selectedCell->set((groundType)g);
	view->field->update();
}

void editorBigDisplay::setTransTile(int)
{
}

void editorBigDisplay::setTransType(int)
{
}

void editorBigDisplay::setTransItem(int)
{
}


