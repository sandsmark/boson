/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONGAMEENGINE_H
#define BOSONGAMEENGINE_H

#include <qobject.h>

class Boson;

class BosonGameEnginePrivate;
class BosonGameEngine : public QObject
{
	Q_OBJECT
public:
	BosonGameEngine(QObject* parent);
	~BosonGameEngine();

	void endGameAndDeleteBoson();
	bool preloadData();
	void initGame();

public slots:
	void slotResetGame();

signals:
	void signalBosonObjectAboutToBeDestroyed(Boson*);

private:
	BosonGameEnginePrivate* d;
};

#endif // BOSONGAMEENGINE_H

