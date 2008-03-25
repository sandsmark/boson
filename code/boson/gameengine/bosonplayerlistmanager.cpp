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

#include "bosonplayerlistmanager.h"
#include "bosonplayerlistmanager.moc"

#include "player.h"

BosonPlayerListManager::BosonPlayerListManager(QObject* parent)
	: QObject(parent)
{
}

BosonPlayerListManager::~BosonPlayerListManager()
{
}

const QPtrList<Player>& BosonPlayerListManager::allPlayerList() const
{
 return mAllPlayerList;
}


const QPtrList<Player>& BosonPlayerListManager::gamePlayerList() const
{
 return mGamePlayerList;
}

const QPtrList<Player>& BosonPlayerListManager::activeGamePlayerList() const
{
 return mActiveGamePlayerList;
}

unsigned int BosonPlayerListManager::allPlayerCount() const
{
 return mAllPlayerList.count();
}

unsigned int BosonPlayerListManager::gamePlayerCount() const
{
 return mGamePlayerList.count();
}

unsigned int BosonPlayerListManager::activeGamePlayerCount() const
{
 return mActiveGamePlayerList.count();
}

void BosonPlayerListManager::recalculatePlayerLists(const QPtrList<KPlayer>& playerList)
{
 recalculatePlayerListsWithPlayerRemoved(playerList, 0);
}

void BosonPlayerListManager::recalculatePlayerListsWithPlayerRemoved(const QPtrList<KPlayer>& playerList, KPlayer* removedPlayer)
{
 mAllPlayerList.clear();
 mGamePlayerList.clear();
 mActiveGamePlayerList.clear();
 for (QPtrListIterator<KPlayer> it(playerList); it.current(); ++it) {
	if (it.current() == removedPlayer) {
		continue;
	}
	Player* p = (Player*)it.current();
	mAllPlayerList.append(p);
	if (p->isGamePlayer()) {
		mGamePlayerList.append(p);
	}
	if (p->isActiveGamePlayer()) {
		mActiveGamePlayerList.append(p);
	}
 }
}


