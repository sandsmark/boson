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

#include "bosonstartwidgetbase.h"
#include "bosonstartwidgetbase.moc"

#include "../defines.h"
#include "../bosonmessage.h"
#include "../boson.h"
#include "../bosonplayfield.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"

#include <klocale.h>

BosonStartWidgetBase::BosonStartWidgetBase(BosonStartupNetwork* interface, QWidget* parent)
    : QWidget(parent)
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);

 mNetworkInterface = interface;

 initKGame();

 initPlayFields();
}

BosonStartWidgetBase::~BosonStartWidgetBase()
{
}

void BosonStartWidgetBase::initKGame()
{
 BO_CHECK_NULL_RET(boGame);
 connect(boGame, SIGNAL(signalPlayFieldChanged(const QString&)), 
		this, SLOT(slotPlayFieldChanged(const QString&)));
 connect(boGame, SIGNAL(signalStartGameClicked()),
		this, SLOT(slotStart()));

 // We must manually set maximum players number to some bigger value, because
 //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
 //  players (it's a bug actually)
 boGame->setMaxPlayers(BOSON_MAX_PLAYERS);
 boDebug() << k_funcinfo << " minPlayers(): " << boGame->minPlayers() << endl;
 boDebug() << k_funcinfo << " maxPlayers(): " << boGame->maxPlayers() << endl;
}

void BosonStartWidgetBase::initPlayFields()
{
 BosonPlayField::preLoadAllPlayFields();
 // AB: not much to do here, since we use BosonPlayField::availablePlayFields()
 // everywhere. if we manage something like mIndex2Identifier one day we should
 // init it here (not in derived classes)
}

void BosonStartWidgetBase::slotSendPlayFieldChanged(int index)
{
 networkInterface()->sendChangePlayField(index);
}

void BosonStartWidgetBase::slotPlayFieldChanged(const QString& id)
{
 mMapId = id;
 BosonPlayField* field = 0;
 if (!mMapId.isEmpty()) {
	field = BosonPlayField::playField(mMapId);
 }
 setCurrentPlayField(field);
}

void BosonStartWidgetBase::slotStartGameClicked()
{
 networkInterface()->sendStartGameClicked();
}

