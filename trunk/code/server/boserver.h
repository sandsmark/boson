/***************************************************************************
                          boserver.h  -  description                    
                             -------------------                                         

    version              :                                   
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
//#include "../common/boconnect.h"
#include "../common/unitType.h"
//#include "../common/unit.h"	// for Facility
#include "serverUnit.h"	// for Facility
#include "../common/log.h"
//#include "unit.h"		// for mobUnit
#include "../common/playField.h"
#include "connect.h"
#include "player.h"

class serverCell;
class Unit;
class Facility;
class mobUnit;

///orzel : plus temporaire que cette merde, tu meurs :
#define MAX_UNIT	40

struct serverMap {
	serverCell	**cells;
	int		width;
	int		height;
	};


/** 
  * This is the main widget of the Boson Server GUI
  */
class BosonServer : public KTMainWindow
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

  void	createMobUnit(uint who, uint x, uint y, mobType);
  void	createFixUnit(uint who, uint x, uint y, facilityType);
	
/* initialization of playfield */ 
  void	initPeople(void);
  void	createMobUnit(origMobile o) {createMobUnit(o.who, o.x, o.y, o.t);}
  void	createFixUnit(origFacility o) {createFixUnit(o.who, o.x, o.y, o.t);}

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

  serverState	state;
  KServerSocket	*socket;
  uint		confirmedJiffies;


//  int		maxX, maxY;
//  serverCell	**cells; 
  serverMap 	map;
  origPeople	people;

  QIntDict<serverMobUnit>	mobile;
  QIntDict<serverFacility>	facility;

  long		key;
};

#endif // BOSERVER_H
