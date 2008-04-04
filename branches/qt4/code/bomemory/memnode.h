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

#ifndef MEMNODE_H
#define MEMNODE_H

#include <sys/types.h>
#include <stdio.h>

struct MemNode
{
	size_t mSize;
	void* mPointer;
	int mLine;
	const char* mFile; // pointing to static memory (a "" string)
	const char* mFunction; // pointing to static memory (a "" string)
	bool mIsMalloc;
};

MemNode* makeMemNode(MemNode* node, size_t size, void* p, const char* file, int line, const char* function, bool isMalloc);


#endif
