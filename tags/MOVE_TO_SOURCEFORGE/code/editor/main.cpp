/***************************************************************************
                          main.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : ?????
                                           
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

#include <assert.h>
#include <stdlib.h> // exit()

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kstddirs.h>

#include "common/log.h"
#include "editorCanvas.h"
#include "editorTopLevel.h"
#include "boeditor.h"
#include "game.h"


/* log.h , should be moved to common !! */
FILE *logfile = (FILE *) 0L;

int main(/*int argc, char* argv[] */)
{ 
	KAboutData aboutData(
		"boeditor"
		, I18N_NOOP("Boson level editor")
		, VERSION
		, I18N_NOOP("A level editor for the boson game")
		, KAboutData::License_GPL
		, "(c) 1999-2000, The boson team"
		, 0l
		, "http://boson.eu.org"
		, "boson-fb@yalbi.com" );
	   
	aboutData.addAuthor("Thomas Capricelli", I18N_NOOP("Game Design & Coding"), "orzel@yalbi.com", "http://aquila.rezel.enst.fr/thomas/");
	aboutData.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
		                                                              
	static char *fake_arg= (char*)"boeditor";
	KCmdLineArgs::init( 1, &fake_arg, &aboutData );
	//KCmdLineArgs::init( argc, argv, &aboutData );

	BoEditorApp app;
 
	//BoEditorApp* boEditor = new BoEditorApp( (argc>1)?argv[1]:0l);


	/* find dataPath */
	dataPath = new QString(KGlobal::instance()->dirs()->findResourceDir("data", "boson/pics/biglogo.bmp") + "boson/"); 
	logf(LOG_INFO, "dataPath set to %s", dataPath->latin1());

	/* XXX orzel : temp, until GUI is really functionnal */
	QString themePath = *dataPath +  "themes/grounds/earth.png";
//	printf("loading groundTheme : %s\n", themePath.latin1() );
	bigBackground = new QPixmap(themePath);
	if (bigBackground->isNull() ) {
		logf(LOG_ERROR, "can't load %s", themePath.latin1());
		// XXX some ::sorry() box here..
		exit(1);
	}


	/* the canvas is created when a game is created */
	vcanvas = ecanvas = new editorCanvas(*bigBackground);

	// first window, more may be added later with edit/new window
	// app
	app.slot_newWindow();

	app.do_open( *dataPath + "map/basic.bpf");

	return app.exec();
}  
 