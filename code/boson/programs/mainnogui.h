/*
    This file is part of the Boson game
    Copyright (C) 2005-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef MAINNOGUI_H
#define MAINNOGUI_H

#include <qobject.h>
#include <q3valuelist.h>
#include "../defines.h"

class KPlayer;
class Player;
class BosonStarting;

class MainNoGUIAIPlayerOptions
{
public:
	enum IO {
		DefaultIO = -1,
		NoIO = 0,

		// the following values can be OR'ed together
		ComputerIO = 1,
		LocalPlayerIO = 2
	};

public:
	MainNoGUIAIPlayerOptions()
	{
		io = DefaultIO;
	}
	int io;
	QString species;
};

class MainNoGUIStartOptions
{
public:
	MainNoGUIStartOptions()
	{
		load = false;
		remotePlayers = 0;

		isClient = false;
		host = "localhost";
		port = BOSON_PORT;
	}

	void addAIPlayer();

	bool load;
	unsigned int remotePlayers;
	QString playField;

	Q3ValueList<MainNoGUIAIPlayerOptions> computerPlayers;

	bool isClient; // either client or server
	QString host; // only used if isClient == true
	int port; // only used if isClient == true || remotePlayers > 0
};

class MainNoGUIPrivate;
class MainNoGUI : public QObject
{
	Q_OBJECT
public:
	MainNoGUI();
	~MainNoGUI();

	bool init();
	bool startGame(const MainNoGUIStartOptions&);

	/**
	 * @return The @ref BosonStarting object that will be used to start the
	 * game. The gameengine is automatically loaded by this, you may need to
	 * modify this if you want to load the GUI, too.
	 **/
	BosonStarting* startingObject() const;

signals:
	/**
	 * Emitted when the player is supposed to get IOs.
	 *
	 * A slot connecting to this signal (such as @ref slotAddIOs) should
	 * remove any added IO from @p ioMask.
	 *
	 * Usually @ref slotAddIOs does all you need and you don't need to
	 * connect anything to this slot yourself. However if you need IOs that
	 * depend on libs other than the gameengine (such as the gameview), you
	 * need to use this signal.
	 *
	 * @param ioMask The IOs that should be added to the player, OR'ed
	 * together. See @ref MainNoGUIAIPlayerOptions
	 **/
	void signalAddIOs(Player* player, int* ioMask, bool* failure);

protected:
	QByteArray loadPlayFieldFromDisk(const MainNoGUIStartOptions& options);
	bool addComputerPlayersToGame(const MainNoGUIStartOptions& options, unsigned int needPlayers = 0);

protected slots:
	void slotGameStarted();
	void slotPlayerJoinedGame(KPlayer*);
	void slotCheckStart();

	/**
	 * Add IOs. This adds primarily the computer player IO.
	 **/
	void slotAddIOs(Player* player, int* ioMask, bool* failure);

private:
	MainNoGUIPrivate* d;
};


#endif // MAINNOGUI_H

