/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

	/**
	 * Remove all widgets from memory
	 **/
	void resetWidgets();
	void showLoadingWidget();

	void setLocalPlayer(Player* player);
	Player* localPlayer() const { return mPlayer; }

public slots:
	void slotLoadingType(int);
	void slotLoadingShowProgressBar(bool);
	void slotLoadingProgress(int progress);
	void slotLoadingTileProgress(int, int);
	void slotLoadingUnitProgress(int progress, int current, int total);

	void slotShowWelcomeWidget();
	void slotSaveGame();
	void slotLoadGame();

signals:
	/**
	 * Emitted when the player asks to quit the game.
	 **/
	void signalQuit();

	void signalLoadGame(const QString& fileName);
	void signalSaveGame(const QString& fileName, const QString& description);

	/**
	 * Emitted by the editor. The first player that gets added for editor is
	 * meant to be the local player.
	 * FIXME: this is ugly. newgame widget uses @ref signalAddLocalPlayer
	 * and editorwidget uses signalSetLocalPlayer. We should somehow use the
	 * same signal for both.
	 **/
	void signalSetLocalPlayer(Player*);

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
	void showWidget(WidgetId widgetId);  // TODO: make protected

	virtual bool eventFilter(QObject* o, QEvent* e);

	// AB: do NOT make this public!
	BosonLoadingWidget* loadingWidget() const;

protected slots:
	void slotNewGame();
	void slotStartEditor();

	void slotShowNetworkOptions();
	void slotHideNetworkOptions();

private:
	void init();
	void initBackgroundOrigin(QWidget* w);

private:
	class BosonStartupWidgetPrivate;
	BosonStartupWidgetPrivate* d;

	Player* mPlayer;
};

#endif
