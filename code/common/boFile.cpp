/***************************************************************************
                         boFile.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Feb 14 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#include <qdatastream.h>
#include <qfile.h>

#include "common/log.h"
#include "common/msgData.h"
#include "boFile.h"


#define		TAG_MOB		(0xba)
#define		TAG_FIX		(0xbe)
#define		TAG_CELL	(0xde)
#define		TAG_MAP		(0xad)
#define		TAG_PEOPLE	(0xf0)
#define		TAG_FIELD	"boeditor_magic_0_3"
#define		TAG_FIELD_LEN	18			// len of the above


boFile::boFile()
{
	BFstate		= None;
	error		= false;
}


bool boFile::openRead(const char *filename)
{
	char magic[ TAG_FIELD_LEN+4 ];	// paranoiac spaces
	int	i;

	/* open  stream */
	f = new QFile(filename);
	if (!f->open(IO_ReadOnly)){
		logf(LOG_ERROR, "boFile : Can't open file \"%s\"", filename);
		return false;
		}
	stream = new QDataStream(f);


	/* magic */
	// Qt marshalling for a string is 4-byte-len + data
	*stream >> i;
	if ( TAG_FIELD_LEN+1 != i ) {
		logf(LOG_ERROR, "boFile : Magic doesn't match(len), check file name");
		return false;
	}

	for (i=0; i< TAG_FIELD_LEN+1 ; i++) { 
		Q_INT8	b;
		*stream >> b;
		magic[i] = b;
	}

	if (strncmp(magic, TAG_FIELD, TAG_FIELD_LEN) ) {
		logf(LOG_ERROR, "boFile : Magic doesn't match(string), check file name");
		return false;
		}
	

	/* read  Header */
	*stream >> nbPlayer;
	*stream >> map_width;
	*stream >> map_height;
	*stream >> nbMobiles;
	*stream >> nbFacilities;
	*stream >> _worldName;

	/*check 'realityness' */
	boAssert (nbPlayer	> 1 );
	boAssert (map_width	> 10 );
	boAssert (map_height	> 10 );
	boAssert (nbMobiles	> 1 );
	boAssert (nbFacilities	> 1 );

	boAssert (nbPlayer	< 4 );
	boAssert (map_width	< 500 );
	boAssert (map_height	< 500 );
	boAssert (nbMobiles	< 50 );
	boAssert (nbFacilities	< 50 );

	BFstate = Read;
	error = false;
	return true;

}

bool boFile::openWrite(const char *filename)
{

	/* open  stream */
	f = new QFile(filename);
	if (!f->open(IO_WriteOnly)){
		puts("boFile : Can't open file");
		return false;
		}
	stream = new QDataStream(f);

	/* magic */
	*stream << TAG_FIELD;

	/* dump data */
	*stream << nbPlayer;
	*stream << map_width;
	*stream << map_height;
	*stream << nbMobiles;
	*stream << nbFacilities;
	*stream << _worldName;

	BFstate = Write;
	error = false;
	return true;

}



bool boFile::Close()
{
	/* close */
	f->close();
	delete f;
	delete stream;
	return true;
}


/*
 * Here are some tools..
 */

/** load ***/

void	boFile::load(mobileMsg_t &m)
{
	int j;

	stateAssert(Read);

	*stream >> j;
	if (TAG_MOB !=j ) {
		error = true;
		logf(LOG_ERROR, "TAG_MOB missing");
		return;
	}
	
	*stream >> j >> m.x >> m.y >> m.who;
	m.type  = (mobType)j ;
	return;
}

void	boFile::load(facilityMsg_t &f)
{
	int j;

	stateAssert(Read);

	*stream >> j;
	if (TAG_FIX !=j ) {
		error = true;
		logf(LOG_ERROR, "TAG_FIX missing");
		return;
	}
	
	*stream >> j >> f.x >> f.y >> f.who;
	f.type  = (facilityType)j ;
	return ;
}

void boFile::load(cell_t &c)
{
	int g;
	byte b;

	stateAssert(Read);

	*stream >> g;
	if (TAG_CELL !=g ) {
		error = true;
		logf(LOG_ERROR, "TAG_CELL missing");
		return;
	}
	
	*stream >> g;
	boAssert( IS_VALID_GROUND(g));
	
	*stream >> b;
	boAssert( b<4);

	c = makeCell( (groundType)g, b);
}

/** write ***/

void boFile::write(mobileMsg_t &m)
{
	stateAssert(Write);

	*stream << TAG_MOB;
	*stream << (int)m.type << m.x << m.y << m.who;
}

void boFile::write(facilityMsg_t &f)
{
	stateAssert(Write);

	*stream << TAG_FIX;
	*stream << (int)f.type << f.x << f.y << f.who;
}

void boFile::write(cell_t c)
{
	stateAssert(Write);

	*stream << TAG_CELL;
	*stream << (int) ground(c);
	*stream << (byte) version(c);
}

