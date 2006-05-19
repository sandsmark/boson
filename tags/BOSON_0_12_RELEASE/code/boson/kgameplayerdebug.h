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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KGAMEPLAYERDEBUG_H
#define KGAMEPLAYERDEBUG_H

#include <qwidget.h>

class Boson;
class Player;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGamePlayerDebug : public QWidget
{
	Q_OBJECT
public:
	KGamePlayerDebug(QWidget* parent);
	~KGamePlayerDebug();

	void setBoson(Boson* boson);
	void setLocalPlayer(Player* p);

protected slots:
	void slotUpdate();
	
private:
	class KGamePlayerDebugPrivate;
	KGamePlayerDebugPrivate* d;
};

#endif
