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
#ifndef BOSONWIDGETBASE_H
#define BOSONWIDGETBASE_H

#include <kxmlguiclient.h>

#include <qwidget.h>

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
 * [obsolete docs got deleted]
 *
 * BosonMiniMap and BosonCommandFrame are in KDockWidgets, which you can drag
 * around and place to wherever you want
 *
 * The @ref BosonCommandFrame is currently a quite tricky part as the frame
 * differs heavily between game and editor mode. Maybe it will become two
 * classes one day, but the basic structure will stay.
 *
 * All game specific stuff should be done in other classes - e.g. visual stuff
 * (click on a unit) in @ref BosonBigDisplay, constructing in @ref
 * BosonCommandFrame and so on. These classes should emit signals which get
 * connected by BosonWidgetBase to the necessary slots - probably mainly to @ref
 * Boson.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 */
class BosonWidgetBase : public QWidget, virtual public KXMLGUIClient
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	BosonWidgetBase(TopWidget* top, QWidget* parent, bool loading = false);

	/**
	 * Default Destructor
	 **/
	virtual ~BosonWidgetBase();

	void setLocalPlayer(Player* p) { mLocalPlayer = p; }

	TopWidget* top() const { return mTop; }
	BosonCanvas* canvas() const;
	inline BosonMiniMap* minimap() const { return mMiniMap; }
	inline BoDisplayManager* displayManager() const { return mDisplayManager; }
	Boson* game() const;
	BosonPlayField* playField() const;
	Player* localPlayer() const { return mLocalPlayer; }

	void initGameMode();

	virtual void saveConfig();

	void setZoomFactor(float factor);

	bool sound() const;
	bool music() const;

	void setShowChat(bool s);

	void debugKillPlayer(KPlayer* p);

	bool isCmdFrameVisible() const;
	bool isChatVisible() const;
	void setChatVisible(bool visible);
	void setCmdFrameVisible(bool visible);

	/**
	 * Add and initialize the first @ref BosonBigDisplayBase. Note that at
	 * this point all tiles have to be loaded. See @ref BosonMap::tileSet
	 * and @ref BosonCanvas::loadTiles
	 *
	 * Note that this also calls @ref slotChangeCursor in order to load the
	 * initial cursor.
	 **/
	void addInitialDisplay();

	void init();
	virtual void initPlayer();
	virtual void quitGame();

public slots:
//	void slotPreferences();

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
	
	void slotInitFogOfWar();

protected slots:
	virtual void slotPlayerJoinedGame(KPlayer*);
	virtual void slotPlayerLeftGame(KPlayer*);

	void slotHack1();

signals:
	// hmm.. these *never* get emitted?
	// // hmm.. these *never* get emitted?!!
//	void signalPlayerJoinedGame(KPlayer* p); // used by the map editor (and debug)
//	void signalPlayerLeftGame(KPlayer* p); // used by the map editor (and debug)

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

	void signalGameOver();

	/**
	 * Emitted when the user wants to quit the game and has confirmed the
	 * "are you sure" messagebox.
	 **/
	void signalQuit();
	void signalEndGame();

	void signalCheckDockStatus();

protected slots:
	void slotChatDockHidden();
	void slotCmdFrameDockHidden();

	void slotDebug();
	void slotDebugMode(int);
	void slotDebugPlayer(int);
	void slotProfiling();
	void slotZoom(int);
	void slotToggleCmdFrameVisible();
	void slotToggleChatVisible();

	void slotCmdBackgroundChanged(const QString& file);
	void slotMiniMapScaleChanged(double);

	virtual void slotStartScenario();

	void slotUnfog(int x, int y);
	void slotFog(int x, int y);

	void slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*);

	virtual void slotChangeCursor(int mode, const QString& dir) = 0;

	void slotAddUnit(Unit* unit, int x, int y);
	void slotRemoveUnit(Unit* unit);

	/**
	 * Make display the currently active view
	 * @param active The new active display.
	 * @param old The previously active display, if non-NULL
	 **/
	void slotSetActiveDisplay(BosonBigDisplayBase* display, BosonBigDisplayBase* old);

	void slotDebugRequestIdName(int msgid, bool userid, QString& name);

	/**
	 * Directly add a chat message from the system (i.e. the game)
	 * <em>without</em> sending it over network.
	 **/
	void slotAddChatSystemMessage(const QString& fromName, const QString& text);

protected:
	void checkDockStatus();
	
	void initBigDisplay(BosonBigDisplayBase*);

	void changeCursor(BosonCursor* cursor);

	BosonCursor* cursor() const { return mCursor; }
	BosonCommandFrameBase* cmdFrame() const;

	virtual BosonCommandFrameBase* createCommandFrame(QWidget* parent) = 0;

	virtual void initKActions();
	virtual void initDisplayManager();
	virtual void initConnections();
	virtual void initMap();
	virtual void setBosonXMLFile();

private:
	void initChat();

	void initMiniMap();
	void initCommandFrame();
	void initLayout();

	/**
	 * Initialize the debug player menu and the usual player menu in editor
	 * mode. This iterates all available players and calls @ref
	 * slotPlayerJoinedGame on them
	 **/
	void initPlayersMenu();

private:
	class BosonWidgetBasePrivate;
	BosonWidgetBasePrivate* d;

	Player* mLocalPlayer;

	BosonCursor* mCursor;

	TopWidget* mTop;
	BosonMiniMap* mMiniMap;
	BoDisplayManager* mDisplayManager;

	bool mLoading;
};

#endif
