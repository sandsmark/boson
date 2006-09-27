/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOLOCALPLAYEREVENTLISTENER_H
#define BOLOCALPLAYEREVENTLISTENER_H

#include "../gameengine/boeventlistener.h"

class Boson;
class KGamePropertyHandler;
class QDomElement;
class BoEvent;
class BoEventManager;
class BoCondition;
class Player;
class PlayerIO;
class BosonScript;
template<class T1, class T2> class QMap;


class BoLocalPlayerEventListener : public BoEventListener
{
	Q_OBJECT
public:
	BoLocalPlayerEventListener(PlayerIO* io, BoEventManager* manager, QObject* parent);
	~BoLocalPlayerEventListener();

	PlayerIO* playerIO() const
	{
		return mPlayerIO;
	}

	virtual QString scriptFileName() const
	{
		return QString("localplayer.py");
	}
	virtual QString xmlFileName() const
	{
		// AB: note: in contrast to scriptFileName(), this filename is
		// relative to the root
		return QString("localplayer.xml");
	}

	virtual void processEvent(const BoEvent* event);
	virtual bool canSee(const BoEvent* event) const;

signals:
	void signalShowMiniMap(bool);

protected:
	virtual BosonScript* createScriptParser() const;

private:
	PlayerIO* mPlayerIO;
};

#endif

