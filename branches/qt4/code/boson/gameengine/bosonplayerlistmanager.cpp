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

const QList<Player*>& BosonPlayerListManager::allPlayerList() const
{
 return mAllPlayerList;
}


const QList<Player*>& BosonPlayerListManager::gamePlayerList() const
{
 return mGamePlayerList;
}

const QList<Player*>& BosonPlayerListManager::activeGamePlayerList() const
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

void BosonPlayerListManager::recalculatePlayerLists(const QList<KPlayer*>& playerList)
{
 recalculatePlayerListsWithPlayerRemoved(playerList, 0);
}

void BosonPlayerListManager::recalculatePlayerListsWithPlayerRemoved(const QList<KPlayer*>& playerList, KPlayer* removedPlayer)
{
 mAllPlayerList.clear();
 mGamePlayerList.clear();
 mActiveGamePlayerList.clear();
 foreach (KPlayer* kp, playerList) {
	if (kp == removedPlayer) {
		continue;
	}
	Player* p = (Player*)kp;
	mAllPlayerList.append(p);
	if (p->isGamePlayer()) {
		mGamePlayerList.append(p);
	}
	if (p->isActiveGamePlayer()) {
		mActiveGamePlayerList.append(p);
	}
 }
}

// AB: this is not completely equivalent to KGame::findPlayerByUserId(), as we
// ignore the KGame::inactivePlayerList().
// however since Boson does not use that list this should not make any
// difference.
Player* BosonPlayerListManager::findPlayerByUserId(int id) const
{
 foreach (Player* p, allPlayerList()) {
   if (p->userId() == id)
   {
     return p;
   }
 }
 return 0;
}

