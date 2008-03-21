/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONLOADSAVEGAMEHANDLER_H
#define BOSONLOADSAVEGAMEHANDLER_H

#include <qobject.h>

class BosonStartupNetwork;

class BosonLoadSaveGameHandlerPrivate;
/**
 * Class that handles a @ref BoUfoLoadSaveGameWidget::signalLoadGame and @ref
 * BoUfoLoadSaveGameWidget::signalSaveGame
 *
 * This class is used because we do not subclass @ref BoUfoLoadSaveGameWidget
 * and therefore cannot do boson-specific tasks in there.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonLoadSaveGameHandler : public QObject
{
	Q_OBJECT
public:
	BosonLoadSaveGameHandler(BosonStartupNetwork* interface, QObject* parent);
	~BosonLoadSaveGameHandler();

public slots:
	/**
	 * Called by the @ref BoUfoLoadSaveGameWidget . This will do the actual game loading
	 * from the file into a stream and then will start the usual data
	 * loading procedure.
	 **/
	void slotLoadGame(const QString& fileName);
	void slotSaveGame(const QString& fileName, const QString& description, bool forceOverwrite = false);

signals:
	void signalCancelLoadSave();
	void signalGameOver();

protected:
	BosonStartupNetwork* networkInterface() const { return mNetworkInterface; }

private:
	/**
	 * Prepare for loading a game. This loads the playfield from @ref
	 * fileName and adds the players necessary for loading the game.
	 * @return An empty @ref QByteArray if an error occurred or the data
	 * necessary for @ref BosonStarting (such as the playField) if it
	 * succeeded.
	 **/
	QByteArray prepareLoadGame(const QString& loadingFileName);

	bool addLoadGamePlayers(const QString& playersXML);

private:
	BosonLoadSaveGameHandlerPrivate* d;
	BosonStartupNetwork* mNetworkInterface;
};

#endif

