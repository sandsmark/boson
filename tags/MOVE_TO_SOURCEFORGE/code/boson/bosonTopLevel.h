/***************************************************************************
                          bosonTopLevel.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#ifndef BOSONTOPLEVEL_H 
#define BOSONTOPLEVEL_H 

#include "mainWidget.h"
#include "visualTopLevel.h"


class BosonApp;

/** 
 * the bosonTopLevel is the window containing everything the user can actually see
 * ie : miniMap/map/orders-info window, and so on. It has decoration on it as all
 * KDE windows.
  */
class bosonTopLevel : public visualTopLevel
{
	Q_OBJECT
	
public:
	bosonTopLevel(BosonApp *parent, const char *name = 0L, WFlags f = WDestructiveClose );

	enum orderType_t { OT_NONE =-1 , OT_FACILITY=10, OT_MOBILE=11};

	/*
         * selection handling
         */
	virtual void	setSelected(QPixmap *);
	virtual void	setOrders(int what , int who=-1);

	// XXX dirty HACK, clean this object_put
	void	_object_put(QPoint p);

protected:
	virtual	void	updateViews(void);

private slots:
	void bc0(void) { handleOrder(0); } // button clicked
	void bc1(void) { handleOrder(1); } // button clicked
	void bc2(void) { handleOrder(2); } // button clicked
	void bc3(void) { handleOrder(3); } // button clicked
	void bc4(void) { handleOrder(4); } // button clicked
	void bc5(void) { handleOrder(5); } // button clicked
	void bc6(void) { handleOrder(6); } // button clicked
	void bc7(void) { handleOrder(7); } // button clicked
	void bc8(void) { handleOrder(8); } // button clicked
	void bc9(void) { handleOrder(9); } // button clicked
	void bc10(void) { handleOrder(10); } // button clicked

private:
	void	handleOrder(int);

	mainWidget	mw;
	QPushButton	*orderButton[11];

	orderType_t	orderType;
	mobConstrMsg_t	mobConstruct;
	fixConstrMsg_t	fixConstruct;
};

#endif // BOSONTOPLEVEL_H


