/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __BOSONCOMMANDWIDGET_H__
#define __BOSONCOMMANDWIDGET_H__

#include <qwidget.h>

class Unit;
class Player;
class BosonTiles;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCommandWidget : public QWidget
{
	Q_OBJECT
public:
	BosonCommandWidget(QWidget* parent);
	~BosonCommandWidget();

	enum CommandType {
		CommandNothing = 0,
		CommandCell = 1,
		CommandUnit = 2,
		CommandUnitSelected = 3
	};

	virtual void setUnit(Unit* unit);
	void setUnit(int unitType, Player* owner);
	void setCell(int tileNo, BosonTiles* tileSet);

	void setToolTip(const QString& text);

signals:
	/**
	 * Emitted when the player clicks on this widget and it is a cell. See
	 * @ref setCell
	 **/
	void signalPlaceCell(int tileNumber);

	/**
	 * Emitted when the player clicks on this widget and it is an order
	 * button. See @ref setUnit.
	 *
	 * The player will expect the unitType to be produced by the currently
	 * selected factory (aka facility).
	 * @param unitType The unit type that is to be produced
	 **/
	void signalProduceUnit(int unitType);

protected:
	virtual void mousePressEvent(QMouseEvent* e);

	virtual void displayUnitPixmap(Unit* unit);
	virtual void displayUnitPixmap(int unitType, Player* owner);

	void setPixmap(const QPixmap& pixmap);

protected slots:
	void slotClicked();
	void slotUnitChanged(Unit*);
	
private:
	class BosonCommandWidgetPrivate;
	BosonCommandWidgetPrivate* d;
};

#endif
