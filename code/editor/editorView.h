/***************************************************************************
                         editorView.h  -  description                              
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

#ifndef EDITORVIEW_H
#define EDITORVIEW_H


#include <visualView.h>
#include "../common/groundType.h"

class QCheckBox;
class QPushButton;
class QPixmap;
class playerFacility;
class QLabel;
class QWidgetStack;
class QScrollView;
//class visualBigDisplay;
class QVBoxLayout;
class QComboBox;

#define	TILES_NB	(11)	
#define	BIG_TILES_NB	(4)


enum object_type {
	OT_NONE,
	OT_GROUND,
	OT_FACILITY,
	OT_UNIT,
};

class editorView : public visualView
{

	Q_OBJECT

public:
	editorView(visualField *,QWidget *parent=0, const char *name=0);

	virtual void setSelected(QPixmap *);
	virtual void setOrders(int what , int who=-1);

signals:
	void setSelectedObject(object_type , int);
	void setWho(int);

private slots:
	void setTransRef(int);
	void _setWho(int);
	void setInverted(bool);
	void setWhich(int what) { setOrders(what); }
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
	void bc9(void) { handleButton(9); } // button clicked
	void bc10(void) { handleButton(10); } // button clicked
//	void bc11(void) { handleButton(11); } // button clicked

protected:
	virtual void object_put(int, int){}; // not useful for the editor yet

private:
	void	redrawTiles(void);
	void	handleButton(int);

/* state view (for selected items) */
	QWidgetStack	*stack;
	QLabel		*view_one;
	QScrollView	*view_many;
	QPixmap		*view_none;

/* tiles selection */
	QComboBox	*qcb_transRef, *qcb_which, *qcb_who;
	QPushButton	*tiles[TILES_NB];
	QPushButton	*bigTiles[BIG_TILES_NB];
	bool		inverted;
	int		trans;
	enum which_t {
		W_SMALL_PLAIN,
		W_SMALL_GROUND,
		W_BIG_GROUND_1,
		W_BIG_GROUND_2,
		W_FACILITIES,
		W_UNITS
	} which;
	object_type	otype;
	int		who;
	QCheckBox	*invertBox;
};


#endif // EDITORVIEW_H

