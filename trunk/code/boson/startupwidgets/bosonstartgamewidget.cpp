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

#include "bosonstartgamewidget.h"
#include "bosonstartgamewidget.moc"

#include "../defines.h"
#include "../bosonmessage.h"
#include "../player.h"
#include "../boson.h"
#include "bosonstartupnetwork.h"
#include "bosonnewgamewidget.h"
#include "bodebug.h"

#include <qpushbutton.h>
#include <qlayout.h>

#include <kdialog.h>
#include <klocale.h>
#include <kgame/kgamechat.h>

class BosonStartGameWidgetPrivate
{
public:
	BosonStartGameWidgetPrivate()
	{
		mNewGameWidget = 0;

		mChatWidget = 0;

		mStartGameButton = 0;
		mCancelButton = 0;
		mNetworkButton = 0;
	}

	BosonNewGameWidget* mNewGameWidget;

	KGameChat* mChatWidget;

	QPushButton* mStartGameButton;
	QPushButton* mCancelButton;
	QPushButton* mNetworkButton;
};

BosonStartGameWidget::BosonStartGameWidget(BosonStartupNetwork* interface, QWidget* parent)
    : QWidget(parent)
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BosonStartGameWidgetPrivate;
 mNetworkInterface = interface;

 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

 // the thing that lets you configure everything..
 // this could be tabbed or whatever - but the chat widget and start/cancel
 // buttons always need to be visible, so we embed it in this widget instead of
 // showing it directly in BosonStartupWidget.
 d->mNewGameWidget = new BosonNewGameWidget(networkInterface(), this);
 topLayout->addWidget(d->mNewGameWidget, 1);
 d->mNewGameWidget->show();

 d->mChatWidget = new KGameChat(0, BosonMessage::IdChat, this);
 d->mChatWidget->setKGame(boGame);
 topLayout->addWidget(d->mChatWidget);

 QHBoxLayout* startGameLayout = new QHBoxLayout(topLayout);
 d->mCancelButton = new QPushButton(i18n("&Cancel"), this);
 d->mNetworkButton = new QPushButton(i18n("&Network Options"), this);
 d->mStartGameButton = new QPushButton(i18n("S&tart Game"), this);
 startGameLayout->addWidget(d->mCancelButton);
 startGameLayout->addWidget(d->mNetworkButton);
 startGameLayout->addWidget(d->mStartGameButton);
 connect(d->mCancelButton, SIGNAL(clicked()), this, SIGNAL(signalCancelled()));
 connect(d->mNetworkButton, SIGNAL(clicked()), this, SIGNAL(signalShowNetworkOptions()));
 connect(d->mStartGameButton, SIGNAL(clicked()), this, SLOT(slotStartGameClicked()));

 connect(networkInterface(), SIGNAL(signalSetLocalPlayer(Player*)),
		this, SLOT(slotSetLocalPlayer(Player*)));

 slotSetAdmin(boGame->isAdmin());
}

BosonStartGameWidget::~BosonStartGameWidget()
{
 delete d;
}

void BosonStartGameWidget::slotStartGameClicked()
{
 boDebug() << k_funcinfo << endl;
 networkInterface()->sendStartGameClicked();
}

void BosonStartGameWidget::slotSetAdmin(bool admin)
{
 if (admin) {
	d->mStartGameButton->setEnabled(true);
 } else {
	d->mStartGameButton->setEnabled(false);
 }
}

void BosonStartGameWidget::addAIPlayer()
{
 // forward in order to make cmd line args work.
 d->mNewGameWidget->addDummyComputerPlayer();
}

void BosonStartGameWidget::slotSetLocalPlayer(Player* p)
{
 if (!p) {
	// AB: a NULL local player is totally valid!
	// we are meant to unset the player now

	// AB: this is bad... KGameChat doesn't allow NULL players...
	// how can we unset the player from there?
	return;
 }
 d->mChatWidget->setFromPlayer(p);
}
