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

#include "memorymanager.h"
#include "memnode.h"

#include <bodebug.h>

//#include <stdlib.h>
#include <string.h>
#include <dlfcn.h> // dlopen, dlsym

#include <qstring.h>
#include <qptrdict.h>

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

static void loadMalloc();
static void* (*libc_malloc)(size_t) = 0;
static void (*libc_free)(void*) = 0;

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
 d->mMemoryInfo.clear();
 delete d;
}

void* MemoryManager::bomalloc(size_t size, const char* file, int line, const char* function, bool isMalloc)
{
 if (!libc_malloc) {
	loadMalloc();
 }
 if (mDisabled) {
	// the memory manager also allocates memory, but we dont want to track
	// that (to avoid infinite recursion)
	return libc_malloc(size);
 }
 mDisabled++;
 void* p = libc_malloc(size);
 MemNode* node = (MemNode*)libc_malloc(sizeof(struct MemNode));
 makeMemNode(node, size, p, file, line, function, isMalloc);
 d->mMemoryInfo.insert(p, node);
 d->mMemory += size;
 mDisabled--;
 return p;
}

void MemoryManager::bofree(void* p, bool isFree)
{
 if (!libc_free) {
	loadMalloc();
 }
 if (!p) {
	return;
 }
 if (mDisabled) {
	libc_free(p);
	return;
 }
 mDisabled++;
 MemNode* node = d->mMemoryInfo.take(p);
 if (!node) {
	boError() << k_funcinfo << "deleting memory that is unkown to the MemoryManager: " << p << endl;
	mDisabled --;
 } else {
	d->mMemory -= node->mSize;
	if (node->mIsMalloc != isFree) {
		boError() << k_funcinfo << "isMalloc != isFree" << endl;
	}
 }
 libc_free(node);
 libc_free(p);
 mDisabled--;
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
	boError() << k_funcinfo << "invalid use of enable()/disable() !" << endl;
 }
}


// mostly taken from ccmalloc/wrapper.c
static void loadMalloc()
{
 void* handle = 0;
 const char* malloc_name = "__libc_malloc";
 const char* free_name = "__libc_free";

 /*
 const char* libcname = "libc"; // FIXME: configure check
 handle = dlopen(libcname, RTLD_NOW);
 if (!handle) {
	const char* err = dlerror();
	fprintf(stderr, "Could not open %s. Exit now.\n", libcname);
	fprintf(stderr, err);
	fprintf(stderr, "\n");
	exit(1);
	return;
 }
 */

 libc_malloc = (void*(*)(size_t))dlsym(handle, malloc_name);
 if (!libc_malloc) {
	fprintf(stderr, "Could not find %s symbol. Exit now\n", malloc_name);
	exit(1);
	return;
 }
 libc_free = (void(*)(void*))dlsym(handle, free_name);
 if (!libc_free) {
	fprintf(stderr, "Could not find %s symbol. Exit now\n", free_name);
	exit(1);
	return;
 }
}

