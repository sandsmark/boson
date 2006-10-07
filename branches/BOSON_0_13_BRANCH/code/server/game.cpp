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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "game.h"
#include "game.moc"

#include "server.h"
#include "bodebug.h"
#include "player.h"
#include "../boson/gameengine/bosonmessageids.h"

#include <qcstring.h>
#include <qmap.h>
#include <qfile.h>
#include <qdom.h>
#include <qptrlist.h>

#include <kgame/kmessageclient.h>
#include <kgame/kgamepropertyhandler.h>
#include <kgame/kgamemessage.h>


#define BOSON_SAVEGAME_END_COOKIE 1718


Game::Game(Server* s, Q_UINT16 cookie) : KGame(cookie)
{
  setPolicy(PolicyClean);

  mServer = s;

  mCycle = 0;
  mGameStarted = false;
  mGameInited = false;

  mMapWidth = -1;
  mMapHeight = -1;

  connect(this, SIGNAL(signalNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)),
      this, SLOT(slotNetworkData(int, const QByteArray&, Q_UINT32, Q_UINT32)));
  connect(dataHandler(), SIGNAL(signalPropertyChanged(KGamePropertyBase*)),
      this, SLOT(slotPropertyChanged(KGamePropertyBase*)));

  mGamePaused.setEmittingSignal(false); // make valgrind happy
  mGamePaused.registerData(IdGamePaused, dataHandler(),
      KGamePropertyBase::PolicyClean, "GamePaused");
  mGamePaused.setLocal(false);
  mGamePaused.setEmittingSignal(true);
  mGameSpeed.registerData(IdGameSpeed, dataHandler(),
      KGamePropertyBase::PolicyClean, "GameSpeed");
  mGameSpeed.setLocal(0);
  mAdvanceFlag.registerData(IdAdvanceFlag, dataHandler(),
      KGamePropertyBase::PolicyLocal, "AdvanceFlag");
  mAdvanceCallsCount.registerData(IdAdvanceCallsCount, dataHandler(),
      KGamePropertyBase::PolicyLocal, "AdvanceCallsCount");

  mAdvanceFlag.setLocal(0);
  mAdvanceCallsCount.setLocal(0);
  mAdvanceCallsCount.setEmittingSignal(false);
  mAdvanceFlag.setEmittingSignal(false);
}

Game::~Game()
{
}

bool Game::connectToServer()
{
  if(!KGameNetwork::connectToServer("localhost", mServer->serverPort()))
  {
    boDebug() << k_funcinfo << "Couldn't connect to server" << endl;
    return false;
  }

  return true;
}

KPlayer* Game::createPlayer(int rtti, int io, bool isvirtual)
{
  boDebug() << k_funcinfo << endl;
  if(!isvirtual)
  {
    boError() << k_funcinfo << "Non-virtual player!!!" << endl;
  }
  Player* p = new Player;
  return p;
}

bool Game::savegame(QDataStream& stream, bool network, bool saveplayers)
{
  boError() << k_funcinfo << endl;
  return true;
}

bool Game::loadgame(QDataStream& stream, bool network, bool reset)
{
  boDebug() << k_funcinfo << endl;

  // Load magic data
  Q_UINT8 a, b1, b2, b3;
  Q_INT32 c;
  Q_UINT32 v;
  stream >> a >> b1 >> b2 >> b3;
  if((a != 128) || (b1 != 'B' || b2 != 'S' || b3 != 'G'))
  {
    // Error - not Boson SaveGame
    boError() << k_funcinfo << "invalid magic cookie" << endl;
    return false;
  }
  stream >> c;
  if(c != cookie())
  {
    // Error - wrong cookie
    boError() << k_funcinfo << "Invalid cookie in header (found: " << c << "; should be: " << cookie() << ")" << endl;
    return false;
  }
  // Load game version
  stream >> v;

  // Load KGame stuff
  boDebug() << "calling KGame::loadgame" << endl;
  if(!KGame::loadgame(stream, network, reset))
  {
    // KGame loading error
    boError() << k_funcinfo << "KGame loading error" << endl;
    return false;
  }
  boDebug() << k_funcinfo << "kgame loading successful" << endl;

  // KGame::loadgame() also loads the gamestatus. some functions depend on KGame
  // to be in Init status as long as it is still loading, so we set it manually
  // here. we can't do this using KGame::setStatus(), as the policy is clean, but
  // we need Init state *now*. Changing policy would also change our property
  // policies (we use both clean and local policies, so this would not work).
  {
    // set gameStatus to Init. Will be set to Run later
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
    KGameMessage::createPropertyHeader(s, KGamePropertyBase::IdGameStatus);
    s << (int)KGame::Init;
    QDataStream readStream(b, IO_ReadOnly);
    dataHandler()->processMessage(readStream, dataHandler()->id(), false);
  }

  // Check end cookie
  Q_UINT32 endcookie;
  stream >> endcookie;
  if(endcookie != BOSON_SAVEGAME_END_COOKIE)
  {
    boError() << k_funcinfo << "Invalid end cookie!" << endl;
    return false;
  }

  boDebug() << k_funcinfo << " done" << endl;
  return true;
}

bool Game::playerInput(QDataStream& msg, KPlayer* player)
{
  return true;
}

void Game::slotNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 receiver, Q_UINT32 sender)
{
 QDataStream stream(buffer, IO_ReadOnly);
  switch(msgid)
  {
    case BosonMessageIds::IdNewGame:
    {
      if (mGameStarted) {
        boError() << k_funcinfo << "received IdNewGame, but game is already running" << endl;
        return;
      }
      mGameInited = true;
      Q_INT8 gameMode; // game/editor mode
      stream >> gameMode;
      if(gameMode != 1) {
        boError() << k_funcinfo << "Not in game mode??? mode: " << gameMode << endl;
        return;
      }
      QByteArray compresseddata;
      stream >> compresseddata;
      QByteArray data = qUncompress(compresseddata);
      boDebug() << "Got " << compresseddata.count() << " bytes of compressed newgame data (" <<
          data.count() << "b uncompressed)" << endl;
      loadGameData(data);
      break;
    }
    case BosonMessageIds::IdGameStartingCompleted:
    {
      if(sender == messageClient()->adminId())
      {
        // We don't load the data but we create the game object and BosonStarting
        //  requires every client to send the md5sum of the speciestheme data, so
        //  we need to fake it here.
        QByteArray b = buffer.copy();
        sendMessage(b, BosonMessageIds::IdGameStartingCompleted);
      }
      mGameInited = false;
    }
    case BosonMessageIds::IdNetworkSyncCheckACK:
    {
      if(sender == messageClient()->adminId())
      {
        // Fake ACK message
        QByteArray b = buffer.copy();
        sendMessage(b, BosonMessageIds::IdNetworkSyncCheckACK);
      }
    }
    case BosonMessageIds::IdGameIsStarted:
    {
      mCycle = 0;
      mGameStarted = true;
      mGameStartedTime = QDateTime::currentDateTime();
      break;
    }
    case BosonMessageIds::AdvanceN:
    {
      mCycle += mGameSpeed;
      if((int)(mCycle % 100) < mGameSpeed)
        boDebug() << mCycle << " cycles have passed" << endl;
      break;
    }
  }
}

void Game::slotPropertyChanged(KGamePropertyBase* p)
{
  switch (p->id()) {
    case IdGameSpeed:
      boDebug() << k_funcinfo << "speed has changed, new speed: " << mGameSpeed << endl;
      break;
    case IdGamePaused:
      boDebug() << k_funcinfo << "game paused changed! now=" << mGamePaused << endl;
      break;
    default:
      break;
  }
}

bool Game::loadGameData(const QByteArray& data)
{
  QMap<QString, QByteArray> files;
  if(!unstreamPlayfieldFiles(files, data))
  {
    boError(270) << k_funcinfo << "invalid newgame stream" << endl;
    return false;
  }

  /*QMap<QString, QByteArray>::Iterator it;
  for(it = files.begin(); it != files.end(); ++it)
  {
    QString filename(it.key());
    filename.replace("/", "_");
    QFile f("map/" + filename);
    f.open(IO_WriteOnly);
    f.writeBlock(it.data());
  }*/

  // Load map description and size
  if(!loadMapData(files))
  {
    return false;
  }

  if(!loadPlayersData(files))
  {
    return false;
  }

  return true;
}

bool Game::loadMapData(QMap<QString, QByteArray>& files)
{
  QString errorMsg;
  int line = 0, column = 0;

  QDomDocument description("BosonMapDescription");
  if(!description.setContent(QString(files["C/description.xml"]), &errorMsg, &line, &column))
  {
    boError() << k_funcinfo << "unable to set description XML content - error in line=" << line << ",column=" << column << " errorMsg=" << errorMsg << endl;
    return false;
  }
  QDomElement descriptionroot = description.documentElement();

  QDomElement nameelement = descriptionroot.namedItem("Name").toElement();
  mMapName = nameelement.firstChild().toText().nodeValue();
  QDomElement commentelement = descriptionroot.namedItem("Comment").toElement();
  mMapComment = commentelement.firstChild().toText().nodeValue();

  QDomDocument map("BosonMap");
  if(!map.setContent(QString(files["map/map.xml"]), &errorMsg, &line, &column))
  {
    boError() << k_funcinfo << "unable to set map XML content - error in line=" << line << ",column=" << column << " errorMsg=" << errorMsg << endl;
    return false;
  }
  QDomElement maproot = map.documentElement();

  QDomElement geometry = maproot.namedItem("Geometry").toElement();
  mMapGroundTheme = maproot.attribute(QString::fromLatin1("GroundTheme"));
  mMapWidth = geometry.attribute(QString::fromLatin1("Width")).toInt();
  mMapHeight = geometry.attribute(QString::fromLatin1("Height")).toInt();

  return true;
}

bool Game::loadPlayersData(QMap<QString, QByteArray>& files)
{
  QByteArray canvasXML = files["canvas.xml"];
  QString errorMsg;
  int line = 0, column = 0;

  QDomDocument canvas("BosonMapDescription");
  if(!canvas.setContent(QString(canvasXML), &errorMsg, &line, &column))
  {
    boError() << k_funcinfo << "unable to set canvas XML content - error in line=" << line << ",column=" << column << " errorMsg=" << errorMsg << endl;
    return false;
  }
  QDomElement canvasroot = canvas.documentElement();

  QDomNodeList list = canvasroot.elementsByTagName(QString::fromLatin1("Items"));
  for(unsigned int i = 0; i < list.count(); i++)
  {
    QDomElement items = list.item(i).toElement();
    bool ok = false;

    unsigned int id = items.attribute(QString::fromLatin1("PlayerId")).toUInt(&ok);
    if(!ok)
    {
      boError(260) << k_funcinfo << "PlayerId of Items Tag " << i << " is not a valid number" << endl;
      continue;
    }
    Player* owner = (Player*)findPlayerByUserId(id);
    if(!owner)
    {
      // AB: this is totally valid. less players in game, than in the
      // file.
      continue;
    }

    unsigned int units = 0;

    QDomNodeList itemList = items.elementsByTagName(QString::fromLatin1("Item"));
    for(unsigned int j = 0; j < itemList.count(); j++)
    {
      QDomElement item = itemList.item(j).toElement();
      if(item.isNull())
      {
        continue;
      }

      // Check if the item is a unit
      int rtti = item.attribute(QString::fromLatin1("Rtti")).toInt(&ok);
      if(!ok)
      {
        boError(260) << k_funcinfo << "Rtti attribute of Item is not a valid number" << endl;
        continue;
      }

      if(rtti >= 200)
      {
        // It's a unit
        units++;
      }
    }

    owner->setUnitCount(units);
  }

  return true;
}

bool Game::unstreamPlayfieldFiles(QMap<QString, QByteArray>& files, const QByteArray& buffer)
{
  // Copied from BosonPlayField::unstreamFiles()
  QDataStream stream(buffer, IO_ReadOnly);
  // magic cookie
  QCString magic;
  QCString magicEnd;
  Q_UINT32 version;
  stream >> magic;
  if(magic != QCString("boplayfield"))
  {
    boError() << k_funcinfo << "magic cookie does not match" << endl;
    return false;
  }
  stream >> version;
  if(version != 0x00)
  {
    boError() << k_funcinfo << "invalid version" << endl;
    return false;
  }
  stream >> files;
  stream >> magicEnd;
  if(magicEnd != QCString("boplayfield_end"))
  {
    boError() << k_funcinfo << "magic end-cookie does not match" << endl;
    return false;
  }
  return true;
}


