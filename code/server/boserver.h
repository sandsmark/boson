/***************************************************************************
                          boserver.h  -  description                    
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

#ifndef BOSERVER_H 
#define BOSERVER_H 

/*
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 
*/

#include <qintdict.h>

#include <ktmainwindow.h>

#include "common/boconfig.h"
#include "common/unitType.h"
#include "common/log.h"
#include "common/boFile.h"

#include "serverCell.h"
#include "serverUnit.h"		// for Facility
#include "connect.h"
#include "player.h"

class serverCell;
class Unit;
class KSocket;
class KServerSocket;
class Facility;
class mobUnit;
class serverMobUnit;
class serverFacility;


/** 
  * This is the main widget of the Boson Server GUI
  */
class BosonServer : public KMainWindow, public boFile
{
  Q_OBJECT

 public:
  BosonServer(int port, const char *mapfile, const char *name=0L);
  ~BosonServer();

  void	handleDialogMessage(uint playerId, bosonMsgTag, int, bosonMsgData *);
  void	playerHasDied(uint playerId);

  void		placeMob(serverMobUnit *);
  void		placeFix(serverFacility *);

  protected:
  void	initLog(void);
  void	initSocket(int port);
  void	handleGameMessage(uint playerId, bosonMsgTag, int, bosonMsgData *);

  void	initMap(const char *);

  void	createMobUnit(mobileMsg_t &);
  void	createFixUnit(facilityMsg_t &);

  void  checkUnitVisibility(Unit *u);

/* playing utilities */
  void checkFixKnown(serverFacility *f);
  void checkMobileKnown(serverMobUnit *m);

  public slots:
  void handleNewConnection(KSocket *);

/* playing utilities */
  void requestAction();
  void checkKnownState();

  private:

  bool		loadGround();
  bool		loadUnits();
  serverCell	&cell(int x, int y) {return cells[ x + y * map_width ]; }
  
  serverState	state;
  KServerSocket	*socket;
  uint		confirmedJiffies;

  serverCell	*cells;

  QIntDict<serverMobUnit>	mobile;
  QIntDict<serverFacility>	facility;

  long		key;
  
  /* gui  */
  	QLabel		*l_state, *l_connected;
};

#endif // BOSERVER_H
