/***************************************************************************
                          boson.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include <stdio.h>
#include <unistd.h>

#include <qframe.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kmenubar.h>
#include <khelpmenu.h>
#include <kstddirs.h>
#include <kinstance.h>
#include <kdebug.h>

#include "common/boconfig.h"
#include "common/log.h"

#include "bosonTopLevel.h"
#include "bosonCanvas.h"
#include "boson.h"
//#include "ressource.h"
#include "game.h"
#include "connectDlg.h"


FILE *logfile = (FILE *) 0L;
/* the different topLevel Window */
QList<bosonTopLevel>	topLevels;

BosonApp::BosonApp()
{

	/* logfile handling */
	logfile = fopen(BOSON_LOGFILE_CLIENT, "a+b");
	if (!logfile) {
		logfile = stderr;
		logf(LOG_ERROR, "Can't open logfile, using stderr");
	}
	logf(LOG_INFO, "========= New Log File ==============");

	/* find the boson data dir */
	dataPath = new QString(KGlobal::instance()->dirs()->findResourceDir("data", "boson/pics/biglogo.bmp") + "boson/"); 
	logf(LOG_INFO, "dataPath initialised at %s", dataPath->latin1());

	/* application initialisation */
	init();

	/* first window */
	initSocket(); 	// XXX temporary : should be moved
}

BosonApp::~BosonApp()
{
	/* logfile handling */
	logf(LOG_INFO, "Closing logfile.\n+++\n\n");
	if (logfile != stderr) fclose(logfile);
}

void BosonApp::init()
{ 
}


void BosonApp::initSocket(char *servername)
{ 

	int ret;
	connectDlg *dlg;

	/* find a connection to a server */
	dlg = new connectDlg(this, servername, "connect_0");
	ret = dlg->exec();
	delete dlg;
	if ( ret != QDialog::Accepted ) {
		logf(LOG_ERROR, "initSocket : connectDlg rejected with value %d", ret);
		return;
	}
	logf(LOG_INFO, "initSocket : connectDlg accepted, going on");
}


void BosonApp::initCanvas(uint w, uint h, uint nb)
{
	/* the field is created when a game is created */
	QString themePath = *dataPath + "themes/grounds/earth.png";

	QPixmap *p = new QPixmap(themePath);
	if (p->isNull() ) {
		logf(LOG_ERROR, "can't load earth.png");
		kdError() << "can't load earth.png" << endl;
		exit(1);
	}

	/* the canvas is created when a game is created */
	vcanvas = bocanvas = new bosonCanvas(*p, w, h, nb);

	bosonTopLevel *btl = new bosonTopLevel(this);
	btl->show();
	topLevels.append(btl);
	logf(LOG_INFO, "canvas and first TopLevel window created");

}


void BosonApp::serverDied(KProcess *)
{
	logf(LOG_FATAL,"boson : server died unexpectedly ");
  	KMessageBox::error(0l ,
			"The server has died unexpectedly, please report the bug"
			"to the author <orzel@yalbi.com>. Please send the boson-server.log"
			"and boson-client.log you may find in the directory from where"
		        "you've launched boson (probably your home if you've used a menu entry",
			"unexpected server death"
			);
	return;
}

