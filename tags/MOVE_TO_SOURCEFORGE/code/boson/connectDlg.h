/***************************************************************************
                         connectDlg.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Nov 11 17:19:00 CET 1999
                                           
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

#ifndef CONNECT_DLG_H 
#define CONNECT_DLG_H 

#include <kdialogbase.h>

class QLineEdit;
class QPushButton;
class BosonApp;


class connectDlg : public KDialogBase 
{
	Q_OBJECT

public:
	connectDlg(BosonApp *parent, char *servername=0l, const char *name=0l);

public slots:
	void	configure(const char *server, const char *port);

protected slots:
	virtual void	slotCancel();
	virtual void	slotOk();
	virtual void	slotUser1();
	
private slots:
//	void	timeOut(void);

private:
	BosonApp	*_parent;
	QLineEdit	*e_server, *e_port;
	QPushButton	*b_ok;
};


#endif // CONNECT_DLG_H 

