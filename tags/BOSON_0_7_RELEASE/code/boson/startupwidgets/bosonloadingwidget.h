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
        SendMap = 0,
        ReceiveMap,
        LoadMap,
        InitClasses,
        LoadTiles,
        LoadUnits,
        LoadGame,
        InitGame,
        StartingGame,
        LoadingDone
      };

    BosonLoadingWidget(QWidget* parent);
    ~BosonLoadingWidget();

    void setLoading(LoadingType load);
    /**
     * See @ref QProgressBase::setProgress
     **/
    void setProgress(int prog);

    /**
     * Set the total steps (i.e. value for @ref setProgress) of the progress
     * bar.
     * @param steps The basic total steps. This does <em>not</em> include the
     * steps for the player datas (such as units, ...)
     * @param playerCount For how many players data will be loaded.
     **/
    void setTotalSteps(int steps, int playerCount);

    void showProgressBar(bool show = true);

    // Loading units datas for one player takes so many loading steps
    static int unitDataLoadingFactor() { return 1600; }

    // Loading map tiles takes so many loading steps
    static int mapTilesLoadingFactor() { return 2200; }

    /**
     * Set the progress for unit loading. This gets called whenever another unit
     * has completed loading for a player.
     * @param baseProgress Where the progress was before unit loading for this player
     * started
     * @param currentUnit The number (1..maxUnit) of the unit that has just
     * completed loading.
     * @param totalUnits The total number of units to-be-loaded for this player.
     **/
    void setUnitProgress(int baseProgress, int currentUnit, int totalUnits)
    {
      setProgress(baseProgress + (int)(((double)currentUnit / totalUnits) * unitDataLoadingFactor()));
    }

    void setTileProgress(int baseProgress, int tiles)
    {
      setProgress(baseProgress + (int)((double)tiles / 1244.0 * mapTilesLoadingFactor()));
    }

  protected:
    /**
     * See @ref QProgressBar::setTotalSteps
     **/
    void setTotalSteps(int steps);

  private:
    QVBoxLayout* mBosonLoadingWidgetLayout;
    QLabel* mHeader;
    QLabel* mPleaseWaitLabel;
    QLabel* mLoadingLabel;
    QProgressBar* mProgress;
};

#endif // BOSONLOADINGWIDGET_H

