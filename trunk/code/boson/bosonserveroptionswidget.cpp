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

#include "bosonserveroptionswidget.h"
#include "bosonserveroptionswidget.moc"

#include <knuminput.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "top.h"
#include "boson.h"
#include "defines.h"

/*
 *  Constructs a BosonServerOptionsWidget which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
BosonServerOptionsWidget::BosonServerOptionsWidget(TopWidget* top, QWidget* parent)
    : QWidget(parent)
{
  mTop = top;

  BosonServerOptionsWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonServerOptionsWidgetLayout"); 

  header = new QLabel( this, "header" );
  QFont header_font(  header->font() );
  header_font.setPointSize( 30 );
  header_font.setBold( TRUE );
  header->setFont( header_font ); 
  header->setText( i18n( "Network settings" ) );
  header->setAlignment( int( QLabel::AlignCenter ) );
  BosonServerOptionsWidgetLayout->addWidget( header );

  Layout10 = new QHBoxLayout( 0, 0, 6, "Layout10"); 
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout10->addItem( spacer );

  Layout8 = new QVBoxLayout( 0, 0, 6, "Layout8"); 
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
  Layout8->addItem( spacer_2 );

  Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 

  netstatustext = new QLabel( this, "netstatustext" );
  netstatustext->setText( i18n( "Network status:" ) );
  Layout1->addWidget( netstatustext );
  QSpacerItem* spacer_3 = new QSpacerItem( 31, 31, QSizePolicy::Fixed, QSizePolicy::Minimum );
  Layout1->addItem( spacer_3 );

  netstatuslabel = new QLabel( this, "netstatuslabel" );
  Layout1->addWidget( netstatuslabel );
  Layout8->addLayout( Layout1 );

  Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2"); 
  QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout2->addItem( spacer_4 );

  disconnectbutton = new QPushButton( this, "disconnectbutton" );
  disconnectbutton->setEnabled( FALSE );
  disconnectbutton->setText( i18n( "Disconnect" ) );
  Layout2->addWidget( disconnectbutton );
  Layout8->addLayout( Layout2 );
  QSpacerItem* spacer_5 = new QSpacerItem( 31, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
  Layout8->addItem( spacer_5 );

  netconfgroupbox = new QGroupBox( this, "netconfgroupbox" );
  netconfgroupbox->setTitle( i18n( "Network configuration" ) );
  netconfgroupbox->setColumnLayout(0, Qt::Vertical );
  netconfgroupbox->layout()->setSpacing( 6 );
  netconfgroupbox->layout()->setMargin( 11 );
  netconfgroupboxLayout = new QVBoxLayout( netconfgroupbox->layout() );
  netconfgroupboxLayout->setAlignment( Qt::AlignTop );

  Layout7 = new QVBoxLayout( 0, 0, 6, "Layout7"); 

  connectionstylegroup = new QButtonGroup( netconfgroupbox, "connectionstylegroup" );
  connectionstylegroup->setTitle( i18n( "What would you like to do" ) );
  connectionstylegroup->setColumnLayout(0, Qt::Vertical );
  connectionstylegroup->layout()->setSpacing( 6 );
  connectionstylegroup->layout()->setMargin( 11 );
  connectionstylegroupLayout = new QVBoxLayout( connectionstylegroup->layout() );
  connectionstylegroupLayout->setAlignment( Qt::AlignTop );

  creategamebutton = new QRadioButton( connectionstylegroup, "creategamebutton" );
  creategamebutton->setText( i18n( "Create network game" ) );
  creategamebutton->setChecked( TRUE );
  connectionstylegroupLayout->addWidget( creategamebutton );

  joingamebutton = new QRadioButton( connectionstylegroup, "joingamebutton" );
  joingamebutton->setText( i18n( "Join network game" ) );
  connectionstylegroupLayout->addWidget( joingamebutton );
  Layout7->addWidget( connectionstylegroup );

  Layout6 = new QGridLayout( 0, 1, 1, 0, 6, "Layout6"); 

  portlabel = new QLabel( netconfgroupbox, "portlabel" );
  portlabel->setText( i18n( "Port to connect to:" ) );

  Layout6->addWidget( portlabel, 0, 0 );

  hostedit = new QLineEdit( netconfgroupbox, "hostedit" );

  Layout6->addWidget( hostedit, 2, 2 );

  portedit = new KIntNumInput( netconfgroupbox, "portedit" );

  Layout6->addMultiCellWidget( portedit, 0, 1, 2, 2 );
  QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout6->addItem( spacer_6, 2, 1 );

  hostlabel = new QLabel( netconfgroupbox, "hostlabel" );
  hostlabel->setText( i18n( "Host to connect to:" ) );

  Layout6->addMultiCellWidget( hostlabel, 1, 2, 0, 0 );
  QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout6->addItem( spacer_7, 0, 1 );
  Layout7->addLayout( Layout6 );

  startnetworkbutton = new QPushButton( netconfgroupbox, "startnetworkbutton" );
  startnetworkbutton->setText( i18n( "Start network" ) );
  Layout7->addWidget( startnetworkbutton );
  netconfgroupboxLayout->addLayout( Layout7 );
  Layout8->addWidget( netconfgroupbox );
  QSpacerItem* spacer_8 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  Layout8->addItem( spacer_8 );
  Layout10->addLayout( Layout8 );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout10->addItem( spacer_9 );
  BosonServerOptionsWidgetLayout->addLayout( Layout10 );

  Layout13 = new QHBoxLayout( 0, 0, 6, "Layout13"); 

  QSpacerItem* spacer_10 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout13->addItem( spacer_10 );

  okbutton = new QPushButton( this, "okbutton" );
  okbutton->setText( i18n( "Ok" ) );
  okbutton->setMinimumWidth(50);  // Looks too small without it
  Layout13->addWidget( okbutton );
  BosonServerOptionsWidgetLayout->addLayout( Layout13 );
  
  connect(disconnectbutton, SIGNAL(clicked()), this, SLOT(slotDisconnect()));
  connect(startnetworkbutton, SIGNAL(clicked()), this, SLOT(slotStartNetwork()));
  connect(connectionstylegroup, SIGNAL(clicked(int)), this, SLOT(slotConnectionTypeChanged(int)));
  connect(game(), SIGNAL(signalConnectionBroken()), this, SLOT(slotConnectionBroken()));
  connect(okbutton, SIGNAL(clicked()), this, SIGNAL(signalOkClicked()));
  
  hostedit->setText("localhost");
  portedit->setValue(BOSON_PORT);
  connectionstylegroup->setButton(0);
  slotConnectionTypeChanged(0);
  setConnected(game()->isNetwork(), game()->isMaster());
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BosonServerOptionsWidget::~BosonServerOptionsWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

void BosonServerOptionsWidget::slotDisconnect()
{
  game()->disconnect();
  setConnected(false, false);
}

void BosonServerOptionsWidget::slotStartNetwork()
{
  bool connected = false;
  bool master = true;
  unsigned short int port = portedit->value();
  QString host = hostedit->text();
  if(! hostedit->isEnabled())
  {
    master = true;
    connected = game()->offerConnections(port);
  }
  else
  {
    master = false;
    connected = game()->connectToServer(host, port);
  }
  setConnected(connected, master);
}

void BosonServerOptionsWidget::slotConnectionTypeChanged(int type)
{
  if(type == 0)  // Create network game
    hostedit->setEnabled(false);
  else  // Join network game
    hostedit->setEnabled(true);
}

void BosonServerOptionsWidget::setConnected(bool connected, bool master)
{
  if(!connected)
  {
    netstatuslabel->setText(i18n("No network"));
    netconfgroupbox->setEnabled(true);
    disconnectbutton->setEnabled(false);
    return;
  }
  if(master)
    netstatuslabel->setText(i18n("You are MASTER"));
  else
    netstatuslabel->setText(i18n("You are connected"));
  netconfgroupbox->setEnabled(false);
  disconnectbutton->setEnabled(true);
}

void BosonServerOptionsWidget::slotConnectionBroken()
{
  setConnected(false, false);
  KMessageBox::error(this, i18n("Cannot connect to network!"));
}

Boson* BosonServerOptionsWidget::game()
{
  return mTop->game();
}
