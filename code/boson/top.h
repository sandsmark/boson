/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef TOP_H
#define TOP_H

#include <kdockwidget.h>
#include <kdeversion.h>

class QString;
class Boson;
class Player;
class BosonPlayField;
class BosonCanvas;
class KCmdLineArgs;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class TopWidget : public KDockMainWindow
{
	Q_OBJECT
public:
	/**
	 * These are the IDs of the startup widgets, as used by e.g. @ref
	 * showStartupWidget.
	 **/
	enum StartupWidgetIds {
		IdWelcome = 0,
		IdNewGame = 1,
		IdStartEditor = 2,
		IdBosonWidget = 3,
		IdNetwork = 4,
		IdLoading = 5,
		IdLoadSaveGame = 6
	};

	/**
	 * Default Constructor
	 **/
	TopWidget();

	/**
	 * Default Destructor
	 **/
	~TopWidget();

	BosonPlayField* playField() const { return mPlayField; };
	BosonCanvas* canvas() const { return mCanvas; };

	void loadGameDockConfig();
	void loadInitialDockConfig();
	void saveGameDockConfig();
	void saveInitialDockConfig();

public slots:
	/** 
	 * Called when user clicks "start new game" button
	 * This shows BosonStartGameWidget from where you can start new game
	 **/
	void slotNewGame(KCmdLineArgs* args = 0);

	void slotStartEditor(KCmdLineArgs* args = 0);
	void slotLoadGame(KCmdLineArgs* args = 0);

	/**
	 * Starts a new game. Called when user clicks "Start game" button in
	 * BosonStartGameWidget
	 **/
	void slotStartNewGame();
	
	/** 
	 * Toggles sound
	 **/
	void slotToggleSound();

	/**
	 * Toggles music 
	 **/
	void slotToggleMusic();

	/**
	 * Toggles if menubar is shown or hidden 
	 **/
	void slotToggleMenubar();

	/**
	 * Toggles if togglebar is shown or hidden 
	 **/
	void slotToggleStatusbar();

	void slotConfigureKeys();

	/**
	 * Toggles if Boson is shown fullscreen or normally 
	 **/
	void slotToggleFullScreen();

	/**
	 * Ends current game and reinits all game data, so that a new game can
	 * be started.
	 * */
	void slotEndGame();

	/** Splits active display horzontally */
	void slotSplitDisplayHorizontal();

	/** Splits active display vertically */
	void slotSplitDisplayVertical();

	/** Removes active display */
	void slotRemoveActiveDisplay();

	void slotGameOver();

#if KDE_VERSION < 310
	virtual void setGeometry(const QRect&);
#endif

signals:
	void signalSetMobilesCount(int);// mobiles of the local player
	void signalSetFacilitiesCount(int);// facilities of the local player
	void signalMineralsUpdated(int);
	void signalOilUpdated(int);
	void signalFPSUpdated(double);
	void signalParticlesCountUpdated(int);
	void signalCanvasItemsCountUpdated(int);
	void signalCanvasAnimationsCountUpdated(int);
	void signalUnitsUpdated(int);// number of units on the canvas
	void signalShotsUpdated(int);// number of shots on the canvas

protected:
	/**
	 * This function is called when it is time for the app to save its
	 * properties for session management purposes.
	 **/
	void saveProperties(KConfig *);

	/**
	 * This function is called when this app is restored.  The KConfig
	 * object points to the session management config file that was saved
	 * with @ref saveProperties
	 **/
	void readProperties(KConfig *);

	virtual bool queryClose();
	virtual bool queryExit();

	/**
	 * End the game. All relevant classes are deleted. You
	 * may want to call @ref reinitGame after @ref endGame so that you can
	 * start a new game now.
	 **/
	void endGame();

	/**
	 * Initialize all classes and member vars so that a new game can be
	 * started. Also show the welcome widget.
	 *
	 * Note that you <em>must</em> call @ref endGame before - otherwise
	 * you'll experience a <em>big</em> memory hole (and probably a lot of
	 * instability).
	 *
	 * You may want to call @ref slotGameOver instead, which calls both @ref
	 * endGame and @ref reinitGame
	 **/
	void reinitGame();

//	virtual bool eventFilter(QObject* o, QEvent* e);

	void hideMenubar();
	void showMenubar();

	/**
	 * Shows or hides the menubar, depending on the currently raised widget
	 * (i.e. startup widget or game/editor widget) and the config settings
	 **/
	void showHideMenubar();

	void changeLocalPlayer(Player* p, bool init = true);

protected slots:
	void slotChangeLocalPlayer(Player* p) { changeLocalPlayer(p); }
	void slotUpdateStatusBar();

	/**
	 * Assign the map (from starting/loading a game) to the game.
	 *
	 * From this point on we can actually use the map.
	 **/
	void slotAssignMap();

	/**
	 * Cancenl the load/save widget and return to the game (if running), or
	 * the welcome widget.
	 **/
	void slotCancelLoadSave();

	/**
	 * Called by the @ref KLoadSaveGameWidget . This will do the actual game loading
	 * from the file into a stream and then will start the usual data
	 * loading procedure.
	 **/
	void slotLoadGame(const QString& fileName);
	void slotSaveGame(const QString& fileName, const QString& description);

	void slotSaveGame();

	void slotGameStarted();

	/**
	 * See @ref Boson::signalPlayFieldChanged.
	 *
	 * This applies the map identifier to the @ref BosonStarting object so
	 * that it can be started
	 **/
	void slotPlayFieldChanged(const QString& id);

	void slotAddLocalPlayer();
	void slotResetGame();

	void slotEditorNewMap(const QByteArray&);

private:
	void initDisplayManager();
	void initBoson();
	void initCanvas();
	void initPlayer();
	void initPlayField();
	void initKActions();
	void initStatusBar();
	void enableGameActions(bool enable);

	void slotWaitForMap();

	void initBosonWidget(); // a special case for initStartupWidget - this must get called from outside a show*Widget().



private:
	Player* mPlayer;
	BosonPlayField* mPlayField;
	BosonCanvas* mCanvas;
	KDockWidget* mMainDock;

	class TopWidgetPrivate;
	TopWidgetPrivate* d;
};

#endif
