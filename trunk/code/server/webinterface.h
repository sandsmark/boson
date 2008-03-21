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

#include <qserversocket.h>

class Server;
class QTextStream;

class WebInterface : public QServerSocket
{
  Q_OBJECT
  public:
    WebInterface(Server* s, Q_UINT16 port);
    ~WebInterface();

    virtual void newConnection(int socket);

  protected slots:
    void readClient();
    void discardClient();

  protected:
    void sendStatistics(QTextStream& os);

    void writeHTTPHeader(QTextStream& os);
    void writeHTMLHeader(QTextStream& os, const QString& title);
    void writeHTMLFooter(QTextStream& os);
    void writeServerStatistics(QTextStream& os);
    void writeServerInfos(QTextStream& os);
    void writeGameInfos(QTextStream& os);
    void writeGameStatistics(QTextStream& os);
    void writeClientStats(QTextStream& os);
    void writePlayerStats(QTextStream& os);
    void writeGame(QTextStream& os);
    void writeTraffic(QTextStream& os);

    //void writeln(QTextStream& os, const QString& line);


  private:
    Server* mServer;
};

#endif

