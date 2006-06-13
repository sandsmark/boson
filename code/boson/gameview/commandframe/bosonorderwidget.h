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
#ifndef BOSONORDERWIDGET_H
#define BOSONORDERWIDGET_H

#include "../../boufo/boufowidget.h"

#include "../../global.h"
#include "../../boaction.h"

class Unit;
class Player;
class BosonGroundTheme;
class BosonOrderButton;
class UnitProperties;
class BosonCommandFrame;
class UpgradeProperties;

template<class T> class QPtrList;
template<class T> class QValueList;

class BosonOrderWidgetPrivate;
/**
 * This is scrollable widget in the commandframe that contains buttons of unit
 * actions (when single unit is selected) or selected units (when multiple
 * units are selected)
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonOrderWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BosonOrderWidget(BosonCommandFrame* cmdframe);
	~BosonOrderWidget();

	/**
	 * Ensure that at least @p number buttons are available.
	 **/
	void ensureButtons(unsigned int number);

	/**
	 * Hide all buttons
	 **/
	void hideOrderButtons();

	/**
	 * This function is used to display production options (e.g. when the
	 * unit has a @ref ProductionPlugin or in editor mode units should get
	 * placed). The list contains actions that will be shown
	 *
	 * See also @ref hideOrderButtons, @ref showUnits and @ref
	 * setOrderButtonsGround
	 **/
	void setOrderButtons(const QValueList<BoSpecificAction>& actions);

	/**
	 * @overload
	 *
	 * This version takes an additional parameter that forces the actions
	 * at the specified indices to be "grayed out".
	 **/
	void setOrderButtons(const QValueList<BoSpecificAction>& actions, const QValueList<int>& grayActionsOut);

	/**
	 * Use the @ref BosonGroundTheme, for the order buttons, i.e. allow
	 * ground placing. See also @ref setGroundTheme.
	 **/
	void setOrderButtonsGround();

	/**
	 * Display the @p units. This is used for multiple selections, i.e. when
	 * the player selected more than one unit.
	 *
	 * Use @ref setOrderButtons or @ref hideOrderButtons if the player
	 * selected one unit only.
	 **/
	void showUnits(const QPtrList<Unit>& units);

	void productionAdvanced(Unit* factory, double percentage);

	void setGroundTheme(BosonGroundTheme* theme);

	/**
	 * @return TRUE if the widget display production options (i.e. order
	 * buttons), otherwise FALSE
	 **/
	bool isProduceAction() const;

	/**
	 * Resets button by setting it's production count to 0 and making it
	 * not-grayed-out
	 **/
	void resetButton(BosonOrderButton* button);

protected:
	/**
	 * In editor mode hide the widget to configure cells. Should be called
	 * when no cells are displayed.
	 **/
	void hideCellConfigWidgets();

	/**
	 * Call this once the user wants to place cells. This will show the
	 * cell configuration widgets. Note that you need to call @ref
	 * initEditor once before this has an effect.
	 **/
	void showCellConfigWidgets();

	/**
	 * @return TRUE if @p action is producable or placable, i.e. a
	 * information widget may display information about this action. See
	 * also @ref signalUnitTypeHighlighted and @ref
	 * signalTechnologyHighlighted
	 **/
	bool isProducibleOrPlaceable(const BoSpecificAction& action) const;

protected slots:
	void slotPlaceGround(unsigned int groundtype);

	void slotMouseEnteredButton();
	void slotMouseLeftButton();
	void slotMouseMoved(QMouseEvent* e);
	void slotCheckCursor();

signals:
	void signalAction(const BoSpecificAction& action);

	/**
	 * @param groundTypeCount See @ref BosonGroundTheme::groundTypeCount
	 * @param alpha The desired alpha values, i.e. how much of every groundtype
	 * should be displayed (255=maximum, 0=nothing). This is an array of size
	 * @p groundTypeCount.
	 **/
	void signalPlaceGround(unsigned int groundTypeCount, unsigned char* alpha);

	/**
	 * This unit should become the only selected unit. See @ref
	 * BosonOrderButton::signalSelectUnit
	 **/
	void signalSelectUnit(Unit*);

	/**
	 * Emitted when the mouse is over a button containing a producable or
	 * placable unittype. A receiving slot may want to display information
	 * about this unittype to the user.
	 **/
	void signalUnitTypeHighlighted(const PlayerIO* player, const UnitProperties* prop);

	/**
	 * Emitted when the mouse is over a button containing a producable or
	 * "placable" technology. A receiving slot may want to display information
	 * about this technology to the user.
	 **/
	void signalTechnologyHighlighted(const PlayerIO* player, const UpgradeProperties*);

private:
	BosonOrderWidgetPrivate* d;
};

#endif

