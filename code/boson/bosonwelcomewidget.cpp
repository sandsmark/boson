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

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <stdlib.h>

/*
 *  Constructs a BosonWelcomeWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
BosonWelcomeWidget::BosonWelcomeWidget(QWidget* parent) : QWidget(parent)
{
  mBosonWelcomeWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonWelcomeWidgetLayout");

  mMainLayout = new QVBoxLayout( 0, 0, 6, "mainlayout");

  QPixmap startupPix(locate("data", "boson/pics/startup.png"));
  if (startupPix.isNull()) {
	KMessageBox::error(this, i18n("You do not have Boson data files installed!\n"
			"Please install data package of Boson and restart Boson."), i18n("Data files not found!"));
	exit(1);
	 return;
  }
  setErasePixmap(startupPix);
  setFixedSize(startupPix.size());
  QSpacerItem* spacer_1 = new QSpacerItem( 20, 420, QSizePolicy::Preferred, QSizePolicy::Minimum );
  mMainLayout->addItem( spacer_1 );

  mLowerLayout = new QHBoxLayout( 0, 0, 6, "lowerlayout");

  QHBoxLayout* buttonsLayout = new QHBoxLayout( 0, 0, 6, "buttonslayout");

  mNewGameButton = new QPushButton( this, "newgamebutton" );
  mNewGameButton->setText( i18n( "S&tart new game" ) );
  mNewGameButton->setMinimumWidth(150);
  buttonsLayout->addWidget( mNewGameButton );
  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonsLayout->addItem( spacer_3 );

#ifndef NO_EDITOR
  mEditorButton = new QPushButton( this, "editorbutton" );
  mEditorButton->setText( i18n( "Start &Editor" ) );
  mEditorButton->setMinimumWidth(150);
  buttonsLayout->addWidget( mEditorButton );
  connect(mEditorButton, SIGNAL(clicked()), this, SIGNAL(signalStartEditor()));
#else
  mEditorButton = 0;
#endif

  mQuitButton = new QPushButton( this, "quitbutton" );
  mQuitButton->setText( i18n( "&Quit Boson" ) );
  mQuitButton->setMinimumWidth(150);
  buttonsLayout->addWidget( mQuitButton );
  QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonsLayout->addItem( spacer_4 );
  QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonsLayout->addItem( spacer_5 );

  QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
  QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed );
  mLowerLayout->addItem(spacer_6);
  mLowerLayout->addLayout(buttonsLayout);
  mLowerLayout->addItem(spacer_7);

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

