/***************************************************************************
                          player.h  -  description                    
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

#ifndef PLAYER_H 
#define PLAYER_H 

#include <qstring.h>
#include <qobject.h>

#include "connect.h"

class KSocket;
class BosonServer;
class boBuffer;


#define BO_INITIAL_OIL		300
#define BO_INITIAL_MINERAL	2000


/**
  * Those are infos about one player
  */
class Player : public QObject
{
  Q_OBJECT

public slots :
  void handleSocketMessage(KSocket *);

public :
  Player(void);

  serverSocketState	 socketState;
  KSocket		*socket;
  QString		*name;
  BosonServer		*server;
  int			 id;		// 'id' means that this player is (global)player[id], see game.h
  boBuffer		*buffer;

  uint 			lastConfirmedJiffies;

/* Statistics : */
  uint			fixUnitDestroyed;
  uint			mobUnitDestroyed;
  uint			UnitDestroyed;
  
/* ressources */
  bool			needFlushing;
  uint			oil;
  uint			mineral;

/* methods */
  void			flush(void);
  void			changeOil(int delta);
  void			changeMineral(int delta);

};

#endif // PLAYER_H
