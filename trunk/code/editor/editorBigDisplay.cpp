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

#include "../common/log.h"

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

	qpm->insertItem("grass - water", TRANS_GW);
	qpm->insertItem("grass - desert", TRANS_GD);
	qpm->insertItem("water - desert", TRANS_WD);
	qpm->insertItem("deep water - water", TRANS_DWD);

	connect(qpm, SIGNAL(activated(int)), this, SLOT(setTransRef(int)));
	popup->insertItem("Transition type", qpm);

	// transitions tiles
	qpm = new QPopupMenu();

	qpm->insertItem("up", TRANS_UP);
	qpm->insertItem("down", TRANS_DOWN);
	qpm->insertItem("left", TRANS_LEFT);
	qpm->insertItem("right", TRANS_RIGHT);

	qpm->insertItem("upper-left", TRANS_UL);
	qpm->insertItem("upper-right", TRANS_UR);
	qpm->insertItem("lower-left", TRANS_DL);
	qpm->insertItem("lower-right", TRANS_DR);

	qpm->insertItem("upper-left-inverted", TRANS_ULI);
	qpm->insertItem("upper-right-inverted", TRANS_URI);
	qpm->insertItem("lower-left-inverted", TRANS_DLI);
	qpm->insertItem("lower-right-inverted", TRANS_DRI);

	connect(qpm, SIGNAL(activated(int)), this, SLOT(setTransTile(int)));
	popup->insertItem("Transition sprite", qpm);

	// transitions type
	qpm = new QPopupMenu();

	qpm->insertItem("# 1", 0);
	qpm->insertItem("# 2", 1);
	qpm->insertItem("# 3", 2);
	qpm->insertItem("# 4", 3);

	connect(qpm, SIGNAL(activated(int)), this, SLOT(setItem(int)));
	popup->insertItem("Transition Item", qpm);

}


void editorBigDisplay::setCell(int g)
{
	selectedCell->set((groundType)g);
	view->field->update();
}

void editorBigDisplay::setTransTile(int t)
{
	int ref, tile;
	groundType g = selectedCell->getGroundType();

	boAssert(t>=0 && t< TILES_PER_TRANSITION);

	if (! IS_TRANS(g)) {
		g = GET_TRANS_NUMBER( TRANS_GW, t);
		selectedCell->set(g);
		view->field->update();
	}

	ref	= GET_TRANS_REF(g);
	tile	= t;

	g = GET_TRANS_NUMBER(ref,tile);
	selectedCell->set(g);
	view->field->update();
}

void editorBigDisplay::setTransRef(int r)
{
	int ref, tile;
	groundType g = selectedCell->getGroundType();

	boAssert(r>=0 && r<TRANS_LAST);

	if (! IS_TRANS(g)) {
		g = GET_TRANS_NUMBER(r, 0);
		selectedCell->set(g);
		view->field->update();
	}

	ref	= r;
	tile	= GET_TRANS_TILE(g);

	g = GET_TRANS_NUMBER(ref,tile);
	selectedCell->set(g);
	view->field->update();
}

void editorBigDisplay::setItem(int i)
{
	boAssert(i>=0 && i<4);
	selectedCell->frame( i);
	view->field->update();
}


