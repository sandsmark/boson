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
#ifndef COMMANDINPUT_H
#define COMMANDINPUT_H

#include <kgame/kgameio.h>

class UnitBase;
class BosonCommandFrameBase;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class CommandInput : public KGameIO
{
	Q_OBJECT
public:
	CommandInput();
	virtual ~CommandInput();

	void setCommandFrame(BosonCommandFrameBase*);

	virtual int rtti() const { return 125; } // just any unique number

protected slots:
//	virtual void slotPlaceCell(int tile);
//
	/**
	 * Called when the player starts to produce a unit in the cmdframe. This
	 * just sends the event as a message through the network.
	 **/
	void slotProduceUnit(unsigned long int unitType, UnitBase* factory, KPlayer* owner);

	/**
	 * Called when the player aborts a production in the cmdframe. This
	 * just sends the event as a message through the network.
	 **/
	void slotStopProduction(unsigned long int unitType, UnitBase* factory, KPlayer* owner);
};

#endif

