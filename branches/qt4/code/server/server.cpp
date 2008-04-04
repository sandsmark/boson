/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

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

#include "server.h"

#include "../boson/gameengine/bosonmessageids.h"
#include "webinterface.h"
#include "game.h"
#include "bodebug.h"
#include "player.h"

#include <kgame/kgamemessage.h>
#include <kgame/kmessageio.h>
#include <kgame/kmessageclient.h>

#include <qcstring.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qtimer.h>

#include "server.moc"



Server::Server(Q_UINT16 cookie, QObject* parent) : KMessageServer(cookie, parent)
{
  mCookie = cookie;

  mWeb = 0;
  mGame = 0;
  mGameClientId = 0;

  mInTraffic = 0;
  mOutTraffic = 0;
  mTimeServerStarted = QDateTime::currentDateTime();

  connect(this, SIGNAL(clientConnected(KMessageIO*)),
      this, SLOT(slotClientConnected(KMessageIO*)));
  connect(this, SIGNAL(connectionLost(KMessageIO*)),
      this, SLOT(slotConnectionLost(KMessageIO*)));
  connect(this, SIGNAL(messageReceived(const QByteArray&, Q_UINT32, bool&)),
      this, SLOT(slotMessageReceived(const QByteArray&, Q_UINT32, bool&)));
}

Server::~Server()
{
  delete mWeb;
  delete mGame;
}

bool Server::init(Q_UINT16 port, Q_UINT16 webport)
{
  mPort = port;
  bool ok = initNetwork(mPort);
  if(ok)
  {
    boDebug() << "Server listening on port " << mPort << endl;
    mWeb = new WebInterface(this, webport);
  }

  return ok;
}

void Server::setMaxClients(int max)
{
  KMessageServer::setMaxClients(max);
}

void Server::slotClientNumChanged()
{
  int clients = clientCount();
  boDebug() << "There are now " << clients << " clients" << endl;
  if(clients == 0)
  {
    delete mGame;
    mGame = 0;
  }
  else if(clients == 1)
  {
    if(!mGame)
    {
      // The game object must be created after the first client has joined,
      //  otherwise the client won't become admin
      mGame = new Game(this, mCookie);
      mGameClientId = 0;
      if(!mGame->connectToServer())
      {
        delete mGame;
        mGame = 0;
      }
    }
    else
    {
      // Last real client left the game, so remove the fake client as well
      if(mGame && mGame->gameStarted())
      {
        gameWasEnded();
      }
      delete mGame;
      mGame = 0;
    }
  }
}

void Server::slotClientConnected(KMessageIO* client)
{
  KMessageSocket* sock = (KMessageSocket*)client;
  boDebug() << "New client: " << sock->peerName() << ":" << sock->peerPort() << " (id: " << sock->id() << ")" << endl;
  QTimer::singleShot(0, this, SLOT(slotClientNumChanged()));

  if(!mGameClientId && mGame)
  {
    mGameClientId = sock->id();
    boDebug() << "Game's clientid is " << mGameClientId << endl;
  }
}

void Server::slotConnectionLost(KMessageIO* client)
{
  boDebug() << "Connection lost with client " << client->id() << endl;
  QTimer::singleShot(0, this, SLOT(slotClientNumChanged()));
}

void Server::broadcastMessage(const QByteArray& msg)
{
  mOutTraffic += (msg.count() + 5) * clientCount();
  KMessageServer::broadcastMessage(msg);
}

void Server::sendMessage(Q_UINT32 id, const QByteArray& msg)
{
  mOutTraffic += msg.count() + 5;
  KMessageServer::sendMessage(id, msg);
}

void Server::sendMessage(const QValueList<Q_UINT32>& ids, const QByteArray& msg)
{
  mOutTraffic += (msg.count() + 5) * ids.count();
  KMessageServer::sendMessage(ids, msg);
}

void Server::getReceivedMessage(const QByteArray& message)
{
  // Make adeep copy of the message data and create a datastream on it
  QByteArray msg = message.copy();
  QDataStream stream(msg, IO_ReadOnly);

  mInTraffic += 5 + msg.count();  // 5 bytes is packet header

  // Find the sender of the msg
  KMessageIO* client = (KMessageIO*)sender();

  Q_UINT32 kgamemsgid;
  stream >> kgamemsgid;

  if(kgamemsgid == REQ_BROADCAST)
  {
    receivedGameMessage(stream);
  }
  else if(kgamemsgid == REQ_FORWARD)
  {
    receivedPlayerMessage(stream);
  }

  KMessageServer::getReceivedMessage(message);
}

void Server::receivedGameMessage(QDataStream& stream)
{
  // Extract KGame sender/receiver and message id
  Q_UINT32 kgamesender, kgamereceiver;
  int msgid;
  KGameMessage::extractHeader(stream, kgamesender, kgamereceiver, msgid);

  //kdDebug() << "  gameMsg, id: " << msgid << endl;

  if(msgid > 256)
  {
    processBosonMessage(stream, msgid - 256);
  }
  else
  {
    processKGameMessage(stream, msgid);
  }
}

void Server::receivedPlayerMessage(QDataStream& stream)
{
  // Extract KGame sender/receiver and message id
  Q_UINT32 kgamesender, kgamereceiver;
  int kgamemsgid;
  KGameMessage::extractHeader(stream, kgamesender, kgamereceiver, kgamemsgid);

  boDebug() << "  playerMsg, id: " << kgamemsgid << endl;
}

void Server::slotMessageReceived(const QByteArray& data, Q_UINT32 clientId, bool& unknown)
{
  return;
  // Make adeep copy of the message data and create a datastream on it
  QByteArray msg = data.copy();
  QDataStream stream(msg, IO_ReadOnly);

  // Extract KGame sender/receiver and message id
  Q_UINT32 kgamesender, kgamereceiver;
  int kgamemsgid;
  KGameMessage::extractHeader(stream, kgamesender, kgamereceiver, kgamemsgid);

  bool kgamesenderplayer = KGameMessage::isPlayer(kgamesender);

  Q_UINT32 msgid;
  stream >> msgid;
  if(msgid > 256)
  {
    boDebug() << k_funcinfo << "client: " << clientId << "; kgmsgid: " << kgamemsgid << "; msgid: " << msgid << " (" << msgid - 256 << ")" << endl;
  }
  else
  {
    boDebug() << k_funcinfo << "client: " << clientId << "; kgmsgid: " << kgamemsgid << "; msgid: " << msgid << endl;
  }
}


void Server::processBosonMessage(QDataStream& stream, int msgid)
{
  if(msgid == BosonMessageIds::IdGameIsStarted)
  {
    gameWasStarted();
  }
  else if(msgid == BosonMessageIds::IdStatus)
  {
    processStatusMessage(stream);
  }
  else if(msgid == BosonMessageIds::IdNetworkSyncCheckACK)
  {
    Q_UINT32 id;
    QCString md5;
    Q_INT8 verify;
    stream >> id;
    stream >> md5;
    stream >> verify;
    if(!verify)
    {
      boError() << "A CLIENT SEEMS TO OUT OF SYNC!!!" << endl;
      boDebug() << "id: " << id << "; md5: " << md5 << endl;
    }
  }
}

void Server::processKGameMessage(QDataStream& stream, int msgid)
{
}

void Server::processStatusMessage(QDataStream& stream)
{
  Q_UINT32 playerCount;
  stream >> playerCount;
  for(unsigned int i = 0; i < playerCount; i++)
  {
    Q_UINT32 playerId;
    Q_UINT32 mobiles;
    Q_UINT32 facilities;
    stream >> playerId;
    stream >> mobiles;
    stream >> facilities;
    Player* owner = (Player*)mGame->findPlayerByUserId(playerId);
    if(owner)
    {
      owner->setUnitCount(mobiles + facilities);
    }
  }
}

void Server::gameWasStarted()
{
  boDebug() << "Game started" << endl;
  // Don't accept any more incoming connections
  stopNetwork();
}

void Server::gameWasEnded()
{
  boDebug() << "Game ended" << endl;
  // Accept incoming connections again
  initNetwork(mPort);
}

/*
 * vim: et sw=2
 */
