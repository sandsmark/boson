/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonnetworkoptionswidget.h"
#include "bosonnetworkoptionswidget.moc"

#include "../defines.h"
#include "../boson.h"
#include "bodebug.h"

#include <knuminput.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kgame/kmessageserver.h>

#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>

BosonNetworkOptionsWidget::BosonNetworkOptionsWidget(QWidget* parent)
    : QWidget(parent)
{
  boDebug() << k_funcinfo << endl;
  if (!boGame)
  {
    boError() << k_funcinfo << "NULL Boson object" << endl;
    return;
  }

  setPaletteForegroundColor( QColor( 255, 255, 255 ) );

  mBosonNetworkOptionsWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonNetworkOptionsWidgetLayout");

  mHeader = new QLabel( this, "header" );
  QFont header_font(  mHeader->font() );
  header_font.setFamily( "verdana" );
  header_font.setPointSize( 30 );
  header_font.setBold( TRUE );
  mHeader->setFont( header_font );
  mHeader->setText( i18n( "Network Settings" ) );
  mHeader->setAlignment( int( QLabel::AlignCenter ) );
  mBosonNetworkOptionsWidgetLayout->addWidget( mHeader );

  mLayout10 = new QHBoxLayout( 0, 0, 6, "Layout10");
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mLayout10->addItem( spacer );

  mLayout8 = new QVBoxLayout( 0, 0, 6, "Layout8");
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
  mLayout8->addItem( spacer_2 );

  mNetStatusText = new QLabel( this, "netstatustext" );
  mNetStatusText->setText( i18n( "Network Status:" ) );
  mLayout8->addWidget( mNetStatusText );

  mNetStatusLabel = new QLabel( this, "netstatuslabel" );
  mLayout8->addWidget( mNetStatusLabel );

  mLayout2 = new QHBoxLayout( 0, 0, 6, "Layout2");
  QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mLayout2->addItem( spacer_4 );

  mDisconnectButton = new QPushButton( this, "disconnectbutton" );
  mDisconnectButton->setEnabled( FALSE );
  mDisconnectButton->setText( i18n( "Dis&connect" ) );
  mLayout2->addWidget( mDisconnectButton );
  mLayout8->addLayout( mLayout2 );
  QSpacerItem* spacer_5 = new QSpacerItem( 31, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
  mLayout8->addItem( spacer_5 );

  mNetConfGroupBox = new QGroupBox( this, "netconfgroupbox" );
  mNetConfGroupBox->setTitle( i18n( "Network Configuration" ) );
  mNetConfGroupBox->setColumnLayout(0, Qt::Vertical );
  mNetConfGroupBox->layout()->setSpacing( 6 );
  mNetConfGroupBox->layout()->setMargin( 11 );
  mNetConfGroupBoxLayout = new QVBoxLayout( mNetConfGroupBox->layout() );
  mNetConfGroupBoxLayout->setAlignment( Qt::AlignTop );

  mLayout7 = new QVBoxLayout( 0, 0, 6, "Layout7"); 

  mConnectionStyleGroup = new QButtonGroup( mNetConfGroupBox, "connectionstylegroup" );
  mConnectionStyleGroup->setTitle( i18n( "What would you like to do" ) );
  mConnectionStyleGroup->setColumnLayout(0, Qt::Vertical );
  mConnectionStyleGroup->layout()->setSpacing( 6 );
  mConnectionStyleGroup->layout()->setMargin( 11 );
  mConnectionStyleGroupLayout = new QVBoxLayout( mConnectionStyleGroup->layout() );
  mConnectionStyleGroupLayout->setAlignment( Qt::AlignTop );

  mCreateGameButton = new QRadioButton( mConnectionStyleGroup, "creategamebutton" );
  mCreateGameButton->setText( i18n( "Create Network Game" ) );
  mCreateGameButton->setChecked( TRUE );
  mConnectionStyleGroupLayout->addWidget( mCreateGameButton );

  mJoinGameButton = new QRadioButton( mConnectionStyleGroup, "joingamebutton" );
  mJoinGameButton->setText( i18n( "Join Network Game" ) );
  mConnectionStyleGroupLayout->addWidget( mJoinGameButton );
  mLayout7->addWidget( mConnectionStyleGroup );

  mLayout6 = new QGridLayout( 0, 1, 1, 0, 6, "Layout6");

  mPortLabel = new QLabel( mNetConfGroupBox, "portlabel" );
  mPortLabel->setText( i18n( "Port to Connect to:" ) );

  mLayout6->addWidget( mPortLabel, 0, 0 );

  mHostEdit = new QLineEdit( mNetConfGroupBox, "hostedit" );

  mLayout6->addWidget( mHostEdit, 2, 2 );

  mPortEdit = new KIntNumInput( mNetConfGroupBox, "portedit" );

  mLayout6->addMultiCellWidget( mPortEdit, 0, 1, 2, 2 );
  QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mLayout6->addItem( spacer_6, 2, 1 );

  mHostLabel = new QLabel( mNetConfGroupBox, "hostlabel" );
  mHostLabel->setText( i18n( "Host to Connect to:" ) );

  mLayout6->addMultiCellWidget( mHostLabel, 1, 2, 0, 0 );
  QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mLayout6->addItem( spacer_7, 0, 1 );
  mLayout7->addLayout( mLayout6 );

  mStartNetworkButton = new QPushButton( mNetConfGroupBox, "startnetworkbutton" );
  mStartNetworkButton->setText( i18n( "S&tart Network" ) );
  mLayout7->addWidget( mStartNetworkButton );
  mNetConfGroupBoxLayout->addLayout( mLayout7 );
  mLayout8->addWidget( mNetConfGroupBox );
  QSpacerItem* spacer_8 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  mLayout8->addItem( spacer_8 );
  mLayout10->addLayout( mLayout8 );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mLayout10->addItem( spacer_9 );
  mBosonNetworkOptionsWidgetLayout->addLayout( mLayout10 );

  mLayout13 = new QHBoxLayout( 0, 0, 6, "Layout13"); 

  QSpacerItem* spacer_10 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mLayout13->addItem( spacer_10 );

  mOkButton = new QPushButton( this, "okbutton" );
  mOkButton->setText( i18n( "&Ok" ) );
  mOkButton->setMinimumWidth(50);  // Looks too small without it
  mLayout13->addWidget( mOkButton );
  mBosonNetworkOptionsWidgetLayout->addLayout( mLayout13 );

  connect(mDisconnectButton, SIGNAL(clicked()), this, SLOT(slotDisconnect()));
  connect(mStartNetworkButton, SIGNAL(clicked()), this, SLOT(slotStartNetwork()));
  connect(mConnectionStyleGroup, SIGNAL(clicked(int)), this,
        SLOT(slotConnectionTypeChanged(int)));
  connect(boGame, SIGNAL(signalConnectionBroken()), this,
        SLOT(slotConnectionBroken()));
  connect(boGame, SIGNAL(signalClientJoinedGame(Q_UINT32, KGame*)), this,
        SLOT(slotClientJoinedGame(Q_UINT32, KGame*)));
  connect(mOkButton, SIGNAL(clicked()), this, SIGNAL(signalOkClicked()));

  mHostEdit->setText("localhost");
  mPortEdit->setValue(BOSON_PORT);
  mConnectionStyleGroup->setButton(0);
  slotConnectionTypeChanged(0);
  setConnected(boGame->isNetwork(), boGame->isMaster());
}

BosonNetworkOptionsWidget::~BosonNetworkOptionsWidget()
{
  boDebug() << k_funcinfo << endl;
}

void BosonNetworkOptionsWidget::slotDisconnect()
{
  boGame->disconnect();
  setConnected(false, false);
}

void BosonNetworkOptionsWidget::slotStartNetwork()
{
  bool connected = false;
  bool master = true;
  unsigned short int port = mPortEdit->value();
  QString host = mHostEdit->text();
  if(!mHostEdit->isEnabled())
  {
    master = true;
    connected = boGame->offerConnections(port);
    setConnected(connected, master);
    if(connected)
    {
      emit signalOfferingConnections();
    }
    else
    {
      KMessageBox::error(this, i18n("Cannot start server! There might be another server already active."));
    }
  }
  else
  {
    master = false;
    emit signalConnectingToServer();
    connected = boGame->connectToServer(host, port);
    // don't call setConnected() here - connectToServer() is asynchron
  }
}

void BosonNetworkOptionsWidget::slotConnectionTypeChanged(int type)
{
  if(type == 0)  // Create network game
  {
    mHostEdit->setEnabled(false);
  }
  else  // Join network game
  {
    mHostEdit->setEnabled(true);
  }
}

void BosonNetworkOptionsWidget::setConnected(bool connected, bool master)
{
  if(!connected)
  {
    mNetStatusLabel->setText(i18n("No Network\n"));
    mNetConfGroupBox->setEnabled(true);
    mDisconnectButton->setEnabled(false);
    return;
  }
  if(master)
  {
    mNetStatusLabel->setText(i18n("You are MASTER\nListening at port %1").arg(boGame->bosonPort()));
  }
  else
  {
    mNetStatusLabel->setText(i18n("You are Connected\nServer: %1:%2").arg(boGame->bosonHostName()).arg(boGame->bosonPort()));
  }
  mNetConfGroupBox->setEnabled(false);
  mDisconnectButton->setEnabled(true);
  if(boGame && connected)
  {
    unsigned short int port = boGame->bosonPort();
    if (!boGame->isNetwork()) {
      boWarning()<< k_funcinfo << "no network" << endl;
    }
    boDebug() << k_funcinfo << port << endl;
    if (port == 0)
    {
      port = BOSON_PORT;
    }
    mPortEdit->setValue(port);
    mHostEdit->setText(boGame->bosonHostName());
  }
}

void BosonNetworkOptionsWidget::slotConnectionBroken()
{
  setConnected(false, false);

  // this *can* be the case, but it also may be that the master closed the
  // connection (e.g. the master has quit).
  KMessageBox::error(this, i18n("Cannot Connect to Network!"));
}

void BosonNetworkOptionsWidget::slotClientJoinedGame(Q_UINT32 gameId, KGame*)
{
  if (gameId == boGame->gameId())
  {
    boDebug() << k_funcinfo << "connection succeeded - gameid: " << gameId << endl;
    if (boGame->isNetwork())
    {
      emit signalConnectedToServer();
    }
    setConnected(boGame->isNetwork(), boGame->isMaster());
  }
}

/*
 * vim: et sw=2
 */
