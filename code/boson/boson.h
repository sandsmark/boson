/***************************************************************************
                          boson.h  -  description                              
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

#ifndef BOSON_H 
#define BOSON_H 
 
// include files for QT
// include files for KDE 
#include <kapp.h> 
// application specific includes
#include "common/msgData.h"


class KSocket;
class KProcess;


/*
 * The boson Application : handle the different communication layers
 *
 */
class BosonApp : public KApplication
{
	Q_OBJECT

public:
	/** construtor */
	BosonApp(void); 
	/** destructor */
	~BosonApp();


public slots:
	/** first communication layer : socket */
	void handleSocketMessage(KSocket *);

protected slots:
	/** socket initialisation : try to connect to the BosonServer */
	void initSocket(char *servername=0l);

	/** slots used when the connection is lost, whoever detected this */
	void connectionLost(KSocket *s);

	/** second communication layer : client/server */
	void handleDialogMessage(bosonMsgTag, int, bosonMsgData *);
	/** third (and last) communication layer : game */
	void handleGameMessage(bosonMsgTag, int, bosonMsgData *);

	/** called whenever the server launched by serverDlg died */
	void serverDied(KProcess *);

private:
	void init(void); // internal
	void initCanvas(int, int);
	void gameEnd( endMsg_t::endReasonType reason );

signals:
	void ressourcesUpdated(void);
};

#endif // BOSON_H
 
