/***************************************************************************
                             stats.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Nov 11 00:34:56 EST 2000
                                           
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

#include <stdio.h> 	// printf()

// boson
#include "../boson/bosonCell.h"
#include "../boson/bosonBigDisplay.h"
#include "../boson/boshot.h"
#include "../boson/bosonTopLevel.h"
#include "../boson/playerUnit.h"
#include "../boson/bosonCanvas.h"

// server
#include "../server/boserver.h"

// editor
#include "../editor/editorTopLevel.h"
#include "../editor/editorBigDisplay.h"
#include "../editor/editorCanvas.h"

// visual
#include "visualMiniDisplay.h"
#include "selectPart.h"

// common
#include "common/bobuffer.h"

#define SIZEOF(a) printf("%20s : %d\n", #a, sizeof(a));

int main (int , char **)
{

	printf("\n\n");
	printf("Big widgets\n");

	SIZEOF(QCanvas);
	SIZEOF(visualCanvas);
	SIZEOF(bosonCanvas);
	SIZEOF(editorCanvas);

	SIZEOF(QCanvasView);
	SIZEOF(visualBigDisplay);
	SIZEOF(bosonBigDisplay);
	SIZEOF(editorBigDisplay);

	SIZEOF(visualMiniDisplay);

	SIZEOF(editorTopLevel);
	SIZEOF(bosonTopLevel);

	printf("\n\n");
	printf("Data\n");

	SIZEOF(boFile);
	SIZEOF(boBuffer);
	SIZEOF(bosonMsgData);
	SIZEOF(unitProperties_t);
	SIZEOF(mobileProperties_t);
	SIZEOF(speciesTheme);

	printf("\n\n");
	printf("Cells\n");

	SIZEOF(Cell);
	SIZEOF(bosonCell);

	SIZEOF(knownBy);
	SIZEOF(serverCell);

	SIZEOF(CellMap);
	SIZEOF(serverCellMap);

	printf("\n\n");
	printf("Misc\n");

	SIZEOF(boShot);
	SIZEOF(selectPart);
	SIZEOF(boPath);
	SIZEOF(Player);


	printf("\n\n");
	printf("Common Units\n");

	SIZEOF(Unit);
	SIZEOF(mobUnit);
	SIZEOF(Facility);

	printf("visual Units\n");
	SIZEOF(QCanvasSprite);
	SIZEOF(visualUnit);
	SIZEOF(visualMobUnit);
	SIZEOF(visualFacility);

	printf("boson Units\n");
	SIZEOF(bosonUnit);
	SIZEOF(playerMobUnit);
	SIZEOF(harvesterUnit);
	SIZEOF(playerFacility);

	printf("server Units\n");
	SIZEOF(serverUnit);
	SIZEOF(serverMobUnit);
	SIZEOF(serverFacility);
	SIZEOF(serverHarvester);

//SIZEOF();

}
