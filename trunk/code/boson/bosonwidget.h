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
#ifndef BOSONWIDGET_H
#define BOSONWIDGET_H

#include "bosonwidgetbase.h"

class KPlayer;
class KGamePropertyBase;
class KActionCollection;

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

/**
 * This is the actual main widget of boson for the game
 *
 * [obsolete docs got removed]
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonWidget : public BosonWidgetBase
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	BosonWidget(QWidget* parent);

	/**
	 * Default Destructor
	 **/
	virtual ~BosonWidget();

	virtual void saveConfig();

signals:
	void signalGameOver();
	void signalSaveGame();
	void signalLoadGame();

protected slots:
	virtual void slotChangeCursor(int mode, const QString& dir);

protected:
	virtual void initDisplayManager();

	virtual void startScenarioAndGame();

private:
	class BosonWidgetPrivate;
	BosonWidgetPrivate* d;
};

#endif
