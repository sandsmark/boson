/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOUFONETWORKOPTIONSWIDGET_H
#define BOUFONETWORKOPTIONSWIDGET_H

#include "boufonetworkoptionswidgetbase.h"

class QString;
class KGame;

class BoUfoNetworkOptionsWidget : public BoUfoNetworkOptionsWidgetBase
{
	Q_OBJECT
public:
	BoUfoNetworkOptionsWidget();
	~BoUfoNetworkOptionsWidget();

signals:
	void signalOfferingConnections();
	void signalConnectingToServer();
	void signalConnectedToServer();
	void signalCancelled();

protected slots:
	virtual void slotConnectionTypeChanged(BoUfoRadioButton*);
	virtual void slotDisconnect();
	virtual void slotStartNetwork();

	void slotConnectionBroken();
	void slotClientJoinedGame(Q_UINT32 clientId, KGame* client);
	void slotCancel();

private:
	void setConnected(bool connected, bool master);
};

#endif

