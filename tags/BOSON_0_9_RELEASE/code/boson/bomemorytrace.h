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
#ifndef BOMEMORYTRACE_H
#define BOMEMORYTRACE_H

#define boMem BoMemoryTrace::memoryTrace()

class QString;

class BoMemoryTracePrivate;
class BoMemoryTrace
{
public:
	BoMemoryTrace();
	~BoMemoryTrace();

	static void initStatic();
	static BoMemoryTrace* memoryTrace() { initStatic(); return mMemoryTrace; }

	float* allocateFloatArray(int size);
	unsigned int* allocateUIntArray(int size);

	/**
	 * delete[]s the array and sets the pointer to NULL
	 **/
	void freeFloatArray(float*& p);
	void freeUIntArray(unsigned int*& p);

	void addBytes(int bytes);
	void subBytes(int bytes);

	void startCatching();
	int stopCatching(const QString& text);

protected:
	void add(void* p, int bytes);
	void remove(void* p);

private:
	static BoMemoryTrace* mMemoryTrace;

private:
	BoMemoryTracePrivate* d;
};

#endif
