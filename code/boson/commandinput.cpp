/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "commandinput.h"

#include "bosoncommandwidget.h"
#include "bosonmessage.h"
#include "unitbase.h"

#include <kdebug.h>
#include <kgame/kplayer.h>

#include "commandinput.moc"

class CommandInput::CommandInputPrivate
{
public:
	CommandInputPrivate()
	{
	}
	
	
};


CommandInput::CommandInput() : KGameIO()
{
}
CommandInput::CommandInput(KPlayer* player) : KGameIO(player)
{
}

CommandInput::~CommandInput()
{
}

void CommandInput::slotProduceUnit(int unitType, UnitBase* factory, KPlayer* owner)
{
 if (!player()) {
	kdError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 if (!game()) {
	kdError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!owner) {
	kdError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (!factory) {
	// FIXME: this is possible if we use CommandInput for Editor mode!
	kdError() << k_funcinfo << "NULL factory" << endl;
	return;
 }
 kdDebug() << k_funcinfo << endl;
 QByteArray b;
 QDataStream stream(b, IO_WriteOnly);
 stream << (Q_UINT32)BosonMessage::MoveProduce;
 stream << (Q_UINT32)owner->id();
 stream << (Q_UINT32)factory->id();
 stream << (Q_INT32)unitType;

 QDataStream msg(b, IO_ReadOnly);
 sendInput(msg);
}

void CommandInput::slotPlaceCell(int) // obsolete
{
}
