/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONSTARTUPWIDGET_H
#define BOSONSTARTUPWIDGET_H

#include <qwidget.h>

class Player;
class BosonLoadingWidget;
class KCmdLineArgs;
class BosonStartupNetwork;

class BosonStartupWidget : public QWidget
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
		IdLast // MUST be the last entry!
	};

	BosonStartupWidget(QWidget* parent);
	~BosonStartupWidget();

	void setLocalPlayer(Player* p);

	/**
	 * Remove all widgets from memory
	 **/
	void resetWidgets();
	void showLoadingWidget();

	BosonStartupNetwork* networkInterface() const;

public slots:
	void slotLoadingType(int);
	void slotLoadingShowProgressBar(bool);
	void slotLoadingReset();
	void slotLoadingSetAdmin(bool isAdmin);
	void slotLoadingSetLoading(bool isLoading);
	void slotLoadingPlayersCount(int count);
	void slotLoadingPlayer(int current);
	void slotLoadingUnitsCount(int count);
	void slotLoadingUnit(int current);

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
	 * @ref BosonNewGameWidget) where the player can select the playfield.
	 **/
	void slotNewGame(KCmdLineArgs* args = 0);

	/**
	 * Mainly used internally. This will display the start editor widget
	 * (see @ref BosonNewEditorWidget) where player can select the
	 * playfield to be edited.
	 **/
	void slotStartEditor(KCmdLineArgs* args = 0);

signals:
	/**
	 * Emitted when the player asks to quit the game.
	 **/
	void signalQuit();

	void signalLoadGame(const QString& fileName);
	void signalSaveGame(const QString& fileName, const QString& description);

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

protected:
	void initWidget(WidgetId widgetId);
	void removeWidget(WidgetId widgetId);
	void showWidget(WidgetId widgetId);

	virtual bool eventFilter(QObject* o, QEvent* e);

	// AB: do NOT make this public!
	BosonLoadingWidget* loadingWidget() const;

protected slots:
	void slotShowNetworkOptions();
	void slotHideNetworkOptions();

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

private:
	void init();
	void initBackgroundOrigin(QWidget* w);

private:
	class BosonStartupWidgetPrivate;
	BosonStartupWidgetPrivate* d;
};

#endif
