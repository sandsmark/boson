/***************************************************************************
                          config.h  -  description                    
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

#ifndef BOCONFIG_H 
#define BOCONFIG_H 

#define BOSON_VERSION_MAJOR	0
#define BOSON_VERSION_MINOR	3
#define BOSON_VERSION_PATCH	0
#define BOSON_VERSION_TEXT	"0.3.0"
#define BOSON_MAGIC		"Orzel"
#define BOSON_VERSION_MAGIC	"boson/orzel@eagle, 1999"
#define BOSON_MAGIC_LENGHT	(sizeof(BOSON_MAGIC)/sizeof(char))

#define BOSON_MAX_CONNECTION	(5)
#define BOSON_MAX_PLAYERS	BOSON_MAX_CONNECTION ///orzel : to clean up
#define BOSON_DEFAULT_PORT	(5454)
#define BOSON_DEFAULT_PORT_CHAR	"5454"

#define BOSON_SERVER_LAUNCHED	"Boson server launched"

#ifdef EAGLE
#define BOSON_LOGFILE_SERVER	"/tmp/boson-server.log"
#define BOSON_LOGFILE_CLIENT	"/tmp/boson-client.log"
#define BOSON_LOGFILE_EDITOR	"/tmp/boson-editor.log"
#else
#define BOSON_LOGFILE_SERVER	"boson-server.log"
#define BOSON_LOGFILE_CLIENT	"boson-client.log"
#define BOSON_LOGFILE_EDITOR	"boson-editor.log"
#endif

#endif // BOCONFIG_H
