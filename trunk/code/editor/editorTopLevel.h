/***************************************************************************
                          editorTopLevel.h  -  description                              
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

#ifndef EDITORTOPLEVEL_H 
#define EDITORTOPLEVEL_H 

#include "mainWidget.h"
#include "visualTopLevel.h"


#define	TILES_NB	(11)	
#define	BIG_TILES_NB	(4)

class	editorTopLevel;
class	visualMiniDisplay;
class	editorBigDisplay;
class	QFrame;
class	QCheckBox;
class	QPushButton;
class	QPixmap;
class	QLabel;
class	QWidgetStack;
class	QScrollView;
class	QVBoxLayout;
class	QComboBox;

class	mainWidget;
class	BoEditorApp;


enum object_type {
	OT_NONE,
	OT_GROUND,
	OT_FACILITY,
	OT_UNIT
};


/** 
 * the editor main Window. There might be several TopLevel on the same map
  */
class editorTopLevel : public visualTopLevel
{
	Q_OBJECT
	
	friend mainWidget;
public:
	editorTopLevel(BoEditorApp *app, const char *name = "boeditor", WFlags f = WDestructiveClose );

	/*
         * selection handling
         */
	virtual void	setSelected(QPixmap *);
	virtual void	setOrders(int what , int who=-1);

signals:
	void setSelectedObject(object_type , int);
	void setWho(uint);

public slots:
	void slot_editDestroy(void);
	void slot_close(void);

protected:
	virtual bool queryExit();
	void	setSelectionMode(selectionMode_t t);

	/*
	 * put object 
	 */
	virtual	void updateViews(void);

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


private:
	void	makeCommandGui(void);
	void	redrawTiles(void);
	void	handleButton(int);

	mainWidget	mw;

	QFrame		*mainFrame;

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
	uint		who;

	/*
	 * GUI
	 */

	/* state view (for selected items) */
	QWidgetStack	*stack;
	QLabel		*view_one;
	QScrollView	*view_many;
	QPixmap		*view_none;
	QCheckBox	*invertBox;

	/* tiles selection */
	QComboBox	*qcb_transRef, *qcb_which, *qcb_who;
	QPushButton	*tiles[TILES_NB];
	QPushButton	*bigTiles[BIG_TILES_NB];



};

#endif // EDITORTOPLEVEL_H


