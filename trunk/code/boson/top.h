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

class QWidgetStack;
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

	Boson* game() const { return mBoson; };
	Player* player() const { return mPlayer; };
	BosonPlayField* map() const { return mMap; };
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
	
	/**
	 * Starts loading new game. Called when user clicks "Start game" button in
	 * BosonStartGameWidget
	 **/
	void slotStartGame();
	
	/**
	 * Shows BosonWelcomeWidget
	 * From there, user can start new game or quit
	 **/
	void slotShowMainMenu();
	/** Shows game network options */
	void slotShowNetworkOptions();
	/** Hides game network options and shows BosonStartGameWidget */
	void slotHideNetworkOptions();
	/** Toggles if chat dock is shown or hidden */
	void slotToggleChat();
	/** Toggles if  is shown or hidden */
	void slotToggleCmdFrame();
	/** Toggles sound */
	void slotToggleSound();
	/** Toggles music */
	void slotToggleMusic();
	/** Toggles if  is shown or hidden */
	void slotToggleStatusbar();
	void slotConfigureKeys();
	/** Toggles if Boson is shown fullscreen or normally */
	void slotToggleFullScreen();
	/** Ends current game */
	void slotEndGame();
	/** Shows game preferences dialog */
	void slotGamePreferences();
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
	void signalSetMobilesCount(int);
	void signalSetFacilitiesCount(int);
	void signalMineralsUpdated(int);
	void signalOilUpdated(int);

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

	void endGame();

	/**
	 * Load the background for the newgames, welcome and similar widgets.
	 * not used for the game widget.
	 **/
	void loadWidgetBackground(QWidget* widget);

protected slots:
	void slotCanvasTilesLoading(int);
	void slotCanvasTilesLoaded();
	void slotReceiveMap(const QByteArray& buffer);
	void slotCmdFrameDockHidden();
	void slotChatDockHidden();
	void slotDebugPlayer(int);

private slots:
	void loadGameData3();
	void slotDebugMode(int index);
	void slotZoom(int index);
	void slotDebug();
	void slotUnfogAll();

private:
	void initMusic();
	void initBoson();
	void initPlayer();
	void initMap();
	void initActions();
	void initStatusBar();
	void enableGameActions(bool enable);
	void initDebugPlayersMenu();

	void loadGameData1();
	void loadGameData2();
	void slotWaitForMap();
	void checkEvents();
	void checkDockStatus();

	void initWelcomeWidget();
	void showWelcomeWidget();
	void initNewGameWidget();
	void initStartEditorWidget();
	void showNewGameWidget();
	void showStartEditorWidget();
	void initBosonWidget();
	void showBosonWidget();
	void initNetworkOptions();
	void showNetworkOptions();
	void initLoadingWidget();
	void showLoadingWidget();

	void raiseWidget(int id);

private:
	QWidgetStack* mWs;
	Boson* mBoson;
	Player* mPlayer;
	BosonPlayField* mMap;
	BosonCanvas* mCanvas;
	bool mMapReceived;
	KDockWidget* mMainDock;
	bool mGame;

	class TopWidgetPrivate;
	TopWidgetPrivate* d;
};

#endif
