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
#include <qlayout.h>

#include <ksock.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "common/log.h"
#include "common/boconfig.h"
#include "common/bobuffer.h"
#include "common/msgData.h"

#include "serverDlg.h"
#include "game.h"

serverDlg::serverDlg(const char *name)
	:KDialogBase( Plain, i18n("Server Parameters"), Ok|Cancel, Ok, 
	0l, name,true, true, i18n("Start Server")) // parentless
{
	QLabel		*label;
	char		host[2000];
	proc = 0l;

	/* layout */
	QVBoxLayout* topLayout = new QVBoxLayout(plainPage(), spacingHint());
	QGridLayout* grid = new QGridLayout(topLayout, 2, 4, 20);
	grid->addColSpacing(0, 10);
	grid->addColSpacing(3, 10);
	
	/* params */
	label = new QLabel("Boson Server :", plainPage());
	grid->addWidget(label, 0, 1);
	
	if (gethostname(host, 1999)) {
		logf(LOG_ERROR, "can't get hostname, aborting");
		return;
	}
	l_host = new QLabel(host, plainPage());
	grid->addWidget(l_host, 0, 2);

	label = new QLabel("Connecting Port :", plainPage());
	grid->addWidget(label, 1, 1);
	e_port = new QLineEdit(plainPage());
	e_port->setText( BOSON_DEFAULT_PORT_CHAR );
	e_port->setMaxLength(6);
	grid->addWidget(e_port, 1, 2);

	/* beautification */
	label = new QLabel(plainPage());
	label->setPixmap ( *dataPath + "pics/biglogo.bmp"); 
	//label->setPixmap( QPixmap( *dataPath + "pics/biglogo.bmp" ));
	boAssert(!label->pixmap()->isNull());
	topLayout->insertWidget(0, label);
}

void serverDlg::slotOk(void)
{
	int port = atoi(e_port->text());

	if ( ! (port>1000) ) {
		logf(LOG_FATAL,"slotOk : unexpected port %s.", (const char *)e_port->text());
  		KMessageBox::error(this, "The port must be an integer bigger than 1000", "unexpected port");
		return;
	}

	emit configure(l_host->text(), (const char *)e_port->text()); // so that connectDlg knows where to connect

	proc = new KProcess();
	*proc << "boserver"; // FIXME: won't work if server isn't installed in $PATH
	*proc << "-port"<< e_port->text();
	
	connect( proc, SIGNAL(processExited(KProcess *)),		SLOT(serverDied(KProcess *)) );
	connect( proc, SIGNAL(receivedStdout(KProcess *, char *, int)), SLOT(receivedStdout(KProcess *, char *, int)) );
	proc->start(KProcess::NotifyOnExit, KProcess::Stdout);
	// perhaps start with DontCare otherwise the server will exit when this client exits

	enableButtonOK(false);
	
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
	enableButtonOK(true);
	if (!proc) // server can crash before timeout
		return;
	delete proc;
  	KMessageBox::error(this,
			"The server hasn't been quick enough to connect",
			"server timed out");
}


void serverDlg::serverDied(KProcess *p)
{
	boAssert(proc==p);
	delete proc;
	proc = 0l;
	enableButtonOK(true);
  	KMessageBox::error(this,
			"The server died for an unknown reason, please report\n"
			"a bug to the author <orzel@yalbi.com>...",
			"server died");
	return;
}

