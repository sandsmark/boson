/***************************************************************************
                          viewMap.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#ifndef VIEW_MAP_H 
#define VIEW_MAP_H 

#include <qintdict.h>

#include <visualUnit.h>
#include "physMap.h"

class physMap;


enum selectionMode_t {
	SELECT_NONE, 		/* is doing nothing */
/*	SELECT_FACILITY, 
	SELECT_MOBILE, */
	SELECT_RECT,		/* is drawing a selection rect */
	SELECT_MOVE,		// is waiting for 'where to move' 
/*	SELECT_, 
	SELECT_, 
	SELECT_,  */
	};

/** 
  * This class is the global object concerning a view : where, how large..
  * It's used by mainMap and miniMap
  * It's also the place where selections are handled
  */
class viewMap : public QObject
{
	Q_OBJECT

public:
	viewMap(physMap *, QObject *parent=0, const char *name=0L);

	int X(void) { return viewX; }
	int Y(void) { return viewY; }

	int L(void) { return viewL; }
	int H(void) { return viewH; }

	int maxX(void) { return (phys)?phys->maxX:0; }
	int maxY(void) { return (phys)?phys->maxY:0; }

	///orzel : should be moved private ?
	physMap	*phys;

signals:
	void repaint(bool);

public slots:
	void reCenterView(int x, int y);
	void relativeReCenterView(int x, int y) {reCenterView(x+viewX, y+viewY);}
	void reSizeView(int l, int h);

private:
	int	viewL, viewH;	// size of the viewing window
	int	viewX, viewY;	// relative position of the upper-left corner

	/* selection handling */

public:
	selectionMode_t	getSelectionMode(void) {return selectionMode;}
	void		setSelectionMode(selectionMode_t t) {selectionMode=t;}
	void		selectFix(visualFacility *);
	void		selectMob(long key, visualMobUnit *);
	visualFacility	*unSelectFix(void);
	visualMobUnit	*unSelectMob(long key);
	void		unSelectAll(void);
signals:
	void		setSelected(QPixmap *); //null -> nothing is selected
	void		setOrders(unsigned long);  //orzel : should be something like 'actionSet'

public slots:
	/* from display classes */
	void leftClicked(int, int);		// selecting, moving...
	/* to handle orderButton 'clicked' event */
	void u_goto(void);

public: ///orzel : bof...
	visualFacility		*fixSelected;
	QIntDict<visualMobUnit>	mobSelected;
private:
	selectionMode_t		selectionMode;
	int			selectionWho; // -1 is nobody

};

#endif // VIEW_MAP_H


