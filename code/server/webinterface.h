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

#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include <q3serversocket.h>
//Added by qt3to4:
#include <Q3TextStream>

class Server;
class Q3TextStream;

class WebInterface : public Q3ServerSocket
{
  Q_OBJECT
  public:
    WebInterface(Server* s, quint16 port);
    ~WebInterface();

    virtual void newConnection(int socket);

  protected slots:
    void readClient();
    void discardClient();

  protected:
    void sendStatistics(Q3TextStream& os);

    void writeHTTPHeader(Q3TextStream& os);
    void writeHTMLHeader(Q3TextStream& os, const QString& title);
    void writeHTMLFooter(Q3TextStream& os);
    void writeServerStatistics(Q3TextStream& os);
    void writeServerInfos(Q3TextStream& os);
    void writeGameInfos(Q3TextStream& os);
    void writeGameStatistics(Q3TextStream& os);
    void writeClientStats(Q3TextStream& os);
    void writePlayerStats(Q3TextStream& os);
    void writeGame(Q3TextStream& os);
    void writeTraffic(Q3TextStream& os);

    //void writeln(QTextStream& os, const QString& line);


  private:
    Server* mServer;
};

#endif

