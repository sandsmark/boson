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

#ifndef SERVER_H
#define SERVER_H

#include <kgame/kmessageserver.h>

#include <qdatetime.h>
//Added by qt3to4:
#include <Q3ValueList>


class WebInterface;
class Game;


class Server : public KMessageServer
{
  Q_OBJECT
  public:
    Server(quint16 cookie, QObject* parent = 0);
    ~Server();

    bool init(quint16 port, quint16 webport);

    void setMaxClients(int max);


    virtual void broadcastMessage(const QByteArray& msg);
    virtual void sendMessage(quint32 id, const QByteArray& msg);
    virtual void sendMessage(const Q3ValueList<quint32>& ids, const QByteArray& msg);


    unsigned int inTraffic()  { return mInTraffic; }
    unsigned int outTraffic()  { return mOutTraffic; }

    const QDateTime& timeServerStarted()  { return mTimeServerStarted; }

    Game* game()  { return mGame; }
    quint32 gameClientId()  { return mGameClientId; }


  public slots:
    void slotClientNumChanged();


  protected:
    void receivedGameMessage(QDataStream& stream);
    void receivedPlayerMessage(QDataStream& stream);

    void processBosonMessage(QDataStream& stream, int msgid);
    void processKGameMessage(QDataStream& stream, int msgid);

    void processStatusMessage(QDataStream& stream);
    void processStatusEvent(QDataStream& stream);

    void gameWasStarted();
    void gameWasEnded();


  protected slots:
    void slotClientConnected(KMessageIO* client);
    void slotConnectionLost(KMessageIO* client);
    void slotMessageReceived(const QByteArray& data, quint32 clientId, bool& unknown);

    virtual void getReceivedMessage(const QByteArray& msg);

  private:
    //KMessageServer* mServer;
    quint16 mCookie;
    quint16 mPort;

    WebInterface* mWeb;
    Game* mGame;
    quint32 mGameClientId;

    unsigned int mInTraffic;
    unsigned int mOutTraffic;

    QDateTime mTimeServerStarted;
};

#endif
