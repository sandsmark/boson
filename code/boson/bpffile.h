/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BPFFILE_H
#define BPFFILE_H

#include <ktar.h>

/**
 * A BosonPlayField File. This class is the interface to @ref KTar, which allows
 * loading from a .tar.gz file.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BPFFile : public KTar
{
public:
	BPFFile(const QString& file, bool readOnly);
	~BPFFile();

	/**
	 * Ensure that the .bpf file is valid, e.g. contains a map.xml file, ...
	 **/
	bool checkTar() const;

	/**
	 * @return The content of a toplevel file. Doesn't work with
	 * subdirectories (i.e. C/description.xml doesn't work whereas map.xml
	 * does)
	 **/
	QByteArray fileData(const QString& fileName) const;

	/**
	 * @return The content of the map.xml file
	 **/
	QByteArray mapData() const
	{
		return fileData(QString::fromLatin1("map.xml"));
	}

	/**
	 * @return The content of the scenario.xml file
	 **/
	QByteArray scenarioData() const
	{
		return fileData(QString::fromLatin1("scenario.xml"));
	}

	/**
	 * @return An identifier (the filename) for this file. You should use
	 * this in boson to identify a map.
	 **/
	QString identifier() const
	{
		// ab we use the filename as identifier. This will cause trouble
		// if one player edited/renamed/replaced a map. but will save a
		// lot of trouble otherwise.
		return mIdentifier; // we can't use fileName(), cause it is not const :(
	}

	/**
	 * @return The content of the description.xml file of the locale
	 * directory.
	 **/
	QByteArray descriptionData() const;

private:
	QString mIdentifier;
};

#endif

