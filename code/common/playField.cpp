/***************************************************************************
                         playField.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
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

#include <kapp.h>

#include "playField.h"

#include "../common/groundType.h"


#define		TAG_MOB		(0xba)
#define		TAG_FIX		(0xbe)
#define		TAG_CELL	(0xde)
#define		TAG_MAP		(0xad)
#define		TAG_PEOPLE	(0xf0)
#define		TAG_FIELD	"boeditor_magic_0_1"


playField::playField()
{
	//filename is null;
	stream		= 0l;
	map.cells	= 0l;
}


playField::playField(const QString name)
{
	filename	= name;
	stream		= 0l;
	map.cells	= 0l;
}



playField::~playField()
{
//if (map.cells) delete map.cells;
}

void playField::setFile(const QString name)
{
	filename	= name;
}


bool playField::write(void)
{
	QFile f(filename.data());

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
	delete stream;
	return true;
}

bool playField::load()
//bool playField::load(createCellFunc *CCFunc)
{
	char *magic;
	QFile f(filename.data());
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

	is_ok &= loadMap();
//	is_ok &= loadMap(CCFunc);
	is_ok &= loadPeople();

	/* close */
	f.close();
	delete stream;
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

bool	playField::load(groundType &gt)
{
	int g;
	*stream >> g;
	if (TAG_CELL !=g ) return false;
	
	*stream >> g;
	gt = (groundType) g;
	return true;
}

bool	playField::loadMap()
{
	int i,j;
	*stream >> j;
	if (TAG_MAP !=j ) return false;
	
	*stream >> map.width >> map.height;
	
	map.cells = new (groundType *)[map.width];
	for (i=0; i< map.width; i++)
		map.cells[i] = new (groundType)[map.height];
		
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

void	playField::write(groundType &g)
{
	*stream << TAG_CELL;
	*stream << (int)g;
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
