/***************************************************************************
                          unitType.cpp  -  description                              
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

#include "groundType.h"

/*
	GROUND_UNKNOWN = 0
	GROUND_DEEP_WATER,
	GROUND_WATER,
	GROUND_GRASS,
	GROUND_DESERT,

	GROUND_WATER_OIL,
	GROUND_GRASS_OIL,
*/

const groundProperties_t groundProp[] = {
	{"hidden", },
	{"dwater", },
	{"water", },
	{"grass", },
	{"desert", },
	{"water_oil", },
	{"grass_oil", },
	};		// XXX this one could be moved to  map/createGround.cpp

const groundTransProperties_t groundTransProp[] = {
	{"grass->water",  GROUND_GRASS,		GROUND_WATER},		// TRANS_GW,
	{"grass->desert", GROUND_GRASS,		GROUND_DESERT},		// TRANS_GD,
	{"desert->water", GROUND_DESERT,	GROUND_WATER},		// TRANS_DW,
	{"dwater->water", GROUND_DEEP_WATER,	GROUND_WATER},		// TRANS_DWD,
	};

const int groundPropNb = (sizeof(groundProp)/sizeof(groundProp [0]));
const int groundTransPropNb = (sizeof(groundTransProp)/sizeof(groundTransProp [0]));

