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
#ifndef BOSONORDERBUTTON_H
#define BOSONORDERBUTTON_H

#include <qwidget.h>
#include <qpushbutton.h>
#include "../global.h"

class Unit;
class Player;
class BosonTiles;
class BoButton;
class BoProgress;
class BoToolTip;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonOrderButton : public QWidget
{
	Q_OBJECT
public:
	BosonOrderButton(QWidget* parent);
	~BosonOrderButton();

	/**
	 * Shows small overview pixmap of unit, it's health and maybe some other
	 * information
	 * This is used when you select multiple units
	 **/
	virtual void setUnit(Unit* unit);

	/**
	 * Shows only small overview pixmap of unit with type unitType
	 * This is used to show units that factory can produce
	 **/
	void setProduction(ProductionType type, unsigned long int id, Player* owner);

	/**
	 * Shows pixmap of action
	 * This is used to show available unit actions (such as attack, move or stop)
	 **/
	void setAction(UnitAction action, Player* owner);

	void setCell(int tileNo, BosonTiles* tileSet);

	/**
	 * @return The displayed unit or 0 if no unit is displayed. See also
	 * @ref tile and @ref unitType
	 **/
	Unit* unit() const
	{
		return (orderType() == OrderUnitSelected) ? mUnit : 0;
	}

	/**
	 * @return The production id that is displayed or 0 if none. See also @ref
	 * tile and @ref unit
	 **/
	unsigned long int productionId() const
	{
		return (orderType() == OrderProduce) ? mProductionId : 0;
	}

	ProductionType productionType() const
	{
		return (orderType() == OrderProduce) ? mProductionType : ProduceNothing;
	}

	/**
	 * @return The displayed action or -1 if none
	 **/
	int action() const
	{
		return (orderType() == OrderAction) ? mAction : -1;
	}

	/**
	 * Only valid if @ref unitType is > 0! If @ref unitType is 0 then this
	 * will also be NULL !
	 * @return Usually NULL, except if the widget displays a production
	 * entry (i.e. an order button) then it is the player that produces
	 * here.
	 **/
	Player* productionOwner() const
	{ 
		return (orderType() == OrderProduce) ? mProductionOwner : 0;
	}

	/**
	 * @return The displayed tilenumber or -1 if none is displayed. See also
	 * @ref unit and @ref unitType
	 **/
	int tile() const
	{ 
		return (orderType() == OrderCell) ? mTileNumber : -1;
	}

	OrderType orderType() const { return mOrderType; }

	void unset();

	/**
	 * A call of this function does only make sense when @ref orderType is
	 * @ref OrderUnit.
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
	void signalProduce(ProductionType type, unsigned long int id);

	void signalStopProduction(ProductionType type, unsigned long int id);

	/**
	 * Emitted when the player clicks on the action
	 **/
	void signalAction(int);

	/**
	 * Emitted when there are several units selected and the player clicks
	 * on one of them. This unit should become the only selected unit now.
	 **/
	void signalSelectUnit(Unit*);

protected:
	virtual void displayUnitPixmap(Unit* unit);
	virtual void displayUnitPixmap(unsigned long int unitType, Player* owner);
	virtual void displayTechPixmap(unsigned long int techType, Player* owner);

	void setPixmap(const QPixmap& pixmap);

protected slots:
	void slotClicked();
	void slotRightClicked();

private:
	friend class BoToolTip;

private:
	class BosonOrderButtonPrivate;
	BosonOrderButtonPrivate* d;

	Unit* mUnit;
	// FIXME: use only one int for all order modes
	unsigned long int mProductionId;
	ProductionType mProductionType;
	int mTileNumber;
	int mAction;
	OrderType mOrderType;

	Player* mProductionOwner;

	BoButton* mPixmap;
	BoProgress* mHealth;
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
	QPixmap mPixmap; // FIXME: this means addiditional memory space for *every* order button!!!
};

#endif
