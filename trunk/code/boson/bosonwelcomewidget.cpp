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

#include "defines.h"

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
  mBosonWelcomeWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonWelcomeWidgetLayout");

  mMainLayout = new QVBoxLayout( 0, 0, 6, "mainlayout"); 

  mWelcomeLabel = new QLabel( this, "welcomelabel" );
  QFont mWelcomeLabel_font(  mWelcomeLabel->font() );
  mWelcomeLabel_font.setPointSize( 30 );
  mWelcomeLabel_font.setBold( TRUE );
  mWelcomeLabel->setFont( mWelcomeLabel_font ); 
  mWelcomeLabel->setText( i18n( "Welcome to Boson!" ) );
  mWelcomeLabel->setAlignment( int( QLabel::AlignCenter ) );
  mMainLayout->addWidget( mWelcomeLabel );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
  mMainLayout->addItem( spacer );

  mLowerLayout = new QHBoxLayout( 0, 0, 6, "lowerlayout"); 

  mBosonPixmap = new QLabel( this, "bosonpixmap" );
  mBosonPixmap->setAlignment( int( QLabel::AlignCenter ) );
  mLowerLayout->addWidget( mBosonPixmap );
  //FIXME: at least alignment.. maybe even stretch the pixmap?
  mBosonPixmap->setPixmap(QPixmap(locate("data", "boson/pics/biglogo.png")));
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
  mLowerLayout->addItem( spacer_2 );

  mButtonsLayout = new QVBoxLayout( 0, 0, 6, "buttonslayout"); 

  mNewGameButton = new QPushButton( this, "newgamebutton" );
  mNewGameButton->setText( i18n( "S&tart new game" ) );
  mButtonsLayout->addWidget( mNewGameButton );
  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  mButtonsLayout->addItem( spacer_3 );
  
#ifndef NO_EDITOR
  mEditorButton = new QPushButton( this, "editorbutton" );
  mEditorButton->setText( i18n( "Start Editor" ) ); // TODO: accel
  mButtonsLayout->addWidget( mEditorButton );
  connect(mEditorButton, SIGNAL(clicked()), this, SIGNAL(signalStartEditor()));
#else
  mEditorButton = 0;
#endif

  mQuitButton = new QPushButton( this, "quitbutton" );
  mQuitButton->setText( i18n( "&Quit Boson" ) );
  mButtonsLayout->addWidget( mQuitButton );
  QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  mButtonsLayout->addItem( spacer_4 );
  QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  mButtonsLayout->addItem( spacer_5 );
  mLowerLayout->addLayout( mButtonsLayout );
  mMainLayout->addLayout( mLowerLayout );
  mBosonWelcomeWidgetLayout->addLayout( mMainLayout );

  connect(mNewGameButton, SIGNAL(clicked()), this, SIGNAL(signalNewGame()));
  connect(mQuitButton, SIGNAL(clicked()), this, SIGNAL(signalQuit()));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BosonWelcomeWidget::~BosonWelcomeWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

