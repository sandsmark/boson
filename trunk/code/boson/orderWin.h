/***************************************************************************
                         orderWin.h  -  description                              
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

#ifndef ORDER_WIN_H
#define ORDER_WIN_H


#include <qframe.h>
//#include <qintdict.h>
//#include <playerUnit.h>

class QPushButton;
class QPixmap;
class playerFacility;
class QLabel;
class QWidgetStack;
class QScrollView;
//class fieldMap;
class QVBoxLayout;

#define ORDER_BUTTONS_NB  (8)


class orderWin : public QFrame
{
  Q_OBJECT

public:
	orderWin(QWidget *parent=0, const char *name=0);

private:
/* state view (for selected items) */
	QWidgetStack	*stack;
	QLabel		*view_one;
	QScrollView	*view_many;
	QPixmap		*view_none;

private:
//	fieldMap	*field;

/* GUI */
	QPushButton	*orderButton[ORDER_BUTTONS_NB];
};


#endif // ORDER_WIN_H

