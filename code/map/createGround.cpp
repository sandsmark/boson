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

#include <stdlib.h>	// exit()
#include <qimage.h>
#include <qstring.h>

#include "common/log.h"
#include "common/bomap.h"

QImage		theBig( BIG_W*BO_TILE_SIZE, BIG_H*BO_TILE_SIZE, 32);
QString		themePath;
QString		transName[TRANS_LAST];
FILE		*logfile = (FILE *) 0L;	// log.h
bool		_debug = false;

void begin (const char *themeName)
{
	int i ; 
	QString transS;

	themePath	= "/opt/kde2/share/apps/boson/themes/grounds/";
	themePath	+= themeName;
	themePath	+= "/";


	for (i=0; i< TRANS_LAST; i++) { // pre-load transitions name
		transS	 = groundProp[groundTransProp[i].from].name;
		transS	+= "_";
		transS	+= groundProp[groundTransProp[i].to].name;
		transName[i] = themePath +  transS + "/" + transS;
	}

	theBig.fill( 0x00000000); // black filling, FOW _is_ black
}

void end(void)
{
	theBig.save("earth.png", "PNG");
}

void putOne(int z, QImage &p, int xoffset=0, int yoffset=0)
{
	int x = GET_BIG_X(z), y = GET_BIG_Y(z);

#if 0
	printf("\nputOne(%d), on %d,%d (%d,%d)", z, x, y, x/BO_TILE_SIZE, y/BO_TILE_SIZE);
	if (xoffset || yoffset)
		printf(", offset : %d,%d", xoffset, yoffset);
#endif

	if (_debug) { // draw a rectangle on top of that
		int i;
		for(i=0; i< BO_TILE_SIZE; i++) {
			p.setPixel( xoffset+ 0,			yoffset + i, 0x00ff0000 );
			p.setPixel( xoffset+ BO_TILE_SIZE-1,	yoffset + i, 0x00ff0000 );
			p.setPixel( xoffset+ i,			yoffset + 0, 0x00ff0000 );
			p.setPixel( xoffset+ i,			yoffset + BO_TILE_SIZE-1, 0x00ff0000 );
		}
	}

	bitBlt(&theBig, x, y, &p, xoffset, yoffset, BO_TILE_SIZE, BO_TILE_SIZE);
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
		if (IS_BIG_TRANS(j)) {
			putOne( 4*(j+1)+i, p, BO_TILE_SIZE, 0);
			putOne( 4*(j+2)+i, p, 0, BO_TILE_SIZE);
			putOne( 4*(j+3)+i, p, BO_TILE_SIZE, BO_TILE_SIZE);
		}
	}
}

void loadTransition(groundType gt)
{
	int ref = GET_TRANS_REF(gt);
	int t, tile;

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

	t = GET_TRANS_TILE(gt);
	if (t<SMALL_TILES_PER_TRANSITION)
		tile = t;
	else {
		t-= SMALL_TILES_PER_TRANSITION;		// bigtile #
		t/=4;					// which one
		tile = t+SMALL_TILES_PER_TRANSITION;	// tile is the index into trans_ext
	}

	loadGround(gt, transName[ref] + trans_ext[tile]);
}


int main (int argc, char **argv)
{

	int i, j;

	stderr = stdout;
	/* logfile handling */
	logfile = stdout;
	logf(LOG_ERROR, "logfile : using stderr");
	logf(LOG_INFO, "========= New Log File ==============");


	if (argc>2) {
		puts("usage: createGround [-d]\n-d is for a debugging earth.png");
		exit(1);
	}
	if (argc>1)
		if ( !strncmp(argv[1], "-d", 2)) {
			logf(LOG_INFO, "DEBUG MODE ON");
			_debug = true;
		} else {
			puts("\nunrecognised optoin\n\nusage: createGround [-d]\n-d is for a debugging earth.png");
			exit(1);
		}
	begin("earth");
	for (i=0; i< GROUND_LAST; i++) // load non-transitions
		loadGround(i, themePath + groundProp[i].name);
		;
	for (i=0; i< TRANS_LAST; i++) {
		for (j=0; j< SMALL_TILES_PER_TRANSITION; j++)
			loadTransition( (groundType) GET_TRANS_NUMBER(i,j) );
			;
		for ( ; j< TILES_PER_TRANSITION; j+=4)
			loadTransition( (groundType) GET_TRANS_NUMBER(i,j) );
	}

	end();

	printf("\n");
	return 0;
}
