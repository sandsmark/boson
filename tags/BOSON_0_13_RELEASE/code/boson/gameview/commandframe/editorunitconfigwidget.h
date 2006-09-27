/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef EDITORUNITCONFIGWIDGET_H
#define EDITORUNITCONFIGWIDGET_H

#include "bosoncommandframe.h"

class EditorUnitConfigWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class EditorUnitConfigWidget : public BoUnitDisplayBase
{
	Q_OBJECT
public:
	EditorUnitConfigWidget(BosonCommandFrame* frame);
	~EditorUnitConfigWidget();

	/**
	 * Update @p unit with values from the widget
	 **/
	void updateUnit(Unit* unit);

signals:
	void signalUpdateUnit();

protected:
	/**
	 * Display @p unit in the widget
	 **/
	virtual bool display(Unit* unit);
	void displayProductionPlugin(Unit* unit);
	void displayHarvesterPlugin(Unit* unit);
	void displayResourceMinePlugin(Unit* unit);

	void updateProductionPlugin(Unit* unit);
	void updateHarvesterPlugin(Unit* unit);
	void updateResourceMinePlugin(Unit* unit);

	virtual bool useUpdateTimer() const { return false; }


private:
	EditorUnitConfigWidgetPrivate* d;
};

#endif
