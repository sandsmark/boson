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
#ifndef BOSONLOADINGWIDGET_H
#define BOSONLOADINGWIDGET_H

#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QProgressBar;
class QString;

class BosonLoadingWidget : public QWidget
{
  Q_OBJECT
  public:
    enum LoadingType
      {
        AdminLoadMap = 0,  // Admin loads map and saves it to stream
        SendMap,  // Map stream is sent to network (admin only)
        ReceiveMap,  // Map is being received over the net
        LoadMap,  // Map has been received and is being loaded
        LoadTiles,  // Map tiles (textures) // AB: obsolete!
        LoadEffects,  // Loading effects
        LoadUnitConfigs,  // Loading unit config files for a player
        LoadUnits,  // Loading unit models and textures for a player
        LoadTechnologies,  // Loading technologies for a player
        LoadObjects,  // Loading objects for a player
        LoadActions,  // Loading actions for a player
        LoadGeneralData,  // General data loaded only once per game (only general sounds for now)
        LoadSavedGameHeader,  // Loading header of saved game file
        LoadSavedGame,  // Loading saved game data (map and KGame stuff)
        LoadSavedUnits,  // Loading units from saved game file for a player
        LoadWater,  // Loading water (textures)
        InitGame,  // Game is being initialized (InitFogOfWar and StartScenario messages)
        StartingGame,  // Last phase, BosonWidget is about to being shown
        LoadingDone  // All done
      };

    BosonLoadingWidget(QWidget* parent);
    ~BosonLoadingWidget();

    void setLoading(LoadingType load);

    void showProgressBar(bool show = true);

    // Loading units datas for one player takes so many loading steps
    static int unitDataLoadingFactor() { return 1600; }

    // Loading map tiles takes so many loading steps
    static int mapTilesLoadingFactor() { return 2200; }

    void setCurrentTile(int tile);

    void setTotalPlayers(int players);
    void setCurrentPlayer(int playerindex);

    void setTotalUnits(int units);
    void setCurrentUnit(int unitindex);

    void resetProgress();

    void setAdmin(bool isAdmin)  { mAdmin = isAdmin; };
    void setLoading(bool isLoading)  { mLoading = isLoading; };

  protected:
    /**
     * See @ref QProgressBar::setTotalSteps
     **/
    void setTotalSteps(int steps);

    /**
     * See @ref QProgressBase::setProgress
     **/
    void setProgress(int prog);

    void update();

    void updateTotalSteps();

  private:
    QVBoxLayout* mBosonLoadingWidgetLayout;
    QLabel* mHeader;
    QLabel* mPleaseWaitLabel;
    QLabel* mLoadingLabel;
    QProgressBar* mProgress;
    int mTotalPlayers;
    int mCurrentPlayer;
    int mTotalUnits;
    int mCurrentUnit;
    int mCurrentTile;
    bool mAdmin;
    bool mLoading;
    LoadingType mLoadingType;
    QString mLoadingPlayerText;
};

#endif // BOSONLOADINGWIDGET_H

