/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosoncommandframebase.h"

class EditorUnitConfigWidgetPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class EditorUnitConfigWidget : public BoUnitDisplayBase
{
	Q_OBJECT
public:
	EditorUnitConfigWidget(BosonCommandFrameBase* frame, QWidget* parent);
	~EditorUnitConfigWidget();

	void updateUnit(Unit*);

signals:
	void signalUpdateUnit();

protected:
	virtual bool display(Unit* unit);
	virtual bool useUpdateTimer() const { return false; }


private:
	EditorUnitConfigWidgetPrivate* d;
};

#endif
