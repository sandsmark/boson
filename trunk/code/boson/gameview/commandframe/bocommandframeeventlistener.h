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
#ifndef BOCOMMANDFRAMEEVENTLISTENER_H
#define BOCOMMANDFRAMEEVENTLISTENER_H

#include "../../gameengine/boeventlistener.h"

class Player;
class PlayerIO;
class Unit;
class BoSelection;
class BoUnitDisplayBase;
class BoSpecificAction;
class BosonGroundTheme;

class BoCommandFrameEventListenerPrivate;
class BoCommandFrameEventListener : public BoEventListener
{
	Q_OBJECT
public:
	BoCommandFrameEventListener(PlayerIO* io, BoEventManager* manager, QObject* parent);
	~BoCommandFrameEventListener();

	PlayerIO* playerIO() const
	{
		return mPlayerIO;
	}

	virtual QString scriptFileName() const
	{
		return "commandframeeventlistener.py";
	}
	virtual QString xmlFileName() const
	{
		// AB: note: in contrast to scriptFileName(), this filename is
		// relative to the root
		return "commandframe.xml";
	}

	virtual bool canSee(const BoEvent* event) const;

signals:
	void signalUpdateSelection();
	void signalUpdateProductionOptions();
	void signalUpdateProduction(unsigned long int unitId);

	/**
	 * Emitted when a the construction of a facility has been completed.
	 *
	 * If the command frame is currently displaying this facility, it may
	 * consider re-displaying it, so that e.g. production options will be
	 * shown.
	 **/
	void signalFacilityConstructed(unsigned long int unitId);

	void signalUnitDestroyed(unsigned long int unitId);

protected:
	virtual void processEvent(const BoEvent* event);
	virtual BosonScript* createScriptParser() const
	{
		return 0;
	}

private:
	BoCommandFrameEventListenerPrivate* d;
	PlayerIO* mPlayerIO;
};

#endif
