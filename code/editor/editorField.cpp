/***************************************************************************
                          editorField.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
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

#include <assert.h>

//#include <kapp.h>
//#include <kmsgbox.h>

#include "../common/log.h"
#include "../common/playField.h"
#include "../common/boconfig.h" // MAX_PLAYERS
#include "../common/map.h"

#include "editorField.h"
  
editorField::editorField(uint w, uint h, QObject *parent, const char *name=0L)
	: visualField(w,h,parent,name)
{
}


bool editorField::load(QString filename)
{
	int	i, j;
	playField field(filename);

	if (false == field.load()) return false;

	map.width = field.map.width;
	map.height = field.map.height;

	/*
	 * creation of the ground map
	 */
	map.cells = new (visualCell *)[map.width];
	for (i=0; i< map.width; i++)
		map.cells[i] = new (visualCell)[map.height];

	/* initialisation */
	for (i=0; i< map.width; i++)
		for (j=0; j< map.height; j++)
			map.cells[i][j].set( field.map.cells[i][j], i, j);

	printf("***********field.load : %d, map : %d\n", field.map.cells[3][3], map.cells[3][3].getGroundType());
	printf("***********field.load : %d, map : %d\n", field.map.cells[10][9], map.cells[10][9].getGroundType());
	printf("***********field.load : %d, map : %d\n", field.map.cells[20][13], map.cells[20][13].getGroundType());


	/* freeing of field.map.cells */
	for (i=0; i< map.width; i++)
		delete [] field.map.cells[i];
	delete [] field.map.cells;


	/*
	 * creation of facilities/units
	 */  
/* //orzel : not enabled yet
	people  = field.people;  
	for (i=0; i<people.nbMobiles; i++)
	        createMobUnit(people.mobile[i]);
	for (i=0; i<people.nbFacilities; i++)
	        createFixUnit(people.facility[i]);  
*/

	// ok, it's all right
	isModified = false;
	return true;

}

bool editorField::save(QString filename)
{
	int	i, j;
	bool	ret;
	playField field(filename);

	field.nbPlayer = 0;
	field.people.nbMobiles = 0;
	field.people.nbFacilities = 0;
	field.map.width = map.width;
	field.map.height = map.height;

	/* creation of the ground map */
	field.map.cells = new (groundType *)[map.width];
	for (i=0; i< map.width; i++)
		field.map.cells[i] = new (groundType)[map.height];

	/* initialisation */
	for (i=0; i< map.width; i++)
		for (j=0; j< map.height; j++)
			field.map.cells[i][j] = (map.cells[i][j]).getGroundType();

	printf("***********field.save : %d, map : %d\n", field.map.cells[3][3], map.cells[3][3].getGroundType());
	printf("***********field.save : %d, map : %d\n", field.map.cells[10][9], map.cells[10][9].getGroundType());
	printf("***********field.save : %d, map : %d\n", field.map.cells[20][13], map.cells[20][13].getGroundType());
	ret =  field.write();

	/* freeing of field.map.cells */
	for (i=0; i< map.width; i++)
		delete [] field.map.cells[i];
	delete [] field.map.cells;

	// ok, it's all right
	isModified = false;
	return ret;

}
