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

#include "bosonstarteditorwidget.h"
#include "bosonstarteditorwidget.moc"

#include "../defines.h"
#include "../boson.h"
#include "bosonstartupnetwork.h"
#include "bosonneweditorwidget.h"
#include "bodebug.h"

#include <klocale.h>
#include <kdialog.h>

#include <qpushbutton.h>
#include <qlayout.h>

class BosonStartEditorWidgetPrivate
{
public:
	BosonStartEditorWidgetPrivate()
	{
		mCancelButton = 0;
		mStartGameButton = 0;
		mNewEditorWidget = 0;
	}
	QPushButton* mCancelButton;
	QPushButton* mStartGameButton;
	BosonNewEditorWidget* mNewEditorWidget;

};

BosonStartEditorWidget::BosonStartEditorWidget(BosonStartupNetwork* interface, QWidget* parent)
	: QWidget(parent, "bosonstarteditorwidget")
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BosonStartEditorWidgetPrivate;
 mNetworkInterface = interface;

 QVBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

 d->mNewEditorWidget = new BosonNewEditorWidget(networkInterface(), this);
 topLayout->addWidget(d->mNewEditorWidget, 1);
 d->mNewEditorWidget->show();

 QHBoxLayout* startGameLayout = new QHBoxLayout(topLayout);
 d->mCancelButton = new QPushButton(i18n("&Cancel"), this, "cancelbutton");
 d->mStartGameButton = new QPushButton(i18n("S&tart Editor"), this, "startgamebutton" );
 startGameLayout->addWidget(d->mCancelButton);
 startGameLayout->addWidget(d->mStartGameButton);
 connect(d->mCancelButton, SIGNAL(clicked()), this, SIGNAL(signalCancelled()));

 connect(d->mStartGameButton, SIGNAL(clicked()), this, SLOT(slotStartGameClicked()));

 // AB: this widget isn't the ideal place for this...
 initKGame();
}

BosonStartEditorWidget::~BosonStartEditorWidget()
{
 delete d;
}

void BosonStartEditorWidget::slotStartGameClicked()
{
 boDebug() << k_funcinfo << endl;
 networkInterface()->sendStartGameClicked();
}

void BosonStartEditorWidget::initKGame()
{
 // We must manually set maximum players number to some bigger value, because
 //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
 //  players (it's a bug actually)
 boGame->setMaxPlayers(BOSON_MAX_PLAYERS);
 boDebug() << k_funcinfo << " minPlayers(): " << boGame->minPlayers() << endl;
 boDebug() << k_funcinfo << " maxPlayers(): " << boGame->maxPlayers() << endl;

}

