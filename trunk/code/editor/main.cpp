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

#include "boeditor.h" 
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
 
int main(int argc, char* argv[])
{ 
	KAboutData aboutData(
		"boeditor"
		, I18N_NOOP("Boson level editor")
		, "0.5" // XXX should use the #define somewhere
		, I18N_NOOP("A level editor for the boson game")
		, KAboutData::License_GPL
		, "(c) 1999-2000, The boson team"
		, 0l
		, "http://aquila.rezel.enst.fr/boson"
		, "boson-fb@yalbi.com" );
	   
	aboutData.addAuthor("Thomas Capricelli", I18N_NOOP("Game Design & Coding"), "orzel@yalbi.com", "http://aquila.rezel.enst.fr/thomas/");
	aboutData.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
		                                                              
	KCmdLineArgs::init( argc, argv, &aboutData );

	KApplication app;  
 
	BoEditorApp* boEditor = new BoEditorApp( (argc>1)?argv[1]:0l);
	app.setMainWidget(boEditor);
	app.setTopWidget(boEditor);
	boEditor->show();

	return app.exec();
}  
 
