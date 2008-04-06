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

#include "webinterface.h"

#include "server.h"
#include "game.h"
#include "player.h"
#include "boson/boversion.h"

#include <q3socket.h>
#include <qregexp.h>
#include <q3textstream.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <kdebug.h>
#include <kgame/kmessageio.h>

#include "webinterface.moc"


WebInterface::WebInterface(Server* s, quint16 port) : Q3ServerSocket(port)
{
  mServer = s;
  boDebug() << "Web interface listening on port " << port << endl;
}

WebInterface::~WebInterface()
{
}

void WebInterface::newConnection(int socket)
{
  Q3Socket* s = new Q3Socket(this);
  connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
  connect(s, SIGNAL(delayedCloseFinished()), this, SLOT(discardClient()));
  s->setSocket(socket);
}

void WebInterface::readClient()
{
  Q3Socket* socket = (Q3Socket*)sender();
  if(socket->canReadLine())
  {
    QStringList tokens = QStringList::split(QRegExp("[ \r\n][ \r\n]*"), socket->readLine());
    if(tokens[0] == "GET")
    {
      Q3TextStream os(socket);
      os.setEncoding(Q3TextStream::UnicodeUTF8);
      sendStatistics(os);
      socket->close();
    }
  }
}

void WebInterface::discardClient()
{
  Q3Socket* socket = (Q3Socket*)sender();
  delete socket;
}

void WebInterface::sendStatistics(Q3TextStream& os)
{
  writeHTTPHeader(os);
  writeHTMLHeader(os, "Boson Server Statistics");
  writeServerInfos(os);
  os << "</td></tr><tr><td>\r\n";
  writeServerStatistics(os);
  os << "</td></tr></table></td><td valign=\"top\">\r\n";
  os << "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" class=\"sidebar\"><tr><td>\r\n";
  writeGameInfos(os);
  // end
  writeGameStatistics(os);

  writeHTMLFooter(os);
}

void WebInterface::writeHTTPHeader(Q3TextStream& os)
{
  os << "HTTP/1.0 200 Ok\r\n";
  os << "Content-Type: text/html; charset:=\"utf-8\"\r\n";
  os << "\r\n";
}

void WebInterface::writeHTMLHeader(Q3TextStream& os, const QString& title)
{
  os << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
  os << "<html>\r\n";
  os << "<head>\r\n";
  os << "  <title>" << title << "</title>\r\n";
  os << " <style type=\"text/css\"> \r\n\
                body {color: #00ff00;background-color: #000000;font-family: helvetica,arial;}\r\n \
                td {color: #00ff00;}\r\n \
                a {color: #40ff40;background-color: transparent;text-decoration: underline;}\r\n \
                a:visited {background-color: transparent;color: #40d040;}\r\n \
                a:hover {background-color: #204020;color: #00ff00;text-decoration: none;}\r\n \
                .sidebarboxtitle {font-weight: bold;}\r\n \
                td.boxcell {background-color: #202020;color: #00ff00;}\r\n \
                .bigboxtitle {font-weight: bold;}\r\n \
                .bigboxsubheader {font-weight: bold;vertical-align: top;}\r\n \
                table.main {background-color: #000000;color: #00ff00;}\r\n \
                td.mainarea {background-color: #000000;color: #00ff00;}\r\n \
                table.sidebar {background-color: #000000;color: #00ff00;}\r\n \
                table.sidebarbox {background-color: #004000;color: #00ff00;}\r\n \
                td.sidebarboxtitlecell {background-color: #202020;color: #00ff00;}\r\n \
                td.sidebarboxcell {background-color: #081808;color: #00ff00;}\r\n \
            </style>";
  os << "</head>\r\n";
  os << "<body>\r\n";
  os << "<table width=\"100%\" border=\"0\"><tr><td>\r\n";
  os << "<div align=\"center\"><img src=\"http://boson.eu.org/pictures/header_small.jpg\" alt=\"\"></div>\r\n";
  os << "<h1 align=\"center\">Boson Server Statistics</h1>\r\n \
            <table border=\"0\" cellpadding=\"3\" cellspacing=\"2\" width=\"100%\" class=\"main\">\r\n\
            <tr valign=\"top\"><td>\r\n\
            <table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" class=\"sidebar\">\r\n\
            <tr><td>\r\n";
}

void WebInterface::writeHTMLFooter(Q3TextStream& os)
{
  os << "</table></td></tr></table></td></tr></table>\r\n";
  os << "</td></tr>\r\n";
  os << "<tr><td>&copy; 2005 <a href=\"http://boson.eu.org\" target=\"_blank\">The Boson Team</a></td></tr>\r\n";
  os << "</table></body></html>\r\n";
}

void WebInterface::writeServerInfos(Q3TextStream& os)
{
  QString sServerStatus;
  Game* game = mServer->game();

  if(mServer->isOfferingConnections())
  {
      if (!game)
            sServerStatus = "Listening for new connections on port " + QString::number(mServer->serverPort());
      else if(game->gameInited())
            sServerStatus = "NOT accepting new connections";
      else
            sServerStatus = "Listening for new connections on port " + QString::number(mServer->serverPort());
  }
  else
  {
      sServerStatus = "NOT accepting new connections";
  }

  os << "<table cellpadding=\"2\" cellspacing=\"1\" border=\"0\" width=\"100%\" class=\"sidebarbox\">\r\n \
                    <tr><td class=\"sidebarboxtitlecell\">\r\n \
                        <font class=\"sidebarboxtitle\">&nbsp;Server Info</font>\r\n \
                    </td></tr>\r\n \
                    <tr><td class=\"sidebarboxcell\">\r\n \
                        <table width=\"100%\">\r\n \
                            <tr><td class=\"bigboxsubheader\">Version</td><td>" << BOSON_VERSION_STRING << "</td></tr>\r\n \
                            <tr><td class=\"bigboxsubheader\">Started</td><td>" << mServer->timeServerStarted().toString() << "</td></tr>\r\n \
                            <tr><td class=\"bigboxsubheader\">Status</td><td>" << sServerStatus << "</td></tr>\r\n \
                        </table>\r\n \
                    </td></tr>\r\n \
                </table>\r\n \
                <br>\r\n";
}

void WebInterface::writeServerStatistics(Q3TextStream& os)
{
  os << "<table cellpadding=\"2\" cellspacing=\"1\" border=\"0\" width=\"100%\" class=\"sidebarbox\">\r\n \
                    <tr><td class=\"sidebarboxtitlecell\">\r\n \
                        <font class=\"sidebarboxtitle\">&nbsp;Server Statistics</font>\r\n \
                    </td></tr>\r\n \
                    <tr><td class=\"sidebarboxcell\">\r\n \
                        <table width=\"100%\">\r\n \
                            <tr><td class=\"bigboxsubheader\">Incoming traffic</td><td>" << mServer->inTraffic() / 1024 << "kb</td></tr>\r\n \
                            <tr><td class=\"bigboxsubheader\">Outgoing traffic</td><td>" << mServer->outTraffic() / 1024 << "kb</td></tr>\r\n \
                        </table>\r\n \
                    </td></tr>\r\n \
                </table>\r\n \
                <br>\r\n";
}


void WebInterface::writeGameInfos(Q3TextStream& os)
{
  int clientcount = 0;
  int playercount = 0;
  QString sGameMap;
  QString sGameSize;
  QString sGameGroundTheme;
  QString sGameComment;
  QString sGameStarted;

  Game* game = mServer->game();

  if(mServer->clientCount() > 0)
  {
    clientcount = mServer->clientCount();
    playercount = game->playerCount();
  }
  else
  {
    clientcount = mServer->clientCount();
    playercount = 0;
  }

  if(game)
  {
    clientcount--;
    if(game->gameStarted())
    {
      sGameStarted = "The game is running";
      sGameMap = game->mapName();
      sGameSize = QString::number(game->mapWidth()) + "x" +  QString::number(game->mapHeight());
      sGameGroundTheme = game->mapGroundTheme();
      sGameComment = game->mapComment();
    }
    else if(game->gamePaused())
    {
      sGameStarted = "The game is paused";
      sGameMap = game->mapName();
      sGameSize = QString::number(game->mapWidth()) + "x" +  QString::number(game->mapHeight());
      sGameGroundTheme = game->mapGroundTheme();
      sGameComment = game->mapComment();
    }
    else if(game->gameInited())
      sGameStarted = "The game is loading...";
    else
        sGameStarted = "The game is not yet started";
  }
  else
  {
    sGameStarted = "The game is not yet started";
  }


  os << "<table cellpadding=\"2\" cellspacing=\"1\" border=\"0\" width=\"100%\" class=\"sidebarbox\">\r\n \
                <tr><td class=\"sidebarboxtitlecell\">\r\n \
                    <font class=\"sidebarboxtitle\">&nbsp;Game Info</font>\r\n \
                </td></tr>\r\n \
                <tr><td class=\"sidebarboxcell\">\r\n \
                        <table width=\"100%\">\r\n \
                            <tr><td class=\"bigboxsubheader\">Status</td><td>" << sGameStarted << "</td></tr>\r\n \
                            <tr><td class=\"bigboxsubheader\">Clients/Players</td><td>" << clientcount  << "/" << playercount << "</td></tr>\r\n";
  if(game && (game->gameStarted() || game->gamePaused()))
  {
    os << "                            <tr><td class=\"bigboxsubheader\">Map</td><td>" << sGameMap << "</td></tr>\r\n \
                            <tr><td class=\"bigboxsubheader\">Map size</td><td>" << sGameSize << "</td></tr>\r\n \
                            <tr><td class=\"bigboxsubheader\">Map ground theme</td><td>" << sGameGroundTheme<< "</td></tr>\r\n \
                            <tr><td class=\"bigboxsubheader\">Map comment</td><td width=\"100\">" << sGameComment << "</td></tr>\r\n";
  }
  os << "                        </table>\r\n \
                </td></tr>\r\n \
            </table>\r\n \
            <br>\r\n";
}

void WebInterface::writeGameStatistics(Q3TextStream& os)
{
  if(mServer->clientCount() > 0)
  {
    os << "<table cellpadding=\"2\" cellspacing=\"1\" border=\"0\" width=\"100%\" class=\"sidebarbox\">\r\n \
                <tr><td class=\"sidebarboxtitlecell\">\r\n \
                    <font class=\"sidebarboxtitle\">&nbsp;Game Statistics</font>\r\n \
                </td></tr>\r\n \
                <tr><td class=\"sidebarboxcell\">\r\n";
    writeClientStats(os);
    writePlayerStats(os);
    os << "</td></tr></table><br>";
  }
}

void WebInterface::writeClientStats(Q3TextStream& os)
{
    int clientcount = mServer->clientCount();
    if(mServer->game())
    {
        clientcount--;
    }
    if(clientcount == 1)
        os << "  There is 1 client in the game:<br>\r\n";
    else
        os << "  There are " << clientcount << " clients in the game:<br>\r\n";
    Q3ValueList<quint32> clientids = mServer->clientIDs();
    Q3ValueList<quint32>::iterator it = clientids.begin();
    for(; it != clientids.end(); it++)
    {
        quint32 id = *it;
        if(id == mServer->gameClientId())
        {
        continue;
        }
        KMessageIO* client = mServer->findClient(id);
        os << "  &nbsp;&nbsp;" << id << ": " << client->peerName() << ":" << client->peerPort() << "<br>\r\n";
    }
}

void WebInterface::writePlayerStats(Q3TextStream& os)
{
    Game* game = mServer->game();
    if(game->playerCount() == 1)
        os << "  There is 1 player in game:<br>\r\n";
    else
        os << "  There are " << game->playerCount() << " players in game:<br>\r\n";
    Q3PtrListIterator<KPlayer> itP(*game->playerList());
    while(itP.current())
    {
        Player* p = (Player*)itP.current();
        if(p->name() != "Neutral")
            os << "  &nbsp;&nbsp;" << p->userId() << ": " << p->name() << " - " << p->unitCount() << " units<br>\r\n";
        else
            os << "<i>  &nbsp;&nbsp;" << p->userId() << ": " << p->name() << " - " << p->unitCount() << " units</i><br>\r\n";
        ++itP;
    }
}
