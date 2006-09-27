/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOSAVEGAMECONVERTER_H
#define BOSAVEGAMECONVERTER_H

#include <qstring.h>

template<class T, class T2> class QMap;


/**
 * Converter class from one savegame/playfield format to another. You need this
 * class only if you want to write a new converter. If you want use an existing
 * converter, use @ref BosonPlayFieldConverter instead.
 *
 * To add a new converter:
 * @li Derive a new class from BoSaveGameConverter
 * @li implement the @ref convert method
 * @li implement @ref handlesVersion
 * @li Add the new converter to @ref createConverters
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoSaveGameConverter
{
public:
	BoSaveGameConverter();
	virtual ~BoSaveGameConverter();

	/**
	 * Create all available converters in a map that maps the @ref
	 * handlesVersion to the actual converter.
	 *
	 * Don't forget to delete the objects!
	 **/
	static QMap<int, BoSaveGameConverter*> createConverters();

	/**
	 * @return Which savegame format version this converter handles, i.e. which
	 * version it should receive as input.
	 *
	 * This is meant to be the @em internal version code for savegames, i.e.
	 * it is NOT the release version of Boson.
	 * See also the BOSON_MAKE_SAVEGAME_FORMAT_VERSION macro.
	 **/
	virtual int handlesVersion() const = 0;

	void setHandlesBosonVersionString(const QString& string);

	/**
	 * @return A string describing the (human readable) Boson version this
	 * savegame format belongs to. Usually "0.9", "0.12", and so on.
	 *
	 * This string has informational purposes only and is meant to be
	 * displayed in error messages and debug output.
	 **/
	const QString& handlesBosonVersionString() const;

	bool convertFiles(QMap<QString, QByteArray>& destFiles);

protected:
	/**
	 * Start the conversion.
	 *
	 * @return TRUE on success, FALSE if an error occurred.
	 **/
	virtual bool convert(QMap<QString, QByteArray>& destFiles) = 0;

private:
	static void insertNewConverterToMap(QMap<int, BoSaveGameConverter*>&, BoSaveGameConverter*);

	/**
	 * Initialize @ref BoSaveGameConverter::handlesBosonVersionString with
	 * meaningful information
	 **/
	static void initVersionNames(QMap<int, BoSaveGameConverter*>& converters);

private:
	QString mHandlesBosonVersionString;
};


#endif

