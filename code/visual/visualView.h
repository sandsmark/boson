/***************************************************************************
                          visualView.h  -  description                              
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

#ifndef VISUALVIEW_H 
#define VISUALVIEW_H 

#include <qframe.h>

#include <visualUnit.h>
#include "visualField.h"

enum selectionMode_t {
	SELECT_NONE, 		/* is doing nothing */
/*	SELECT_FACILITY, 
	SELECT_MOBILE, */
	SELECT_RECT,		/* is drawing a selection rect */
	SELECT_PUT,		/* something is being put on the field */
//	SELECT_ACTION,		// something is selected
/*	SELECT_, 
	SELECT_, 
	SELECT_,  */
	};

/** 
  * This class is the global object concerning a view : where, how large..
  * It's used by mainMap and miniMap
  * It's also the place where selections are handled
  * 
  * it inherits from QFrame only because it will _also_ be part of the GUI in boson or boeditor
  */
class visualView : public QFrame
{
	Q_OBJECT

public:
	visualView(visualField *, QWidget *parent=0, const char *name=0L);

	/*
	 * size / position handling
 	 */
	int X(void) { return viewX; }
	int Y(void) { return viewY; }

	int L(void) { return viewL; }
	int H(void) { return viewH; }

	int maxX(void) { return (field)?field->maxX:0; }
	int maxY(void) { return (field)?field->maxY:0; }

	///orzel : should be moved private ?
	visualField	*field;

signals:
	void repaint(bool);
	
protected:
//	void		putSomething(void);

public slots:
	void reCenterView(int x, int y);
	void relativeReCenterView(int x, int y) {reCenterView(x+viewX, y+viewY);}
	void reSizeView(int l, int h);

public:
	void relativeMoveView(int dx, int dy);
	/*
	 * put object 
	 */
	virtual void object_put(int, int)=0;


private:
	void checkMove();

	int	viewL, viewH;	// size of the viewing window
	int	viewX, viewY;	// relative position of the upper-left corner



	/*
         * selection handling
         */

public:
	selectionMode_t	getSelectionMode(void) {return selectionMode;}
	void		setSelectionMode(selectionMode_t t) {selectionMode=t;}
	void		selectFix(visualFacility *);
	void		selectMob(long key, visualMobUnit *);
	visualFacility	*unSelectFix(void);
	visualMobUnit	*unSelectMob(long key);
	void		unSelectAll(void);
	/** add to selection all units in this area */
	void		selectArea(int x1, int y1, int x2, int y2);

	virtual void	setSelected(QPixmap *)=0; //null -> nothing is selected
	virtual void	setOrders(int what , int who=-1)=0;

	/*
public slots:
	void leftClicked(int, int);		// selecting, moving...
	void u_goto(void);
	*/

public: ///orzel : bof...
	visualFacility		*fixSelected;
	QIntDict<visualMobUnit>	mobSelected;
	int			selectionWho; // -1 is nobody
protected:
	selectionMode_t		selectionMode;

};

#endif // VISUALVIEW_H


