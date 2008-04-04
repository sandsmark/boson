/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bomousemovediff.h"

#include "../bomemory/bodummymemory.h"

#include <qpoint.h>
#include <qevent.h>


BoMouseMoveDiff::BoMouseMoveDiff()
{
 mButton = QMouseEvent::NoButton;
 mX = 0;
 mY = 0;
 mOldX = 0;
 mOldY = 0;
}

void BoMouseMoveDiff::moveEvent(QMouseEvent* e)
{
 moveEvent(e->pos(), e->state());
}

void BoMouseMoveDiff::moveEvent(const QPoint& pos, int buttonState)
{
 moveEvent(pos.x(), pos.y(), buttonState);
}

void BoMouseMoveDiff::moveEvent(int x, int y, int buttonState)
{
 mOldX = mX;
 mOldY = mY;
 mX = x;
 mY = y;

 if (buttonState == -1) {
	buttonState = QMouseEvent::NoButton;
 }
 if (mButton != QMouseEvent::NoButton) {
	if (!(buttonState & mButton)) {
		// button is not pressed anymore
		stop();
	}
 }
}

void BoMouseMoveDiff::stop()
{
 mButton = QMouseEvent::NoButton;
}

bool BoMouseMoveDiff::isStopped() const
{
 return button() == QMouseEvent::NoButton;
}

void BoMouseMoveDiff::start(int button)
{
 mButton = button;
}

