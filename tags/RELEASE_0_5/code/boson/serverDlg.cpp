/***************************************************************************
                         serverDlg.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Nov 25 17:57:00 CET 1999
                                           
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

#include <stdlib.h>	// atoi
#include <unistd.h>	// gethostname()

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <ksock.h>
#include <kprocess.h>
#include <kmessagebox.h>

#include "common/log.h"
#include "common/boconfig.h"
#include "common/bobuffer.h"
#include "common/msgData.h"

#include "serverDlg.h"
#include "game.h"

serverDlg::serverDlg(const char *name)
	:QDialog( 0l, name,true) // parentless
{
        QPushButton	*button;
	QLabel		*label;
	char		host[2000];

	/* layout */
	resize( 390, 340 );
	setCaption( "Server Parameters" );     
	
	/* buttons */
	b_ok = new QPushButton( "Launch", this );
	b_ok->setGeometry( 10,300, 100,30 );
	connect( b_ok, SIGNAL(clicked()), SLOT(doIt()) );

	button = new QPushButton( "Cancel", this );
	button->setGeometry( 280,300, 100,30 );
	connect( button, SIGNAL(clicked()), SLOT(reject()) );

	/* params */
	label = new QLabel("Boson Server :", this);
	label->setGeometry( 10,200, 140,30 );
	label->setAlignment(AlignVCenter | AlignRight);
	
	if (gethostname(host, 1999)) {
		logf(LOG_ERROR, "can't get hostname, aborting");
		return;
	}
	l_host = new QLabel(host, this);
	l_host->setGeometry( 160,200, 220,30 );

	label = new QLabel("Connecting Port :", this);
	label->setGeometry( 10,250, 140,30 );
	label->setAlignment(AlignVCenter | AlignRight);
	e_port = new QLineEdit(this);
	e_port->setText( BOSON_DEFAULT_PORT_CHAR );
	e_port->setGeometry( 160,250, 60,30 );
	e_port->setMaxLength(6);

	/* beautification */
	label = new QLabel(this);
	label->move( (390-352)/2, 10);		// biglogo is 352x160
	label->setAutoResize(true);
	label->setPixmap ( *dataPath + "pics/biglogo.bmp"); 
	//label->setPixmap( QPixmap( *dataPath + "pics/biglogo.bmp" ));
	boAssert(!label->pixmap()->isNull());

}

void serverDlg::doIt(void)
{
	int port = atoi(e_port->text());

	if ( ! (port>1000) ) {
		logf(LOG_FATAL,"launchServer : unexpected port %s.", (const char *)e_port->text());
  		KMessageBox::error(this, "The port must be an integer bigger than 1000", "unexpected port");
		return;
	}

	emit configure(l_host->text(), (const char *)e_port->text()); // so that connectDlg knows where to connect

	proc = new KProcess();
	*proc << "boserver";
	*proc << e_port->text();
	
	connect( proc, SIGNAL(processExited(KProcess *)),		SLOT(serverDied(KProcess *)) );
	connect( proc, SIGNAL(receivedStdout(KProcess *, char *, int)), SLOT(receivedStdout(KProcess *, char *, int)) );
	proc->start(KProcess::NotifyOnExit, KProcess::Stdout);

	b_ok->setEnabled(false);
	
	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(timeOut()) );
	timer->start( 3000, TRUE );                 // 3 seconds single-shot          

	// accept();

}

void serverDlg::receivedStdout(KProcess *p, char *buffer, int buflen)
{
	boAssert(proc==p);
	
	if (buflen != (int)strlen(BOSON_SERVER_LAUNCHED) || !strcmp(BOSON_SERVER_LAUNCHED "\n", buffer)) {
		buffer[buflen]='\0';
		logf(LOG_ERROR, "received \"%s\", waiting for \"%s\"", buffer, BOSON_SERVER_LAUNCHED);
		return;
	}

//	connect( proc, SIGNAL(processExited(KProcess *)), parent(), SLOT(serverDied(KProcess *)) );
	logf(LOG_INFO, BOSON_SERVER_LAUNCHED);
	accept();	
}

void serverDlg::timeOut(void)
{
	delete proc;
	b_ok->setEnabled(true);
  	KMessageBox::error(this,
			"The server hasn't been quick enough to connect",
			"server timed out");
	return;
}


void serverDlg::serverDied(KProcess *p)
{
	boAssert(proc==p);

	delete proc;
	b_ok->setEnabled(true);
  	KMessageBox::error(this,
			"The server died for an unknown reason, please report\n"
			"a bug to the author <orzel@yalbi.com>...",
			"server died");
	return;
}

