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
#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include "bosonwidgetbase.h"

class KPlayer;
class KGamePropertyBase;

class BosonCursor;
class BosonCanvas;
class BosonBigDisplay;
class BosonBigDisplayBase;
class Unit;
class Player;
class BoDisplayManager;
class Boson;
class BosonMiniMap;
class BosonPlayField;
class BosonGroundTheme;

/**
 * [obsolete docs got removed]
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class EditorWidget : public BosonWidgetBase
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	EditorWidget(QWidget* parent);

	/**
	 * Default Destructor
	 **/
	virtual ~EditorWidget();

	virtual void saveConfig();

	virtual void initMap();
	virtual void initPlayer();

public slots:
signals:

protected slots:
	virtual void slotChangeCursor(int mode, const QString& dir);

	void slotEditPlayerMinerals();
	void slotEditPlayerOil();
	void slotEditHeight(bool);

	void slotLockAction(bool);

	void slotGameStarted();

	void slotEditorPreferences();

protected:
	virtual void initDisplayManager();
	virtual void initConnections();
	virtual void startScenarioAndGame();

private:
	class EditorWidgetPrivate;
	EditorWidgetPrivate* d;

	QString mCursorTheme; // path to cursor pixmaps
};

#endif
