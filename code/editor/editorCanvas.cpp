/***************************************************************************
                          editorCanvas.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Sep  9 01:27:00 CET 1999
                                           
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

#include <stdlib.h>		// random

#include <klocale.h>		// i18n()
#include <kmessagebox.h>	// KMessageBox::information()

#include "common/log.h"
#include "visual/speciesTheme.h"
#include "editorCanvas.h"

editorCanvas::editorCanvas(QPixmap p)
	: visualCanvas(p, 2, 2) // fake values, will be changed by either Load() or New()
{
	mobiles.resize(149);
	facilities.resize(149);
	mobiles.setAutoDelete(TRUE);
	facilities.setAutoDelete(TRUE);   
	key = 253;

	modified = false;
}


bool editorCanvas::Load(QString filename)
{
	uint ii;
	int i,j;
	mobileMsg_t	mob;
	facilityMsg_t	fix;
	cell_t		c;

	freeRessources();

	if (!openRead(filename.data())) return false;

	// QCanvas configuratoin
	resize(map_width, map_height);

	// "who" combobox update
	emit nbPlayerChanged(nbPlayer);
	// Themes
	for (uint i=0; i<nb_player(); i++)
		loadSpecyTheme(i);

	emit syncMini();	// let the miniMap synchronized with the new parameters

	// initialisation
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++) {
			boFile::load( c);
			setCell( i, j, c); // XXX can't this loading be done in visual now ? 
		}
	
	// checking
	for (int i=0; i< 3; i++)
		for (int j=0; j< 3; j++)
			boAssert ( ground (tile(i,j)) );


	logf(LOG_INFO, "loading map, %d mobiles, %d facilities", nbMobiles, nbFacilities);

	for (ii=0; ii<nbMobiles; ii++) {
		boFile::load(mob);
		if (!isOk()) return false;
		createMobUnit(mob);
	}

	for (ii=0; ii<nbFacilities; ii++) {
		boFile::load(fix);
		if (!isOk()) return false;
		createFixUnit(fix);
	}

	// ok, it's all right
	Close();
	modified = false;
	update();
	return isOk();
}


bool editorCanvas::Save(QString filename)
{
	int i,j;
	mobileMsg_t	mob;
	facilityMsg_t	fix;
	QIntDictIterator<visualMobUnit> mobIt(mobiles);
	QIntDictIterator<visualFacility> fixIt(facilities);

	if (nbPlayer<2) {
  		KMessageBox::information(0, i18n("You need at least two players to get a usable scenario"), i18n("Not enough player") );
		logf(LOG_INFO, "tried to save with %d player only", nbPlayer);
	}

	boAssert (nbMobiles == mobiles.count());
	boAssert (nbFacilities == facilities.count());
	// safety
	nbMobiles = mobiles.count();
	nbFacilities = facilities.count();

	if (!openWrite(filename.data())) return false;

	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++)
			write( (cell_t) tile(i,j));
	
	for (mobIt.toFirst(); mobIt; ++mobIt) {
		mobIt.current()->fill(mob);
		boFile::write(mob);
		}

	for (fixIt.toFirst(); fixIt; ++fixIt) {
		fixIt.current()->fill(fix);
		boFile::write(fix);
		}


	// ok, it's all right
	Close();
	modified = false;
	return isOk();
}


bool editorCanvas::New(groundType fill_ground, uint w, uint h, const QString &name)
{
	int i,j;

	freeRessources();

	/* boFile configuration */
	emit nbPlayerChanged(nbPlayer = 0); // nobody at beginning
	map_width = w;
	map_height = h;
	_worldName = name;

	/* QCanvas configuratoin */
	resize(w,h);
	emit syncMini();	// let the miniMap synchronized with the new parameters

	/* initialisation */
	for (i=0; i< map_width; i++)
		for (j=0; j< map_height; j++)
			setCell( i, j, makeCell ( fill_ground, random()%4 ));

	modified = true;
	return true;
}


void editorCanvas::freeRessources()
{
	/* freeing of mobiles */
	mobiles.clear();
	/* freeing of facilities */
	facilities.clear();
	modified = false;
}


void editorCanvas::createMobUnit(mobileMsg_t &msg)
{
	visualMobUnit *m;

	msg.key = key++;

	m = new visualMobUnit(&msg);
	mobiles.insert(msg.key, m);

	modified = true;
//	emit updateMobile(m);
}


void editorCanvas::createFixUnit(facilityMsg_t &msg)
{
	visualFacility *f;

	msg.key = key ++;
	msg.state = CONSTRUCTION_STEPS - 1 ;

	f = new visualFacility(&msg);
	facilities.insert(msg.key, f);

	modified = true;
//	emit updateFix(f);
}


void editorCanvas::destroyMobUnit(int mkey)
{
	mobiles.remove(mkey);
	emit mobileNbUpdated(--nbMobiles);
}


void editorCanvas::destroyFixUnit(int mkey)
{
	facilities.remove(mkey);
	emit facilityNbUpdated(--nbFacilities);
}


void editorCanvas::changeCell(int x, int y, cell_t c)
{
//	printf ("setting %d,%d at %d\n", x, y, g);
	modified = true;
	visualCanvas::setCell(x,y,c);
	if ( IS_BIG_TRANS( ground(c) ) ) {
		c+=4; if (x<maxX-1)
			visualCanvas::setCell(x+1,y,c);
		c+=4; if (y<maxY-1)
			visualCanvas::setCell(x,y+1,c);
		c+=4; if (x<maxX-1 && y<maxY-1)
			visualCanvas::setCell(x+1,y+1,c);
	}
}


void editorCanvas::addPlayer(void)
{
	boAssert(nbPlayer<BOSON_MAX_PLAYERS-1);
	loadSpecyTheme(nbPlayer++);
	emit nbPlayerChanged(nbPlayer);
}


void editorCanvas::loadSpecyTheme(uint i)
{
	if (species[i]) return;	// already there

	logf(LOG_INFO, "Loading speciesTheme[%d]", i);
	species[i]	= new speciesTheme("human");
	/* XXX todo: test if the theme has been loaded
		if (!species[1]->isOk()) KMsgBox::message(0l,
			i18n("Pixmap loading error"),
			i18n("Error while loading \"blue\" specie theme,\nsome images will show up awfully"),
			KMsgBox::EXCLAMATION);
	*/
}


