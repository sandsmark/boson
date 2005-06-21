/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bomemorytrace.h"

#include "bodebug.h"
#include "../bomemory/bodummymemory.h"

#include <kstaticdeleter.h>

#include <qmap.h>

#include <stdlib.h>

// set this to 1 to enable startCatchin()  and stopCatching().
// without this these functions are noops.
#define ALLOW_CATCHING 0

BoMemoryTrace* BoMemoryTrace::mMemoryTrace = 0;
static KStaticDeleter<BoMemoryTrace> sd;

static bool _catchAllocAndFree = false;
static long int _catchAllocAndFreeBytes = 0;
static QMap<void*, int> _catchedEntries;

class BoMemoryTracePrivate
{
public:
	BoMemoryTracePrivate()
	{
	}

	unsigned long int mBytes;
	QMap<void*, int> mEntries;
};

BoMemoryTrace::BoMemoryTrace()
{
 d = new BoMemoryTracePrivate;
 d->mBytes = 0;
}

BoMemoryTrace::~BoMemoryTrace()
{
 delete d;
}

void BoMemoryTrace::initStatic()
{
 if (mMemoryTrace) {
	return;
 }
 sd.setObject(mMemoryTrace, new BoMemoryTrace);
}

void BoMemoryTrace::addBytes(int bytes)
{
 d->mBytes += bytes;
}

void BoMemoryTrace::subBytes(int bytes)
{
 d->mBytes -= bytes;
}

void BoMemoryTrace::add(void* p, int bytes)
{
 d->mEntries.insert(p, bytes);
 addBytes(bytes);
}

void BoMemoryTrace::remove(void* p)
{
 if (!p) {
	return;
 }
 if (!d->mEntries.contains(p)) {
	boWarning() << k_funcinfo << "did not find " << p << endl;
	return;
 }
 int bytes = d->mEntries[p];
 subBytes(bytes);
 d->mEntries.remove(p);
}

float* BoMemoryTrace::allocateFloatArray(int size)
{
 float* p = new float[size];
 int bytes = size * 4; // AB: not 100% correct, btw, because of some system dependant alignment. e.g. malloc() and therefore new never returns something smaller than 16 bytes for me. but for allocating an array that small overhead doesn't matter.
 add(p, bytes);
// boDebug() << k_funcinfo << "bytes: " << d->mBytes << endl;
 return p;
}

void BoMemoryTrace::freeFloatArray(float*& p)
{
 if (!p) {
	return;
 }
 remove(p);
 delete[] p;
 p = 0;
// boDebug() << k_funcinfo << "bytes: " << d->mBytes << endl;
}

unsigned int* BoMemoryTrace::allocateUIntArray(int size)
{
 unsigned int* p = new unsigned int[size];
 int bytes = size * 4;
 add(p, bytes);
// boDebug() << k_funcinfo << "bytes: " << d->mBytes << endl;
 return p;
}

void BoMemoryTrace::freeUIntArray(unsigned int*& p)
{
 if (!p) {
	return;
 }
 remove(p);
 delete[] p;
 p = 0;
// boDebug() << k_funcinfo << "bytes: " << d->mBytes << endl;
}

void BoMemoryTrace::startCatching()
{
#if ALLOW_CATCHING
 _catchAllocAndFree = true;
 // from now on *all* new and delete calls will be counted.
#endif
}

int BoMemoryTrace::stopCatching(const QString& text)
{
#if ALLOW_CATCHING
 // no new/delete calls will be counted anymore now.
 _catchAllocAndFree = false;

 int bytes = _catchAllocAndFreeBytes;
 _catchAllocAndFreeBytes = 0;

 boDebug(888) << k_funcinfo << text << ": " << bytes << endl;
 return bytes;
#else
 return 0;
#endif
}


#if ALLOW_CATCHING
void* operator new(size_t t)
{
 void* p = malloc(t);
 if (_catchAllocAndFree) {
	_catchAllocAndFree = false;
	_catchedEntries.insert(p, t);
	_catchAllocAndFree = true;
	_catchAllocAndFreeBytes += t;
 }
 return p;
}

void* operator new[](size_t t)
{
 return operator new(t);
}

void operator delete(void* p)
{
 if (_catchAllocAndFree) {
	int bytes;
	if (_catchedEntries.contains(p)) {
		bytes = _catchedEntries[p];
		_catchAllocAndFree = false;
		_catchedEntries.remove(p);
		_catchAllocAndFree = true;
	} else {
		// we deleted something that was allocated before catching got
		// switched on.
		// no way to find out how many bytes were free'd ?
		bytes = 0;
	}
	_catchAllocAndFreeBytes -= bytes;
 }
 free(p);
}

void operator delete[](void* p)
{
 operator delete(p);
}

#endif


