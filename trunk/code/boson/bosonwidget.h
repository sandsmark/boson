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

#include <kxmlguiclient.h>

#include <qwidget.h>

class KPlayer;
class KGamePropertyBase;
class KActionCollection;

class BosonCursor;
class BosonCanvas;
class BosonCommandFrame;
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
 * This widget conists of 3 sub-widgets:
 * @li a @ref BosonBigDisplay. This is the actual game view. Here the user can
 * click on units, move units, ...
 * @li a @ref BosonMiniMap. Well, this is just the mini map (wow ;))
 * @li a @ref BosonCommandFrame. The frame where unit can be ordered, the
 * selected unit is displayed and so on.
 *
 * BosonMiniMap and BosonCommandFrame are in KDockWidgets, which you can drag
 * around and place to wherever you want
 *
 * The @ref BosonCommandFrame is currently a quite tricky part as the frame
 * differs heavily between game and editor mode. Maybe it will become two
 * classes one day, but the basic structure will stay.
 *
 * BosonWidget is responsible for connecting all of these widgets and objects
 * together, which is mostly done on constructing.
 *
 * All game specific stuff should be done in other classes - e.g. visual stuff
 * (click on a unit) in @ref BosonBigDisplay, constructing in @ref
 * BosonCommandFrame and so on. These classes should emit signals which get
 * connected by BosonWidget to the necessary slots - probably mainly to @ref
 * Boson.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonWidget : public QWidget, virtual public KXMLGUIClient
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	BosonWidget(TopWidget* top, QWidget* parent);

	/**
	 * Default Destructor
	 **/
	virtual ~BosonWidget();

	inline BosonCanvas* canvas() const;
	inline BosonMiniMap* minimap() const { return mMiniMap; }
	inline BoDisplayManager* displaymanager() const { return mDisplayManager; }
	inline Boson* game() const;
	inline BosonPlayField* map() const;
	inline Player* player() const;

	void initGameMode();

	void saveConfig(bool editor = false);

	void zoom(const QWMatrix&);

	bool sound() const;
	bool music() const;

	/**
	 * Used by the editor: either display or hide all units. You usually
	 * don't want to see the units when you edit the map, but sometimes
	 * (*g*) you need to see the units when you design a scenario...
	 **/
	void displayAllItems(bool display);

	void setShowChat(bool s);

	void debugKillPlayer(KPlayer* p);

	void initKeys();

	bool isCmdFrameVisible() const;
	bool isChatVisible() const;
	void toggleCmdFrameVisible();
	void toggleChatVisible();
	void setChatVisible(bool visible);
	void setCmdFrameVisible(bool visible);


public slots:
	void slotDebug();
	void slotGamePreferences();
	void slotEndGame();

	void slotToggleSound();
	void slotToggleMusic();

	void slotSetCommandButtonsPerRow(int b);

	/**
	 * Unfogs the map for the specified player
	 * @param player The player that shall see the map or NULL for all
	 * players
	 **/
	void slotUnfogAll(Player* player = 0);

	void slotSplitDisplayHorizontal();
	void slotSplitDisplayVertical();
	void slotRemoveActiveDisplay();

signals:
	void signalPlayerJoinedGame(KPlayer* p); // used by the map editor (and debug)
	void signalPlayerLeftGame(KPlayer* p); // used by the map editor (and debug)

	/**
	 * Emitted when the number of units of the local player changes.
	 **/
	void signalMobilesCount(int mobileUnits);

	/**
	 * Emitted when the number of units of the local player changes.
	 **/
	void signalFacilitiesCount(int facilities);

	void signalMineralsUpdated(int);
	void signalOilUpdated(int);

	void signalGameStarted();

	void signalMoveCommandFrame(int);

	void signalInitDone();

	void signalChatDockHidden();
	void signalCmdFrameDockHidden();

	void signalGameOver();

protected slots:
	void slotCmdBackgroundChanged(const QString& file);
	void slotMiniMapScaleChanged(double);

	void slotStartScenario();

	void slotUnfog(int x, int y);
	void slotFog(int x, int y);

	void slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*);

	void slotInitFogOfWar();

	void slotNotEnoughMinerals(Player*);
	void slotNotEnoughOil(Player*);

	void slotChangeCursor(int mode, const QString& dir);
	void slotChangeGroupMove(int mode);

	void slotArrowScrollChanged(int speed);
	void slotAddUnit(Unit* unit, int x, int y);
	void slotRemoveUnit(Unit* unit);

	/**
	 * Make display the currently active view
	 * @param active The new active display.
	 * @param old The previously active display, if non-NULL
	 **/
	void slotSetActiveDisplay(BosonBigDisplayBase* display, BosonBigDisplayBase* old);

	void slotOutOfGame(Player* p);

	void slotDebugRequestIdName(int msgid, bool userid, QString& name);

	void slotGameOverDialogFinished();

protected:
	void addChatSystemMessage(const QString& fromName, const QString& text);
	
	void initBigDisplay(BosonBigDisplayBase*);

private:
	void init();
	void initChat();

	void initMap();
	void initMiniMap();
	void initConnections();
	void initDisplayManager();
	void initPlayer();
	void initGameCommandFrame();
	void initLayout();


private:
	class BosonWidgetPrivate;
	BosonWidgetPrivate* d;

	BosonCursor* mCursor;
	QString mCursorTheme; // path to cursor pixmaps

	TopWidget* mTop;
	BosonMiniMap* mMiniMap;
	BoDisplayManager* mDisplayManager;

	// for performance:
	int mMobilesCount;
	int mFacilitiesCount;
};

#endif
