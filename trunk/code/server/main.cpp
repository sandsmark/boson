/***************************************************************************
                          boserver.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : ????
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
    email                : orzel@yalbi.com                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>	// exit(), atoi()

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapp.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "boserver.h" 
#include "game.h" 

static KCmdLineOptions options[] = {
	{ "m", 0, 0 }, 
	{ "map <file.bpf>", 
		I18N_NOOP("The name of the map to use for the game, refers to $KDEDIR/share/apps/boson/map/"),
		"basic.bpf"},
	{ "p", 0, 0 }, 
	{ "port <port>", 
		I18N_NOOP("The TCP/IP port the server is listening to. Must be >1000 "),
		BOSON_DEFAULT_PORT_CHAR },
	KCmdLineLastOption
};

 
int main(int argc, char* argv[])
{ 
	// AboutData
	KAboutData aboutData(
		"boserver"
		, I18N_NOOP("Server for boson")
		, VERSION
		, I18N_NOOP("The server for the boson Game")
		, KAboutData::License_GPL
		, "(c) 1999-2000, The boson team"
		, 0l
		, "http://boson.eu.org"
		, "boson-fb@yalbi.com" );
	   
	aboutData.addAuthor("Thomas Capricelli", I18N_NOOP("Game Design & Coding"), "orzel@yalbi.com", "http://aquila.rezel.enst.fr/thomas/");
	aboutData.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
		                                                              
	// arguments handling
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions(options);


	KApplication	app;  
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	//my args
	int		port;
	QString		_map("basic.bpf");

	if (args->isSet("port")) {
		bool ok = true;
		port = args->getOption("port").toInt(&ok);
		if (!ok || port <= 1000)
			KCmdLineArgs::usage();
	} else
		port = BOSON_DEFAULT_PORT;

	if (args->isSet("map"))
		_map = args->getOption("map");
	_map = KGlobal::instance()->dirs()->findResourceDir("data", "boson/pics/biglogo.bmp") + "boson/map/" + _map;

	// actual server

	server = new BosonServer (port, _map.latin1() );
	server->show();
	return app.exec();
}  
 
