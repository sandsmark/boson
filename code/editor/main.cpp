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

#include <assert.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kstddirs.h>

#include "common/log.h"
#include "editorCanvas.h"
#include "editorTopLevel.h"
#include "boeditor.h"
#include "visual.h"


/* log.h , should be moved to common !! */
FILE *logfile = (FILE *) 0L;
 
extern QPixmap *bigBackground;

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
		                                                              
	static char *fake_arg= "boeditor";
	KCmdLineArgs::init( 1, &fake_arg, &aboutData );
	//KCmdLineArgs::init( argc, argv, &aboutData );

	BoEditorApp app;


	/* temp XXX : logfile initialisation  */
	logfile = fopen(BOSON_LOGFILE_EDITOR, "a+b");
	if (!logfile) {
		logfile = stderr;
		logf(LOG_ERROR, "Can't open logfile, using stderr");
		}
	
	logf(LOG_INFO, "========= New Log File ==============");


 
	//BoEditorApp* boEditor = new BoEditorApp( (argc>1)?argv[1]:0l);


	/* XXX orzel : temp, until GUI is really functionnal */
	QString themePath = KGlobal::instance()->dirs()->findResourceDir("data", "boson/map/basic.bpf");
	themePath	+= "boson/themes/grounds/earth.png";
	printf("loading groundTheme : %s\n", themePath.latin1() );
	bigBackground = new QPixmap(themePath);
	if (bigBackground->isNull() ) {
		printf("can't load earth.jpeg\n");
		exit(1);
	}


	/* the canvas is created when a game is created */
	editorCanvas *ecanvas;
	vcanvas = ecanvas = new editorCanvas(*bigBackground);
	assert (true == ecanvas->Load("/opt/be/share/apps/boson/map/basic.bpf"));


	(new editorTopLevel(&app))->show();

	return app.exec();
}  
 
