/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOSONORDERWIDGET_H
#define BOSONORDERWIDGET_H

#include <qwidget.h>
#include <qvaluelist.h>
#include "../global.h"

class Unit;
class Facility;
class Player;
class BosonOrderButton;
class BosonTiles;

/**
 * This is scrollable widget in the commandframe that contains buttons of unit
 * actions (when single unit is selected) or selected units (when multiple
 * units are selected)
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonOrderWidget : public QWidget
{
	Q_OBJECT
public:
	BosonOrderWidget(QWidget* parent);
	~BosonOrderWidget();

	void ensureButtons(unsigned int number);

	/**
	 * Hide all buttons
	 **/
	void hideOrderButtons();
	void setButtonsPerRow(int);

	void setOrderButtons(QValueList<unsigned long int> produceList, Player* owner, Facility* factory = 0);

	void showUnit(Unit* unit); // TODO if this is the only unit -> use slotShowSingleUnit

	void productionAdvanced(Unit* factory, double percentage);

	void initEditor();
	void setCellType(CellType type);
	void setTileSet(BosonTiles* tileSet);

	/**
	 * Resets button by setting it's production count to 0 and making it
	 * not-grayed-out
	 **/
	void resetButton(BosonOrderButton* button);

public slots:
	void slotRedrawTiles();

protected:
	void resetLayout();

signals:
	void signalProduceUnit(unsigned long int unitType);
	void signalStopProduction(unsigned long int unitType);
	void signalPlaceCell(int groundType);
	void signalAction(int actionType);

private:
	class BosonOrderWidgetPrivate;
	BosonOrderWidgetPrivate* d;
};

#endif

