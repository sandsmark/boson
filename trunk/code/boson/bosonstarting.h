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

#ifndef BOSONSTARTING_H
#define BOSONSTARTING_H

#include <qobject.h>

class BosonLoadingWidget;
class BosonPlayField;
class Player;

class BosonStarting : public QObject
{
	Q_OBJECT
public:
	BosonStarting(QObject* parent);

	void setLoadingWidget(BosonLoadingWidget* w) { mLoadingWidget = w; }
	void setPlayField(BosonPlayField* f) { mPlayField = f; }
	void setLocalPlayer(Player* p) { mPlayer = p; }

	void startNewGame();

	bool loadGame(const QString& fileName);

	/**
	 * Check whether there are events and process them. See @ref
	 * QApplication::processEvents.
	 *
	 * Do <em>not</em> (this is very important!!) call this when you have
	 * just done something like @ref QTimer::singleShot() or @ref
	 * Boson::sendMessage!! These calls end up in the even queue and if they
	 * are executed fast then they are executed in checkEvents
	 * <em>before</em> your function returns (which is not inended). But
	 * often they are not executed (when they have not yet reacehed the
	 * queue for example) and so you have a nice race condition that is
	 * <em>reall</em> hard to debug.
	 *
	 * So think twice before adding a call to this function. It is a good
	 * idea for long tasks (pixmap loading usually) but for everything else
	 * it is a very bad idea.
	 **/
	void checkEvents();
	
signals:
	/**
	 * Loading has been completed once this signal is emitted. The big
	 * display should be shown now.
	 *
	 * Note that at this point the <em>scenario</em> has <em>not</em> yet
	 * started. A message starting the scenario has been sent, but it will
	 * be received after this signal only. This also means that @ref
	 * Boson::gameStatus is still @ref KGame::Init
	 **/
	void signalStartGame();

	void signalStartingFailed();

	/**
	 * When this signal is emitted the game is meant to assign the map to
	 * its main classes. The map will be in the playField that has been
	 * provided by @ref setPlayField.
	 *
	 * This signal is necessary for loading code, which needs the map before
	 * units can be loaded.
	 **/
	void signalAssignMap();

protected slots:
	void slotReceiveMap(const QByteArray&);
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

	void slotLoadGameData3();

	void slotTilesLoading(int);
protected:
	/**
	 * @return The playfield. protected, as you should get this from
	 * elsewhere (e.g. @ref TopWidget) if you are outside this class
	 **/
	BosonPlayField* playField() const { return mPlayField; }

	void loadPlayerData();
	void loadUnitDatas(Player* player, int progress);

public: // FIXME
	BosonLoadingWidget* mLoadingWidget;
	BosonPlayField* mPlayField;
	Player* mPlayer;

	bool mLoading; //AB: find a way around this!
};

#endif

