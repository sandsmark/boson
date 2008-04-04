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

#include "boufonetworkoptionswidget.h"
#include "boufonetworkoptionswidget.moc"

#include "../../bomemory/bodummymemory.h"
#include "../gameengine/boson.h"
#include "../defines.h"
#include "bodebug.h"

#include <qtimer.h>

#include <klocale.h>
#include <kmessagebox.h>

BoUfoNetworkOptionsWidget::BoUfoNetworkOptionsWidget()
	: BoUfoNetworkOptionsWidgetBase()
{
 setConnected(false, false);

 connect(boGame, SIGNAL(signalConnectionBroken()),
		this, SLOT(slotConnectionBroken()));
 connect(boGame, SIGNAL(signalClientJoinedGame(Q_UINT32, KGame*)),
		this, SLOT(slotClientJoinedGame(Q_UINT32, KGame*)));

 mJoinGame->setSelected(true);
}

BoUfoNetworkOptionsWidget::~BoUfoNetworkOptionsWidget()
{
}

void BoUfoNetworkOptionsWidget::slotDisconnect()
{
 boGame->disconnect();
 setConnected(false, false);
}

void BoUfoNetworkOptionsWidget::slotConnectionTypeChanged(BoUfoRadioButton* b)
{
 if (b == mCreateGame) {
	mHostEdit->setEnabled(false);
 } else if (b == mJoinGame) {
	mHostEdit->setEnabled(true);
 } else {
	boError() << k_funcinfo << "invalid parameter" << endl;
 }
}

void BoUfoNetworkOptionsWidget::slotStartNetwork()
{
 bool connected = false;
 bool master = true;
 unsigned short int port = (unsigned short int)mPortEdit->value();
 QString host = mHostEdit->text();
 if (!mHostEdit->isEnabled()) {
	master = true;
	connected = boGame->offerConnections(port);
	setConnected(connected, master);
	if (connected) {
		emit signalOfferingConnections();
	} else {
		KMessageBox::error(0, i18n("Cannot start server! There might be another server already active."));
	}
 } else {
	master = false;
	emit signalConnectingToServer();
	connected = boGame->connectToServer(host, port);
	// don't call setConnected() here - connectToServer() is asynchron
	// There doesn't seem to be any way to get notified once we are connected, so
	//  we have to do this here
	setConnected(connected, master);
 }
}

void BoUfoNetworkOptionsWidget::setConnected(bool connected, bool master)
{
 if (!connected) {
	mNetStatusLabel->setText(i18n("Not connected"));
	mNetConfigGroupBox->setEnabled(true);
	mDisconnectButton->setEnabled(false);
	return;
 }
 if (master) {
	mNetStatusLabel->setText(i18n("Multiplayer mode\nYou are MASTER\nListening at port %1").arg(boGame->bosonPort()));
 } else {
	mNetStatusLabel->setText(i18n("Multiplayer mode\nYou are Connected\nServer: %1:%2").arg(boGame->bosonHostName()).arg(boGame->bosonPort()));
 }
 mNetConfigGroupBox->setEnabled(false);
 mDisconnectButton->setEnabled(true);
 if (boGame && connected) {
	unsigned short int port = boGame->bosonPort();
	if (!boGame->isNetwork()) {
		boWarning()<< k_funcinfo << "no network" << endl;
	}
	boDebug() << k_funcinfo << port << endl;
	if (port == 0) {
		port = BOSON_PORT;
	}
	mPortEdit->setValue(port);
	mHostEdit->setText(boGame->bosonHostName());
 }
}

void BoUfoNetworkOptionsWidget::slotConnectionBroken()
{
 setConnected(false, false);

 // this *can* be the case, but it also may be that the master closed the
 // connection (e.g. the master has quit).
 KMessageBox::error(0, i18n("Cannot Connect to Network!"));
}

void BoUfoNetworkOptionsWidget::slotClientJoinedGame(Q_UINT32 gameId, KGame*)
{
 if (gameId == boGame->gameId()) {
	boDebug() << k_funcinfo << "connection succeeded - gameid: " << gameId << endl;
	if (boGame->isNetwork()) {
		emit signalConnectedToServer();
	}
	setConnected(boGame->isNetwork(), boGame->isMaster());
 }
}

void BoUfoNetworkOptionsWidget::slotCancel()
{
 boDebug() << k_funcinfo << endl;

 // AB: we use a timer, so that the widget can be deleted in the slot
 // (otherwise this would not be allowed, as we are in a pushbutton click)
 QTimer::singleShot(0, this, SIGNAL(signalCancelled()));
}

