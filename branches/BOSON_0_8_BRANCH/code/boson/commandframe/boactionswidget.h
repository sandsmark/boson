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
#ifndef BOACTIONSWIDGET_H
#define BOACTIONSWIDGET_H

#include <qwidget.h>

class Unit;
class BosonOrderButton;

// AB: we have a lot of duplicated code from BosonOrderWidget here. I don't want
// to merge them using a base class because the editor will porbably use a
// totally different widget.
class BoActionsWidget : public QWidget
{
	Q_OBJECT
public:
	BoActionsWidget(QWidget* parent);
	~BoActionsWidget();

	void ensureButtons(unsigned int number);

	/**
	 * Hide all buttons
	 **/
	void hideButtons();
	void setButtonsPerRow(int);

	/**
	 * Adds buttons of actions unit can do (such as attack, move or stop)
	 **/
	void showUnitActions(Unit* unit);

	void resetButton(BosonOrderButton* button);

protected:
	void resetLayout();

signals:
	void signalAction(int actionType);

private:
	class BoActionsWidgetPrivate;
	BoActionsWidgetPrivate* d;
};

#endif

