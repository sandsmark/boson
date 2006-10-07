/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONPLAYFIELDCONVERTER_H
#define BOSONPLAYFIELDCONVERTER_H

class BosonMap;
class Boson;
class BPFFile;
class BPFDescription;

class KArchiveDirectory;
class KTar;
class KArchiveFile;
class QDomDocument;
class QDomElement;
class QDataStream;
template<class T, class T2> class QMap;

#include <qstring.h>

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonPlayFieldConverter
{
public:
	BosonPlayFieldConverter();
	~BosonPlayFieldConverter();

	/**
	 * This takes a set of files, as loaded by @ref BosonPlayField::loadFromDiskToFiles, and
	 * checks whether they have the most current file format. If they don't
	 * have, this method tries to convert them.
	 *
	 * @return TRUE if the conversion succeeded (or was not necessary),
	 * FALSE if the files could not be converted to current file format.
	 **/
	bool convertFilesToCurrentFormat(QMap<QString, QByteArray>& destFiles);

protected:

};

#endif
