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
#include "../bosonmap.h"
#include "../bosongroundtheme.h"
#include "../bosondata.h"
#include "../bosonsaveload.h"
#include "../bosonwidgets/bosonplayfieldview.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <klistbox.h>
#include <knuminput.h>
#include <ktextbrowser.h>
#include <klineedit.h>

#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qslider.h>
#include <qptrdict.h>
#include <qvaluelist.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

class BosonNewEditorWidgetPrivate
{
public:
	BosonNewEditorWidgetPrivate()
	{
		mSelectedMap = 0;
	}

	QPtrDict<KPlayer> mItem2Player;
	QMap<QListViewItem*, QString> mItem2Map;

	QMap<int, QString> mSpeciesIndex2Identifier;
	QMap<int, QString> mSpeciesIndex2Comment;

	BosonPlayField* mSelectedMap;
};


BosonNewEditorWidget::BosonNewEditorWidget(BosonStartupNetwork* interface, QWidget* parent)
    : BosonNewEditorWidgetBase(parent, "bosonneweditorwidget")
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BosonNewEditorWidgetPrivate;
 mNetworkInterface = interface;

 initPlayFields();
 initGroundThemes();

 connect(networkInterface(), SIGNAL(signalStartGameClicked()),
		this, SLOT(slotNetStart()));
 connect(networkInterface(), SIGNAL(signalPlayFieldChanged(BosonPlayField*)),
		this, SLOT(slotNetPlayFieldChanged(BosonPlayField*)));

 connect(mCancelButton, SIGNAL(clicked()), this, SIGNAL(signalCancelled()));
 connect(mStartButton, SIGNAL(clicked()), this, SLOT(slotStartClicked()));

 // AB: this widget isn't the ideal place for this...
 initKGame();
}

BosonNewEditorWidget::~BosonNewEditorWidget()
{
 QString playFieldIdentifier;
 if (mSelectMap->currentItem()) {
	if (d->mItem2Map.contains(mSelectMap->currentItem())) {
		playFieldIdentifier = d->mItem2Map[mSelectMap->currentItem()];
	}
 }
 boConfig->saveEditorMap(playFieldIdentifier);
 boConfig->saveEditorCreateNewMap(mCreateNewMap->isChecked());
 delete d;
}

void BosonNewEditorWidget::slotStartClicked()
{
 boDebug() << k_funcinfo << endl;
 // FIXME: it's not start _game_
 networkInterface()->sendStartGameClicked();
}

void BosonNewEditorWidget::initKGame()
{
 // We must manually set maximum players number to some bigger value, because
 //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
 //  players (it's a bug actually)
 boGame->setMaxPlayers(BOSON_MAX_PLAYERS);
 boDebug() << k_funcinfo << " minPlayers(): " << boGame->minPlayers() << endl;
 boDebug() << k_funcinfo << " maxPlayers(): " << boGame->maxPlayers() << endl;

}

void BosonNewEditorWidget::initPlayFields()
{
 boDebug() << k_funcinfo << endl;
 QStringList list = boData->availablePlayFields();
 boDebug() << k_funcinfo << list.count() << endl;
 for (unsigned int i = 0; i < list.count(); i++) {
	if (!boData->playField(list[i])) {
		boWarning() << k_funcinfo << "NULL playField " << list[i] << endl;
		continue;
	}
	QListViewItem* item = new QListViewItem(mSelectMap);
	item->setText(0, boData->playField(list[i])->playFieldName());
	mSelectMap->insertItem(item);
	d->mItem2Map.insert(item, list[i]);
 }

 // Load whether to create new map or edit existing one
 bool createnew = boConfig->readEditorCreateNewMap();
 boDebug() << k_funcinfo << "createnew: " << createnew << endl;
 if (createnew) {
	mCreateNewMap->setChecked(true);
	slotCreateNewToggled(true);
 } else {
	mCreateNewMap->setChecked(false);
	slotCreateNewToggled(false);
	// Load selected map
	QString mapId = boConfig->readEditorMap();
	if (mapId.isNull() || !list.contains(mapId)) {
		// fallback to default map
		mapId = BosonPlayField::defaultPlayField();
	}
	boDebug() << k_funcinfo << "mapId: " << mapId << endl;
	networkInterface()->sendChangePlayField(mapId);
 }
}

void BosonNewEditorWidget::initGroundThemes()
{
 // AB: atm we use identifiers for the combobox only. one day we may want to add
 // names for them, too
 QStringList list = BosonData::bosonData()->availableGroundThemes();
 mGroundTheme->insertStringList(list);
 slotGroundThemeChanged(mGroundTheme->currentItem());
}

void BosonNewEditorWidget::slotNetStart()
{
 BO_CHECK_NULL_RET(boGame);
 boDebug() << k_funcinfo << endl;
 BosonPlayField* field = 0;
 int maxPlayers = 0;
 // WARNING: we have one EVIL assumption here. We assume that the current
 // playfield is actually the currently *selected* playfield!
 // we mustn't assume this for network - for editor it isn't terrible, but it
 // may be possible, that after clicking start editor the player selected
 // another playfield and so we would now add players for the new playfield, but
 // start the old one.
 // We should use the actual playfield from the network here!!

 if (boGame->playerCount() > 0) {
	boError() << k_funcinfo << "playerCount must be 0! trying to clear" << endl;
	boGame->quitGame();
 }
 if (boGame->playerCount() > 0) {
	boError() << k_funcinfo << "playerCount must be 0! clearning did not succeed" << endl;
	return;
 }

 QByteArray newMap;
 if (mCreateNewMap->isChecked()) {
	newMap = createNewMap();
	if (newMap.size() == 0) {
		boError() << k_funcinfo << "could not create new map" << endl;
		KMessageBox::sorry(this, i18n("An error occured while creating a new map"));
		return;
	}
	maxPlayers = mMaxPlayers->value();
 } else {
	// Editing old map
	if (!mSelectMap->currentItem()) {
		boError() << k_funcinfo << "NULL current item" << endl;
		return;
	}
	QString playFieldIdentifier = d->mItem2Map[mSelectMap->currentItem()];
	field = boData->playField(playFieldIdentifier);
	if (!field) {
		boError() << k_funcinfo << "NULL playfield" << endl;
		return;
	}
	maxPlayers = field->information()->maxPlayers();
 }

 QValueList<QColor> availableTeamColors = boGame->availableTeamColors();
 if ((int)availableTeamColors.count() < maxPlayers) {
	boError() << k_funcinfo << "too many players - not enough team colors!" << endl;
	KMessageBox::sorry(this, i18n("Too many (max-)players. Not enough colors available (internal error)."));
	return;
 }
 boDebug() << k_funcinfo << "adding " << maxPlayers << " players" << endl;
 for (int i = 0; i < maxPlayers; i++) {
	// add dummy computer player
	Player* p = new Player;
	p->setName(i18n("Player %1").arg(i + 1));
	QColor color = availableTeamColors.first();
	availableTeamColors.pop_front();
	p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);
	boGame->bosonAddPlayer(p);
 }

 networkInterface()->sendNewGame(field, true, &newMap);
}


void BosonNewEditorWidget::slotNetPlayFieldChanged(BosonPlayField* field)
{
 boDebug() << k_funcinfo << endl;

 if (!field) {
	// New map was selected and playfield was reset. Ignore it
	return;
 }

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
	mSelectMap->setCurrentItem(item);
 }


 BO_CHECK_NULL_RET(field->description());
 boDebug() << k_funcinfo << "id: " << field->identifier() << endl;
 QStringList list = boData->availablePlayFields();

 const BosonPlayFieldInformation* information = field->information();
 BPFDescription* description = field->description();

 // AB: I am not fully sure if a text browser is the right choice for this
 // widget. but being able to use links in the description is surely a good
 // idea.
 if (description->comment().isEmpty()) {
	mMapDescription->setText(i18n("There is no comment for this map available"));
 } else {
	mMapDescription->setText(description->comment());
 }

 mWidth->setValue(information->mapWidth());
 mWidthNum->setValue(information->mapWidth());
 mHeight->setValue(information->mapHeight());
 mHeightNum->setValue(information->mapHeight());
 mGroundTheme->setCurrentItem(0); // TODO - we do not yet support more than one :(
 mMaxPlayers->setValue(information->maxPlayers());
 mMaxPlayersNum->setValue(information->maxPlayers());

 d->mSelectedMap = field;
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

void BosonNewEditorWidget::slotGroundThemeChanged(int)
{
 // we don't transmit over network.

 mFilling->clear();
 QStringList groundThemes = BosonData::bosonData()->availableGroundThemes();
 int themeIndex = mGroundTheme->currentItem();
 if (themeIndex < 0 || (unsigned int)themeIndex >= groundThemes.count()) {
	KMessageBox::sorry(this, i18n("Invalid groundTheme index %1").arg(themeIndex));
	return;
 }
 QString themeId = groundThemes[themeIndex];
 BosonGroundTheme* theme = boData->groundTheme(themeId);
 if (!theme) {
	BO_NULL_ERROR(theme);
	KMessageBox::sorry(this, i18n("An error occured while loading the selected groundtheme"));
	return;
 }

 for (unsigned int i = 0; i < theme->textureCount(); i++) {
	mFilling->insertItem(theme->textureFileName(i));
 }
}

void BosonNewEditorWidget::slotMaxPlayersChanged(int)
{
 // we don't transmit over network.
}

void BosonNewEditorWidget::slotWidthChanged(int)
{
 // we don't transmit over network.
}

void BosonNewEditorWidget::slotHeightChanged(int)
{
 // we don't transmit over network.
}

void BosonNewEditorWidget::slotNewMapToggled(bool isNewMap)
{
 boDebug() << k_funcinfo << "isNewMap: " << isNewMap << endl;
 mNewMapName->setEnabled(isNewMap);
 mSelectMap->setEnabled(!isNewMap);

 if (isNewMap) {
	mHeight->setValue(50);
	mHeightNum->setValue(50);
	mWidth->setValue(50);
	mWidthNum->setValue(50);
	mMaxPlayers->setValue(2);
	mMaxPlayersNum->setValue(2);
	mGroundTheme->setCurrentItem(0);
	mFilling->setCurrentItem(0);
	mMapDescription->setText(i18n("Enter description here"));
 } else if (d->mSelectedMap) {
	BosonMap* map = d->mSelectedMap->map();
	const BosonPlayFieldInformation* information = d->mSelectedMap->information();
	BPFDescription* description = d->mSelectedMap->description();

	BO_CHECK_NULL_RET(information);
	BO_CHECK_NULL_RET(description);

	if (description->comment().isEmpty()) {
		mMapDescription->setText(i18n("There is no comment for this map available"));
	} else {
		mMapDescription->setText(description->comment());
	}
	mWidth->setValue(information->mapWidth());
	mWidthNum->setValue(information->mapWidth());
	mHeight->setValue(information->mapHeight());
	mHeightNum->setValue(information->mapHeight());
	mGroundTheme->setCurrentItem(0); // TODO - we do not yet support more than one :(
	mMaxPlayers->setValue(information->maxPlayers());
	mMaxPlayersNum->setValue(information->maxPlayers());
 }

 mHeight->setEnabled(isNewMap);
 mHeightNum->setEnabled(isNewMap);
 mWidth->setEnabled(isNewMap);
 mWidthNum->setEnabled(isNewMap);
 mMaxPlayers->setEnabled(isNewMap);
 mMaxPlayersNum->setEnabled(isNewMap);
 mGroundTheme->setEnabled(isNewMap);
 mFilling->setEnabled(isNewMap);

 mMapDescription->setReadOnly(!isNewMap);

 if (isNewMap) {
	// Reset playfield
	networkInterface()->sendChangePlayField(QString::null);
 }

 boDebug() << k_funcinfo << "DONE" << endl;
}

void BosonNewEditorWidget::slotCreateNewToggled(bool checked)
{
 mEditExistingMap->setChecked(!checked);
 slotNewMapToggled(checked);
}

void BosonNewEditorWidget::slotEditExistingToggled(bool checked)
{
 mCreateNewMap->setChecked(!checked);
 slotNewMapToggled(!checked);
}

QByteArray BosonNewEditorWidget::createNewMap()
{
 boDebug() << k_funcinfo << endl;
 unsigned int width = mWidth->value();
 unsigned int height = mHeight->value();
 if (!BosonMap::isValidMapGeo(width, height)) {
	boError() << k_funcinfo << "invalid map geo" << endl;
	KMessageBox::sorry(this, i18n("The desired map geo is not valid\nWidth=%1\nHeight=%2").arg(width).arg(height));
	return QByteArray();
 }
 QStringList groundThemes = BosonData::bosonData()->availableGroundThemes();
 int themeIndex = mGroundTheme->currentItem();
 if (themeIndex < 0 || (unsigned int)themeIndex >= groundThemes.count()) {
	boError() << k_funcinfo << "invalid theme index " << themeIndex << endl;
	if (themeIndex < 0) {
		KMessageBox::sorry(this, i18n("Please select a ground theme first"));
	} else {
		KMessageBox::sorry(this, i18n("The selected groundTheme at index %1 could not be found. %2 themes are available").arg(themeIndex).arg(groundThemes.count()));
	}
	return QByteArray();
 }
 QString themeId = boData->availableGroundThemes()[themeIndex];
 BosonGroundTheme* theme = boData->groundTheme(themeId);
 if (!theme) {
	BO_NULL_ERROR(theme);
	KMessageBox::sorry(this, i18n("An error occured while loading the selected groundtheme"));
	return QByteArray();
 }
 BosonPlayField playField;
 BosonMap* map = new BosonMap(0);
 playField.changeMap(map); // takes ownership and will delete it!
 if (!map->createNewMap(width, height, theme)) {
	boError() << k_funcinfo << "map could not be created" << endl;
	return QByteArray();
 }
 unsigned int texture = mFilling->currentItem();
 if (texture >= theme->textureCount()) {
	boError() << k_funcinfo << "invalid texture " << texture << endl;
	KMessageBox::sorry(this, i18n("Could not fill the map with texture %1 - only %2 textures in groundTheme %3").arg(texture).arg(theme->textureCount()).arg(themeId));
	return QByteArray();
 }
 map->fill(texture);

 // we add dummy players in order to save them into the bytearray.
 int maxPlayers = mMaxPlayers->value();
 if (maxPlayers < 2) {
	boError() << k_funcinfo << "maxPlayers < 2 does not make sense" << endl;
	KMessageBox::sorry(this, i18n("Max Players is an invalid value: %1").arg(maxPlayers));
	return QByteArray();
 }
 if (maxPlayers > BOSON_MAX_PLAYERS) {
	boError() << k_funcinfo << "maxPlayers > " << BOSON_MAX_PLAYERS << " is not allowed" << endl;
	KMessageBox::sorry(this, i18n("Max Players is an invalid value: %1 (must be < %2)").arg(maxPlayers).arg(BOSON_MAX_PLAYERS));
	return QByteArray();
 }

 BosonSaveLoad* save = new BosonSaveLoad(boGame);
 save->setPlayField(&playField);
 boDebug() << k_funcinfo << "saving to playfield completed" << endl;
 QMap<QString, QByteArray> files;
 if (!save->saveToFiles(files)) {
	delete save;
	boError() << k_funcinfo << "error occured while saving" << endl;
	KMessageBox::sorry(this, i18n("An error occured while saving the a game to a stream"));
	return QByteArray();
 }
 boDebug() << k_funcinfo << "saving completed" << endl;
 playField.changeMap(0); // deletes the map!

 QDomDocument playersDoc(QString::fromLatin1("Players"));
 QDomDocument canvasDoc(QString::fromLatin1("Canvas"));
 QDomElement playersRoot = playersDoc.createElement(QString::fromLatin1("Players"));
 QDomElement canvasRoot = canvasDoc.createElement(QString::fromLatin1("Canvas"));
 playersDoc.appendChild(playersRoot);
 canvasDoc.appendChild(canvasRoot);
 canvasRoot.appendChild(canvasDoc.createElement(QString::fromLatin1("DataHandler")));
 canvasRoot.appendChild(canvasDoc.createElement(QString::fromLatin1("Effects")));
 for (int i = 0; i < maxPlayers + 1; i++) {
	QDomElement p = playersDoc.createElement(QString::fromLatin1("Player"));
	p.setAttribute("PlayerId", i);
	p.appendChild(playersDoc.createElement(QString::fromLatin1("Upgrades")));
	playersRoot.appendChild(p);

	QDomElement items = canvasDoc.createElement(QString::fromLatin1("Items"));
	items.setAttribute("PlayerId", i);
	canvasRoot.appendChild(items);

	if (i == maxPlayers) {
		p.setAttribute("IsNeutral", 1);
	}
 }
 QDomElement canvasEventListener = canvasDoc.createElement(QString::fromLatin1("EventListener"));
 canvasRoot.appendChild(canvasEventListener);
 canvasEventListener.appendChild(canvasDoc.createElement(QString::fromLatin1("Conditions")));
 files.insert("players.xml", playersDoc.toCString());
 files.insert("canvas.xml", canvasDoc.toCString());

 BPFDescription desc;
 desc.setName(mNewMapName->text());
 files.insert("C/description.xml", desc.toString().utf8());
 files.insert("scripts/eventlistener/game.py", QByteArray());
 files.insert("scripts/eventlistener/localplayer.py", QByteArray());
 for (unsigned int i = 0; i < maxPlayers + 1; i++) {
	files.insert(QString("scripts/eventlistener/ai-player_%1.py").arg(i), QByteArray());
 }

 QByteArray b = BosonPlayField::streamFiles(files);
 boDebug() << k_funcinfo << "files got streamed" << endl;
 return b;
}

