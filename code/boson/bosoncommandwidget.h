/*
    This file is part of the Boson game
    Copyright (C) 2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONCOMMANDWIDGET_H
#define BOSONCOMMANDWIDGET_H

#include <qwidget.h>
#include <qpushbutton.h>

class Unit;
class Player;
class BosonTiles;
class BoButton;

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

	/**
	 * @return The displayed unit or 0 if no unit is displayed. See also
	 * @ref tile and @ref unitType
	 **/
	Unit* unit() const 
	{
		return (commandType() == CommandUnitSelected) ? mUnit : 0;
	}

	/**
	 * @return The unitType that is displayed or -1 if none. See also @ref
	 * tile and @ref unit
	 **/
	int unitType() const 
	{
		return (commandType() == CommandUnit) ? mUnitType : -1;
	}

	/**
	 * Only valid if @ref unitType is >= 0! If @ref unitType is -1 then this
	 * will also be NULL !
	 * @return Usually NULL, except if the widget displays a production
	 * entry (i.e. an order button) then it is the player that produces
	 * here.
	 **/
	Player* productionOwner() const 
	{ 
		return (commandType() == CommandUnit) ? mProductionOwner : 0;
	}

	/**
	 * @return The displayed tilenumber or -1 if none is displayed. See also
	 * @ref unit and @ref unitType
	 **/
	int tile() const 
	{ 
		return (commandType() == CommandCell) ? mTileNumber : -1;
	}

	CommandType commandType() const { return mCommandType; }

	void unset();

	/**
	 * A call of this function does only make sense when @ref commandType is
	 * @ref CommandUnit.
	 *
	 * This displays the progress of the production.
	 **/
	void advanceProduction(double percentage);


	void setGrayOut(bool g);

	/**
	 * Add a small number to the shown pixmap. The number displays how many
	 * units of this type are in queue for production.
	 *
	 * -1 means display the text "paused"
	 **/
	void setProductionCount(int count);

public slots:
	void slotUnitChanged(Unit*);

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

	void signalStopProduction(int unitType);

protected:
	virtual void displayUnitPixmap(Unit* unit);
	virtual void displayUnitPixmap(int unitType, Player* owner);

	void setPixmap(const QPixmap& pixmap);

protected slots:
	void slotClicked();
	void slotRightClicked();
	
private:
	class BosonCommandWidgetPrivate;
	BosonCommandWidgetPrivate* d;

	Unit* mUnit;
	int mUnitType;
	int mTileNumber;
	CommandType mCommandType;

	Player* mProductionOwner;

	BoButton* mPixmap;
};

class BoButton : public QPushButton
{
	Q_OBJECT
public:
	BoButton(QWidget* p) : QPushButton(p)
	{
		mGrayOut = false;
		mProductionCount = 0;
	}
	
	virtual QSize sizeHint() const;

	virtual void setPixmap(const QPixmap& p);

	void setProductionCount(int c);
	void setGrayOut(bool g);

signals:
	void rightClicked();

protected:
	virtual void drawButton(QPainter*);
	virtual void mouseReleaseEvent(QMouseEvent*);

	void addProductionCount(QPixmap* pix);
	
private:
	bool mGrayOut;
	int mProductionCount;
	QPixmap mPixmap; // FIXME: this means addiditional memory space for *every* command button!!!
};

#endif
