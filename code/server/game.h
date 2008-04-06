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

#ifndef GAME_H
#define GAME_H


#include <qdatetime.h>
#include <qstring.h>

#include <kgame/kgame.h>
#include <kgame/kgameproperty.h>

class Server;
class KGamePropertyBase;
class QDataStream;
class QDomElement;


class Game : public KGame
{
  Q_OBJECT
  public:
    enum PropertyIds
    {
      IdGameSpeed = 10000, // dont wanna #include <kgameproperty.h> - better: KGamePropertyBase::IdUser+...
      IdGamePaused = 10001,
      IdAdvanceCount = 10010,
      IdAdvanceFlag = 10011,
      IdAdvanceCallsCount = 10020
    };


    Game(Server* s, quint16 cookie);
    ~Game();

    bool connectToServer();

    virtual KPlayer* createPlayer(int rtti, int io, bool isvirtual);
    virtual bool savegame(QDataStream& stream, bool network, bool saveplayers = true);
    virtual bool loadgame(QDataStream& stream, bool network, bool reset);


    unsigned int cycle() const  { return mCycle; }
    int gameSpeed() const  { return mGameSpeed; }
    bool gamePaused() const  { return mGamePaused; }
    bool gameStarted() const  { return mGameStarted; }
    bool gameInited() const  { return mGameInited; }
    const QDateTime& gameStartedTime()  { return mGameStartedTime; }

    const QString& mapName() const  { return mMapName; }
    const QString& mapComment() const  { return mMapComment; }
    const QString& mapGroundTheme() const  { return mMapGroundTheme; }
    int mapWidth() const  { return mMapWidth; }
    int mapHeight() const  { return mMapHeight; }


  protected:
    virtual bool playerInput(QDataStream& msg, KPlayer* player);

    bool loadGameData(const QByteArray& data);
    bool loadMapData(QMap<QString, QByteArray>& files);
    bool loadPlayersData(QMap<QString, QByteArray>& files);

    bool unstreamPlayfieldFiles(QMap<QString, QByteArray>& files, const QByteArray& buffer);

  protected slots:
    void slotNetworkData(int msgid, const QByteArray& buffer, quint32 receiver, quint32 sender);
    void slotPropertyChanged(KGamePropertyBase*);


  private:
    Server* mServer;

    unsigned int mCycle;
    KGamePropertyInt mGameSpeed;
    KGamePropertyBool mGamePaused;
    KGamePropertyInt mAdvanceFlag;
    KGameProperty<unsigned int> mAdvanceCallsCount;
    bool mGameStarted;
    bool mGameInited;
    QDateTime mGameStartedTime;

    QString mMapName;
    QString mMapComment;
    QString mMapGroundTheme;
    int mMapWidth;
    int mMapHeight;
};

#endif
