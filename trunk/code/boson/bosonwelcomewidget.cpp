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

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

/*
 *  Constructs a BosonWelcomeWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
BosonWelcomeWidget::BosonWelcomeWidget(QWidget* parent) : QWidget(parent)
{
  mBosonWelcomeWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonWelcomeWidgetLayout");

  // 6 + 60 + 15 == logo == 81

  mMainLayout = new QVBoxLayout( 0, 0, 6, "mainlayout"); 

  mMainLayout->addSpacing(10);//FIXME hardcoded
  QPixmap bannerPix(locate("data", "boson/pics/boson-startup-banner.png"));
  mBanner = new QLabel(this);
  mBanner->setPixmap(bannerPix);
  mMainLayout->addWidget(mBanner);

  // 66 + 25 + 72 == 163

  mMainLayout->addSpacing(33);
  QPixmap textFramePix(locate("data", "boson/pics/boson-startup-textframe.png"));
  mTextFrame = new QLabel(this);
  mTextFrame->setPixmap(textFramePix);
  mTextFrame->setFixedSize(textFramePix.size());
  mMainLayout->addWidget(mTextFrame);

  // 163 + 33 + 248 == 444

  mMainLayout->addSpacing(20);


  QVBoxLayout* textFrameLayout = new QVBoxLayout( mTextFrame, 15, 6, "textframelayout" );
  textFrameLayout->addStretch(1);
  textFrameLayout->addStretch(1);
  textFrameLayout->addStretch(1);
  QHBoxLayout* lowerLayout = new QHBoxLayout( textFrameLayout, -1, "lowerlayout");
  textFrameLayout->addStretch(1);
  QHBoxLayout* buttonsLayout = new QHBoxLayout( 0, 0, 6, "buttonslayout");

  mNewGameButton = new QPushButton( mTextFrame, "newgamebutton" );
  mNewGameButton->setText( i18n( "S&tart new game" ) );
  mNewGameButton->setMinimumWidth(150);
  buttonsLayout->addWidget( mNewGameButton, 0, 0 );
  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonsLayout->addItem( spacer_3 );

#ifndef NO_EDITOR
  mEditorButton = new QPushButton( mTextFrame, "editorbutton" );
  mEditorButton->setText( i18n( "Start &Editor" ) );
  mEditorButton->setMinimumWidth(150);
  buttonsLayout->addWidget( mEditorButton );
  connect(mEditorButton, SIGNAL(clicked()), this, SIGNAL(signalStartEditor()));
#else
  mEditorButton = 0;
#endif

  mQuitButton = new QPushButton( mTextFrame, "quitbutton" );
  mQuitButton->setText( i18n( "&Quit Boson" ) );
  mQuitButton->setMinimumWidth(150);
  buttonsLayout->addWidget( mQuitButton );
  QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonsLayout->addItem( spacer_4 );
  QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonsLayout->addItem( spacer_5 );

  lowerLayout->addStretch(1);
  lowerLayout->addLayout(buttonsLayout);
  lowerLayout->addStretch(1);


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

