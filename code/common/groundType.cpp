/***************************************************************************
                          unitType.cpp  -  description                              
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

#include "groundType.h"

const groundProperties_t noGroundProp =
	{"no_pixmap", };

const groundProperties_t groundProp[] = {
	{"grass", },
	{"water", },
	{"desert", },
	};

const groundTransProperties_t groundTransProp[] = {
	{"grass->water", GROUND_GRASS, GROUND_WATER},		// TRANS_GW,
	{"grass->desert", GROUND_GRASS, GROUND_DESERT, },		// TRANS_GD,
	};

const int groundPropNb = (sizeof(groundProp)/sizeof(groundProp [0]));
const int groundTransPropNb = (sizeof(groundTransProp)/sizeof(groundTransProp [0]));
