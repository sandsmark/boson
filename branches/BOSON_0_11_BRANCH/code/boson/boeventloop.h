/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOEVENTLOOP_H
#define BOEVENTLOOP_H

#include <qeventloop.h>

class BoEventLoopPrivate;

/**
 * @short This implements the main event loop of boson
 *
 * Note that this has <em>absolutely nothing</em> to do with @ref BoEvent. This
 * class manges GUI events (paint widgets, button clicked, mouse moved, ...)
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoEventLoop : public QEventLoop
{
	Q_OBJECT
public:
	BoEventLoop(QObject* parent = 0, const char* name = 0);
	~BoEventLoop();

	virtual bool processEvents(ProcessEventsFlags flags);

	void setAdvanceMessageInterval(int interval);
	void setAdvanceObject(QObject*);
	void receivedAdvanceMessage(int gameSpeed);
	void setAdvanceMessagesWaiting(int count);

protected:
	/**
	 * Post an QtEventAdvanceCall event to the advance object (see @ref
	 * setAdvanceObject).
	 **/
	void postAdvanceCallEvent();

signals:
	void signalUpdateGL();
	void signalAdvanceCall();

private:
	QObject* mAdvanceObject;
	BoEventLoopPrivate* d;
};

#endif

