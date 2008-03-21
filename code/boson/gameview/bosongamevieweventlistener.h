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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONGAMEVIEWEVENTLISTENER_H
#define BOSONGAMEVIEWEVENTLISTENER_H

#include "../gameengine/boeventlistener.h"

class BosonCanvas;
class BosonItem;
class BosonShot;
class Unit;

class BosonGameViewEventListener : public BoEventListener
{
	Q_OBJECT
public:
	BosonGameViewEventListener(BoEventManager* manager, QObject* parent);
	~BosonGameViewEventListener();

	void setCanvas(const BosonCanvas* canvas);

	virtual QString scriptFileName() const
	{
		return QString::fromLatin1("gamevieweventlistener.py");
	}
	virtual QString xmlFileName() const
	{
		// AB: note: in contrast to scriptFileName(), this filename is
		// relative to the root
		return QString::fromLatin1("gameview.xml");
	}

	virtual bool canSee(const BoEvent*) const
	{
		return true;
	}

signals:
	void signalFacilityConstructed(Unit*);

protected:
	virtual void processEvent(const BoEvent* event);

	virtual BosonScript* createScriptParser() const
	{
		return 0;
	}

private:
	const BosonCanvas* mCanvas;
};

#endif

