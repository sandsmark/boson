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

// Loading units datas for one player takes so many loading steps
#define UNITDATAS_LOADINGFACTOR 1600
// Loading map tiles takes so many loading steps
#define MAPTILES_LOADINGFACTOR 2200

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
    void setProgress(int prog);
    void setSteps(int steps);
    
    void showProgressBar(bool show = true);

  protected:
    QVBoxLayout* mBosonLoadingWidgetLayout;

  private:
    QLabel* mHeader;
    QLabel* mPleaseWaitLabel;
    QLabel* mLoadingLabel;
    QProgressBar* mProgress;
};

#endif // BOSONLOADINGWIDGET_H
