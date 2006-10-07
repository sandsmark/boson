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
#define NO_MEMORY_MANAGEMENT 0
#if !NO_MEMORY_MANAGEMENT
#include "memorymanager.h"

#include "memnode.h"

#include <qptrdict.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


extern "C" {
void* __libc_malloc(size_t);
void __libc_free(void*);
void* __libc_memalign(size_t, size_t);
void* __libc_calloc(size_t, size_t);
void* __libc_realloc(void*, size_t);
void* __libc_valloc(size_t);
void* __libc_pvalloc(size_t);
//struct mallinfo __libc_mallinfo();
int __libc_mallopt();
//int __libc_mtrim();
//void __libc_mstats();
//size_t __libc_musable(void*);

};


void* bo_malloc(size_t size, const char* file, int line, const char* function)
{
#if !NO_MEMORY_MANAGEMENT
 MemoryManager::createManager();
 return MemoryManager::manager()->bomalloc(size, file, line, function, true);
#else
 return __libc_malloc(size);
#endif
}


void bo_free(void* p)
{
#if !NO_MEMORY_MANAGEMENT
 MemoryManager::manager()->bofree(p, true);
#else
 __libc_free(p);
#endif
}



#undef malloc
#undef calloc
#undef realloc
#undef free

extern "C" {
void* malloc(size_t size)
{
// TODO: GET_RET_ADDR ?
#if !NO_MEMORY_MANAGEMENT
 MemoryManager::createManager();
 void* p = MemoryManager::manager()->bomalloc(size, "unknown", -1, "unknown", true);
#else
 void* p = __libc_malloc(size);
#endif
 return p;
}

void* calloc(size_t num, size_t size)
{
#if !NO_MEMORY_MANAGEMENT
 MemoryManager::createManager();
 void* p = MemoryManager::manager()->bomalloc(size * num, "unknown", -1, "unknown", true);
 memset(p, 0, size * num);
#else
 void* p = __libc_calloc(num, size);
#endif
 return p;
}

void cfree(void* p)
{
 free(p);
}

void free(void* p)
{
#if !NO_MEMORY_MANAGEMENT
 MemoryManager::manager()->bofree(p, true);
#else
 __libc_free(p);
#endif
}

void* realloc(void* old, size_t size)
{
#if NO_MEMORY_MANAGEMENT
 void* p = __libc_realloc(old, size);
 return p;
#else
 size_t old_size = 0;
 if (old == 0) {
	return MemoryManager::manager()->bomalloc(size, "unknown", -1, "unknown", true);
 }
 if (size == 0) {
	MemoryManager::manager()->bofree(old, true);
	return 0;
 }

 void* p = MemoryManager::manager()->bomalloc(size, "unknown", -1, "unknown", true);

 MemNode* node = MemoryManager::manager()->allNodes().find(old);
 if (!node) {
	printf("realloc() error: cannot find previous memory %p - assuming size of 0\n", old);
	old_size = 0;
 } else {
	old_size = node->mSize;
 }
 if (size < old_size) {
	memcpy(p, old, size);
 } else {
	memcpy(p, old, old_size);
 }
 MemoryManager::manager()->bofree(old, true);
 return p;
#endif // NO_MEMORY_MANAGEMENT
}

void* memalign(size_t b, size_t s)
{
 printf("WARNING: memory returned by memalign() is not handled by memory manager\n");
 return __libc_memalign(b, s);
}
void* valloc(size_t s)
{
 printf("WARNING: memory returned by valloc() is not handled by memory manager\n");
 return __libc_valloc(s);
}
void* pvalloc(size_t s)
{
 printf("WARNING: memory returned by pvalloc() is not handled by memory manager\n");
 return __libc_pvalloc(s);
}

/*
struct mallinfo mallinfo()
{
 return __libc_mallinfo();
}
*/
int mallopt()
{
 return __libc_mallopt();
}
/*
int mtrim()
{
 return __libc_mtrim();
}

void mstats()
{
 __libc_mstats();
}
size_t musable(void* m)
{
 return __libc_musable(m);
}*/

}; // extern "C"

