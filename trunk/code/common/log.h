/***************************************************************************
                          log.h  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Sat Apr 17 23:03:00 CET 1999
                                           
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

#ifndef LOG_H
#define LOG_H

#include <stdio.h>

/** This enum is needed for the boson log system */
enum boLogLevel {
	LOG_FATAL,	/* preventing the game to execute */
	LOG_ERROR,	/* bug : should be corrected */
	LOG_WARNING,	/* unimportant problems */
	LOG_INFO,	/* Info concerning the execution */
	LOG_GAME_HIGH,	/* games message high logs (creation, destroy...)*/
	LOG_GAME_LOW,	/* games message low logs (synchro, money, moves, discovered..) */
	LOG_COMM,	/* Communication logs */
	LOG_LAYER2,	/* Layer 2 comm logs */
	LOG_LAYER1,	/* Layer 1 comm logs */
	LOG_LAYER0,	/* send/recv in common */
	LOG_LAST	/* last, unused */
	};

extern FILE *logfile;

#ifdef NDEBUG
#define boAssert(a)	do {} while(0);
#define boCheck(a,b)	do {} while(0);
#else

#define boAssert(a)	\
	if (!(a))	\
	logf(LOG_WARNING, "Assertion failed file %s, line %d", __FILE__, __LINE__)

#define boCheck(a,b)	\
	if ((a)!=(b))	\
	logf(LOG_WARNING, "Assertion failed file %s, line %d, \"%d\" is not \"%d\"", __FILE__, __LINE__, (int)(a), (int)(b))

#endif

int logf(boLogLevel level, const char *fmt, ...);

#endif /* LOG_H */
