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

#include "bosonstarteditorwidget.h"
#include "bosonstarteditorwidget.moc"

#include "../defines.h"
#include "../bosonconfig.h"
#include "../bosonmessage.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../boson.h"
#include "../top.h"
#include "../bosonplayfield.h"
#include "../bosonmap.h"
#include "../bosonscenario.h"
#include "../cell.h"
#include "bodebug.h"

#include <klocale.h>
#include <knuminput.h>
#include <ksimpleconfig.h>

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qhbox.h>

// AB: this class is a complete fast hack!
// not meant for public use yet

BosonStartEditorWidget::BosonStartEditorWidget(TopWidget* top, QWidget* parent)
    : BosonStartWidgetBase(top, parent)
{
 mTop = top;
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }

 initKGame();
 initPlayer();

 mTopLayout = new QVBoxLayout( this, 11, 6);
 mTopLayout->addSpacing(20);

 QHBoxLayout* mapComboLayout = new QHBoxLayout(mTopLayout);
 QLabel* mapComboLabel = new QLabel(i18n("Map: "), this);
 mapComboLayout->addWidget(mapComboLabel);
 mMapCombo = new QComboBox(this);
 connect(mMapCombo, SIGNAL(activated(int)), this, SLOT(slotSendPlayFieldChanged(int)));
 mapComboLayout->addWidget(mMapCombo);

 // information about the selected map.
 // for new maps the player can enter the infos here, for existing maps we call
 // setEnabled(false) on this.
 mMapBox = new QVGroupBox(i18n("Map Information"), this);
 mTopLayout->addWidget(mMapBox, 1);
 QWidget* map = new QWidget(mMapBox);
 QGridLayout* mapLayout = new QGridLayout(map);

 // AB: should be changeable once map/editor is started, but thats very low
 // priority.
 QLabel* tileSetLabel = new QLabel(i18n("Tileset:"), map);
 mTileSetCombo = new QComboBox(map);
 mapLayout->addWidget(tileSetLabel, 0, 0);
 mapLayout->addWidget(mTileSetCombo, 0, 1);


 // AB: this should be changeable, even when the map/editor is started, but
 // thats very low priority. if the player wants to resize, he's probably
 // creating a mostly new map anyway (but we should support resizing without
 // discarding the map!)
 QLabel* mapWidthLabel = new QLabel(i18n("Map Width:"), map);
 mMapWidth = new KIntNumInput(map);
 mMapWidth->setRange(10, MAX_MAP_WIDTH);
 mapLayout->addWidget(mapWidthLabel, 1, 0);
 mapLayout->addWidget(mMapWidth, 1, 1);

 QLabel* mapHeightLabel = new QLabel(i18n("Map Height:"), map);
 mMapHeight = new KIntNumInput(map);
 mMapHeight->setRange(10, MAX_MAP_HEIGHT);
 mapLayout->addWidget(mapHeightLabel, 2, 0);
 mapLayout->addWidget(mMapHeight, 2, 1);


 // AB: note that once the map/editor is started this must still be changeable!
 // TODO: also add a min players - but that might be more important for the
 // post-map editing stuff.
 QLabel* maxPlayersLabel = new QLabel(i18n("Max Players:"), map);
 mMaxPlayers = new KIntNumInput(map);
 mMaxPlayers->setRange(1, BOSON_MAX_PLAYERS);
 mapLayout->addWidget(maxPlayersLabel, 3, 0);
 mapLayout->addWidget(mMaxPlayers, 3, 1);

 // TODO: species!
 // I guess we shouldn't restrict the user on what species are allowed here.
 // instead we should simply load all available species.
 // -->unfortunately a species must be mostly player-independant for this.
 // especially the models which depend on the teamcolor currently :(


 mTopLayout->addStretch(1); // ok and cancel should be at the button
 QHBoxLayout* startGameLayout = new QHBoxLayout(mTopLayout);
 mCancelButton = new QPushButton(this, "cancelbutton");
 mCancelButton->setText(i18n("&Cancel") );
 connect(mCancelButton, SIGNAL(clicked()), this, SIGNAL(signalCancelled()));
 startGameLayout->addWidget(mCancelButton);
 startGameLayout->addStretch(1);

 mStartGameButton = new QPushButton( this, "startgamebutton" );
 mStartGameButton->setText( i18n( "S&tart Editor" ) );
 startGameLayout->addWidget( mStartGameButton );
 connect(mStartGameButton, SIGNAL(clicked()), this, SLOT(slotStart()));

 initPlayFields();
 initTileSets();
 initSpecies();

 // by default this widget creates a new map
 initNewMap();
}

BosonStartEditorWidget::~BosonStartEditorWidget()
{
}

void BosonStartEditorWidget::initKGame()
{
 // We must manually set maximum players number to some bigger value, because
 //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
 //  players (it's a bug actually)
 boGame->setMaxPlayers(BOSON_MAX_PLAYERS);
 boDebug() << k_funcinfo << " minPlayers(): " << boGame->minPlayers() << endl;
 boDebug() << k_funcinfo << " maxPlayers(): " << boGame->maxPlayers() << endl;

 connect(boGame, SIGNAL(signalPlayFieldChanged(const QString&)), this, SLOT(slotPlayFieldChanged(const QString&)));
}

void BosonStartEditorWidget::initPlayer()
{
 boDebug() << k_funcinfo << "playerCount(): " << boGame->playerCount() << endl;
 player()->setName(boConfig->readLocalPlayerName());
 if (player()->speciesTheme()) {
	boDebug() << k_funcinfo << "Player has speciesTheme already loaded, reloading" << endl;
 }
 QColor playerColor = boConfig->readLocalPlayerColor();
 player()->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), playerColor);
 boGame->addPlayer(player());
}

void BosonStartEditorWidget::slotStart()
{
 BosonPlayField* field = 0;
 if (mMapCombo->currentItem() == 0) {
	field = mTop->playField();
	BosonMap* map = new BosonMap(field);
	BosonScenario* s = new BosonScenario();

	// TODO: fill widget (i.e. fill with grass, water, ... by default)
	map->resize(mMapWidth->value(), mMapHeight->value());
	map->fill((int)Cell::GroundGrass);
	s->setPlayers(mMaxPlayers->value(), mMaxPlayers->value());
	s->initializeScenario();

	field->changeMap(map);
	field->changeScenario(s);

	// ensure that we won't try to load this anymore.
	field->finalizeLoading();
 } else {
	field = BosonPlayField::playField(playFieldIdentifier());
 }
 if (!field) {
	boError() << k_funcinfo << "NULL playfield" << endl;
	return;
 }
 BosonScenario* scenario = field->scenario();
 if (!scenario) {
	boError() << k_funcinfo << "NULL scenario" << endl;
	return;
 }

 for (int i = 1; i < scenario->maxPlayers(); i++) {
	// add dummy computer player
	Player* p = new Player;
	p->setName(i18n("Computer"));
	QColor color = boGame->availableTeamColors().first();
	p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);
	boGame->addPlayer(p);
 }
 sendNewGame();
}

void BosonStartEditorWidget::sendNewGame()
{
 boGame->sendMessage(0, BosonMessage::IdNewEditor);
}

void BosonStartEditorWidget::initPlayFields()
{
 mMapCombo->insertItem(i18n("New Map"));

 QStringList list = BosonPlayField::availablePlayFields();
 for (unsigned int i = 0; i < list.count(); i++) {
	mMapCombo->insertItem(BosonPlayField::playFieldName(list[i]));
 }
 slotSendPlayFieldChanged(0); // by default we create a new map
}

void BosonStartEditorWidget::initTileSets()
{
 QStringList list;
 list.append(QString::fromLatin1("earth"));
 mTileSetCombo->insertStringList(list);
}

void BosonStartEditorWidget::initSpecies()
{
 // TODO: see c'tor
}

void BosonStartEditorWidget::initNewMap()
{
 mMapCombo->setCurrentItem(0);
 mTileSetCombo->setCurrentItem(0);
 mMapWidth->setValue(50);
 mMapHeight->setValue(50);
 mMaxPlayers->setValue(2);
 mMapBox->setEnabled(true);
}

void BosonStartEditorWidget::setCurrentPlayField(BosonPlayField* field)
{
 boDebug() << k_funcinfo << endl;
 if (!field) {
	// create a new map
	initNewMap();
	return;
 }
 mMapBox->setEnabled(false);
 QStringList list = BosonPlayField::availablePlayFields();
 int index = list.findIndex(field->identifier());
 if (index < 0) {
	boError() << k_funcinfo << "invalid index" << endl;
	setCurrentPlayField(0);
	return;
 }
 index++; // index 0 is new map
 mMapCombo->setCurrentItem(index);

 boDebug() << k_funcinfo << "loading map: " << field->identifier() << endl;
 // am afraid we need the entire data here :-(
 field->loadPlayField(QString::null); // QString::null is allowed, as we already opened the file using preLoadPlayField()
 BosonMap* map = field->map();
 BosonScenario* scenario = field->scenario();
 if (!map) {
	boError() << k_funcinfo << "NULL map for " << field->identifier() << endl;
	return;
 }
 if (!scenario) {
	boError() << k_funcinfo << "NULL scenario for " << field->identifier() << endl;
	return;
 }
 mMapWidth->setValue(map->width());
 mMapHeight->setValue(map->height());
 mTileSetCombo->setCurrentItem(0); // TODO - we don't support multiple tilesets yet
 mMaxPlayers->setValue(scenario->maxPlayers());
}

void BosonStartEditorWidget::slotSendPlayFieldChanged(int index)
{
 // index 0 is new map
 BosonStartWidgetBase::slotSendPlayFieldChanged(index - 1);
}

