/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __BOSONMUSIC_H__
#define __BOSONMUSIC_H__

#include <qobject.h>

class QStringList;

/**
 * Music support. Not much to describe here...
 *
 * A lot of code is from kaboodle (as I know nearly nothing about sound in
 * KDE). Just call @ref startLoop to start playing.
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Music support
 **/
class BosonMusic : public QObject
{
	Q_OBJECT
public:
	BosonMusic(QObject* parent);
	~BosonMusic();

	/**
	 * Play a previously loaded file. See @ref load
	 **/
	void play();

	/**
	 * Load a file
	 * @param file Absolute path to the file
	 * @return True if file was loaded successfully, otherwise false
	 **/
	bool load(const QString& file);

	/**
	 * @return All music files that could be found. Note that this start at
	 * the boson/music dir and is recursively. MP3 files are searched as
	 * well as OGG files.
	 **/
	QStringList availableMusic() const;

	/**
	 * Equivalent to startLoop(availableMusic())
	 **/
	void startLoop();

	/**
	 * Start to play files in a loop. If a file from the list could not be
	 * loaded it is removed from the list.
	 *
	 * Note that the files are played in random order!
	 * @prarm files List of absolute file names
	 **/
	void startLoop(const QStringList& files);

	bool isLoop() const;

protected slots:
	void slotUpdateTicker();

private:
	class BosonMusicPrivate;
	BosonMusicPrivate* d;
};

#endif
