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

#include "bosonwelcomewidget.h"
#include "bosonwelcomewidget.moc"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <klocale.h>
#include <kstandarddirs.h>

/*
 *  Constructs a BosonWelcomeWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
BosonWelcomeWidget::BosonWelcomeWidget(QWidget* parent) : QWidget(parent)
{
  BosonWelcomeWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonWelcomeWidgetLayout");

  mainlayout = new QVBoxLayout( 0, 0, 6, "mainlayout"); 

  welcomelabel = new QLabel( this, "welcomelabel" );
  QFont welcomelabel_font(  welcomelabel->font() );
  welcomelabel_font.setPointSize( 30 );
  welcomelabel_font.setBold( TRUE );
  welcomelabel->setFont( welcomelabel_font ); 
  welcomelabel->setText( i18n( "Welcome to Boson!" ) );
  welcomelabel->setAlignment( int( QLabel::AlignCenter ) );
  mainlayout->addWidget( welcomelabel );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
  mainlayout->addItem( spacer );

  lowerlayout = new QHBoxLayout( 0, 0, 6, "lowerlayout"); 

  bosonpixmap = new QLabel( this, "bosonpixmap" );
  bosonpixmap->setAlignment( int( QLabel::AlignCenter ) );
  lowerlayout->addWidget( bosonpixmap );
  //FIXME: at least alignment.. maybe even stretch the pixmap?
  bosonpixmap->setPixmap(QPixmap(locate("data", "boson/pics/biglogo.png")));
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
  lowerlayout->addItem( spacer_2 );

  buttonslayout = new QVBoxLayout( 0, 0, 6, "buttonslayout"); 

  newgamebutton = new QPushButton( this, "newgamebutton" );
  newgamebutton->setText( i18n( "Start new game" ) );
  buttonslayout->addWidget( newgamebutton );
  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonslayout->addItem( spacer_3 );

  quitbutton = new QPushButton( this, "quitbutton" );
  quitbutton->setText( i18n( "Quit Boson" ) );
  buttonslayout->addWidget( quitbutton );
  QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonslayout->addItem( spacer_4 );
  QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonslayout->addItem( spacer_5 );
  lowerlayout->addLayout( buttonslayout );
  mainlayout->addLayout( lowerlayout );
  BosonWelcomeWidgetLayout->addLayout( mainlayout );

  connect(newgamebutton, SIGNAL(clicked()), this, SIGNAL(signalNewGame()));
  connect(quitbutton, SIGNAL(clicked()), this, SIGNAL(signalQuit()));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BosonWelcomeWidget::~BosonWelcomeWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

