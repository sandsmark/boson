/****************************************************************************
** Form interface generated from reading ui file 'bosonloading.ui'
**
** Created: Wed May 22 16:18:58 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef BOSONLOADINGWIDGET_H
#define BOSONLOADINGWIDGET_H

#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QProgressBar;

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
        LoadParticleSystems,  // Loading particle systems for a player
        LoadUnitConfigs,  // Loading unit config files for a player
        LoadUnits,  // Loading unit models and textures for a player
        LoadTechnologies,  // Loading technologies for a player
        LoadObjects,  // Loading objects for a player
        LoadActions,  // Loading actions for a player
        LoadGeneralData,  // General data loaded only once per game (only general sounds for now)
        LoadSavedGameHeader,  // Loading header of saved game file
        LoadSavedGame,  // Loading saved game data (map and KGame stuff)
        LoadSavedUnits,  // Loading units from saved game file for a player
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
};

#endif // BOSONLOADINGWIDGET_H

