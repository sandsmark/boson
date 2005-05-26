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

#include <kapplication.h>
#include <kcmdlineargs.h>

#include "bodebug.h"
#include "server.h"

#define BOSON_COOKIE 992
#define DEFAULT_PORT 5454
#define MAX_CLIENTS 10


int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, "boserver", "BoServer", "Server for the Boson game", "0.1");

  KApplication::disableAutoDcopRegistration();
  KApplication a(false, false);

  // Create message server
  Server* server = new Server(BOSON_COOKIE);
  if(!server->init(DEFAULT_PORT))
  {
    boDebug() << k_funcinfo << "Couldn't init network using port " << DEFAULT_PORT << endl;
    return 1;
  }

  // Set some params
  server->setMaxClients(MAX_CLIENTS);

  return a.exec();
}
