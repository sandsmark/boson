/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONNEWEDITORWIDGET_H
#define BOSONNEWEDITORWIDGET_H

#include "bosonneweditorwidgetbase.h"

#include <qwidget.h>
#include <qptrdict.h>
#include <qcolor.h>
#include <qvaluelist.h>

class BosonStartupNetwork;
class BosonPlayField;
class Player;
class KPlayer;
class KGamePropertyBase;

class BosonNewEditorWidgetPrivate;
class BosonNewEditorWidget : public BosonNewEditorWidgetBase
{
	Q_OBJECT
public:
	BosonNewEditorWidget(BosonStartupNetwork* interface, QWidget* parent);
	~BosonNewEditorWidget();

protected slots: // implementations for the .ui slots
	// these slots describe actions that the local player has executed in
	// his widget. nearly all must be transferred over network before the
	// actual action is performed!
	virtual void slotPlayFieldChanged(QListViewItem*);
	virtual void slotTilesetChanged(int);
	virtual void slotMaxPlayersChanged(int);
	virtual void slotMaxWidthChanged(int);
	virtual void slotMaxHeightChanged(int);
	virtual void slotNewMapToggled(bool);

private slots:
	void slotNetStart();

	void slotNetPlayFieldChanged(BosonPlayField* field);

protected:
	BosonStartupNetwork* networkInterface() const { return mNetworkInterface; }
private:
	void initPlayFields();
	void initSpecies();
	void initTilesets();

private:
	BosonNewEditorWidgetPrivate* d;
	BosonStartupNetwork* mNetworkInterface;
};

#endif

