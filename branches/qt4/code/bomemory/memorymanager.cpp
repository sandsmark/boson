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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "memorymanager.h"
#include "memnode.h"

#include <stdlib.h>
#include <string.h>

#include <qstring.h>
#include <qptrdict.h>

//#define TOO_MUCH_DEBUGGING

#ifndef __GLIBC__
#error __GLIBC__ is not defined - we need glibc (for __libc_malloc)
#endif

#define libc_malloc __libc_malloc
#define libc_free __libc_free
extern "C" {
void* __libc_malloc(size_t);
void __libc_free(void*);
}


// AB: see dmalloc/return.h
#ifdef __i386
#ifdef __GNUC__
#define GET_RET_ADDR(file)      asm("movl 4(%%ebp),%%eax ; movl %%eax,%0" : \
				"=g" (file) : \
				: \
				"eax")
#endif // __GNUC__
#endif // __i386

// testing
#undef GET_RET_ADDR

#ifndef GET_RET_ADDR
#endif // !GET_RET_ADDR

int MemoryManager::mDisabled = 0;
MemoryManager* MemoryManager::mManager= 0;


class MemoryManagerPrivate
{
public:
	MemoryManagerPrivate()
	{
	}

	QPtrDict<MemNode> mMemoryInfo;
	unsigned long int mMemory;
};

MemoryManager::MemoryManager()
{
 mDisabled++;
 d = new MemoryManagerPrivate();
 d->mMemoryInfo.setAutoDelete(true);
 d->mMemoryInfo.resize(106487); // we might require even bigger numbers here
 d->mMemory = 0;


 mDisabled--;
}

MemoryManager::~MemoryManager()
{
 mDisabled++;
 d->mMemoryInfo.clear();
 delete d;
 mDisabled--;
}

void* MemoryManager::bomalloc(size_t size, const char* file, int line, const char* function, bool isMalloc)
{
 if (mDisabled) {
	// the memory manager also allocates memory, but we dont want to track
	// that (to avoid infinite recursion)
#ifdef TOO_MUCH_DEBUGGING
	printf("bomalloc: %d %p line=%d %p %d\n", size, file, line, function, isMalloc ? 1 : 0);
	printf("bomalloc() done (disabled)\n");
#endif
	return libc_malloc(size);
 }
 mDisabled++;
#ifdef TOO_MUCH_DEBUGGING
 printf("bomalloc: %d %p line=%d %p %d\n", size, file, line, function, isMalloc ? 1 : 0);
#endif
 void* p = libc_malloc(size);
 MemNode* node = (MemNode*)libc_malloc(sizeof(struct MemNode));
 makeMemNode(node, size, p, file, line, function, isMalloc);
 d->mMemoryInfo.insert(p, node);
 d->mMemory += size;
 mDisabled--;
#ifdef TOO_MUCH_DEBUGGING
 printf("bomalloc() done\n");
#endif
 return p;
}

void MemoryManager::bofree(void* p, bool isFree)
{
 if (!p) {
	return;
 }
#ifdef TOO_MUCH_DEBUGGING
 printf("bofree()\n");
#endif
 if (mDisabled) {
	libc_free(p);
#ifdef TOO_MUCH_DEBUGGING
	printf("bofree() done\n");
#endif
	return;
 }
 mDisabled++;
 MemNode* node = d->mMemoryInfo.take(p);
 if (!node) {
	printf("ERROR (bofree): deleting memory that is unkown to the MemoryManager: %p\n", p);
	mDisabled --;
 } else {
	d->mMemory -= node->mSize;
	if (node->mIsMalloc != isFree) {
		printf("ERROR (bofree): isMalloc != isFree\n");
	}
 }
 libc_free(node);
 libc_free(p);
 mDisabled--;
#ifdef TOO_MUCH_DEBUGGING
 printf("bofree() done\n");
#endif
}

const QPtrDict<MemNode>& MemoryManager::allNodes() const
{
 return d->mMemoryInfo;
}

unsigned long int MemoryManager::memoryInUse() const
{
 return d->mMemory;
}

void MemoryManager::disable()
{
 mDisabled++;
}

void MemoryManager::enable()
{
 mDisabled--;
 if (mDisabled < 0) {
	printf("ERROR (enable()): invalid use of enable()/disable() !\n");
 }
}


