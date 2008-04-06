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

#include "myqlibrary.h"

#include "bodebug.h"

#include <dlfcn.h>

MyQLibrary::MyQLibrary(const QString& file)
{
 // TODO: maybe do filename guessing here?
 mFile = file;
 mHandle = 0;
}

MyQLibrary::~MyQLibrary()
{
 // note that we do NOT close the library
 // (i.e. we always do a "QLibrary::setAutoUnload(true)")

 // closing libGL.so may cause some problems and thus we simply don't do it.
 // in particular libGL.so must not be closed before the X display has been
 // closed.
}

bool MyQLibrary::isLoaded() const
{
 return (mHandle != 0);
}

bool MyQLibrary::load()
{
 if (isLoaded()) {
	return true;
 }
 // AB: RTLD_GLOBAL is _required_ (mesa in partiucular seems to have problems if
 //     we dont use it).
 //     RTLD_NOW could probably be replaced by RTLD_LAZY, but I think _NOW is at
 //     least as good here for us.
 QByteArray fileTmp = mFile.toLatin1();
 mHandle = dlopen(fileTmp.constData(), RTLD_NOW | RTLD_GLOBAL);

 return isLoaded();
}

QString MyQLibrary::library() const
{
 // TODO: maybe do filename guessing here?
 return mFile;
}

void* MyQLibrary::resolve(const char* symbol)
{
 if (!isLoaded()) {
	return 0;
 }
 void* address = dlsym(mHandle, symbol);
#if 0
 const char* error = dlerror();
 if (error) {
	boError() << k_funcinfo << error << endl;
 }
#endif
 return address;
}

