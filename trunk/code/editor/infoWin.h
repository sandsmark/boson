/***************************************************************************
                         infoWin.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#ifndef INFOWIN_H
#define INFOWIN_H


#include <qframe.h>
//#include <qintdict.h>
//#include <playerUnit.h>

#include "../common/groundType.h"

class QPushButton;
class QPixmap;
class playerFacility;
class QLabel;
class QWidgetStack;
class QScrollView;
//class visualBigDisplay;
class QVBoxLayout;
class QComboBox;

#define ORDER_BUTTONS_NB  (8)


class infoWin : public QFrame
{
  Q_OBJECT

public:
	infoWin(QWidget *parent=0, const char *name=0);

public slots:
	void setSelected(QPixmap*);

signals:
	void setSelectedTile(groundType);

private slots:
	void setTransRef(int);
	void setInverted(bool);
	void setWhich(int);
/* orzel : very ugly, but what the hell should I have used here ? */
	void bc0(void) { handleButton(0); } // button clicked
	void bc1(void) { handleButton(1); } // button clicked
	void bc2(void) { handleButton(2); } // button clicked
	void bc3(void) { handleButton(3); } // button clicked
	void bc4(void) { handleButton(4); } // button clicked
	void bc5(void) { handleButton(5); } // button clicked
	void bc6(void) { handleButton(6); } // button clicked
	void bc7(void) { handleButton(7); } // button clicked
	void bc8(void) { handleButton(8); } // button clicked

private:
	void	redrawTiles(void);
	void	handleButton(int);

/* state view (for selected items) */
	QWidgetStack	*stack;
	QLabel		*view_one;
	QScrollView	*view_many;
	QPixmap		*view_none;

/* tiles selection */
	QComboBox	*qcb_transRef, *qcb_which;
	QPushButton	*tiles[9];
	QPushButton	*bigTiles[4];
	bool		inverted;
	int		trans;
	int		which;
};


#endif // INFOWIN_H

