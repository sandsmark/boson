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

#include "defines.h"
#include "bosonconfig.h"
#include "bosonmessage.h"
#include "player.h"
#include "speciestheme.h"
#include "boson.h"
#include "top.h"
#include "bosonplayfield.h"
#include "speciestheme.h"

#include <klocale.h>
#include <kdebug.h>

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

// AB: this class is a complete fast hack!
// not meant for public use yet

BosonStartEditorWidget::BosonStartEditorWidget(TopWidget* top, QWidget* parent)
    : QWidget(parent)
{
 mTop = top;

 initKGame();
 initPlayer();

 mBosonStartEditorWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonStartEditorWidgetLayout");

 mMainLayout = new QVBoxLayout( 0, 0, 6, "mainlayout");

 mStartGameLayout = new QHBoxLayout( 0, 0, 6, "startgamelayout"); 

 mCancelButton = new QPushButton( this, "cancelbutton" );
 mCancelButton->setText( i18n( "&Cancel" ) );
 mStartGameLayout->addWidget( mCancelButton );
 QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
 mStartGameLayout->addItem( spacer_9 );

 mStartGameButton = new QPushButton( this, "startgamebutton" );
 mStartGameButton->setText( i18n( "S&tart Editor" ) );
 mStartGameLayout->addWidget( mStartGameButton );
 mMainLayout->addLayout( mStartGameLayout );
 mBosonStartEditorWidgetLayout->addLayout( mMainLayout );
  
 // signals and slots connections
 connect(mCancelButton, SIGNAL(clicked()), this, SLOT(slotCancel()));
 connect(mStartGameButton, SIGNAL(clicked()), this, SLOT(slotStart()));
}

BosonStartEditorWidget::~BosonStartEditorWidget()
{
}

/*****  Init* methods  *****/
void BosonStartEditorWidget::initKGame()
{
 // We must manually set maximum players number to some bigger value, because
 //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
 //  players (it's a bug actually)
 game()->setMaxPlayers(BOSON_MAX_PLAYERS);
 kdDebug() << k_funcinfo << " minPlayers(): " << game()->minPlayers() << endl;
 kdDebug() << k_funcinfo << " maxPlayers(): " << game()->maxPlayers() << endl;
}

void BosonStartEditorWidget::initPlayer()
{
 kdDebug() << k_funcinfo << "playerCount(): " << game()->playerCount() << endl;
 player()->setName(boConfig->readLocalPlayerName());
 if(player()->speciesTheme()) {
	kdDebug() << k_funcinfo << "Player has speciesTheme already loaded, reloading" << endl;
 }
 mPlayercolor = boConfig->readLocalPlayerColor();
 player()->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), mPlayercolor);
 game()->addPlayer(player());
}

void BosonStartEditorWidget::slotStart()
{
 playField()->loadPlayField(BosonPlayField::playFieldFileName(BosonPlayField::defaultPlayField()));

 for (uint i = 1; i < game()->playerCount(); i++) {
	// add dummy computer player
	Player* p = new Player;
	p->setName(i18n("Computer"));
	QColor color = game()->availableTeamColors().first();
	p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);
	game()->addPlayer(p);
 }
 sendNewGame();
}

void BosonStartEditorWidget::slotCancel()
{
 emit signalCancelled();
}

Boson* BosonStartEditorWidget::game() const
{
 return mTop->game();
}

Player* BosonStartEditorWidget::player() const
{
 return mTop->player();
}

BosonPlayField* BosonStartEditorWidget::playField() const
{
 return mTop->playField();
}

void BosonStartEditorWidget::sendNewGame() 
{
 game()->sendMessage(0, BosonMessage::IdNewEditor);
}

