/***************************************************************************
                          boserver.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : ????
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
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

#include <kapp.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "boserver.h" 
#include "game.h" 


void usage(void)
{
	char buffer[3000];

	sprintf(buffer, 
		"boserver (c) 1999 Thomas Capricelli <orzel@yalbi.com>\n"
		"\nusage : boserver [Port]\n\n"
		"\tPort is the TCP/IP port the server is listening to (default to %d)\n"
		"\tThe port number should be > 1000\n"
		"\t\n", BOSON_DEFAULT_PORT);
 	KMessageBox::information(0l, buffer, "boserver usage");
	exit(1);
}
 
int main(int argc, char* argv[])
{ 
	int		port;
	KAboutData aboutData(
		"boserver"
		, I18N_NOOP("Server for boson")
		, "0.5" // XXX should use the #define somewhere
		, I18N_NOOP("The server for the boson Game")
		, KAboutData::License_GPL
		, "(c) 1999-2000, The boson team"
		, 0l
		, "http://boson.eu.org"
		, "boson-fb@yalbi.com" );
	   
	aboutData.addAuthor("Thomas Capricelli", I18N_NOOP("Game Design & Coding"), "orzel@yalbi.com", "http://aquila.rezel.enst.fr/thomas/");
	aboutData.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
		                                                              
	KCmdLineArgs::init( argc, argv, &aboutData );

	KApplication	app;  

	port = (argc>1)?atoi(argv[1]): BOSON_DEFAULT_PORT;
	if (! (port>1000) ) usage ();
	
/*	if (app.isRestored()) 
		RESTORE(BosonServer);
	else { */

	server = new BosonServer (port, locate ("data", "boson/map/basic.bpf"));
	server->show();
//}  
	return app.exec();
}  
 
