/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __EDITORINPUT_H__
#define __EDITORINPUT_H__

#include <qobject.h>

/**
 * Essentially like @ref KGameIO but doesn't send anything
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class EditorInput : public QObject
{
	Q_OBJECT
public:
	EditorInput(QObject* parent);
	~EditorInput();

	virtual bool eventFilter(QObject* o, QEvent* e);
	
signals:
	void signalMouseEvent(QMouseEvent* e, bool* eatevent);
};

#endif
