/***************************************************************************
                       boson/boson/mainView.h -  description 
                             -------------------                                         

    version              :                                   
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

#ifndef MAIN_VIEW_H
#define MAIN_VIEW_H

#include <qwidget.h>

class	miniMap;
class	fieldMap;
class	viewMap;
class	physMap;
class	orderWin;

class mainView : public QWidget 
{
	Q_OBJECT
public:
	mainView(physMap *phys, QWidget *parent=0, const char *name=0);

private:
/* the map object we are playing in */
    miniMap		*mini;
    fieldMap		*field;
    viewMap		*view;
//physMap		*phys;
/* Window to send order to the selected unities */
    orderWin		*order;
};


#endif     // MAIN_VIEW_H