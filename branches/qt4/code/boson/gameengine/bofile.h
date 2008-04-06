/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BPFFILE_H
#define BPFFILE_H

#include <qstring.h>

class KTar;
class KArchiveDirectory;
template<class T1, class T2> class QMap;

/**
 * This is the base class for BPFFile. It acts as an interface
 * to @ref KTar which allows loading from a .tar.gz file.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoFile
{
public:
	BoFile(const QString& file, bool readOnly);
	virtual ~BoFile();

	/**
	 * Ensure that the .bpf file is valid. This mainly checks whether the
	 * file exists and can be accessed. Derived classes should check whether
	 * required files (data) are in the tar.
	 **/
	virtual bool checkTar() const;

	/**
	 * @return The filename of the archive
	 **/
	QString fileName() const;

	/**
	 * @return The contents of the specified file.
	 * @param subdir You can specify a subdir (e.g. "C" or "de") here. Even
	 * sub-subdirs (e.g. "de/foobar") are allowed.
	 **/
	QByteArray fileData(const QString& fileName, const QString& subdir = QString::null) const;

	/**
	 * @return Whether the archive has a file named @p fileName. Of @þ
	 * subdir is not empty this will search in @þ subdir, otherwise in the
	 * @ref topLevelDir.
	 **/
	bool hasFile(const QString& fileName, const QString& subdir = QString::null) const
	{
		return hasEntry(fileName, subdir, true);
	}

	/**
	 * @return Whether the archive has a directory named @p dirName. Note
	 * that subdirs (e.g. "de/foobar") are not allowed.
	 **/
	bool hasDirectory(const QString& dirName) const
	{
		return hasEntry(dirName, QString::null, false);
	}

	/**
	 * Write @p data into The archive, with file name @p fileName. If @p
	 * subdir is non-empty the file will end up in the specified
	 * subdirectory.
	 **/
	bool writeFile(const QString& fileName, const QByteArray& data, const QString& subdir = QString::null);

	/**
	 * @overload
	 **/
	bool writeFile(const QString& fileName, const QString& data, const QString& subdir = QString::null);

protected:
	/**
	 * @return The toplevel dir. That is the directory the data will get
	 * extracted to if you do something like tar xzvf on this archive.
	 **/
	const KArchiveDirectory* topLevelDir() const;
	bool hasEntry(const QString& fileName, const QString& subdir = QString::null, bool isFile = true) const;

	/**
	 * @param topLevelDir Mandatory. This is the unique toplevel directory
	 * name of the archive. The toplevel dir is the dir where all data would
	 * get extracted to if you did a "tar xzvf" on the archive.
	 **/
	bool writeFile(const QString& topLevelDir, const QString& fileName, qint64 size, const char* data, const QString& subdir = QString::null);

	/**
	 * @return The desired (!) name of the top dir, as it should get used
	 * in @ref writeFile. This doesn't give usable values if you are in
	 * readOnly mode.
	 **/
	QString topDirName() const;

private:
	KTar* mTar;
};

/**
 * @short A BosonPlayField (.bpf) File.
 *
 * Note: the class @ref BosonPlayField does @em not represent the entire .bpf
 * file, the common name has historic reasons only. The .bpf file also includes
 * player data, unit data, scripts, ... that are part of the game, but not of
 * @ref BosonPlayField.
 *
 * UPDATE: we use this for .bsg files as well now. Both share the same file
 * format since boson 0.9
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BPFFile : public BoFile
{
public:
	BPFFile(const QString& file, bool readOnly);
	~BPFFile();

	/**
	 * Ensure that the .bpf file is valid, e.g. contains a map.xml file, ...
	 **/
	virtual bool checkTar() const;

	/**
	 * @return Whether the "map" directory is present. In older boson
	 * versions (<= 0.8) it was not present. From boson 0.9 on all map
	 * relevant files are in a map directory.
	 **/
	bool hasMapDirectory() const
	{
		return hasDirectory(QString::fromLatin1("map"));
	}

	/**
	 * @obsolete
	 * @return The content of the map file. This is obsolete - the map file
	 * as used in boson 0.8 got split into several files (see @ref
	 * BosonFileConverter). The map file as used in 0.8.128 is now stored as
	 * XML - see @ref mapXMLData.
	 **/
	QByteArray mapData() const
	{
		return fileData(QString::fromLatin1("map"));
	}

	/**
	 * @return The content of the map/map.xml file. Note that this will
	 * always be in the map/ subdir, if present at all.
	 **/
	QByteArray mapXMLData() const
	{
		if (!hasMapDirectory()) {
			return QByteArray();
		}
		return fileData(QString::fromLatin1("map.xml"), QString::fromLatin1("map"));
	}

	/**
	 * @return The content of the map/water.xml file. Note that this will
	 * always be in the map/ subdir, if present at all.
	 **/
	QByteArray waterXMLData() const
	{
		if (!hasMapDirectory()) {
			return QByteArray();
		}
		return fileData(QString::fromLatin1("water.xml"), QString::fromLatin1("map"));
	}

	/**
	 * @return The heightmap, if present. This may either be in the toplevel
	 * directory (boson < 0.9) or in the map directory, depending on @ref
	 * hasMapDirectory.
	 **/
	QByteArray heightMapData() const
	{
		QString dir = QString::null;
		if (hasMapDirectory()) {
			dir = QString::fromLatin1("map");
		}
		return fileData(QString::fromLatin1("heightmap.png"), dir);
	}

	/**
	 * @return The content of the binary tex map file. This is
	 * <em>not</em> an image! This file specifies which corner of a cell will
	 * have how much percent of which texture. The file will be in the
	 * toplevel directory for boson < 0.9, otherwise in the map/ directory
	 * (see @ref hasMapDirectory)
	 **/
	QByteArray texMapData() const
	{
		QString dir = QString::null;
		if (hasMapDirectory()) {
			dir = QString::fromLatin1("map");
		}
		return fileData(QString::fromLatin1("texmap"), dir);
	}

	/**
	 * @return The content of the scenario.xml file
	 *
	 * @obsolete. Has been replaced by @ref canvasData and @ref playersData
	 **/
	QByteArray scenarioData() const
	{
		return fileData(QString::fromLatin1("scenario.xml"));
	}

	/**
	 * @return The content of the canvas.xml file
	 **/
	QByteArray canvasData() const
	{
		return fileData(QString::fromLatin1("canvas.xml"));
	}

	/**
	 * @return The content of the players.xml file
	 **/
	QByteArray playersData() const
	{
		return fileData(QString::fromLatin1("players.xml"));
	}
	QByteArray kgameData() const
	{
		return fileData(QString::fromLatin1("kgame.xml"));
	}
	QByteArray externalData() const
	{
		return fileData(QString::fromLatin1("external.xml"));
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
	 * @return The contents of all description.xml files of all locale
	 * directories. The key of the map is the filename including the
	 * directory, e.g. "C/description.xml" for the default locale.
	 **/
	QMap<QString, QByteArray> descriptionsData() const;

	/**
	 * @return The contents of all files in the scripts directory and all
	 * subdirectories.
	 **/
	QMap<QString, QByteArray> scriptsData() const;

	/**
	 * @return The contents of all files in the eventlistener directory and all
	 * subdirectories.
	 **/
	QMap<QString, QByteArray> eventListenerData() const;

	static QString fileNameToIdentifier(const QString& fileName);

private:
	QString mIdentifier;
};

#endif

