/***************************************************************************
                          cell.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 2000 by Thomas Capricelli                         
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

#include "cell.h"

/*
 *  BOSON CELLs
 */
void Cell::setGround(groundType g)
{

	switch(g) {
		case GROUND_DEEP_WATER:
			ground = g_dwater;
			break;

		case GROUND_WATER:
		case GROUND_WATER_OIL:
			ground = g_water;
			break;

		case GROUND_GRASS:
		case GROUND_GRASS_OIL:
			ground = g_grass;
			break;

		case GROUND_DESERT:
			ground = g_desert;
			break;
		default:
			ground = g_unknown;
			unsetFlag(known_f);
			return;
	}
	setFlag(known_f);
}

bool Cell::canGo(mobType type )
{
	if (!isKnown()) return true;
	uint goFlag = mobileProp[type].goFlag;

	// ... in the air
	if (goFlag & BO_GO_AIR) return flying_unit();

	// on field
	if ( building() || field_unit() ) return false;

	//  nothing ? depends on the ground
	switch(ground) {
		case g_unknown:
			return true;
			break;
		case g_dwater:
			return goFlag & BO_GO_DEEP_WATER;
			break;
		case g_water:
			return goFlag & BO_GO_WATER;
			break;
		case g_grass:
			return goFlag & BO_GO_GRASS;
			break;
		case g_desert:
			return goFlag & BO_GO_DESERT;
			break;
	}
	// dead code to prevent a warning
	return true;
}

