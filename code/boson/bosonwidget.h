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
class BosonCommandFrameBase;
class BosonBigDisplay;
class BosonBigDisplayBase;
class Unit;
class Player;
class TopWidget;
class BoDisplayManager;
class Boson;
class BosonMiniMap;
class BosonPlayField;

/**
 * This is the actual main widget of boson for the game
 *
 * [obsolete docs got removed]
 *
 * The @ref BosonCommandFrame is currently a quite tricky part as the frame
 * differs heavily between game and editor mode. Maybe it will become two
 * classes one day, but the basic structure will stay.
 *
 * All game specific stuff should be done in other classes - e.g. visual stuff
 * (click on a unit) in @ref BosonBigDisplay, constructing in @ref
 * BosonCommandFrame and so on. These classes should emit signals which get
 * connected by BosonWidget to the necessary slots - probably mainly to @ref
 * Boson.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonWidget : public BosonWidgetBase
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	BosonWidget(TopWidget* top, QWidget* parent, bool loading = false);

	/**
	 * Default Destructor
	 **/
	virtual ~BosonWidget();

	virtual void saveConfig();

	virtual void initPlayer();

public slots:
	void slotGamePreferences();

signals:
	void signalGameOver();
	void signalSaveGame();
	void signalLoadGame();

protected slots:
	virtual void slotStartScenario();
	virtual void slotChangeCursor(int mode, const QString& dir);

	void slotOutOfGame(Player* p);
	void slotGameOverDialogFinished();

protected:
	virtual BosonCommandFrameBase* createCommandFrame(QWidget* parent);

	virtual void initKActions();
	virtual void initConnections();
	virtual void setBosonXMLFile();

private:
	class BosonWidgetPrivate;
	BosonWidgetPrivate* d;

	QString mCursorTheme; // path to cursor pixmaps
};

#endif
