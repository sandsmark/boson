/***************************************************************************
                          boserver.h  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : capricel@enst.fr                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef BOSERVER_H 
#define BOSERVER_H 

/*
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 
*/

#include <qintdict.h>

#include <ksock.h>
#include <ktmainwindow.h>

#include "../common/boconfig.h"
#include "../common/unitType.h"
#include "../common/log.h"
#include "../common/boFile.h"

#include "serverUnit.h"		// for Facility
#include "connect.h"
#include "player.h"

class serverCell;
class Unit;
class Facility;
class mobUnit;


/** 
  * This is the main widget of the Boson Server GUI
  */
class BosonServer : public KTMainWindow, public boFile
{
  Q_OBJECT

 public:
  BosonServer(const char *mapfile, const char *name=0L);
  ~BosonServer();

  void	handleDialogMessage(int playerId, bosonMsgTag, int, bosonMsgData *);

  protected:
  void	initLog(void);
  void	initSocket(void);
  void	handleGameMessage(int playerId, bosonMsgTag, int, bosonMsgData *);

  void	initMap(const char *);

  void	createMobUnit(mobileMsg_t &);
  void	createFixUnit(facilityMsg_t &);

  void  checkUnitVisibility(Unit *u);

/* playing utilities */
  void checkFixKnown(serverFacility *f);
  void checkMobileKnown(serverMobUnit *m);

  public slots:
  void handleNewConnection(KSocket *);
  void clientClose(KSocket *);

/* playing utilities */
  void requestAction();
  void checkKnownState();

  private:

  bool		loadGround();
  bool		loadUnits();

  serverState	state;
  KServerSocket	*socket;
  uint		confirmedJiffies;

  serverCell	**cells;

  QIntDict<serverMobUnit>	mobile;
  QIntDict<serverFacility>	facility;

  long		key;
};

#endif // BOSERVER_H