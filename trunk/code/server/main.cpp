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

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "bodebug.h"
#include "server.h"

#define BOSON_COOKIE 992
#define DEFAULT_PORT 5454
#define MAX_CLIENTS 10


static KCmdLineOptions options[] =
{
    { "port <port>", I18N_NOOP("Set the port to use. Default is 5454"), 0 },
    { "webport <webport>", I18N_NOOP("Set the port to use for the web interface. Default is <port>+1"), 0 },
    { 0, 0, 0 }
};


int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, "boserver", "BoServer", "Server for the Boson game", "0.1");
  KCmdLineArgs::addCmdLineOptions(options);

  KApplication::disableAutoDcopRegistration();
  KApplication a(false, false);

  // Parse cmdline args
  unsigned int port = DEFAULT_PORT;
  unsigned int webport = DEFAULT_PORT+1;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if(args->isSet("port"))
  {
    QString s = args->getOption("port");
    bool ok = false;
    port = s.toUInt(&ok);
    if (!ok || port > 65536) {
      boError() << "port " << s << " is invalid!" << endl;
      return 1;
    }
    webport = port + 1;
  }
  if(args->isSet("webport"))
  {
    QString s = args->getOption("webport");
    bool ok = false;
    webport = s.toUInt(&ok);
    if (!ok || webport > 65536) {
      boError() << "webport " << s << " is invalid!" << endl;
      return 1;
    }
  }

  // Create message server
  Server* server = new Server(BOSON_COOKIE);
  if(!server->init(port, webport))
  {
    boDebug() << k_funcinfo << "Couldn't init network using port " << port << endl;
    return 1;
  }

  // Set some params
  server->setMaxClients(MAX_CLIENTS);

  return a.exec();
}
