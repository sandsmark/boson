/****************************************************************************
** Form implementation generated from reading ui file 'bosonloading.ui'
**
** Created: Wed May 22 16:18:59 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "bosonloadingwidget.h"
#include "bosonloadingwidget.moc"

#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <klocale.h>

/* 
 *  Constructs a BosonLoadingWidget which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
BosonLoadingWidget::BosonLoadingWidget(QWidget* parent)
    : QWidget(parent)
{
  mBosonLoadingWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonLoadingWidgetLayout");

  mLogoSpacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
  mBosonLoadingWidgetLayout->addItem(mLogoSpacer);
	
  mHeader = new QLabel( this, "header" );
  QFont header_font(  mHeader->font() );
  header_font.setPointSize( 30 );
  header_font.setBold( TRUE );
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
  mPleaseWaitLabel->setText( i18n( "Please wait while Boson's data is being loaded. This may take some time..." ) );
  layout4->addWidget( mPleaseWaitLabel );
  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
  layout4->addItem( spacer_3 );

  mLoadingLabel = new QLabel( this, "loadinglabel" );
  mLoadingLabel->setText( i18n( "Loading ..." ) );
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
  if(load == SendMap)
  {
    mLoadingLabel->setText(i18n("Sending map over network..."));
  }
  else if(load == ReceiveMap)
  {
    mLoadingLabel->setText(i18n("Receiving map..."));
  }
  else if(load == InitClasses)
  {
    mLoadingLabel->setText(i18n("Initializing data structures"));
  }
  else if(load == LoadTiles)
  {
    mLoadingLabel->setText(i18n("Loading map tiles..."));
  }
  else if(load == LoadUnits)
  {
    mLoadingLabel->setText(i18n("Loading units..."));
  }
  else if(load == InitGame)
  {
    mLoadingLabel->setText(i18n("Initializing game..."));
  }
  else if(load == StartingGame)
  {
    mLoadingLabel->setText(i18n("Starting game..."));
  }
  else if(load == LoadingDone)
  {
    mLoadingLabel->setText(i18n("Loading completed, starting game..."));
  }
}

void BosonLoadingWidget::setProgress(int prog)
{
  mProgress->setProgress(prog);
}

void BosonLoadingWidget::setSteps(int steps)
{
  mProgress->setTotalSteps(steps);
}

void BosonLoadingWidget::setLogoSpacer(int height)
{
  mLogoSpacer->changeSize( 20, height, QSizePolicy::Minimum, QSizePolicy::Fixed );
}
