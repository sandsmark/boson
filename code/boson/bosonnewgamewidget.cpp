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

#include "bosonconfig.h"
#include "bosonmessage.h"
#include "player.h"
#include "speciestheme.h"
#include "bosoncomputerio.h"
#include "boson.h"
#include "top.h"
#include "bosonplayfield.h"
#include "speciestheme.h"
#include "bosonscenario.h"
#include "defines.h"

#include <klocale.h>
#include <kgame/kgameproperty.h>
#include <kgame/kgamechat.h>
#include <kdebug.h>
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

  initKGame();
  initPlayer();

  BosonNewGameWidgetLayout = new QVBoxLayout( this, 11, 6, "BosonNewGameWidgetLayout");

  mainlayout = new QVBoxLayout( 0, 0, 6, "mainlayout");

  header = new QLabel( this, "header" );
  QFont header_font(  header->font() );
  header_font.setPointSize( 30 );
  header_font.setBold( TRUE );
  header->setFont( header_font ); 
  header->setText( i18n( "Start new game" ) );
  header->setAlignment( int( QLabel::AlignCenter ) );
  mainlayout->addWidget( header );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
  mainlayout->addItem( spacer );

  upperlayout = new QHBoxLayout( 0, 0, 6, "upperlayout"); 

  leftlayout = new QVBoxLayout( 0, 0, 6, "leftlayout"); 

  youroptionslayout = new QGridLayout( 0, 1, 1, 0, 6, "youroptionslayout"); 
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  youroptionslayout->addMultiCell( spacer_2, 2, 2, 2, 3 );

  colorlabel = new QLabel( this, "colorlabel" );
  colorlabel->setText( i18n( "Your Color:" ) );

  youroptionslayout->addWidget( colorlabel, 1, 0 );

  maplabel = new QLabel( this, "maplabel" );
  maplabel->setText( i18n( "Map:" ) );

  youroptionslayout->addMultiCellWidget( maplabel, 3, 3, 0, 1 );
  QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  youroptionslayout->addMultiCell( spacer_3, 0, 0, 1, 2 );
  QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  youroptionslayout->addItem( spacer_4, 1, 2 );

  colorcombo = new QComboBox( this, "colorcombo" );

  youroptionslayout->addMultiCellWidget( colorcombo, 1, 1, 3, 4 );

  namelabel = new QLabel( this, "namelabel" );
  namelabel->setText( i18n( "Your name:" ) );

  youroptionslayout->addWidget( namelabel, 0, 0 );

  nameedit = new QLineEdit( this, "nameedit" );
  nameedit->setText(boConfig->readLocalPlayerName());

  youroptionslayout->addWidget( nameedit, 0, 4 );
  QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  youroptionslayout->addItem( spacer_5, 3, 2 );

  speciescombo = new QComboBox( FALSE, this, "speciescombo" );

  youroptionslayout->addWidget( speciescombo, 2, 4 );

  mapcombo = new QComboBox( FALSE, this, "mapcombo" );
  mapcombo->hide();
  mapname = new QLabel(this, "mapname");
  mapname->hide();

  specieslabel = new QLabel( this, "specieslabel" );
  specieslabel->setText( i18n( "Your species:" ) );

  youroptionslayout->addWidget( specieslabel, 2, 0 );
  leftlayout->addLayout( youroptionslayout );
  QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  leftlayout->addItem( spacer_6 );

  addaigroup = new QGroupBox( this, "addaigroup" );
  addaigroup->setTitle( i18n( "Add computer player" ) );
  addaigroup->setColumnLayout(0, Qt::Vertical );
  addaigroup->layout()->setSpacing( 6 );
  addaigroup->layout()->setMargin( 11 );
  addaigroupLayout = new QHBoxLayout( addaigroup->layout() );
  addaigroupLayout->setAlignment( Qt::AlignTop );

  addainamelabel = new QLabel( addaigroup, "addainamelabel" );
  addainamelabel->setText( i18n( "Name:" ) );
  addaigroupLayout->addWidget( addainamelabel );

  addainame = new QLineEdit( addaigroup, "addainame" );
  addainame->setText(boConfig->readComputerPlayerName());
  addaigroupLayout->addWidget( addainame );
  QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  addaigroupLayout->addItem( spacer_7 );

  addaibutton = new QPushButton( addaigroup, "addaibutton" );
  addaibutton->setText( i18n( "Add" ) );
  addaigroupLayout->addWidget( addaibutton );
  leftlayout->addWidget( addaigroup );
  upperlayout->addLayout( leftlayout );
  QSpacerItem* spacer_8 = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
  upperlayout->addItem( spacer_8 );

  playerslayout = new QVBoxLayout( 0, 0, 6, "playerslayout"); 

  playerslabel = new QLabel( this, "playerslabel" );
  QFont playerslabel_font(  playerslabel->font() );
  playerslabel->setFont( playerslabel_font ); 
  playerslabel->setText( i18n( "Connected players:" ) );
  playerslayout->addWidget( playerslabel );

  playerslist = new QListBox( this, "playerslist" );
  playerslayout->addWidget( playerslist );

  removeplayerbutton = new QPushButton( this, "removeplayerbutton" );
  removeplayerbutton->setEnabled( FALSE );
  removeplayerbutton->setText( i18n( "Remove player" ) );
  playerslayout->addWidget( removeplayerbutton );
  upperlayout->addLayout( playerslayout );
  mainlayout->addLayout( upperlayout );

  line1 = new QFrame( this, "line1" );
  line1->setProperty( "frameShape", (int)QFrame::HLine );
  line1->setFrameShadow( QFrame::Sunken );
  line1->setFrameShape( QFrame::HLine );
  mainlayout->addWidget( line1 );

  chatwidget = new KGameChat(0, BosonMessage::IdChat, this );
  chatwidget->setKGame(game());
  chatwidget->setFromPlayer(player());
  mainlayout->addWidget( chatwidget );

  line2 = new QFrame( this, "line2" );
  line2->setProperty( "frameShape", (int)QFrame::HLine );
  line2->setFrameShadow( QFrame::Sunken );
  line2->setFrameShape( QFrame::HLine );
  mainlayout->addWidget( line2 );

  startgamelayout = new QHBoxLayout( 0, 0, 6, "startgamelayout"); 

  cancelbutton = new QPushButton( this, "cancelbutton" );
  cancelbutton->setText( i18n( "Cancel" ) );
  startgamelayout->addWidget( cancelbutton );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  startgamelayout->addItem( spacer_9 );

  serverbutton = new QPushButton( this, "serverbutton" );
  serverbutton->setText( i18n( "Server options" ) );
  startgamelayout->addWidget( serverbutton );
  QSpacerItem* spacer_10 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  startgamelayout->addItem( spacer_10 );

  startgamebutton = new QPushButton( this, "startgamebutton" );
  startgamebutton->setText( i18n( "Start game" ) );
  startgamelayout->addWidget( startgamebutton );
  mainlayout->addLayout( startgamelayout );
  BosonNewGameWidgetLayout->addLayout( mainlayout );
  
  mHighlightedPlayer = 0l;

  initMaps();
  initSpecies();
  initColors();
  mAdmin = !game()->isAdmin(); // hack to make slotSetAdmin() think, that admin status changed
  slotSetAdmin(game()->isAdmin());

  // signals and slots connections
  connect(removeplayerbutton, SIGNAL(clicked()), this, SLOT(slotRemovePlayer()));
  connect(cancelbutton, SIGNAL(clicked()), this, SLOT(slotCancel()));
  connect(serverbutton, SIGNAL(clicked()), this, SLOT(slotServerOptions()));
  connect(startgamebutton, SIGNAL(clicked()), this, SLOT(slotStart()));
  connect(addaibutton, SIGNAL(clicked()), this, SLOT(slotAddAIPlayer()));
  connect(playerslist, SIGNAL(highlighted(QListBoxItem*)), this, SLOT(slotPlayerSelected(QListBoxItem*)));

  connect(nameedit, SIGNAL(returnPressed()), this, SLOT(slotMyNameChanged()));
  connect(colorcombo, SIGNAL(activated(int)), this, SLOT(slotMyColorChanged(int)));
  connect(mapcombo, SIGNAL(activated(int)), this, SLOT(slotMyMapChanged(int)));
  connect(speciescombo, SIGNAL(activated(int)), this, SLOT(slotMySpeciesChanged(int)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
BosonNewGameWidget::~BosonNewGameWidget()
{
  // Save stuff like player name, color etc.
  boConfig->saveLocalPlayerName(nameedit->text());
  boConfig->saveLocalPlayerColor(mPlayercolor);

  boConfig->saveComputerPlayerName(addainame->text());
}

/*****  Init* methods  *****/
void BosonNewGameWidget::initKGame()
{
  connect(game(), SIGNAL(signalPlayerJoinedGame(KPlayer*)), this, SLOT(slotPlayerJoinedGame(KPlayer*)));
  connect(game(), SIGNAL(signalPlayerLeftGame(KPlayer*)), this, SLOT(slotPlayerLeftGame(KPlayer*)));
  connect(game(), SIGNAL(signalPlayFieldChanged(const QString&)), this, SLOT(slotMapChanged(const QString&)));
  connect(game(), SIGNAL(signalSpeciesChanged(Player*)), this, SLOT(slotSpeciesChanged(Player*)));
  connect(game(), SIGNAL(signalTeamColorChanged(Player*)), this, SLOT(slotColorChanged(Player*)));
  
  // We must manually set maximum players number to some bigger value, because
  //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
  //  players (it's a bug actually)
  game()->setMaxPlayers(BOSON_MAX_PLAYERS);
  kdDebug() << k_funcinfo << " minPlayers(): " << game()->minPlayers() << endl;
  kdDebug() << k_funcinfo << " maxPlayers(): " << game()->maxPlayers() << endl;
}

void BosonNewGameWidget::initPlayer()
{
//  player() = new Player;
  /*player()->setName(boConfig->readLocalPlayerName());
  if(player()->speciesTheme())
  {
    kdDebug() << "Speciestheme loaded, id: " << player()->speciesTheme()->identifier() << endl;
    mPlayercolor = player()->teamColor();
  }
  else
  {
    mPlayercolor = boConfig->readLocalPlayerColor();
    player()->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), mPlayercolor);
  }
  game()->addPlayer(player());*/

  kdDebug() << k_funcinfo << "playerCount(): " << game()->playerCount() << endl;
  player()->setName(boConfig->readLocalPlayerName());
  if(player()->speciesTheme())
    kdDebug() << k_funcinfo << "Player has speciesTheme already loaded, reloading" << endl;
  mPlayercolor = boConfig->readLocalPlayerColor();
  player()->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), mPlayercolor);
  game()->addPlayer(player());
}

void BosonNewGameWidget::initMaps()
{
  QStringList list = BosonPlayField::availablePlayFields();
  for (unsigned int i = 0; i < list.count(); i++)
  {
    KSimpleConfig cfg(list[i]);
    cfg.setGroup("Boson PlayField");
    mapcombo->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
//    d->mPlayFieldIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
//    QString fileName = list[i].left(list[i].length() - strlen(".desktop")) + QString::fromLatin1(".bpf");
//    d->mPlayFieldIndex2FileName.insert(i, fileName);
    mMapIndex2Identifier.insert(i, cfg.readEntry("Identifier", i18n("Unknown")));
  }
  mapcombo->setCurrentItem(0);
  mMap = 0;
  if(game()->isAdmin())
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
    speciescombo->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
//    d->mSpeciesIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
    mSpeciesIndex2Identifier.insert(i, cfg.readEntry("Identifier", "Unknown"));
  }
  speciescombo->setCurrentItem(0);
//  slotSpeciesChanged(0);
}

void BosonNewGameWidget::initColors()
{
  mAvailableColors.clear();
  colorcombo->clear();
  mAvailableColors = game()->availableTeamColors();
  mAvailableColors.prepend(player()->speciesTheme()->teamColor());
  for(unsigned int i = 0; i < mAvailableColors.count(); i++)
  {
    QPainter painter;
    QRect rect(0, 0, colorcombo->width(), QFontMetrics(painter.font()).height() + 4);
    QPixmap pixmap(rect.width(), rect.height());
    painter.begin(&pixmap);
    painter.fillRect(rect, QBrush(mAvailableColors[i]));
    painter.end();
    colorcombo->insertItem(pixmap);
  }
}

/*****  slots, where local player changes something  *****/

void BosonNewGameWidget::slotMyNameChanged()
{
  player()->setName(nameedit->text());
}

void BosonNewGameWidget::slotMyColorChanged(int index)
{
  mPlayercolor = mAvailableColors[index];

  QByteArray buffer;
  QDataStream stream(buffer, IO_WriteOnly);
  stream << (Q_UINT32)player()->id();
  stream << (Q_UINT32)mPlayercolor.rgb();
  game()->sendMessage(buffer, BosonMessage::ChangeTeamColor);
}

void BosonNewGameWidget::slotMyMapChanged(int index)
{
  if (!game()->isAdmin())
  {
    kdWarning() << "Only admin can change the map" << endl;
    return;
  }
  if (index >= (int)mMapIndex2Identifier.count())
  {
    kdError() << k_funcinfo << "invalid index " << index << endl;
    return;
  }
  QByteArray buffer;
  QDataStream stream(buffer, IO_WriteOnly);
  // transmit the identifier/name so that the remote newgame dialogs will be able
  // to display the newly selected playfield
  stream << mMapIndex2Identifier[index];
  game()->sendMessage(buffer, BosonMessage::ChangePlayField);
  // Init map to be able to check max/min players count
  kdDebug() << k_funcinfo << " Loading map, index: " << index << ", name: " << playfield() << endl;
  map()->loadPlayField(BosonPlayField::playFieldFileName(playfield()));
  mMinPlayers = map()->scenario()->minPlayers();
  mMaxPlayers = map()->scenario()->maxPlayers();
}

void BosonNewGameWidget::slotMySpeciesChanged(int index)
{
  if (index >= (int)mSpeciesIndex2Identifier.count())
  {
    kdError() << k_funcinfo << "invalid index " << index << endl;
    return;
  }
  QByteArray buffer;
  QDataStream stream(buffer, IO_WriteOnly);
  stream << (Q_UINT32)player()->id();
  stream << mSpeciesIndex2Identifier[index];
  stream << mPlayercolor; //d->game()->availableTeamColors().first().rgb();
  game()->sendMessage(buffer, BosonMessage::ChangeSpecies);
}

/*****  slots where some other player has changed something  *****/

void BosonNewGameWidget::slotPlayerJoinedGame(KPlayer* p)
{
  kdDebug() << k_funcinfo << ": there are " << game()->playerList()->count() << " players in game now" << endl;
  QListBoxText* t = new QListBoxText(p->name());
  mItem2Player.insert(t, p);
  playerslist->insertItem(t);

  connect(p, SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
      this, SLOT(slotPropertyChanged(KGamePropertyBase*, KPlayer*)));

  initColors();
}

void BosonNewGameWidget::slotPlayerLeftGame(KPlayer* p)
{
  kdDebug() << k_funcinfo << ": there are " << game()->playerList()->count() << "players in game now" << endl;
  this->disconnect(p);
  QPtrDictIterator<KPlayer> it(mItem2Player);
  while(it.current())
  {
    if (it.current() == p)
    {
      playerslist->removeItem(playerslist->index((QListBoxItem*)it.currentKey()));
      return;
    }
    ++it;
  }

  initColors();
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
    playerslist->changeItem(t, playerslist->index(old));
    mItem2Player.remove(old);
    mItem2Player.insert(t, p);
  }
}

void BosonNewGameWidget::slotMapChanged(const QString& id)
{
//  mapname->setText(name);
  // id is map identifier, we want to display name
  QMap<int, QString>::iterator it;
  for(it = mMapIndex2Identifier.begin(); it != mMapIndex2Identifier.end(); ++it)
  {
    if(it.data() == id)
    {
      int index = it.key();
      mapname->setText(mapcombo->text(index));
      return;
    }
  }
  kdDebug() << k_funcinfo << " No such map: " << id << endl;
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
  if (!game()->isAdmin())
  {
    // should not happen anyway
    KMessageBox::sorry(this, i18n("Only ADMIN can start a game"));
    return;
  }
  if((int)game()->playerCount() > mMaxPlayers)
  {
    KMessageBox::sorry(this, i18n("There are too many players in game.\n"
        "Current map supports only %1 players, currently, there are %2 players in game.\n"
        "Please remove some players.").arg(mMaxPlayers).arg(game()->playerCount()),
        i18n("Too many players"));
  }
  else if((int)game()->playerCount() < mMinPlayers)
  {
    KMessageBox::sorry(this, i18n("There are too few players in game.\n"
        "Current map requires at least %1 players, currently, there are only %2 players in game.\n"
        "Please add some players.").arg(mMinPlayers).arg(game()->playerCount()),
        i18n("Too few players"));
  }
  else
  {
    slotSendNewGame();
  }
}

void BosonNewGameWidget::slotAddAIPlayer()
{
  if(!game())
  {
    return;
  }
  Player* p = new Player();
  p->setName(addainame->text());
  QColor color = game()->availableTeamColors().first();
  p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);

  BosonComputerIO* io = new BosonComputerIO();
  io->setReactionPeriod(50);
  p->addGameIO(io);
  game()->addPlayer(p);
}

void BosonNewGameWidget::slotCancel()
{
  emit signalCancelled();
}

void BosonNewGameWidget::slotRemovePlayer()
{
  if(mHighlightedPlayer == player())
    slotCancel();
  else
    game()->removePlayer(mHighlightedPlayer);
}

void BosonNewGameWidget::slotServerOptions()
{
  emit signalShowServerOptions();
}

void BosonNewGameWidget::slotPlayerSelected(QListBoxItem* item)
{
  mHighlightedPlayer = mItem2Player[item];
  if(!mHighlightedPlayer)
  {
    removeplayerbutton->setEnabled(false);
    return;
  }
  if(game()->isAdmin() || !mHighlightedPlayer->isVirtual())
    removeplayerbutton->setEnabled(true);
  else
    removeplayerbutton->setEnabled(false);
}

void BosonNewGameWidget::slotSetAdmin(bool admin)
{
  if(admin == mAdmin)
    return;
  if(admin)
  {
    mapcombo->show();
    mapname->hide();
    youroptionslayout->addWidget( mapcombo, 3, 4 );
    startgamebutton->setEnabled(true);
  }
  else
  {
    mapcombo->hide();
    mapname->show();
    youroptionslayout->addWidget( mapname, 3, 4 );
    startgamebutton->setEnabled(false);
  }
  mAdmin = admin;
}

QString& BosonNewGameWidget::playfield()
{
  return mMapIndex2Identifier[mMap];
}

inline Boson* BosonNewGameWidget::game()
{
  return mTop->game();
}

inline Player* BosonNewGameWidget::player()
{
  return mTop->player();
}

inline BosonPlayField* BosonNewGameWidget::map()
{
  return mTop->map();
}

void BosonNewGameWidget::slotSendNewGame() // FIXME: no slot
{
  game()->sendMessage(0, BosonMessage::IdNewGame);
}

