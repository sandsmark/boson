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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KGAMEADVANCEMESSAGESDEBUG_H
#define KGAMEADVANCEMESSAGESDEBUG_H

#include <qwidget.h>

class Boson;

class KGameAdvanceMessagesDebugPrivate;
class KGameAdvanceMessagesDebug : public QWidget
{
	Q_OBJECT
public:
	KGameAdvanceMessagesDebug(QWidget* parent, const char* name = 0);
	~KGameAdvanceMessagesDebug();

	void setBoson(Boson* b);

protected slots:
	void slotUpdate();

private:
	KGameAdvanceMessagesDebugPrivate* d;
	Boson* mGame;
};


#endif
