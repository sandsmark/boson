/***************************************************************************
                         playField.cpp  -  description                              
                             -------------------                                         

    version              :                                   
    begin                : Sat Feb 14 1999
                                           
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
#include <qdatastream.h>
#include <qfile.h>
#include "playField.h"

#include "../common/groundType.h"
//#include "../common/cell.h"
#include "serverCell.h"



#define		TAG_MOB		01
#define		TAG_FIX		02
#define		TAG_CELL	03
#define		TAG_MAP		04
#define		TAG_PEOPLE	05
#define		TAG_FIELD	"boeditor_magic_0_1"


playField::playField(const char *name)
{

filename = name;

stream = 0L;

}

bool playField::write(void)
{
	QFile f(filename);

	/* open  stream */
	if (!f.open(IO_WriteOnly)){
		puts("playField : Can't open file");
		return false;
		}
	stream = new QDataStream(&f);

	/* magic */
	*stream << TAG_FIELD;

	/* dump data */

	*stream << nbPlayer;
	writeMap();
	writePeople();
	
	/* close */
	f.close();
	return true;
}

bool playField::load(void)
{
	char *magic;
	QFile f(filename);
	bool is_ok = true;

	/* open  stream */
	f.open(IO_ReadOnly);
	stream = new QDataStream(&f);


	/* magic */
	*stream >> magic;

	if (strcmp(magic, TAG_FIELD)) {
		puts("Magic doesn't match, check file name");
		is_ok = false;
		}
	delete magic;

	/* read  data */

	*stream >> nbPlayer;

//	if (!loadMap()) return false;
//	if (!loadPeople()) return false;
	is_ok &= loadMap();
	is_ok &= loadPeople();

	/* close */
	f.close();
	return is_ok;
}



/*
 * Here are some tools..
 */

/** load ***/

bool	playField::load(origMobile &o)
{
	int j;
	*stream >> j;
	if (TAG_MOB !=j ) return false;
	
	*stream >> j >> o.x >> o.y >> o.who;
	o.t  = (mobType)j ;
	return true;
}

bool	playField::load(origFacility &o)
{
	int j;
	*stream >> j;
	if (TAG_FIX !=j ) return false;
	
	*stream >> j >> o.x >> o.y >> o.who;
	o.t  = (facilityType)j ;
	return true;
}

bool	playField::load(serverCell  &c)
{
	int g;
	*stream >> g;
	if (TAG_CELL !=g ) return false;
	
	*stream >> g;
	c.setGroundType((groundType) g);
	return true;
}

bool	playField::loadMap()
{
	int i,j;
	*stream >> j;
	if (TAG_MAP !=j ) return false;
	
	*stream >> map.width >> map.height;
	
	map.cells = new (serverCell *)[map.width];
	for (i=0; i< map.width; i++)
		map.cells[i] = new (serverCell)[map.height];
		
	for (i=0; i< map.width; i++)
		for (j=0; j< map.height; j++)
			if (!load(map.cells[i][j])) return false;
	return true;
}

bool playField::loadPeople()
{
	int i;
	*stream >> i;
	if (TAG_PEOPLE != i) return false;

	*stream >> people.nbMobiles;
	*stream >> people.nbFacilities;
	assert(people.nbMobiles>0);
	assert(people.nbMobiles<20);
	assert(people.nbFacilities>0);
	assert(people.nbFacilities<20);

	people.mobile = new origMobile[people.nbMobiles];
	people.facility = new origFacility[people.nbFacilities];

	for (i=0; i<people.nbMobiles; i++)
		if (!load(people.mobile[i])) return false;
	for (i=0; i<people.nbFacilities; i++)
		if (!load(people.facility[i])) return false;
	
	return true;
}

/** write ***/

void	playField::write(origMobile &o)
{
	*stream << TAG_MOB;
	*stream << (int)o.t << o.x << o.y << o.who;
}

void	playField::write(origFacility &o)
{
	*stream << TAG_FIX;
	*stream << (int)o.t << o.x << o.y << o.who;
}

void	playField::write(serverCell  &c)
{
	*stream << TAG_CELL;
	*stream << (int)c.getGroundType();
}

void	playField::writeMap()
{
	int i,j;
	*stream << TAG_MAP;

	*stream << map.width << map.height;

	for (i=0; i< map.width; i++)
		for (j=0; j< map.width; j++)
			write(map.cells[i][j]);
}


void playField::writePeople()
{
	int i;
	*stream << TAG_PEOPLE;

	*stream << people.nbMobiles;
	*stream << people.nbFacilities;

	for (i=0; i<people.nbMobiles; i++)
		write(people.mobile[i]);
	for (i=0; i<people.nbFacilities; i++)
		write(people.facility[i]);
}
