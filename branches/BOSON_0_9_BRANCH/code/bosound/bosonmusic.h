/*
    This file is part of the Boson game
    Copyright (C) 2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONMUSIC_H
#define BOSONMUSIC_H

#include "bosonabstractaudiointerface.h"

#include <qobject.h>

class QStringList;
class KArtsServer;
class BosonSound;
class BosonAudio;
class Unit;

/**
 * Music support. Not much to describe here...
 *
 * A lot of code is from kaboodle (as I know nearly nothing about sound in
 * KDE). Just call @ref startLoop to start playing.
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Music support
 **/
class BosonMusic : public QObject, public BosonAbstractMusicInterface
{
	Q_OBJECT
	// AB: note that QObject derived classes in threads other the GUI thread
	// are possible, even if they are not a perfect solution. we must take
	// care that no events are sent/received.
	// this isn't a problem for us, since we only use the slot mechanism
	// from QObject here.
public:
	BosonMusic(BosonAudio* parent);
	virtual ~BosonMusic();

	virtual void setMusic(bool m);
	virtual bool music() const;

	 // obsolete. remove!
	void play()
	{
		playMusic();
	}

	/**
	 * Play a previously loaded file. See @ref load
	 **/
	virtual void playMusic();
	virtual void stopMusic();

	virtual void startMusicLoop();
	virtual void addToMusicList(const QString& file);
	virtual void clearMusicList();

	virtual bool isLoop() const; // FIXME: do we need this? should be in the interface only

protected:
	/**
	 * Load a file
	 * @param file Absolute path to the file
	 * @return True if file was loaded successfully, otherwise false
	 **/
	bool loadMusic(const QString& file);

protected slots:
	void slotUpdateTicker();

private:
	class BosonMusicPrivate;
	BosonMusicPrivate* d;
};

#endif
