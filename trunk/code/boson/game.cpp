/***************************************************************************
                         game.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Wen Apr 14 20:22:00 CET 1999
                                           
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


#include "game.h"

/*
 * visual/visual.h
 */
groundTheme		*ground = 0;
speciesTheme		*species[BOSON_MAX_PLAYERS] = {0l, 0l};
int			nb_player;

/*
 * boson/game.h
 */
speciesTheme		*myspecy	= 0l;
int			who_am_i	= 0;
/* The map which handle grouds and units*/
visualField		*vfield;
bosonField		*field		= 0l;
/* synchronization */
uint			jiffies		= 0l;
/* deal with the communication layer */
KSocket			*Socket		= 0l;
playerSocketState	socketState	= PSS_INIT;
playerState		State		= PS_INIT;
boBuffer		*buffer		= 0l;

/* possesions */
uint			oil		= 0;
uint			mineral		= 0;

