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
#include "log.h"

/*
 *  BOSON CELLs
 */

bool Cell::canGo(uint goFlag, groundType g)
{

	// air/requested and air
	if ( BO_GO_AIR == goFlag) { 
	//	printf("flying\n");
		return (!flying_unit() && ! (request_flying_f&flags));
	}

	// field/requested
	if (flags&request_f) {
	//	printf("requested cell\n");
		return false;
	}

	// field
	if ( building() || field_unit() ) {
	//	printf("building/field_unit\n");
		return false;
	}

	//  nothing ? depends on the ground
	if (IS_PLAIN(g))
		return canGoOnGround(g,goFlag);

	if (IS_TRANS(g)) {
		int trans = GET_TRANS_REF(g);
		return
			canGoOnGround( groundTransProp[trans].from, goFlag ) && 
			canGoOnGround( groundTransProp[trans].to, goFlag );
	}
	return false;
}


bool Cell::canGoOnGround( groundType g, uint goFlag)
{
       	switch(g) {
		case GROUND_DEEP_WATER:
		//	printf("deep water\n");
			return goFlag & BO_GO_DEEP_WATER;
		case GROUND_WATER:
//		case GROUND_WATER_OIL:
		//	printf("water\n");
			return goFlag & BO_GO_WATER;
		case GROUND_GRASS:
		case GROUND_GRASS_OIL:
		case GROUND_GRASS_MINERAL:
		//	printf("grass\n");
			return goFlag & BO_GO_GRASS;
		case GROUND_DESERT:
		//	printf("desert\n");
			return goFlag & BO_GO_DESERT;
		default:
			logf(LOG_ERROR, "canGoOnGround : unhandled groundType");
			return false;
	}
}

