/***************************************************************************
                            log.cpp  -  description                    
                             -------------------                                         

    version              : $Id$
    begin                : Wed Jan 27 16:15:13 CET 1999
                                           
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

#include <stdarg.h>
#include <unistd.h>
#include "log.h"

#define LOG_MIN LOG_INFO

static char *logLevelName[] = {
	"FATAL    ", 
	"ERROR    ", 
	"WARNING  ", 
	"INFO     ", 
	"GAME_HIGH", 
	"GAME_LOW ", 
	"COMM     ", 
	"LAYER2   ", 
	"LAYER1   ",
	"LAYER0   ",
	};

int logf(boLogLevel level, char *fmt, ...)
{
static int id = getpid();
va_list args;
int i;

if (level > LOG_MIN) return 0;

if (!logfile) {
	fprintf(stderr, "\n\n**** Call of logf while logfile not initialized *****\n");
	fprintf(stderr, "logfile set to stderr\n");
	fprintf(stderr, "Level is %s\n", logLevelName[LOG_MIN]);
	logfile = stderr;
	}

if (level < 0) {
	fprintf(logfile, "\n*** logf : negative level, set to FATAL");
	level = LOG_FATAL;
	}

if (level >= LOG_LAST) {
	fprintf(logfile, "\n*** logf : too low level, set to LAYER0");
	level = (boLogLevel)(LOG_LAST-1);
	}

fprintf(logfile, "\n[%5d] %s : ", id, logLevelName[level]);

va_start(args, fmt);
i=vfprintf(logfile, fmt, args);
va_end(args);

fflush(logfile);
return i;
}
