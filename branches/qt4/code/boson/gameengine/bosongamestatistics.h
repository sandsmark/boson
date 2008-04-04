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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONGAMESTATISTICS_H
#define BOSONGAMESTATISTICS_H

#include <qobject.h>

class Boson;
class BosonCanvas;

class BosonGameStatisticsPrivate;
/**
 * @short This class provides statistics about the game to the dedicated server.
 *
 * The statistics are currently updated once per advance message in @ref
 * receiveAdvanceMessage, however this could be easily modified to be updated
 * whenever some event occurs, by connecting a slot in this class to a signal.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGameStatistics : public QObject
{
	Q_OBJECT
public:
	BosonGameStatistics(QObject* parent);
	~BosonGameStatistics();

	void setGame(Boson* game);

	/**
	 * Update and send out status information about the game.
	 **/
	void receiveAdvanceMessage(BosonCanvas* canvas);

private:
	BosonGameStatisticsPrivate* d;
};


#endif

