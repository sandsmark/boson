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
#include "editorinput.h"

#include <qevent.h>

#include "editorinput.moc"

EditorInput::EditorInput(QObject* parent) : QObject(parent)
{
 if (parent) {
	parent->installEventFilter(this);
//	parent->setMouseTracking();
 }
}

EditorInput::~EditorInput()
{

}

bool EditorInput::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::Wheel:
	case QEvent::MouseMove:
	{
		bool eatevent = false;
		emit signalMouseEvent((QMouseEvent*)e, &eatevent);
		return eatevent;
		break;
	}
	default:
		break;
 }
 return QObject::eventFilter(o, e);
}

