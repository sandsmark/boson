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
class KActionCollection;

class BosonCursor;
class BosonCanvas;
class BosonCommandFrameBase;
class EditorCommandFrame;
class BosonBigDisplay;
class BosonBigDisplayBase;
class Unit;
class Player;
class TopWidget;
class BoDisplayManager;
class Boson;
class BosonMiniMap;
class BosonPlayField;
class BosonTiles;

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
 * connected by EditorWidget to the necessary slots - probably mainly to @ref
 * Boson.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class EditorWidget : public BosonWidgetBase
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	EditorWidget(TopWidget* top, QWidget* parent, bool loading = false);

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

	virtual void slotPlayerJoinedGame(KPlayer*);
	virtual void slotPlayerLeftGame(KPlayer*);

	void slotOutOfGame(Player* p);

	void slotTileSetChanged(BosonTiles*);

	void slotSavePlayFieldAs();
	void slotSavePlayField();
	void slotChangeLocalPlayer(int);
	void slotPlaceFacilities();
	void slotPlaceMobiles();
	void slotPlaceCellSmall();
	void slotPlaceCellPlain();
	void slotPlaceCellBig1();
	void slotPlaceCellBig2();

protected:
	virtual BosonCommandFrameBase* createCommandFrame(QWidget* parent);

	virtual void initKActions();
	virtual void initDisplayManager();
	virtual void initConnections();
	virtual void setBosonXMLFile();

	void savePlayField(const QString& fileName);

	EditorCommandFrame* editorCmdFrame() const;

private:
	class EditorWidgetPrivate;
	EditorWidgetPrivate* d;

	QString mCursorTheme; // path to cursor pixmaps
};

#endif
