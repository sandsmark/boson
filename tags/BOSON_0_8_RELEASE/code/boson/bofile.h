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

#include <qstring.h>

class KTar;
class KArchiveDirectory;

/**
 * This is the base class for @ref BSGFile and BPFFile. It acts as an interface
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
	 * @return The content of a toplevel file. Doesn't work with
	 * subdirectories (i.e. C/description.xml doesn't work whereas map.xml
	 * does) (AB: obsolete!)
	 * @param subdir You can specify a subdir (e.g. "C" or "de") here. Note
	 * that only a single subdir is supported ("de/foobar" won't work)
	 * currently.
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
	bool writeFile(const QString& topLevelDir, const QString& fileName, int size, const char* data, const QString& subdir = QString::null);

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
	 * @return The content of the map file
	 **/
	QByteArray mapData() const
	{
		return fileData(QString::fromLatin1("map"));
	}

	/**
	 * @return The content of the map.xml file. Note that this file is
	 * obsolete! Use @ref mapData instead
	 **/
	QByteArray mapXMLData() const
	{
		return fileData(QString::fromLatin1("map.xml"));
	}

	QByteArray heightMapData() const
	{
		return fileData(QString::fromLatin1("heightmap.png"));
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

	static QString fileNameToIdentifier(const QString& fileName);

private:
	QString mIdentifier;
};

/**
 * @short A Boson SaveGame (.bsg) File.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BSGFile : public BoFile
{
public:
	BSGFile(const QString& file, bool readOnly);
	~BSGFile();

	virtual bool checkTar() const;

	QByteArray mapData() const
	{
		return fileData(QString::fromLatin1("map"));
	}
	QByteArray kgameData() const
	{
		return fileData(QString::fromLatin1("kgame.xml"));
	}
	QByteArray playersData() const
	{
		return fileData(QString::fromLatin1("players.xml"));
	}
	QByteArray canvasData() const
	{
		return fileData(QString::fromLatin1("canvas.xml"));
	}
	QByteArray externalData() const
	{
		return fileData(QString::fromLatin1("external.xml"));
	}
};

#endif

