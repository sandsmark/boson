/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOAUDIOPROCESSCONTROLLER_H
#define BOAUDIOPROCESSCONTROLLER_H

#include <qobject.h>

class KProcess;
class BoAudioCommand;

class BoAudioProcessControllerPrivate;

/**
 * This class provides a sound process. It could be implemented several ways -
 * e.g. it probably is a read process using @ref KProcess, but it could also be
 * a thread using @ref BoAudioThread.
 *
 * At the moment we need a separate process, as threads didn't solve the
 * blocking problems that occurred for sound playing.
 *
 * This class is meant to be used in the main program only, i.e. not in the
 * audio process itself. It is used to start the audio process.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoAudioProcessController : public QObject
{
	Q_OBJECT
public:
	BoAudioProcessController();
	~BoAudioProcessController();

	/**
	 * @return Whether the audio process is running. See @ref start
	 **/
	bool isRunning() const;

	/**
	 * Start the process (or thread if this is implemented using threads
	 * instead of a process).
	 *
	 * Note that this class is responsible for finding the correct filename
	 * of the process.
	 * @return FALSE if the process could not be started by any reason.
	 * Otherwise TRUE.
	 **/
	bool start();

	/**
	 * Send a command to the process. Note that it may be possible that the
	 * command is queued for later delivery only.
	 **/
	void sendCommand(BoAudioCommand* command);

protected slots:
	void slotShowStdout(KProcess* proc, char* buffer, int length);
	void slotShowStderr(KProcess* proc, char* buffer, int length);
	void slotProcessExited(KProcess* proc);
	void slotWroteStdin(KProcess* proc);

private:
	BoAudioProcessControllerPrivate* d;
};

#endif

