/***************************************************************************
                          boson.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include <stdio.h>
#include <unistd.h>

#include <qframe.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kmenubar.h>
#include <khelpmenu.h>

#include "common/boconfig.h"
#include "common/log.h"

#include "boson.h"
#include "ressource.h"
#include "game.h"
#include "connectDlg.h"


FILE *logfile = (FILE *) 0L;

BosonApp::BosonApp()
{

	/* logfile handling */
	logfile = fopen(BOSON_LOGFILE_CLIENT, "a+b");
	if (!logfile) {
		logfile = stderr;
		logf(LOG_ERROR, "Can't open logfile, using stderr");
	}
	logf(LOG_INFO, "========= New Log File ==============");

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


/*
void BosonApp::dlgModified()
{
	int qret=KMessageBox::warningYesNoCancel(this,
		i18n("The current file has been modified. \nSave Changes ?"),
		i18n("Warning") );     

  switch (qret)
   {
   case 1:
     break;
   case 2:
     break;
   case 3:
     return;
     break;
   default:
     break;
   }
}

bool BosonApp::queryExit()
{

	int exit=KMessageBox::questionYesNo(this, i18n("Really Quit ?"), i18n("Exit"));
	// XXX : not this, but ::Close() ?

  if(exit==1)
    return true;
  else
    return false;
}

void BosonApp::slotAppExit()
{ 

  ///////////////////////////////////////////////////////////////////
  // exits the Application
  if(this->queryExit())
    {
      //saveOptions();
//      KTMainWindow::deleteAll();
      kapp->quit();
    }
  else return;
}
*/
