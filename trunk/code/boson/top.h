/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
class KPlayer;
class Player;
class Unit;
class BosonItem;
class BosonCanvas;
class BosonWidgetBase;
class KCmdLineArgs;
class KDialogBase;
class KGamePropertyBase;

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

	/**
	 * Check the installation. The implementation of this method is free to
	 * pre-load some data (e.g. parts of the playfields) while checking for
	 * existence.
	 *
	 * This method is supposed to find out whether the data files are
	 * installed in the expected path. A check for a single file should be
	 * sufficient for this.
	 *
	 * @return An i18n'ed error string describing what went wrong (to be
	 * displayed in a message box for example), or QString::null if no
	 * problem was found.
	 **/
	static QString checkInstallation();

public slots:
	/**
	 * Called when user clicks "start new game" button
	 * This shows BosonStartGameWidget from where you can start new game
	 **/
	void slotNewGame(KCmdLineArgs* args = 0);

	void slotStartEditor(KCmdLineArgs* args = 0);
	void slotLoadGame(KCmdLineArgs* args = 0);
	void slotLoadFromLog(const QString& logFile);

	/**
	 * Starts a new game. Called when user clicks "Start game" button in
	 * BosonStartGameWidget
	 **/
	void slotStartNewGame();

	/**
	 * Ends current game and reinits all game data, so that a new game can
	 * be started.
	 * */
	void slotEndGame();

	void slotGameOver();

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

	void changeLocalPlayer(Player* p);
	void saveConfig();

protected slots:
	void slotChangeLocalPlayer(Player* p);

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
	void slotStartingFailed();

	void slotAddLocalPlayer();
	void slotResetGame();

	void slotEditorNewMap(const QByteArray&);

	void slotDebugRequestIdName(int msgid, bool userid, QString& name);

	void slotSaveExternalStuffAsXML(QDomElement& root);
	void slotLoadExternalStuffFromXML(const QDomElement& root);
	void slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer);

private:
	void initDisplayManager();
	void initBoson();

private:
	KDockWidget* mMainDock;

	class TopWidgetPrivate;
	TopWidgetPrivate* d;
};

class BoStatusBarHandler : public QObject
{
	Q_OBJECT
public:
	BoStatusBarHandler(KDockMainWindow*, QObject* parent);
	~BoStatusBarHandler() { }

	void setLocalPlayer(Player*);
	void setCanvas(const BosonCanvas*);

public slots:
	/**
	 * Toggles if togglebar is shown or hidden
	 **/
	void slotToggleStatusbar(bool show);

signals:
	/**
	 * @internal
	 **/
	void signalSetMobilesCount(int);

	/**
	 * @internal
	 **/
	void signalSetFacilitiesCount(int);

	/**
	 * @internal
	 **/
	void signalMineralsUpdated(int);

	/**
	 * @internal
	 **/
	void signalOilUpdated(int);

	/**
	 * @internal
	 **/
	void signalEffectsCountUpdated(int);

	/**
	 * @internal
	 **/
	void signalCanvasItemsCountUpdated(int);

	/**
	 * @internal
	 **/
	void signalCanvasAnimationsCountUpdated(int);

	/**
	 * @internal
	 **/
	void signalUnitsUpdated(int);

	/**
	 * @internal
	 **/
	void signalShotsUpdated(int);

protected slots:
	void slotUpdateStatusBar();
	void slotUnitCountChanged(Player*);
	void slotItemAdded(BosonItem*);
	void slotUnitRemoved(Unit*);
	void slotPlayerPropertyChanged(KGamePropertyBase* prop, KPlayer* p);

private:
	void initStatusBar();

	KDockMainWindow* mMainWindow;
	Player* mLocalPlayer;
};

#endif
