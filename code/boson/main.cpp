/***************************************************************************
                          main.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : ?????
                                           
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

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "boson.h" 
 
int main(int argc, char* argv[])
{ 

	/* about data */
	KAboutData aboutData(
		"boson"
		, I18N_NOOP("Boson game")
		, "0.5" // XXX should use the #define somewhere
		, I18N_NOOP("A realtime strategy game for KDE")
		, KAboutData::License_GPL
		, "(c) 1999-2000, The boson team"
		, 0l
		, "http://aquila.rezel.enst.fr/boson"
		, "boson-fb@yalbi.com" );
	   
	aboutData.addAuthor("Thomas Capricelli", I18N_NOOP("Game Design & Coding"), "orzel@yalbi.com", "http://aquila.rezel.enst.fr/thomas/");
	aboutData.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
		                                                              
	/* application */
	KCmdLineArgs::init( argc, argv, &aboutData );
	BosonApp app;  

	/* main event loop */
	return app.exec();
}  
 
