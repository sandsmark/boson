/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann <b_mann@gmx.de>

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

#ifndef MYQLIBRARY_H
#define MYQLIBRARY_H

#include "qstring.h"

/**
 * @internal
 *
 * This class exists because @ref QLibrary insists on calling dlopen with
 * RTLD_LAZY, but we need RTLD_GLOBAL - quoting
 * http://dri.sourceforge.net/doc/DRIuserguide.html:
 * <pre>
 * Specify the RTLD_GLOBAL flag to dlopen(). If you don't do this then you'll likely see a runtime error message complaining that _glapi_Context is undefined when libGL.so tries to open a hardware-specific driver. Without this flag, nested opening of dynamic libraries does not work.
 * </pre>
 *
 * This class is meant to be very simple. We don't even try to reproduce all of
 * QLibrary's features.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class MyQLibrary
{
public:
	MyQLibrary(const QString& file);
	~MyQLibrary();

	bool load();
	bool isLoaded() const;
	QString library() const;
	void* resolve(const char* symbol);

private:
	QString mFile;
	void* mHandle;
};

#endif

