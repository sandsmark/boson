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

#include "bosonnewgamewidget.h"
#include "bosonnewgamewidget.moc"

#include "../defines.h"
#include "../bosonconfig.h"
#include "../bosonmessage.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../bosoncomputerio.h"
#include "../boson.h"
#include "../top.h"
#include "../bosonplayfield.h"
#include "../speciestheme.h"
#include "../bosonscenario.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgameproperty.h>
#include <kgame/kgamechat.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>

#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>

/*
 *  Constructs a BosonNewGameWidget which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
BosonNewGameWidget::BosonNewGameWidget(TopWidget* top, QWidget* parent)
    : QWidget(parent)
{
  mTop = top;

  if (!boGame)
  {
    boError() << k_funcinfo << "NULL Boson object" << endl;
    return;
  }

  initKGame();
  initPlayer();

  mBosonNewGameWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonNewGameWidgetLayout");

  mMainLayout = new QVBoxLayout( 0, 0, 6, "mainlayout");

  //FIXME!
  /*
  QLabel* header = new QLabel( this, "header" );
  QFont header_font(  header->font() );
  header_font.setPointSize( 30 );
  header_font.setBold( TRUE );
  header->setFont( header_font ); 
  header->setText( i18n( "Start New Game" ) );
  header->setAlignment( int( QLabel::AlignCenter ) );
  mMainLayout->addWidget( header );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
  mMainLayout->addItem( spacer );
  */

  mUpperLayout = new QHBoxLayout( 0, 0, 6, "upperlayout"); 

  mLeftLayout = new QVBoxLayout( 0, 0, 6, "eftlayout"); 


  mYourOptionsLayout = new QGridLayout( 0, 1, 1, 0, 6, "youroptionslayout"); 
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mYourOptionsLayout->addMultiCell( spacer_2, 2, 2, 2, 3 );

  mNameLabel = new QLabel( this, "namelabel" );
  mNameLabel->setText( i18n( "Your Name:" ) );
  mYourOptionsLayout->addWidget( mNameLabel, 0, 0 );

  mColorLabel = new QLabel( this, "colorlabel" );
  mColorLabel->setText( i18n( "Your Color:" ) );
  mYourOptionsLayout->addWidget( mColorLabel, 1, 0 );

  mSpeciesLabel = new QLabel( this, "specieslabel" );
  mSpeciesLabel->setText( i18n( "Your Species:" ) );
  mYourOptionsLayout->addWidget( mSpeciesLabel, 2, 0 );

  mMapLabel = new QLabel( this, "maplabel" );
  mMapLabel->setText( i18n( "Map:" ) );
  mYourOptionsLayout->addWidget( mMapLabel, 3, 0 );

  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mYourOptionsLayout->addMultiCell( spacer_3, 0, 0, 1, 2 );
  QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mYourOptionsLayout->addItem( spacer_4, 1, 2 );

  mNameEdit = new QLineEdit( this, "nameedit" );
  mNameEdit->setText(boConfig->readLocalPlayerName());
  mYourOptionsLayout->addWidget( mNameEdit, 0, 4 );

  mColorCombo = new QComboBox( this, "colorcombo" );
  mYourOptionsLayout->addWidget( mColorCombo, 1, 4 );

  QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mYourOptionsLayout->addItem( spacer_5, 3, 2 );

  mSpeciesCombo = new QComboBox( FALSE, this, "speciescombo" );
  mYourOptionsLayout->addWidget( mSpeciesCombo, 2, 4 );

  mMapCombo = new QComboBox( FALSE, this, "mapcombo" );
  mMapCombo->hide();
  mMapName = new QLabel(this, "mapname");
  mMapName->hide();


  mLeftLayout->addLayout( mYourOptionsLayout );
  QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  mLeftLayout->addItem( spacer_6 );

  mAddAIGroup = new QGroupBox( this, "addaigroup" );
  mAddAIGroup->setTitle( i18n( "Add Computer Player" ) );
  mAddAIGroup->setColumnLayout(0, Qt::Vertical );
  mAddAIGroup->layout()->setSpacing( 6 );
  mAddAIGroup->layout()->setMargin( 11 );
  mAddAIGroupLayout = new QHBoxLayout( mAddAIGroup->layout() );
  mAddAIGroupLayout->setAlignment( Qt::AlignTop );

  mAddAINameLabel = new QLabel( mAddAIGroup, "addainamelabel" );
  mAddAINameLabel->setText( i18n( "Name:" ) );
  mAddAIGroupLayout->addWidget( mAddAINameLabel );

  mAddAIName = new QLineEdit( mAddAIGroup, "addainame" );
  mAddAIName->setText(boConfig->readComputerPlayerName());
  mAddAIGroupLayout->addWidget( mAddAIName );
  QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mAddAIGroupLayout->addItem( spacer_7 );

  mAddAIButton = new QPushButton( mAddAIGroup, "addaibutton" );
  mAddAIButton->setText( i18n( "&Add" ) );
  mAddAIGroupLayout->addWidget( mAddAIButton );
  mLeftLayout->addWidget( mAddAIGroup );
  mUpperLayout->addLayout( mLeftLayout );
  QSpacerItem* spacer_8 = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
  mUpperLayout->addItem( spacer_8 );

  mPlayersLayout = new QVBoxLayout( 0, 0, 6, "playerslayout"); 

  mPlayersLabel = new QLabel( this, "playerslabel" );
  QFont playerslabel_font(  mPlayersLabel->font() );
  mPlayersLabel->setFont( playerslabel_font ); 
  mPlayersLabel->setText( i18n( "Connected Players:" ) );
  mPlayersLayout->addWidget( mPlayersLabel );

  mPlayersList = new QListBox( this, "playerslist" );
  mPlayersLayout->addWidget( mPlayersList );

  mRemovePlayerButton = new QPushButton( this, "removeplayerbutton" );
  mRemovePlayerButton->setEnabled( FALSE );
  mRemovePlayerButton->setText( i18n( "&Remove Player" ) );
  mPlayersLayout->addWidget( mRemovePlayerButton );
  mUpperLayout->addLayout( mPlayersLayout );
  mMainLayout->addLayout( mUpperLayout );
/*
  mLine1 = new QFrame( this, "AIne1" );
  mLine1->setProperty( "frameShape", (int)QFrame::HLine );
  mLine1->setFrameShadow( QFrame::Sunken );
  mLine1->setFrameShape( QFrame::HLine );
  mMainLayout->addWidget( mLine1 );
  */

  mChatWidget = new KGameChat(0, BosonMessage::IdChat, this );
  mChatWidget->setKGame(boGame);
  mChatWidget->setFromPlayer(player());
  mMainLayout->addWidget( mChatWidget );

  mLine2 = new QFrame( this, "line2" );
  mLine2->setProperty( "frameShape", (int)QFrame::HLine );
  mLine2->setFrameShadow( QFrame::Sunken );
  mLine2->setFrameShape( QFrame::HLine );
  mMainLayout->addWidget( mLine2 );

  mStartGameLayout = new QHBoxLayout( 0, 0, 6, "startgamelayout"); 

  mCancelButton = new QPushButton( this, "cancelbutton" );
  mCancelButton->setText( i18n( "&Cancel" ) );
  mStartGameLayout->addWidget( mCancelButton );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mStartGameLayout->addItem( spacer_9 );

  mNetworkButton = new QPushButton( this, "networkbutton" );
  mNetworkButton->setText( i18n( "&Network Options" ) );
  mStartGameLayout->addWidget( mNetworkButton );
  QSpacerItem* spacer_10 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  mStartGameLayout->addItem( spacer_10 );

  mStartGameButton = new QPushButton( this, "startgamebutton" );
  mStartGameButton->setText( i18n( "S&tart Game" ) );
  mStartGameLayout->addWidget( mStartGameButton );
  mMainLayout->addLayout( mStartGameLayout );
  mBosonNewGameWidgetLayout->addLayout( mMainLayout );
  
  mHighlightedPlayer = 0l;

  initMaps();
  initSpecies();
  initColors();
  mAdmin = !boGame->isAdmin(); // hack to make slotSetAdmin() think, that admin status changed
  slotSetAdmin(boGame->isAdmin());

  // signals and slots connections
  connect(mRemovePlayerButton, SIGNAL(clicked()), this, SLOT(slotRemovePlayer()));
  connect(mCancelButton, SIGNAL(clicked()), this, SLOT(slotCancel()));
  connect(mNetworkButton, SIGNAL(clicked()), this, SLOT(slotNetworkOptions()));
  connect(mStartGameButton, SIGNAL(clicked()), this, SLOT(slotStart()));
  connect(mAddAIButton, SIGNAL(clicked()), this, SLOT(slotAddAIPlayer()));
  connect(mPlayersList, SIGNAL(highlighted(QListBoxItem*)), this, SLOT(slotPlayerSelected(QListBoxItem*)));

  connect(mNameEdit, SIGNAL(returnPressed()), this, SLOT(slotMyNameChanged()));
  connect(mColorCombo, SIGNAL(activated(int)), this, SLOT(slotMyColorChanged(int)));
  connect(mMapCombo, SIGNAL(activated(int)), this, SLOT(slotMyMapChanged(int)));
  connect(mSpeciesCombo, SIGNAL(activated(int)), this, SLOT(slotMySpeciesChanged(int)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
BosonNewGameWidget::~BosonNewGameWidget()
{
  // Save stuff like player name, color etc.
  boConfig->saveLocalPlayerName(mNameEdit->text());
  boConfig->saveLocalPlayerColor(mPlayercolor);

  boConfig->saveComputerPlayerName(mAddAIName->text());
}

/*****  Init* methods  *****/
void BosonNewGameWidget::initKGame()
{
  connect(boGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)), this, SLOT(slotPlayerJoinedGame(KPlayer*)));
  connect(boGame, SIGNAL(signalPlayerLeftGame(KPlayer*)), this, SLOT(slotPlayerLeftGame(KPlayer*)));
  connect(boGame, SIGNAL(signalPlayFieldChanged(const QString&)), this, SLOT(slotMapChanged(const QString&)));
  connect(boGame, SIGNAL(signalSpeciesChanged(Player*)), this, SLOT(slotSpeciesChanged(Player*)));
  connect(boGame, SIGNAL(signalTeamColorChanged(Player*)), this, SLOT(slotColorChanged(Player*)));
  
  // We must manually set maximum players number to some bigger value, because
  //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
  //  players (it's a bug actually)
  boGame->setMaxPlayers(BOSON_MAX_PLAYERS);
  boDebug() << k_funcinfo << " minPlayers(): " << boGame->minPlayers() << endl;
  boDebug() << k_funcinfo << " maxPlayers(): " << boGame->maxPlayers() << endl;
}

void BosonNewGameWidget::initPlayer()
{
//  player() = new Player;
  /*player()->setName(boConfig->readLocalPlayerName());
  if(player()->speciesTheme())
  {
    boDebug() << "Speciestheme loaded, id: " << player()->speciesTheme()->identifier() << endl;
    mPlayercolor = player()->teamColor();
  }
  else
  {
    mPlayercolor = boConfig->readLocalPlayerColor();
    player()->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), mPlayercolor);
  }
  boGame->addPlayer(player());*/

  boDebug() << k_funcinfo << "playerCount(): " << boGame->playerCount() << endl;
  player()->setName(boConfig->readLocalPlayerName());
  if(player()->speciesTheme())
  {
    boDebug() << k_funcinfo << "Player has speciesTheme already loaded, reloading" << endl;
  }
  mPlayercolor = boConfig->readLocalPlayerColor();
  player()->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), mPlayercolor);
  boGame->addPlayer(player());
}

void BosonNewGameWidget::initMaps()
{
  QStringList list = BosonPlayField::availablePlayFields();
  for (unsigned int i = 0; i < list.count(); i++)
  {
    KSimpleConfig cfg(list[i]);
    cfg.setGroup("Boson PlayField");
    mMapCombo->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
//    d->mPlayFieldIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
//    QString fileName = list[i].left(list[i].length() - strlen(".boson")) + QString::fromLatin1(".bpf");
//    d->mPlayFieldIndex2FileName.insert(i, fileName);
    mMapIndex2Identifier.insert(i, cfg.readEntry("Identifier", i18n("Unknown")));
  }
  mMapCombo->setCurrentItem(0);
  mMap = 0;
  if(boGame->isAdmin())
    slotMyMapChanged(0);
}

void BosonNewGameWidget::initSpecies()
{
  // update possible species:
/*  d->mPlayerSpecies->clear();
  d->mSpeciesIndex2Comment.clear();
  d->mSpeciesIndex2Identifier.clear();*/
  //TODO: some scenarios might not provide all species!
  QStringList list = SpeciesTheme::availableSpecies();
  for (unsigned int i = 0; i < list.count(); i++) {
    KSimpleConfig cfg(list[i]);
    cfg.setGroup("Boson Species");
    mSpeciesCombo->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
//    d->mSpeciesIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
    mSpeciesIndex2Identifier.insert(i, cfg.readEntry("Identifier", "Unknown"));
  }
  mSpeciesCombo->setCurrentItem(0);
//  slotSpeciesChanged(0);
}

void BosonNewGameWidget::initColors()
{
  mAvailableColors.clear();
  mColorCombo->clear();
  mAvailableColors = boGame->availableTeamColors();
  mAvailableColors.prepend(player()->speciesTheme()->teamColor());
  for(unsigned int i = 0; i < mAvailableColors.count(); i++)
  {
    QPainter painter;
    QRect rect(0, 0, mColorCombo->width(), QFontMetrics(painter.font()).height() + 4);
    QPixmap pixmap(rect.width(), rect.height());
    painter.begin(&pixmap);
    painter.fillRect(rect, QBrush(mAvailableColors[i]));
    painter.end();
    mColorCombo->insertItem(pixmap);
  }
}

/*****  slots, where local player changes something  *****/

void BosonNewGameWidget::slotMyNameChanged()
{
  if (mNameEdit->text() != player()->name()) 
  {
    player()->setName(mNameEdit->text());
  }
}

void BosonNewGameWidget::slotMyColorChanged(int index)
{
  mPlayercolor = mAvailableColors[index];

  QByteArray buffer;
  QDataStream stream(buffer, IO_WriteOnly);
  stream << (Q_UINT32)player()->id();
  stream << (Q_UINT32)mPlayercolor.rgb();
  boGame->sendMessage(buffer, BosonMessage::ChangeTeamColor);
}

void BosonNewGameWidget::slotMyMapChanged(int index)
{
  if (!boGame->isAdmin())
  {
    boWarning() << "Only admin can change the map" << endl;
    return;
  }
  if (index >= (int)mMapIndex2Identifier.count())
  {
    boError() << k_funcinfo << "invalid index " << index << endl;
    return;
  }
  QByteArray buffer;
  QDataStream stream(buffer, IO_WriteOnly);
  // transmit the identifier/name so that the remote newgame dialogs will be able
  // to display the newly selected playfield
  stream << mMapIndex2Identifier[index];
  boGame->sendMessage(buffer, BosonMessage::ChangePlayField);
}

void BosonNewGameWidget::slotMySpeciesChanged(int index)
{
  if (index >= (int)mSpeciesIndex2Identifier.count())
  {
    boError() << k_funcinfo << "invalid index " << index << endl;
    return;
  }
  QByteArray buffer;
  QDataStream stream(buffer, IO_WriteOnly);
  stream << (Q_UINT32)player()->id();
  stream << mSpeciesIndex2Identifier[index];
  stream << mPlayercolor; //d->boGame->availableTeamColors().first().rgb();
  boGame->sendMessage(buffer, BosonMessage::ChangeSpecies);
}

/*****  slots where some other player has changed something  *****/

void BosonNewGameWidget::slotPlayerJoinedGame(KPlayer* p)
{
  boDebug() << k_funcinfo << "there are " << boGame->playerList()->count() << " players in game now" << endl;
  QListBoxText* t = new QListBoxText(p->name());
  mItem2Player.insert(t, p);
  mPlayersList->insertItem(t);

  connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
      this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*)));

  initColors();
}

void BosonNewGameWidget::slotPlayerLeftGame(KPlayer* p)
{
  boDebug() << k_funcinfo << "there are " << boGame->playerList()->count() << " players in game now" << endl;
  this->disconnect(p);
  QPtrDictIterator<KPlayer> it(mItem2Player);
  while(it.current())
  {
    if (it.current() == p)
    {
      mPlayersList->removeItem(mPlayersList->index((QListBoxItem*)it.currentKey()));
      initColors();
      return;
    }
    ++it;
  }
}

void BosonNewGameWidget::slotPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
  if(prop->id() == KGamePropertyBase::IdName)
  {
    QListBoxText* old = 0;
    QPtrDictIterator<KPlayer> it(mItem2Player);
    while (it.current() && !old)
    {
      if(it.current() == p)
      {
        //((QListBoxText*)it.currentKey())->setText(p->name());
        old = (QListBoxText*)it.currentKey();
      }
      ++it;
    }
    QListBoxText* t = new QListBoxText(p->name());
    mPlayersList->changeItem(t, mPlayersList->index(old));
    mItem2Player.remove(old);
    mItem2Player.insert(t, p);
  }
}

void BosonNewGameWidget::slotMapChanged(const QString& id)
{
//  mMapName->setText(name);
  // id is map identifier, we want to display name
  QMap<int, QString>::iterator it;
  for(it = mMapIndex2Identifier.begin(); it != mMapIndex2Identifier.end(); ++it)
  {
    if(it.data() == id)
    {
      int index = it.key();
      mMap = index;
      mMapName->setText(mMapCombo->text(index));
      if(boGame->isAdmin())
      {
        // Init map to be able to check max/min players count
        boDebug() << k_funcinfo << " Loading map, index: " << index << ", name: " << playFieldString() << endl;
        playField()->loadPlayField(BosonPlayField::playFieldFileName(playFieldString()));
        mMinPlayers = playField()->scenario()->minPlayers();
        mMaxPlayers = playField()->scenario()->maxPlayers();
      }
      return;
    }
  }
  boDebug() << k_funcinfo << " No such map: " << id << endl;
}

void BosonNewGameWidget::slotSpeciesChanged(Player*)
{
}

void BosonNewGameWidget::slotColorChanged(Player*)
{
  initColors();
}

/*****  other stuff  *****/

void BosonNewGameWidget::slotStart()
{
  if (!boGame->isAdmin())
  {
    // should not happen anyway
    KMessageBox::sorry(this, i18n("Only ADMIN can start a game"));
    return;
  }
  if((int)boGame->playerCount() > mMaxPlayers)
  {
    KMessageBox::sorry(this, i18n("There are too many players in game.\n"
        "Current map supports only %1 players, currently, there are %2 players in game.\n"
        "Please remove some players.").arg(mMaxPlayers).arg(boGame->playerCount()),
        i18n("Too many players"));
  }
  else if((int)boGame->playerCount() < mMinPlayers)
  {
    KMessageBox::sorry(this, i18n("There are too few players in game.\n"
        "Current map requires at least %1 players, currently, there are only %2 players in game.\n"
        "Please add some players.").arg(mMinPlayers).arg(boGame->playerCount()),
        i18n("Too few players"));
  }
  else
  {
    sendNewGame();
  }
}

void BosonNewGameWidget::slotAddAIPlayer()
{
  if(!boGame)
  {
    return;
  }
  Player* p = new Player();
  p->setName(mAddAIName->text());
  QColor color = boGame->availableTeamColors().first();
  p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);

  BosonComputerIO* io = new BosonComputerIO();
  io->setReactionPeriod(50);
  p->addGameIO(io);
  boGame->addPlayer(p);
}

void BosonNewGameWidget::slotCancel()
{
  emit signalCancelled();
}

void BosonNewGameWidget::slotRemovePlayer()
{
  if(mHighlightedPlayer == player())
  {
    slotCancel();
  }
  else
  {
    boGame->removePlayer(mHighlightedPlayer);
  }
}

void BosonNewGameWidget::slotNetworkOptions()
{
  emit signalShowNetworkOptions();
}

void BosonNewGameWidget::slotPlayerSelected(QListBoxItem* item)
{
  mHighlightedPlayer = mItem2Player[item];
  if(!mHighlightedPlayer)
  {
    mRemovePlayerButton->setEnabled(false);
    return;
  }
  if(boGame->isAdmin() || !mHighlightedPlayer->isVirtual())
  {
    mRemovePlayerButton->setEnabled(true);
  }
  else
  {
    mRemovePlayerButton->setEnabled(false);
  }
}

void BosonNewGameWidget::slotSetAdmin(bool admin)
{
  if(admin == mAdmin)
  {
    return;
  }
  if(admin)
  {
    mMapCombo->show();
    mMapName->hide();
    mYourOptionsLayout->addWidget( mMapCombo, 3, 4 );
    mStartGameButton->setEnabled(true);
  }
  else
  {
    mMapCombo->hide();
    mMapName->show();
    mYourOptionsLayout->addWidget( mMapName, 3, 4 );
    mStartGameButton->setEnabled(false);
  }
  mAdmin = admin;
}

QString BosonNewGameWidget::playFieldString() const
{
  return mMapIndex2Identifier[mMap];
}

Player* BosonNewGameWidget::player() const
{
  return mTop->player();
}

BosonPlayField* BosonNewGameWidget::playField() const
{
  return mTop->playField();
}

void BosonNewGameWidget::sendNewGame()
{
  slotMyNameChanged();
  boGame->sendMessage(0, BosonMessage::IdNewGame);
}

