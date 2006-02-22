/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "../boufo/boufowidget.h"
#include "../boufo/boufopushbutton.h"
#include "../boufo/boufoimage.h"
#include "../global.h"
#include "../boaction.h"


class Unit;
class Player;
class PlayerIO;
class BosonGroundTheme;
class BoOrderButtonButton;
class BoUfoProgress;
class BoToolTip;
class BoOrderButtonDrawable;

class BosonOrderButtonPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonOrderButton : public BoUfoWidget
{
	Q_OBJECT
public:
	/**
	 * What this button is showing ATM
	 * @li Nothing - it's an empty button. Nothing is shown
	 * @li Unit - a unit is shown (with health bar)
	 * @li Action - an action is shown
	 * @li ShowGround - ground type is shown (for placement)
	 **/
	enum ShowingType { ShowNothing = 0, ShowUnit, ShowAction, ShowGround };


	BosonOrderButton();
	~BosonOrderButton();

	/**
	 * Shows small overview pixmap of unit, it's health and maybe some other
	 * information
	 * This is used when you select multiple units
	 **/
	virtual void setUnit(Unit* unit);

	void setAction(const BoSpecificAction& action);

	void setGround(unsigned int groundtype, const BosonGroundTheme* theme);

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
	PlayerIO* productionOwner() const;

	/**
	 * @return The displayed ground type or 0 if none is displayed. See also
	 * @ref unit and @ref unitType
	 **/
	unsigned int groundType() const
	{
		return (type() == ShowGround) ? mGroundType : 0;
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
	 * Emitted when the player clicks on this widget and it is a ground type.
	 **/
	void signalPlaceGround(unsigned int groundType);

	/**
	 * Emitted when the player clicks on the action
	 **/
	void signalAction(const BoSpecificAction& action);

	/**
	 * Emitted when there are several units selected and the player clicks
	 * on one of them. This unit should become the only selected unit now.
	 **/
	void signalSelectUnit(Unit*);

	void signalMouseEntered();
	void signalMouseLeft();

protected:
	virtual void displayUnitPixmap(Unit* unit);
	virtual void displayUnitPixmap(unsigned long int unitType, const Player* owner);

	void setImage(const BoUfoImage& image, float progressPercentage = 100.0f);

protected slots:
	void slotClicked();
	void slotRightClicked();

private:
	BosonOrderButtonPrivate* d;

	Unit* mUnit;
	// FIXME: use only one int for all order modes
	unsigned int mGroundType;
	BoSpecificAction mAction;

	ShowingType mType;

	BoOrderButtonButton* mPixmapButton;
	BoUfoProgress* mHealth;
};

/**
 * @internal
 **/
class BoOrderButtonButton : public BoUfoPushButton
{
	Q_OBJECT
public:
	BoOrderButtonButton();
	~BoOrderButtonButton();

	virtual void setImage(const BoUfoImage& img);

	void setProgressPercentage(float percentage);
	void setProductionCount(int c);
	void setGrayOut(bool g);

signals:
	void rightClicked();

protected slots:
	void slotMouseReleaseEvent(QMouseEvent*);

private:
	BoOrderButtonDrawable* mDrawable;
};

#endif
