/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann <b_mann@gmx.de>

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

#include "../bosonabstractaudiointerface.h"

#include <qobject.h>

class BosonAudioAL;

/**
 * Music support. Not much to describe here...
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Music support
 **/
class BosonMusic : public QObject, public BosonAbstractMusicInterface
{
	Q_OBJECT
public:
	BosonMusic(BosonAudioAL* parent);
	virtual ~BosonMusic();

	bool checkALError();

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

	void initMusicSource();

protected slots:
	void slotUpdateTicker();

private:
	class BosonMusicPrivate;
	BosonMusicPrivate* d;
};

#endif
