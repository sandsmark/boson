/***************************************************************************
                         serverDlg.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Nov 25 17:57:00 CET 1999
                                           
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

#ifndef SERVER_DLG_H 
#define SERVER_DLG_H 

#include <qdialog.h>

class QLineEdit;
class QPushButton;
class QLabel;
class KProcess;


class serverDlg : public QDialog 
{
	Q_OBJECT

public:
	serverDlg(QWidget *parent=0l, const char *name=0l);

signals:
	void	configure(const char *server, const char *port);
private slots:
	void	doIt(void);
	void	receivedStdout(KProcess *proc, char *buffer, int buflen);
    	/** called whenever the server died */
	void	serverDied(KProcess *);
	void	timeOut();

private:
	QLineEdit	*e_server, *e_port;
	QPushButton	*b_ok;
	QLabel		*l_host;
	KProcess	*proc;
};


#endif // SERVER_DLG_H 

