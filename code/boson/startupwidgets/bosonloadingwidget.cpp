/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonloadingwidget.h"
#include "bosonloadingwidget.moc"

#include <klocale.h>

#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qlayout.h>

/*
 *  Constructs a BosonLoadingWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
BosonLoadingWidget::BosonLoadingWidget(QWidget* parent)
    : QWidget(parent)
{
  mBosonLoadingWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonLoadingWidgetLayout");

  mHeader = new QLabel( this, "header" );
  QFont header_font(  mHeader->font() );
  header_font.setFamily( "verdana" );
  header_font.setPointSize( 30 );
  header_font.setBold( TRUE );
  mHeader->setPaletteForegroundColor( QColor( 255, 255, 255 ));
  mHeader->setFont( header_font );
  mHeader->setText( i18n( "Loading game data..." ) );
  mHeader->setAlignment( int( QLabel::AlignCenter ) );
  mBosonLoadingWidgetLayout->addWidget( mHeader );
  QSpacerItem* spacer = new QSpacerItem( 31, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
  mBosonLoadingWidgetLayout->addItem( spacer );

  QHBoxLayout* layout5 = new QHBoxLayout( 0, 0, 6, "Layout5");
  QSpacerItem* spacer_2 = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
  layout5->addItem( spacer_2 );

  QVBoxLayout* layout4 = new QVBoxLayout( 0, 0, 6, "Layout4");

  mPleaseWaitLabel = new QLabel( this, "pleasewaitlabel" );
  mPleaseWaitLabel->setPaletteForegroundColor( QColor( 255, 255, 255 ));
  mPleaseWaitLabel->setText( i18n( "Please wait while Boson's data is being loaded. This may take some time..." ) );
  layout4->addWidget( mPleaseWaitLabel );
  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
  layout4->addItem( spacer_3 );

  mLoadingLabel = new QLabel( this, "loadinglabel" );
  layout4->addWidget( mLoadingLabel );

  mProgress = new QProgressBar( this, "progress" );
  mProgress->setProgress( 0 );
  layout4->addWidget( mProgress );
  layout5->addLayout( layout4 );
  QSpacerItem* spacer_4 = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
  layout5->addItem( spacer_4 );
  mBosonLoadingWidgetLayout->addLayout( layout5 );
  QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  mBosonLoadingWidgetLayout->addItem( spacer_5 );
}

/*
 *  Destroys the object and frees any allocated resources
 */
BosonLoadingWidget::~BosonLoadingWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

void BosonLoadingWidget::setLoading(LoadingType load)
{
  mLoadingType = load;
  update();
}

void BosonLoadingWidget::setProgress(int prog)
{
  mProgress->setProgress(prog);
  }

void BosonLoadingWidget::setTotalSteps(int steps)
{
  mProgress->setTotalSteps(steps);
}

void BosonLoadingWidget::showProgressBar(bool show)
{
  if(show)
  {
    mProgress->show();
  }
  else
  {
    mProgress->hide();
  }
}

void BosonLoadingWidget::update()
{
  QString text;
  switch(mLoadingType)
  {
    case AdminLoadMap:
    {
      setProgress(0);
      text = i18n("Sending map over network...");
      break;
    }
    case SendMap:
    {
      setProgress(100);
      text = i18n("Sending map over network...");
      break;
    }
    case ReceiveMap:
    {
      setProgress(200);
      text = i18n("Receiving map...");
      break;
    }
    case LoadMap:
    {
      setProgress(300);
      text = i18n("Loading map...");
      break;
    }
    case LoadTiles:
    {
      setProgress(500);
      text = i18n("Loading map tiles...");
      break;
    }
    case LoadEffects:
    {
      setProgress(650);
      text = i18n("Loading effects...");
      break;
    }
    case LoadActions:
    {
      setProgress(700 + (mCurrentPlayer * 2000));
      text = i18n("%1 (actions)...").arg(mLoadingPlayerText);
      break;
    }
    case LoadObjects:
    {
      setProgress(700 + (mCurrentPlayer * 2000) + 25);
      text = i18n("%1 (objects)...").arg(mLoadingPlayerText);
      break;
    }
    case LoadUnitConfigs:
    {
      setProgress(700 + (mCurrentPlayer * 2000) + 150);
      text = i18n("%1 (unit config files)...").arg(mLoadingPlayerText);
      break;
    }
    case LoadUnits:
    {
      setProgress((int)(700 + (mCurrentPlayer * 2000) + 250 + (mCurrentUnit / (float)mTotalUnits * 1650)));
      text = i18n("%1 (unit model %3 of %4)...").arg(mLoadingPlayerText).arg(mCurrentUnit + 1).arg(mTotalUnits);
      break;
    }
    case LoadTechnologies:
    {
      setProgress(700 + (mCurrentPlayer * 2000) + 1950);
      text = i18n("%1 (technologies)...").arg(mLoadingPlayerText);
      break;
    }
    case LoadGeneralData:
    {
      setProgress(700 + (mTotalPlayers * 2000));
      text = i18n("Loading general data...");
      break;
    }
    case LoadWater:
    {
      setProgress(700 + (mTotalPlayers * 2000) + 100);
      text = i18n("Loading water...");
      break;
    }
    case LoadSavedGameHeader:
    {
    }
    case LoadSavedGame:
    {
    }
    case LoadSavedUnits:
    {
      text = i18n("Loading saved game...");
      break;
    }
    case InitGame:
    {
      setProgress(700 + (mTotalPlayers * 2000) + 200);
      text = i18n("Initializing game...");
      break;
    }
    case StartingGame:
    {
      setProgress(700 + (mTotalPlayers * 2000) + 300);
      text = i18n("Starting game...");
      break;
    }
    case LoadingDone:
    {
      setProgress(700 + (mTotalPlayers * 2500) + 400);
      text = i18n("Loading completed.");
      break;
    }
  }
  mLoadingLabel->setText(text);
  mLoadingLabel->setPaletteForegroundColor( QColor( 255, 255, 255 ));
  mLoadingLabel->repaint();
}

void BosonLoadingWidget::setCurrentTile(int tile)
{
  mCurrentTile = tile;
  update();
}

void BosonLoadingWidget::setTotalPlayers(int players)
{
  mTotalPlayers = players;
  updateTotalSteps();
}

void BosonLoadingWidget::setCurrentPlayer(int playerindex)
{
  mCurrentPlayer = playerindex;
  if(mCurrentPlayer == mTotalPlayers - 1)
  {
    // We're loading neutral player
    mLoadingPlayerText = i18n("Loading datas for neutral player");
  }
  else
  {
    // Loading ordinary player. There's  mTotalPlayers - 1  such players (last
    //  one is neutral player)
    mLoadingPlayerText = i18n("Loading datas for player %1 of %2").arg(mCurrentPlayer + 1).arg(mTotalPlayers - 1);
  }
  update();
}

void BosonLoadingWidget::setTotalUnits(int units)
{
  mTotalUnits = units;
}

void BosonLoadingWidget::setCurrentUnit(int unitindex)
{
  mCurrentUnit = unitindex;
  update();
}

void BosonLoadingWidget::resetProgress()
{
  // reset everything
  mTotalPlayers = 0;
  mCurrentPlayer = 0;
  mTotalUnits = 0;
  mCurrentUnit = 0;
  mCurrentTile = 0;
  mAdmin = true;
  mLoading = false;
  setTotalSteps(100);  // for progress to be 0
  setProgress(0);
}

void BosonLoadingWidget::updateTotalSteps()
{
  setTotalSteps(1100 + (mTotalPlayers * 2000));
}
