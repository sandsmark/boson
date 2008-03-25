/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONPLAYERLISTMANAGER_H
#define BOSONPLAYERLISTMANAGER_H

#include <qobject.h>
#include <qptrlist.h>

class KPlayer;
class Player;

class BosonPlayerListManager : public QObject
{
	Q_OBJECT
public:
	BosonPlayerListManager(QObject* parent = 0);
	~BosonPlayerListManager();

	/**
	 * Calls @ref recalculatePlayerListsWithPlayerRemoved with
	 * removedPlayer==0.
	 *
	 * This should be called whenever the list of players has changed, so
	 * that this class can update all player lists.
	 *
	 * If the update occurs because a player has been removed, @ref
	 * recalculatePlayerListsWithPlayerRemoved should be preferred.
	 *
	 * Note that all @ref KPlayer objects in @p playerList must be @ref
	 * Player objects.
	 **/
	void recalculatePlayerLists(const QPtrList<KPlayer>& playerList);

	/**
	 * Recalculates the internal list of players to match those players in
	 * @p playerList with @p removedPlayer removed. If @p removedPlayer is
	 * not in @p playerList it is ignored.
	 *
	 * This method may in particular be useful if @p removedPlayer is known
	 * to get removed from @p playerList, but currently is still in the
	 * list.
	 *
	 * Note that all @ref KPlayer objects in @p playerList must be @ref
	 * Player objects.
	 **/
	virtual void recalculatePlayerListsWithPlayerRemoved(const QPtrList<KPlayer>& playerList, KPlayer* removedPlayer);

	unsigned int allPlayerCount() const;
	unsigned int gamePlayerCount() const;
	unsigned int activeGamePlayerCount() const;

	/**
	 * @return The same as @ref KGame::playerList
	 **/
	const QPtrList<Player>& allPlayerList() const;

	/**
	 * "game players" are players with ID >= 128 and <= 511. These are
	 * players who actually may own and move units in the game.
	 *
	 * This includes both, human controllable and neutral players.
	 **/
	const QPtrList<Player>& gamePlayerList() const;

	/**
	 * "active game players" are players with ID >= 128 and <= 255. These
	 * players are relevant for winning conditions and therefore take an
	 * actual part in the game.
	 *
	 * They are in particular @em not neutral players.
	 **/
	const QPtrList<Player>& activeGamePlayerList() const;

private:
	QPtrList<Player> mAllPlayerList;
	QPtrList<Player> mGamePlayerList;
	QPtrList<Player> mActiveGamePlayerList;
};

#endif

