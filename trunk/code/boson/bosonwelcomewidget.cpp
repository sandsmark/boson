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

  mMainLayout = new QVBoxLayout( 0, 0, 6, "mainlayout"); 

//  mMainLayout->addSpacing(10);//FIXME hardcoded
  QPixmap bannerPix(locate("data", "boson/pics/boson-startup-banner.png"));
  mBanner = new QLabel(this);
  mBanner->setPixmap(bannerPix);
  mMainLayout->addWidget(mBanner, 0, AlignHCenter);

  mMainLayout->addSpacing(10);

  QWidget* textFrame = new QWidget(this);
  mMainLayout->addWidget(textFrame, 0, AlignHCenter);
  QVBoxLayout* textFrameLayout = new QVBoxLayout(textFrame, 0, AlignHCenter);
  QPixmap textPixmap(locate("data", "boson/pics/boson-startup-textframe.png"));
  QLabel* textPix = new QLabel(textFrame);
  textPix->setPixmap(textPixmap);
  textFrameLayout->addWidget(textPix, 0, AlignHCenter);
  
  // we fix this to the size of the pixmap!
  textPix->setFixedSize(textPixmap.size());
  textFrame->setFixedWidth(textPixmap.width());

  
  QWidget* buttonWidget = new QWidget(textFrame);
  textFrameLayout->addWidget(buttonWidget, 0, AlignHCenter);
  QHBoxLayout* buttonsLayout = new QHBoxLayout( buttonWidget, 0, 6, "buttonslayout");
  buttonsLayout->addStretch(1);

  mNewGameButton = new QPushButton( buttonWidget, "newgamebutton" );
  mNewGameButton->setText( i18n( "S&tart new game" ) );
  mNewGameButton->setMinimumWidth(120);
  buttonsLayout->addWidget( mNewGameButton );
  buttonsLayout->addStretch(1);

  mLoadGameButton = new QPushButton( buttonWidget, "loadgamebutton" );
  mLoadGameButton->setText( i18n( "&Load saved game" ) );
  mLoadGameButton->setMinimumWidth(120);
  buttonsLayout->addWidget( mLoadGameButton );
  buttonsLayout->addStretch(1);

  mEditorButton = new QPushButton( buttonWidget, "editorbutton" );
  mEditorButton->setText( i18n( "Start &Editor" ) );
  mEditorButton->setMinimumWidth(120);
  buttonsLayout->addWidget( mEditorButton );
  connect(mEditorButton, SIGNAL(clicked()), this, SIGNAL(signalStartEditor()));
  buttonsLayout->addStretch(1);

  mQuitButton = new QPushButton( buttonWidget, "quitbutton" );
  mQuitButton->setText( i18n( "&Quit Boson" ) );
  mQuitButton->setMinimumWidth(120);
  buttonsLayout->addWidget( mQuitButton );
  buttonsLayout->addStretch(1);

  mBosonWelcomeWidgetLayout->addLayout( mMainLayout );

  connect(mNewGameButton, SIGNAL(clicked()), this, SIGNAL(signalNewGame()));
  connect(mLoadGameButton, SIGNAL(clicked()), this, SIGNAL(signalLoadGame()));
  connect(mQuitButton, SIGNAL(clicked()), this, SIGNAL(signalQuit()));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BosonWelcomeWidget::~BosonWelcomeWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

