/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boufonetworkoptionswidget.h"
#include "boufonetworkoptionswidget.moc"

#include "../boson.h"
#include "../defines.h"
#include "bodebug.h"

#include <klocale.h>
#include <kmessagebox.h>

BoUfoNetworkOptionsWidget::BoUfoNetworkOptionsWidget()
	: BoUfoNetworkOptionsWidgetBase()
{
 setConnected(false, false);
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
 }
}

void BoUfoNetworkOptionsWidget::setConnected(bool connected, bool master)
{
 if (!connected) {
	mNetStatusLabel->setText(i18n("Singleplayer mode\n"));
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

