/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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

#ifndef BOUFOSTARTUPWIDGET_H
#define BOUFOSTARTUPWIDGET_H

#include <boufo/boufo.h>

class Player;
class BoUfoLoadingWidget;
class KCmdLineArgs;
class BosonStartupNetwork;

class BoUfoStartupWidgetPrivate;
class BoUfoStartupWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	enum WidgetId {
		IdWelcome = 0,
		IdNewGame = 1,
		IdLoadSaveGame = 2,
		IdStartEditor = 3,
		IdLoading = 4,
		IdNetwork = 5,
		IdLoadFromLog = 6,
		IdLast // MUST be the last entry!
	};

	BoUfoStartupWidget();
	~BoUfoStartupWidget();

	void setLocalPlayer(Player* p);

	/**
	 * Remove all widgets from memory
	 **/
	void resetWidgets();
	void showLoadingWidget();

	BosonStartupNetwork* networkInterface() const;

	/**
	 * Directly load a game - this is a shortcut for @ref slotLoadGame and
	 * then entering the filename + loading the game there.
	 **/
	void loadGame(const QString& fileName);
	void saveGame(const QString& fileName, const QString& description, bool forceOverwrite);

public slots:
	void slotLoadingMaxDuration(unsigned int maxDuration);
	void slotLoadingTaskCompleted(unsigned int duration);
	void slotLoadingStartTask(const QString& text);
	void slotLoadingStartSubTask(const QString& text);

	/**
	 * Show the welcome widget and reset all previous widget, i.e. delete
	 * all widgets except the welcome widget.
	 *
	 * See also @ref resetWidgets
	 **/
	void slotShowWelcomeWidget();

	/**
	 * Display the @ref KLoadSaveGameWidget in save status, i.e. when this
	 * is called the player can save a game.
	 **/
	void slotSaveGame();

	/**
	 * Display the @ref KLoadSaveGameWidget in load status, i.e. when this
	 * is called the player can select a game to be loaded.
	 **/
	void slotLoadGame();

	/**
	 * Mainly used internally. This will display the new game widget (see
	 * @ref BosonNewGameWidget) for a single-player game where the player can
	 * select the playfield.
	 **/
	void slotNewSinglePlayerGame(KCmdLineArgs* args = 0);
	void slotLoadFromLog(const QString& fileName);

	/**
	 * Mainly used internally. This will display the network widget and then new
	 * game widget (see @ref BosonNewGameWidget) for a multi-player game.
	 **/
	void slotNewMultiPlayerGame(KCmdLineArgs* args = 0);

	/**
	 * Mainly used internally. This will display the start editor widget
	 * (see @ref BosonNewEditorWidget) where player can select the
	 * playfield to be edited.
	 **/
	void slotStartEditor(KCmdLineArgs* args = 0);

signals:
	/**
	 * Force updating the GL widget. Usually this is not necessary, as
	 * there is a timer regulary updating the widget.
	 **/
	void signalUpdateGL();

	/**
	 * Emitted when the player asks to quit the game.
	 **/
	void signalQuit();

	/**
	 * The load/save widget has been canceled. We should return to the
	 * welcome widget or to the game (depends on whether a game is currently
	 * running).
	 **/
	void signalCancelLoadSave();

	/**
	 * This gets emitted when the new game dialog gets constructed. now a
	 * player should get added that will be our local player.
	 **/
	void signalAddLocalPlayer();

	/**
	 * Gets emitted when the welcome widget gets shown. The game should
	 * perform any kind of reset that might be necessary - especially it
	 * should clear <em>all</em> players out of the game (as the newgame
	 * widget currently requires this - please delete this doc if that has
	 * changed!!)
	 **/
	void signalResetGame();

	/**
	 * AB: this is an ugly signal. @ref signalResetGame or so should be used
	 * directly probably.
	 *
	 * This signal is meant to call @ref BosonMainWidget::slotGameOver to
	 * reset the game.
	 **/
	void signalGameOver();

	void signalPreferredSizeChanged();

	void signalPreferences();

protected:
	void initWidget(WidgetId widgetId);
	void removeWidget(WidgetId widgetId);
	void showWidget(WidgetId widgetId);

	void newGame(KCmdLineArgs* args = 0);

	virtual bool eventFilter(QObject* o, QEvent* e);

	// AB: do NOT make this public!
	BoUfoLoadingWidget* loadingWidget() const;

protected slots:
	void slotNetworkOptionsOk();
	void slotNetworkOptionsCancel();
	void slotNewGameCancelled();

	void slotOfferingConnections();
	void slotConnectingToServer();
	void slotConnectedToServer();

	/**
	 * Called when the local player is kicked out of a game (in newgame
	 * widget only).
	 *
	 * This calls @ref Boson::disconnect and re-adds the local player
	 **/
	void slotKickedOut();

	void slotVisibleWidgetChanged(BoUfoWidget* w);

private:
	void init();

private:
	BoUfoStartupWidgetPrivate* d;
};

#endif
