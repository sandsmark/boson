/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonneweditorwidget.h"
#include "bosonneweditorwidget.moc"

#include "../defines.h"
#include "../bosonconfig.h"
#include "../bosonmessage.h"
#include "../player.h"
#include "../speciestheme.h"
#include "../bosoncomputerio.h"
#include "../boson.h"
#include "../bosonplayfield.h"
#include "../bpfdescription.h"
#include "../bosonscenario.h"
#include "../bosonmap.h"
#include "../cell.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgameproperty.h>
#include <kgame/kgamechat.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <klistbox.h>
#include <ktextbrowser.h>
#include <kgamemisc.h>

#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qslider.h>

class BosonNewEditorWidgetPrivate
{
public:
	BosonNewEditorWidgetPrivate()
	{
	}

	QPtrDict<KPlayer> mItem2Player;
	QMap<QListViewItem*, QString> mItem2Map;

	QMap<int, QString> mSpeciesIndex2Identifier;
	QMap<int, QString> mSpeciesIndex2Comment;
};


BosonNewEditorWidget::BosonNewEditorWidget(BosonStartupNetwork* interface, QWidget* parent)
    : BosonNewEditorWidgetBase(parent, "bosonneweditorwidget")
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BosonNewEditorWidgetPrivate;
 mNetworkInterface = interface;

 initSpecies();
 initPlayFields();
 initTilesets();

 connect(networkInterface(), SIGNAL(signalStartGameClicked()),
		this, SLOT(slotNetStart()));
 connect(networkInterface(), SIGNAL(signalPlayFieldChanged(BosonPlayField*)),
		this, SLOT(slotNetPlayFieldChanged(BosonPlayField*)));
}

BosonNewEditorWidget::~BosonNewEditorWidget()
{
 delete d;
}

void BosonNewEditorWidget::initPlayFields()
{
 QStringList list = BosonPlayField::availablePlayFields();
 boDebug() << k_funcinfo << list.count() << endl;
 for (unsigned int i = 0; i < list.count(); i++) {
	QListViewItem* item = new QListViewItem(mChooseBosonMap);
	item->setText(0, BosonPlayField::playFieldName(list[i]));
	mChooseBosonMap->insertItem(item);
	d->mItem2Map.insert(item, list[i]);
 }
 QListViewItem* item = new QListViewItem(mChooseBosonMap);
 item->setText(0, i18n("New map"));
 mChooseBosonMap->insertItem(item);
 d->mItem2Map.insert(item, QString::null);

 networkInterface()->sendChangePlayField(QString::null);
}

void BosonNewEditorWidget::initSpecies()
{
// well... we should allow selecting species in this widget. or not? dunno...
// anyway we should allow changing the species during the editor, so it won't be
// important to allow that here.
}

void BosonNewEditorWidget::initTilesets()
{
 QStringList list;
 list.append(QString::fromLatin1("earth"));
 mChangeTileset->insertStringList(list);
}

void BosonNewEditorWidget::slotNetStart()
{
 BosonPlayField* field = 0;
 int maxPlayers = 0;
 // WARNING: we have one EVIL assumption here. We assume that the current
 // playfield is actually the currently *selected* playfield!
 // we mustn't assume this for network - for editor it isn't terrible, but it
 // may be possible, that after clicking start editor the player selected
 // another playfield and so we would now add players for the new playfield, but
 // start the old one.
 // We should use the actual playfield from the network here!!
 if (!mChooseBosonMap->currentItem()) {
	boError() << k_funcinfo << "NULL current item" << endl;
	return;
 }
 QString playFieldIdentifier = d->mItem2Map[mChooseBosonMap->currentItem()];

 if (playFieldIdentifier.isNull()) { // "New Map" selected
	BosonMap* map = new BosonMap(0);

	// TODO: fill widget (i.e. fill with grass, water, ... by default)
	map->resize(mChangeMaxWidth->value(), mChangeMaxHeight->value());
	map->fill((int)Cell::GroundGrass);

	QByteArray b;
	QDataStream stream(b, IO_WriteOnly);
	map->saveMap(stream);

	// WARNING: this is a hack! the message should contain the *map* only,
	// not the scenario. I do not yet know how the scenario will be handled
	// and if the maxplayers input will remain in this widget (we could
	// start with a single player and further players in the editor itself).
	maxPlayers = mChangeMaxPlayers->value();
	stream << (Q_INT32)maxPlayers;
	stream << (Q_INT32)maxPlayers;

	boGame->sendMessage(b, BosonMessage::ChangeMap);

	delete map;
 } else {
	field = BosonPlayField::playField(playFieldIdentifier);
	if (!field) {
		boError() << k_funcinfo << "NULL playfield" << endl;
		return;
	}
	BosonScenario* scenario = field->scenario();
	if (!scenario) {
		boError() << k_funcinfo << "NULL scenario" << endl;
		return;
	}
	maxPlayers = scenario->maxPlayers();
 }

 QValueList<QColor> availableTeamColors = boGame->availableTeamColors();
 if ((int)availableTeamColors.count() < maxPlayers) {
	boError() << k_funcinfo << "too many players - not enough team colors!" << endl;
	KMessageBox::sorry(this, i18n("Too many (max-)players. Not enough colors available (internal error)."));
	return;
 }
 for (int i = 0; i < maxPlayers; i++) {
	// add dummy computer player
	Player* p = new Player;
	p->setName(i18n("Player %1").arg(i + 1));
	QColor color = availableTeamColors.first();
	availableTeamColors.pop_front();
	p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);
	boGame->addPlayer(p);
 }
 networkInterface()->sendNewGame(true);
}


void BosonNewEditorWidget::slotNetPlayFieldChanged(BosonPlayField* field)
{
 boDebug() << k_funcinfo << endl;
 if (!field) {
	slotNewMapToggled(true);
	return;
 }
 BO_CHECK_NULL_RET(field->scenario());
 BO_CHECK_NULL_RET(field->description());
 boDebug() << k_funcinfo << "id: " << field->identifier() << endl;
 slotNewMapToggled(false);
 QStringList list = BosonPlayField::availablePlayFields();
 QMap<QListViewItem*, QString>::Iterator it;
 QListViewItem* item = 0;
 for (it = d->mItem2Map.begin(); it != d->mItem2Map.end() && !item; ++it) {
	if (it.data() == field->identifier()) {
		item = it.key();
	}
 }
 if (!item) {
	boError() << k_funcinfo << "Cannot find playfield item for " << field->identifier() << endl;
 } else {
	mChooseBosonMap->setCurrentItem(item);
 }

 boDebug() << k_funcinfo << "loading map: " << field->identifier() << endl;
 // am afraid we need the entire data here :-(
 field->loadPlayField(QString::null); // QString::null is allowed, as we already opened the file using preLoadPlayField()
 BO_CHECK_NULL_RET(field->map());

 BosonMap* map = field->map();
 BosonScenario* scenario = field->scenario();
 BPFDescription* description = field->description();

 // AB: I am not fully sure if a text browser is the right choice for this
 // widget. but being able to use links in the description is surely a good
 // idea.
 if (description->comment().isEmpty()) {
	mMapPropertiesTextBrowser->setText(i18n("There is no comment for this map available"));
 } else {
	mMapPropertiesTextBrowser->setText(description->comment());
 }

 mChangeMaxWidth->setValue(map->width());
 mChangeMaxHeight->setValue(map->height());
 mChangeTileset->setCurrentItem(0); // TODO - we do not yet support more than one :(
 mChangeMaxPlayers->setValue(scenario->maxPlayers());
}

void BosonNewEditorWidget::slotPlayFieldChanged(QListViewItem* item)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(item);
 if (!d->mItem2Map.contains(item)) {
	boWarning() << k_funcinfo << "invalid item" << endl;
	return;
 }
 networkInterface()->sendChangePlayField(d->mItem2Map[item]);
}

void BosonNewEditorWidget::slotTilesetChanged(int)
{
 boDebug() << k_funcinfo << "only one tileset supported currently :(" << endl;
}

void BosonNewEditorWidget::slotMaxPlayersChanged(int)
{
}

void BosonNewEditorWidget::slotMaxWidthChanged(int)
{
}

void BosonNewEditorWidget::slotMaxHeightChanged(int)
{
}

void BosonNewEditorWidget::slotNewMapToggled(bool isNewMap)
{
 if (isNewMap) {
	mChangeMaxHeight->setValue(50);
	mChangeMaxWidth->setValue(50);
	mChangeMaxPlayers->setValue(2);
	mChangeTileset->setCurrentItem(0);

	// FIXME
//	mMapCombo->setCurrentItem(0);
 }
 mMapSettingsGroupBox->setEnabled(isNewMap);
}

