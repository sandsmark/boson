/***************************************************************************
                         orderWin.h  -  description                              
                             -------------------                                         

    version              :                                   
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

#ifndef ORDER_WIN_H
#define ORDER_WIN_H


#include <qframe.h>
#include <qintdict.h>
#include <playerUnit.h>

class QPushButton;
class QPixmap;
class playerFacility;
class QLabel;
class QWidgetStack;
class QScrollView;
class fieldMap;
class QVBoxLayout;

#define ORDER_BUTTONS_NB  (8)

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


class orderWin : public QFrame
{
  Q_OBJECT

public:
	orderWin(fieldMap *, QWidget *parent=0, const char *name=0);

	selectionMode_t	getSelectionMode(void) {return selectionMode;}
	void setSelectionMode(selectionMode_t t) {selectionMode=t;}
	void selectFix(playerFacility *);
	void selectMob(long key, playerMobUnit *);
	playerFacility	*unSelectFix(void);
	playerMobUnit	*unSelectMob(long key);
	void		unSelectAll(void);

/* to handle orderButton 'clicked' event */
public slots:
  void u_goto(void);

/* from display classes */
  void leftClicked(int, int);		// selecting, moving...

private:
/* state view (for selected items) */
	QWidgetStack	*stack;
	QLabel		*view_one;
	QScrollView	*view_many;
	QPixmap		*view_none;

/* selection handling */
	selectionMode_t		selectionMode;
	int			selectionWho; // -1 is nobody

public: ///orzel : bof...
	playerFacility		*fixSelected;
	QIntDict<playerMobUnit>	mobSelected;
private:
	fieldMap	*field;

/* GUI */
	QPushButton	*orderButton[ORDER_BUTTONS_NB];
};


#endif // ORDER_WIN_H

