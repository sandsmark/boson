/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOMEMORY_H
#define BOMEMORY_H

#include <sys/types.h>
#include <stdlib.h>

#include <config.h>

#ifdef BOSON_USE_BOMEMORY

/*
 * Warning: this file gets included using "g++ -include", meaning it will always
 * be included in all of our files, but if it changes g++ won't notice that all
 * these files have to be recompiled.
 *
 * You have to do make clean manually then.
 */


#ifdef __PRETTY_FUNCTION__
#define BOMEM_PRETTY_FUNCTION __PRETTY_FUNCITON__
#else
#define BOMEM_PRETTY_FUNCTION ""
#endif /* __PRETTY_FUNCTION__ */

void* operator new(size_t size, const char* file, int line, const char* function);
void* operator new[](size_t size, const char* file, int line, const char* function);
#if 1
#define new new(__FILE__, __LINE__, __PRETTY_FUNCTION__)


#undef calloc
#undef malloc
#undef realloc
#undef free

void* bo_malloc(size_t size, const char* file, int line, const char* function);
void bo_free(void* ptr);

#define malloc(size) bo_malloc((size), __FILE__, __LINE__, BOMEM_PRETTY_FUNCTION)
#define calloc(count, size) bo_malloc((count) * (size), __FILE__, __LINE__, BOMEM_PRETTY_FUNCTION)
#define realloc(ptr, size) bo_malloc((count) * (size), __FILE__, __LINE__, BOMEM_PRETTY_FUNCTION)
#define free(ptr) bo_free(ptr)

#endif

#endif // BOSON_USE_BOMEMORY

#endif

