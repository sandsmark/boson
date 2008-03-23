/*
    This file is part of the Boson game
    Copyright (C) 2002-2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BPFLOADER_H
#define BPFLOADER_H

#include <qstring.h>
#include <qmap.h>

class BPFDescription;
class BPFPreviewPrivate;
class BPFFile;

/**
 * @short Short information/preview of "BosonPlayField" (.bpf) files
 *
 * This class contains a "preview" of .bpf files, i.e. the name, description, a
 * map preview and possibly other informations that are relatively simply to
 * load and may be usable.
 *
 * The contents of objects of this class may be used for example in a "newgame"
 * dialog where the user may select the map she wants to play on.
 *
 * This class is explicitly shared, i.e. copies of an object (using the copy
 * c'tor or operator=()) are fast (O(1)) and share the same memory. See also the
 * Qt docs on explicit sharing.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BPFPreview
{
public:
	BPFPreview();
	BPFPreview(const BPFPreview& p);
	BPFPreview(BPFPreviewPrivate* copy);
	~BPFPreview();

	BPFPreview& operator=(const BPFPreview& p);

	static BPFPreview loadFilePreviewFromFiles(const QMap<QString, QByteArray>& files);

	bool isLoaded() const;
	const QString& identifier() const;
	const BPFDescription* description() const;
	BPFDescription* description(); // AB: non-const, so that a non-const pointer can be retrieved from a non-const BPFPreview only
	QByteArray mapPreviewPNGData() const;

	unsigned int mapWidth() const;
	unsigned int mapHeight() const;

	unsigned int minPlayers() const;
	int maxPlayers() const;

	// AB: additional information, such as winning conditions, should be
	// added here

private:
	static bool loadFilePreviewPlayersInformation(BPFPreviewPrivate* data, const QByteArray& xml);
	static bool loadFilePreviewMapInformation(BPFPreviewPrivate* data, const QByteArray& xml);

private:
	BPFPreviewPrivate* d; // explicitly shared!
};

/**
 * This class loads "BosonPlayField" (.bpf) files. Although the name is
 * identical, the class @ref BosonPlayField does @em not represent or load these files,
 * but a subset of these files only.
 *
 * This loader uses @ref BoFile for the actual loading and is meant to know
 * which files should be present in the .bpf archive (actually a .tar.gz
 * archive). This includes (but is not limited to) the @ref BosonMap/@ref
 * BosonPlayField files, the files describing the players and units on the map,
 * the @ref BosonScript and @ref BoEventListener files and so on.
 *
 * @ref loadFromDiskToFiles or @ref loadFromDiskToStream should be used to load
 * a complete file. If only a preview of the file is required, for example to
 * let the user select the playfield she wants to play on, @ref loadFilePreview
 * can be used.
 *
 * When the .bpf file is loaded, it is also checked whether the file format
 * matches the format of the current boson version and if not, converts the
 * virtual files to the current version using @ref BosonPlayFieldConverter.
 *
 * The .bpf file is loaded intop a set of virtual files described by a @ref
 * QMap. The key of the map describes the filename, the data of the map
 * describes the actual (virtual) file. This class does not know how to
 * interpret these information, the files are meant to be given to the
 * responsible classes (@ref BosonPlayField, @ref BoEventListener, ...)
 *
 * @short Loader for "BosonPlayField" (.bpf) files
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BPFLoader
{
public:
	BPFLoader();
	~BPFLoader();

	/**
	 * Shortcut for @ref loadFilePreviewFromDiskToFiles followed by @ref
	 * BPFPreview::loadFilePreviewFromFiles
	 **/
	static BPFPreview loadFilePreview(const QString& file);

	/**
	 * Like @ref loadFromDiskToFiles, but loads only those files needed to
	 * load a @ref BPFPreview. See also @ref
	 * BPFPreview::loadFilePreviewFromFiles
	 **/
	static bool loadFilePreviewFromDiskToFiles(const QString& file, QMap<QString,QByteArray>& destFiles);

	/**
	 * Load the .bpf file specified to @p fileName and place the virtual
	 * files in it (a .bpf file is actually an archive) into @p destFiles.
	 *
	 * Note: this method does not really belong here, as it is meant to load
	 * ALL files from the .bpf file, including player data and game data,
	 * that does not belong to the playfield. This method is here mainly for
	 * historic reasons.
	 **/
	static bool loadFromDiskToFiles(const QString& fileName, QMap<QString, QByteArray>& destFiles);

	/**
	 * Convenience function for @ref loadFromDiskToFiles followed by @ref
	 * streamFiles.
	 *
	 * @return A @ref QByteArray containing the specified playfield
	 * or an empty @ref QByteArray if an error occurred.
	 **/
	static QByteArray loadFromDiskToStream(const QString& file);

	/**
	 * @short Stream all (virtual) files in @p files.
	 *
	 * Helper method for loading a playfield. This takes a set of virtual
	 * files (in @p files), as e.g. loaded from a .bpf file, and writes it
	 * into a single @ref QByteArray along with some additional information
	 * that makes it easier to read the data back again.
	 **/
	static QByteArray streamFiles(const QMap<QString, QByteArray>& files);

	/**
	 * @short Unstream all (virtual) files to @p destFiles
	 *
	 * Helper method for loading a playfield. This takes a @ref QByteArray
	 * @p buffer which is meant to contain a playfield that was streamed
	 * using @ref streamFiles and writes the files to @p destFiles.
	 *
	 * @return TRUE on success, FALSE otherwise.
	 **/
	static bool unstreamFiles(QMap<QString, QByteArray>& destFiles, const QByteArray& buffer);

protected:
	/**
	 * @return A virtual "file" that contains an identifier for the
	 * specified file. It should be used for @ref BosonPlayField::identifier
	 * primarily. The "file" is simply a QDataStream containing a single
	 * QString.
	 **/
	static QByteArray createIdentifier(const BPFFile& boFile, const QMap<QString, QByteArray>& files);
};

#endif
