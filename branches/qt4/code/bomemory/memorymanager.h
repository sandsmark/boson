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
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <sys/types.h>
#include <stdio.h>

template<class T> class QPtrDict;
class MemNode;

class MemoryManagerPrivate;
class MemoryManager
{
public:
	~MemoryManager();

	inline static MemoryManager* manager()
	{
		return mManager;
	}

	static void createManager()
	{
		if (mManager || mDisabled) {
			return;
		}
		mDisabled++;
		mManager = new MemoryManager;
		mDisabled--;
	}

	/**
	 * Do not log memory anymore until @ref enable is called.
	 *
	 * You must call @ref enable exactly as often as you call disable!
	 **/
	void disable();
	void enable();

	void* bomalloc(size_t size, const char* file = "unknown", int line = -1, const char* function = "unkown", bool isMalloc = false);

	void bofree(void* p, bool isFree = false);

	const QPtrDict<MemNode>& allNodes() const;

	unsigned long int memoryInUse() const;

private:
	MemoryManager();

private:
	static MemoryManager* mManager;
	MemoryManagerPrivate* d;
	static int mDisabled;
};

#endif

