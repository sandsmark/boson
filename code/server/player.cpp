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

#include "player.h"


Player::Player(void)
{
	socket = (KSocket *) 0L;
	socketState = SSS_NO_CONNECT;
	name = new QString("Orzel Land");

	lastConfirmedJiffies = 1;

	fixUnitDestroyed = 0;
	mobUnitDestroyed = 0;
	UnitDestroyed = 0;
}
