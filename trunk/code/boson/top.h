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

//#include <qstring.h>

class QWidgetStack;
class QString;
class Boson;
class Player;
class BosonPlayField;
class BosonCanvas;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class TopWidget : public KDockMainWindow
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	TopWidget();

	/**
	 * Default Destructor
	 **/
	~TopWidget();

	Player* player() const { return mPlayer; };
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
	void slotNewGame();

	void slotStartEditor();

	void slotLoadGame();

	/**
	 * Starts a new game. Called when user clicks "Start game" button in
	 * BosonStartGameWidget
	 **/
	void slotStartNewGame();
	
	/**
	 * Shows BosonWelcomeWidget
	 * From there, user can start new game or quit
	 **/
	void slotShowMainMenu();

	/**
	 * Shows game network options 
	 **/
	void slotShowNetworkOptions();

	/**
	 * Hides game network options and shows BosonStartGameWidget 
	 **/
	void slotHideNetworkOptions();

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
	void slotSaveGame();

#if KDE_VERSION < 310
	virtual void setGeometry(const QRect&);
#endif

signals:
	void signalSetMobilesCount(int);
	void signalSetFacilitiesCount(int);
	void signalMineralsUpdated(int);
	void signalOilUpdated(int);
	void signalFPSUpdated(double);

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

	virtual bool eventFilter(QObject* o, QEvent* e);

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
	void slotTilesLoading(int);
	void slotReceiveMap(const QByteArray& buffer);
	void slotUpdateFPS();

private slots:
	/**
	 * Tile loading is most time consuming action on startup.
	 *
	 * Note that this function doesn't return before all tiles are loaded,
	 * but still is non-blocking, as @ref QApplication::processEvents is
	 * called while loading
	 *
	 * This slot is called from @ref slotReceiveMap only. Once the map has
	 * been received we load its tiles.
	 **/
	void slotLoadTiles();

	void loadGameData3();

	/**
	 * Called by the @ref KLoadSaveGameWidget . This will do the actual game loading
	 * from the file into a stream and then will start the usual data
	 * loading procedure.
	 **/
	void slotLoadGame(const QString& loadingFileName);

private:
	void initBoson();
	void initCanvas();
	void initPlayer();
	void initPlayField();
	void initKActions();
	void initStatusBar();
	void enableGameActions(bool enable);

	void slotWaitForMap();
	void checkEvents();

	void initStartupWidget(int id);
	void showStartupWidget(int id);
	void initBosonWidget(bool loading = false); // a special case for initStartupWidget - this must get called from outside a show*Widget().

	void raiseWidget(int id);


private:
	void loadPlayerData(); // sound, models, textures, ...
	void loadUnitDatas(Player* p, int progress);

private:
	QWidgetStack* mWs;
	Player* mPlayer;
	BosonPlayField* mPlayField;
	BosonCanvas* mCanvas;
	KDockWidget* mMainDock;
	bool mGame;
	bool mLoading;

	class TopWidgetPrivate;
	TopWidgetPrivate* d;
};

#endif
