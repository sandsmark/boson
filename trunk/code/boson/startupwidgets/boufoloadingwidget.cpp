/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boufoloadingwidget.h"
#include "boufoloadingwidget.moc"

#include <klocale.h>

#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qlayout.h>

BoUfoLoadingWidget::BoUfoLoadingWidget()
    : BoUfoWidget()
{
  setLayoutClass(BoUfoWidget::UVBoxLayout);

  BoUfoLabel* header = new BoUfoLabel();
  addWidget(header);
  // TODO
#if 0
  QFont header_font(header->font());
  header_font.setFamily("verdana");
  header_font.setPointSize(30);
  header_font.setBold(TRUE);
  header->setPaletteForegroundColor( QColor( 255, 255, 255 ));
  header->setFont(header_font);
#endif
#if 0
  header->setAlignment( int( QLabel::AlignCenter ) );
#endif
  header->setText( i18n( "Loading game data..." ) );

  addSpacing(20);

  BoUfoLabel* pleaseWaitLabel = new BoUfoLabel();
  addWidget(pleaseWaitLabel);
  // TODO
#if 0
  pleaseWaitLabel->setPaletteForegroundColor(QColor(255, 255, 255));
#endif
  pleaseWaitLabel->setText(i18n("Please wait while Boson's data is being loaded. This may take some time..."));
  addSpacing(20);

  mLoadingLabel = new BoUfoLabel();
  addWidget(mLoadingLabel);

  mProgress = new BoUfoProgress();
  addWidget(mProgress);
  mProgress->setValue(0.0);

  addSpacing(20);
}

BoUfoLoadingWidget::~BoUfoLoadingWidget()
{
}

void BoUfoLoadingWidget::setLoading(LoadingType load)
{
  mLoadingType = load;
  update();
}

void BoUfoLoadingWidget::setProgress(int prog)
{
  mProgress->setValue((double)prog);
}

void BoUfoLoadingWidget::setTotalSteps(int steps)
{
  mProgress->setRange(0, (double)steps);
}

void BoUfoLoadingWidget::showProgressBar(bool show)
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

void BoUfoLoadingWidget::update()
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
  // TODO
#if 0
  mLoadingLabel->setPaletteForegroundColor( QColor( 255, 255, 255 ));
#endif
}

void BoUfoLoadingWidget::setCurrentTile(int tile)
{
  mCurrentTile = tile;
  update();
}

void BoUfoLoadingWidget::setTotalPlayers(int players)
{
  mTotalPlayers = players;
  updateTotalSteps();
}

void BoUfoLoadingWidget::setCurrentPlayer(int playerindex)
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

void BoUfoLoadingWidget::setTotalUnits(int units)
{
  mTotalUnits = units;
}

void BoUfoLoadingWidget::setCurrentUnit(int unitindex)
{
  mCurrentUnit = unitindex;
  update();
}

void BoUfoLoadingWidget::resetProgress()
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

void BoUfoLoadingWidget::updateTotalSteps()
{
  setTotalSteps(1100 + (mTotalPlayers * 2000));
}
