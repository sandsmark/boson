/*
    This file is part of the Boson game
    Copyright (C) 2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "../boaction.h"

class Unit;
class Player;
class BosonGroundTheme;
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
	/**
	 * What this button is showing ATM
	 * @li Nothing - it's an empty button. Nothing is shown
	 * @li Unit - a unit is shown (with health bar)
	 * @li Action - an action is shown
	 * @li Cell - cell is shown (for placement)
	 **/
	enum ShowingType { ShowNothing = 0, ShowUnit, ShowAction, ShowCell };


	BosonOrderButton(QWidget* parent);
	~BosonOrderButton();

	/**
	 * Shows small overview pixmap of unit, it's health and maybe some other
	 * information
	 * This is used when you select multiple units
	 **/
	virtual void setUnit(Unit* unit);

	void setAction(const BoSpecificAction& action);

	void setGround(unsigned int texture, BosonGroundTheme* theme);

	/**
	 * @return The displayed unit or 0 if no unit is displayed. See also
	 * @ref texture and @ref unitType
	 **/
	Unit* unit() const
	{
		return mUnit;
	}

	/**
	 * @return The production id that is displayed or 0 if none. See also @ref
	 * texture and @ref unit
	 **/
	unsigned long int productionId() const;

	ProductionType productionType() const;

	/**
	 * @return The displayed action
	 **/
	const BoSpecificAction& action() const { return mAction; }

	/**
	 * Only valid if @ref unitType is > 0! If @ref unitType is 0 then this
	 * will also be NULL !
	 * @return Usually NULL, except if the widget displays a production
	 * entry (i.e. an order button) then it is the player that produces
	 * here.
	 **/
	Player* productionOwner() const;

	/**
	 * @return The displayed texturenumber or 0 if none is displayed. See also
	 * @ref unit and @ref unitType
	 **/
	unsigned int texture() const
	{
		return (type() == ShowCell) ? mTextureNumber : 0;
	}

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

	ShowingType type() const { return mType; }

public slots:
	void slotUnitChanged(Unit*);

signals:
	/**
	 * Emitted when the player clicks on this widget and it is a cell.
	 **/
	void signalPlaceGround(unsigned int textureNumber);

	/**
	 * Emitted when the player clicks on the action
	 **/
	void signalAction(const BoSpecificAction& action);

	/**
	 * Emitted when there are several units selected and the player clicks
	 * on one of them. This unit should become the only selected unit now.
	 **/
	void signalSelectUnit(Unit*);

protected:
	virtual void displayUnitPixmap(Unit* unit);
	virtual void displayUnitPixmap(unsigned long int unitType, Player* owner);

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
	unsigned int mTextureNumber;
	BoSpecificAction mAction;

	ShowingType mType;

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
