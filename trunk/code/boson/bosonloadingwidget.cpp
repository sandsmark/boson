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
  BosonLoadingWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonLoadingWidgetLayout");

  header = new QLabel( this, "header" );
  QFont header_font(  header->font() );
  header_font.setPointSize( 30 );
  header_font.setBold( TRUE );
  header->setFont( header_font ); 
  header->setText( i18n( "Loading game data..." ) );
  header->setAlignment( int( QLabel::AlignCenter ) );
  BosonLoadingWidgetLayout->addWidget( header );
  QSpacerItem* spacer = new QSpacerItem( 31, 20, QSizePolicy::Minimum, QSizePolicy::Fixed );
  BosonLoadingWidgetLayout->addItem( spacer );

  Layout5 = new QHBoxLayout( 0, 0, 6, "Layout5"); 
  QSpacerItem* spacer_2 = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
  Layout5->addItem( spacer_2 );

  Layout4 = new QVBoxLayout( 0, 0, 6, "Layout4"); 

  pleasewaitlabel = new QLabel( this, "pleasewaitlabel" );
  pleasewaitlabel->setText( i18n( "Please wait while Boson's data is being loaded. This may take some time..." ) );
  Layout4->addWidget( pleasewaitlabel );
  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
  Layout4->addItem( spacer_3 );

  loadinglabel = new QLabel( this, "loadinglabel" );
  loadinglabel->setText( i18n( "Loading ..." ) );
  Layout4->addWidget( loadinglabel );

  progress = new QProgressBar( this, "progress" );
  progress->setProgress( 0 );
  Layout4->addWidget( progress );
  Layout5->addLayout( Layout4 );
  QSpacerItem* spacer_4 = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
  Layout5->addItem( spacer_4 );
  BosonLoadingWidgetLayout->addLayout( Layout5 );
  QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  BosonLoadingWidgetLayout->addItem( spacer_5 );
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
    loadinglabel->setText(i18n("Sending map over network..."));
  else if(load == ReceiveMap)
    loadinglabel->setText(i18n("Receiving map..."));
  else if(load == InitClasses)
    loadinglabel->setText(i18n("Initializing data structures"));
  else if(load == LoadTiles)
    loadinglabel->setText(i18n("Loading map tiles..."));
  else if(load == InitGame)
    loadinglabel->setText(i18n("Initializing game..."));
  else if(load == StartingGame)
    loadinglabel->setText(i18n("Starting game..."));
  else if(load == LoadingDone)
    loadinglabel->setText(i18n("Loading completed, starting game..."));
}

void BosonLoadingWidget::setProgress(int prog)
{
  progress->setProgress(prog);
}

void BosonLoadingWidget::setSteps(int steps)
{
  progress->setTotalSteps(steps);
}
