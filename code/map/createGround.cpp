/***************************************************************************
                          createGround.cpp  -  description                              
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

#include <qimage.h>
#include <qstring.h>

#include "common/groundType.h"
#include "common/log.h"
#include "common/map.h"

#define		BIG_W	28
#define		BIG_H	24


QImage		theBig( BIG_W*BO_TILE_SIZE, BIG_H*BO_TILE_SIZE, 32);
QString		themePath;
QString		transName[TRANS_LAST];
FILE		*logfile = (FILE *) 0L;	// log.h

void begin (const char *themeName)
{
	int i ; 
	QString transS;

	themePath	= "/usr/share/apps//boson.qt2//themes/grounds/earth/";

	for (i=0; i< TRANS_LAST; i++) { // pre-load transitions name
		transS	 = groundProp[groundTransProp[i].from].name;
		transS	+= "_";
		transS	+= groundProp[groundTransProp[i].to].name;
		transName[i] = themePath +  transS + "/" + transS;
	}

	theBig.fill( 0x00ff0000); // red filling
}

void end(void)
{
	theBig.save("earth.png", "PNG");
}

void putOne(int z, QImage &p)
{
	int x = (z % BIG_W) * BO_TILE_SIZE;
	int y = (z / BIG_W) * BO_TILE_SIZE;
	int i,j;

	printf("\nputOne(%d), on %d,%d (%d,%d)", z, x, y, x/BO_TILE_SIZE, y/BO_TILE_SIZE);

	for(i=0; i< BO_TILE_SIZE; i++)
		for(j=0; j< BO_TILE_SIZE; j++) {
			theBig.setPixel( x+i, y+j, p.pixel(i,j) );
		}
}

void loadGround(int j, const QString &path)
{
	QString _path;
	QImage	p;
	int i;

	for (i=0; i<4; i++) {
		_path.sprintf(".%.2d.bmp",i);
		p.load(path+_path);

//		QString toto = path+_path;
//		printf("\ntrying to load %s", (char*)(toto.latin1()));

		if (p.isNull()) {
			logf(LOG_ERROR, "createGround : Can't load %s.%.2d.bmp ...", (const char *)path, i);
			return;
		}
		putOne(4*j+i, p);
	}
}

void loadTransition(groundType gt)
{
	int ref = GET_TRANS_REF(gt);
	int tile = GET_TRANS_TILE(gt);

	static const char *trans_ext[TILES_PER_TRANSITION] = {
		".01", ".03", ".07", ".05",	// 48x48 transitions
		".02", ".06", ".08", ".04",
		".09", ".10", ".12", ".11",
		".13", ".14", ".15", ".16",	// 96x96 transitions
		".17", ".18", ".19", ".20",
		".21", ".22", ".23", ".24",
		".25", ".26", ".27", ".28",
		};

	boAssert(IS_TRANS(gt));
//	printf("ref=%d, tile=%d", ref, tile); fflush(stdout);
	loadGround(gt, transName[ref] + trans_ext[tile]);
}


int main (int argc, char **argv)
{

	int i;

	/* logfile handling */
	logfile = stderr;
	logf(LOG_ERROR, "logfile : using stderr");
	logf(LOG_INFO, "========= New Log File ==============");

	begin("earth");

	for (i=0; i< GROUND_LAST; i++) // load non-transitions
		loadGround(i, themePath + groundProp[i].name);
	for (; i< NB_GROUND_TILES ; i++) // load non-transitions
		loadTransition( (groundType) i);
		

	end();
	printf("\n");
	return 0;
}
