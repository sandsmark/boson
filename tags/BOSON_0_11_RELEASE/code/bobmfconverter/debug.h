/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef DEBUG_H
#define DEBUG_H


#include <sys/time.h>
#include <stdio.h>
extern double starttime;
extern char dbgtimestr[20];
void initdbgtime();
char* dbgtime();

// Wrappers for bo*() debug functions
#include <iostream>
using namespace std;

#define boDebug(n) cerr << dbgtime()
#define boWarning(n) cerr << "WARNING: "
#define boError(n) cerr << "ERROR: "

// KDE macros
#ifdef __GNUC__
#define k_funcinfo "[" << __PRETTY_FUNCTION__ << "] "
#else
#define k_funcinfo "[" << __FILE__ << ":" << __LINE__ << "] "
#endif

#define k_lineinfo "[" << __FILE__ << ":" << __LINE__ << "] "


// 4 macros for boson:

/**
 * Output a NULL pointer error for @p p (including the variable name).
 * Won't check whether p is actually NULL.
 *
 * Usually you will use BO_CHECK_NULL* instead.
 **/
#define BO_NULL_ERROR(p) boError() << k_funcinfo << "NULL pointer: " << #p << endl;

/**
 * Ensure that the pointer p is non-NULL. If it is NULL output an error. will
 * also output the supplied variable name.
 **/
#define BO_CHECK_NULL(p) if (!p) { BO_NULL_ERROR(p) }

/**
 * Just like BO_CHECK_NULL, but will also return the current function (without
 * return value)
 **/
#define BO_CHECK_NULL_RET(p) if (!p) { BO_NULL_ERROR(p) return; }

/**
 * Just like BO_CHECK_NULL, but will also return the current function, return
 * value is 0.
 **/
#define BO_CHECK_NULL_RET0(p) if (!p) { BO_NULL_ERROR(p) return 0; }


#endif // DEBUG_H
