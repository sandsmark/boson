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

#ifndef BOGL_PRIVATE_H
#define BOGL_PRIVATE_H

#include <qstring.h>
#include <qstringlist.h>

class BoGLPrivate
{
public:
	BoGLPrivate()
	{
	}

	bool mIsInitialized;
	bool mIsResolved;
	QString mOpenGLLibraryFile;
	QString mGLULibraryFile;

	unsigned int mOpenGLVersion;
	QString mOpenGLVersionString;
	QString mOpenGLVendorString;
	QString mOpenGLRendererString;
	QStringList mOpenGLExtensions;
	QString mGLUVersionString;
	QStringList mGLUExtensions;
};

#endif