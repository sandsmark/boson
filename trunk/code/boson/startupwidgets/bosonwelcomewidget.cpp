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

#include "../defines.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kbugreport.h>
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
  QVBoxLayout* widgetLayout = new QVBoxLayout( this, 11, 6, "BosonWelcomeWidgetLayout");

  QVBoxLayout* mainLayout = new QVBoxLayout( 0, 0, 6, "mainlayout"); 

//  mainLayout->addSpacing(10);//FIXME hardcoded
  QPixmap bannerPix(locate("data", "boson/pics/boson-startup-banner.png"));
  QLabel* banner = new QLabel(this);
  banner->setPixmap(bannerPix);
  mainLayout->addWidget(banner, 0, AlignHCenter);

  mainLayout->addSpacing(10);

  QWidget* textFrame = new QWidget(this);
  mainLayout->addWidget(textFrame, 0, AlignHCenter);
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

  QPushButton* newGameButton = new QPushButton( buttonWidget, "newgamebutton" );
  newGameButton->setText( i18n( "S&tart new game" ) );
  newGameButton->setMinimumWidth(120);
  buttonsLayout->addWidget( newGameButton );
  buttonsLayout->addStretch(1);

  QPushButton* loadGameButton = new QPushButton( buttonWidget, "loadgamebutton" );
  loadGameButton->setText( i18n( "&Load saved game" ) );
  loadGameButton->setMinimumWidth(120);
  buttonsLayout->addWidget( loadGameButton );
  buttonsLayout->addStretch(1);

  QPushButton* editorButton = new QPushButton( buttonWidget, "editorbutton" );
  editorButton->setText( i18n( "Start &Editor" ) );
  editorButton->setMinimumWidth(120);
  buttonsLayout->addWidget( editorButton );
  buttonsLayout->addStretch(1);

  QPushButton* quitButton = new QPushButton( buttonWidget, "quitbutton" );
  quitButton->setText( i18n( "&Quit Boson" ) );
  quitButton->setMinimumWidth(120);
  buttonsLayout->addWidget( quitButton );
  buttonsLayout->addStretch(1);

  widgetLayout->addLayout( mainLayout );

  connect(newGameButton, SIGNAL(clicked()), this, SIGNAL(signalNewGame()));
  connect(loadGameButton, SIGNAL(clicked()), this, SIGNAL(signalLoadGame()));
  connect(editorButton, SIGNAL(clicked()), this, SIGNAL(signalStartEditor()));
  connect(quitButton, SIGNAL(clicked()), this, SIGNAL(signalQuit()));

  mBugReport = 0;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
BosonWelcomeWidget::~BosonWelcomeWidget()
{
  delete mBugReport;
}

void BosonWelcomeWidget::slotReportBug()
{
  if (!mBugReport) {
      mBugReport = new KBugReport(this, false);
      connect(mBugReport, SIGNAL(finished()), this, SLOT(slotBugDialogFinished()));
  }
}

