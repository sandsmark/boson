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
#ifndef BOACTIONSWIDGET_H
#define BOACTIONSWIDGET_H

#include "../../boufo/boufowidget.h"

#include "../../boaction.h"

class Unit;
class BosonOrderButton;


class BoActionsWidgetPrivate;
// AB: we have a lot of duplicated code from BosonOrderWidget here. I don't want
// to merge them using a base class because the editor will porbably use a
// totally different widget.
class BoActionsWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoActionsWidget();
	~BoActionsWidget();

	void ensureButtons(unsigned int number);

	/**
	 * Hide all buttons
	 **/
	void hideButtons();

	/**
	 * Adds buttons of actions unit can do (such as attack, move or stop)
	 *
	 * @param allUnits All currently selected units. Actions that should
	 * apply to all these units will use this.
	 **/
	void showUnitActions(Unit* unit, const QPtrList<Unit>& allUnits);

	void resetButton(BosonOrderButton* button);

signals:
	void signalAction(const BoSpecificAction& action);

private:
	BoActionsWidgetPrivate* d;
};

#endif

