/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)
    Copyright (C) 2002-2008 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "boufostarteditorwidget.h"
#include "boufostarteditorwidget.moc"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "../bosonconfig.h"
#include "../gameengine/bosonmessageids.h"
#include "../gameengine/player.h"
#include "../gameengine/speciestheme.h"
#include "../gameengine/bosoncomputerio.h"
#include "../gameengine/boson.h"
#include "../gameengine/bosonplayfield.h"
#include "../gameengine/bpfdescription.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/bosongroundtheme.h"
#include "../bosondata.h"
#include "../gameengine/bosonsaveload.h"
#include "../gameengine/bpfloader.h" // BPFPreview. FIXME: dedicated file!
#include "../gameview/bosonlocalplayerinput.h" // ugly. we should not include stuff from the gameview in here.
#include "bosonstartupnetwork.h"
#include "bocreatenewmap.h"
#include "bodebug.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <q3ptrdict.h>
#include <qmap.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3ValueList>

class BoUfoStartEditorWidgetPrivate
{
public:
	BoUfoStartEditorWidgetPrivate()
	{
		mSelectedMap = 0;
	}

	Q3PtrDict<KPlayer> mItem2Player;
	QMap<int, QString> mIndex2Map;

	QMap<int, QString> mSpeciesIndex2Identifier;
	QMap<int, QString> mSpeciesIndex2Comment;

	BPFPreview* mSelectedMap;

	Q3ValueList<int> mMapSizes;
};


BoUfoStartEditorWidget::BoUfoStartEditorWidget(BosonStartupNetwork* interface)
	: BoUfoStartEditorWidgetBase()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BoUfoStartEditorWidgetPrivate;
 mNetworkInterface = interface;

 initPlayFields();
 initGroundThemes();
 initMapSizes();

 connect(networkInterface(), SIGNAL(signalStartGameClicked()),
		this, SLOT(slotNetStart()));
 connect(networkInterface(), SIGNAL(signalPlayFieldChanged(BPFPreview*)),
		this, SLOT(slotNetPlayFieldChanged(BPFPreview*)));

 // AB: this widget isn't the ideal place for this...
 initKGame();
}

BoUfoStartEditorWidget::~BoUfoStartEditorWidget()
{
 QString playFieldIdentifier;
 if (mSelectMap->selectedItem() >= 0) {
	if (d->mIndex2Map.contains(mSelectMap->selectedItem())) {
		playFieldIdentifier = d->mIndex2Map[mSelectMap->selectedItem()];
	}
 }
 boConfig->saveEditorMap(playFieldIdentifier);
 boConfig->saveEditorCreateNewMap(mCreateNewMap->selected());
 delete d;
}

void BoUfoStartEditorWidget::slotStartClicked()
{
 boDebug() << k_funcinfo << endl;
 // FIXME: it's not start _game_
 networkInterface()->sendStartGameClicked();
}

void BoUfoStartEditorWidget::initKGame()
{
 // We must manually set maximum players number to some bigger value, because
 //  KGame in KDE 3.0.0 (what about 3.0.1?) doesn't support infinite number of
 //  players (it's a bug actually)
 boGame->setMaxPlayers(BOSON_MAX_PLAYERS);
 boDebug() << k_funcinfo << " minPlayers(): " << boGame->minPlayers() << endl;
 boDebug() << k_funcinfo << " maxPlayers(): " << boGame->maxPlayers() << endl;

}

void BoUfoStartEditorWidget::initPlayFields()
{
 boDebug() << k_funcinfo << endl;
 QStringList list = boData->availablePlayFields();
 boDebug() << k_funcinfo << list.count() << endl;
 for (int i = 0; i < list.count(); i++) {
	if (!boData->playFieldPreview(list[i])) {
		boWarning() << k_funcinfo << "NULL playFieldpreview " << list[i] << endl;
		continue;
	}
	if (!boData->playFieldPreview(list[i])->description()) {
		boWarning() << k_funcinfo << "NULL description in playFieldpreview " << list[i] << endl;
		continue;
	}
	int index = mSelectMap->count();
	mSelectMap->insertItem(boData->playFieldPreview(list[i])->description()->name());
	d->mIndex2Map.insert(index, list[i]);
 }

 // Load whether to create new map or edit existing one
 bool createnew = boConfig->readEditorCreateNewMap();
 boDebug() << k_funcinfo << "createnew: " << createnew << endl;
 if (createnew) {
	mCreateNewMap->setSelected(true);
	slotCreateNewToggled(true);
 } else {
	mCreateNewMap->setSelected(false);
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

void BoUfoStartEditorWidget::initGroundThemes()
{
 // AB: atm we use identifiers for the combobox only. one day we may want to add
 // names for them, too
 QStringList list = BosonData::bosonData()->availableGroundThemes();
 mGroundTheme->setItems(list);

 if (mGroundTheme->count() > 0) {
	mGroundTheme->setCurrentItem(0);
 }
}

void BoUfoStartEditorWidget::initMapSizes()
{
 d->mMapSizes.clear();

 // Map sizes
 d->mMapSizes.append(64); // Tiny
 d->mMapSizes.append(128); // Small
 d->mMapSizes.append(192); // Medium
 d->mMapSizes.append(256); // Large
 d->mMapSizes.append(320); // Huge
 d->mMapSizes.append(384); // Super
 d->mMapSizes.append(-1); // Custom

 // Labels
 QStringList labels;
 labels.append("Tiny");
 labels.append("Small");
 labels.append("Medium");
 labels.append("Large");
 labels.append("Huge");
 labels.append("Super");
 labels.append("Custom");

 mMapSize->setItems(labels);

 // Medium is the default size
 mMapSize->setCurrentItem(2);
}

void BoUfoStartEditorWidget::slotNetStart()
{
 BO_CHECK_NULL_RET(boGame);
 boDebug() << k_funcinfo << endl;
 int maxPlayers = 0;
 // WARNING: we have one EVIL assumption here. We assume that the current
 // playfield is actually the currently *selected* playfield!
 // we mustn't assume this for network - for editor it isn't terrible, but it
 // may be possible, that after clicking start editor the player selected
 // another playfield and so we would now add players for the new playfield, but
 // start the old one.
 // We should use the actual playfield from the network here!!

 if (boGame->allPlayerCount() > 0) {
	boError() << k_funcinfo << "allPlayerCount must be 0! trying to clear" << endl;
	boGame->quitGame();
 }
 if (boGame->allPlayerCount() > 0) {
	boError() << k_funcinfo << "allPlayerCount must be 0! clearning did not succeed" << endl;
	return;
 }

 BPFPreview* preview = 0;
 QByteArray newMap;
 if (mCreateNewMap->selected()) {
	preview = 0;
	newMap = createNewMap();
	if (newMap.size() == 0) {
		boError() << k_funcinfo << "could not create new map" << endl;
		KMessageBox::sorry(0, i18n("An error occured while creating a new map"));
		return;
	}
	maxPlayers = (int)mMaxPlayers->value();
 } else {
	// Editing old map
	if (mSelectMap->selectedItem() < 0) {
		boError() << k_funcinfo << "invalid selected index" << endl;
		return;
	}
	QString playFieldIdentifier = d->mIndex2Map[mSelectMap->selectedItem()];
	preview = boData->playFieldPreview(playFieldIdentifier);
	if (!preview) {
		boError() << k_funcinfo << "NULL playfieldpreview" << endl;
		return;
	}
	maxPlayers = preview->maxPlayers();
 }

 Q3ValueList<QColor> availableTeamColors = boGame->availableTeamColors();
 if ((int)availableTeamColors.count() < maxPlayers) {
	boError() << k_funcinfo << "too many players - not enough team colors!" << endl;
	KMessageBox::sorry(0, i18n("Too many (max-)players. Not enough colors available (internal error)."));
	return;
 }
 boDebug() << k_funcinfo << "adding " << maxPlayers << " players" << endl;
 for (int i = 0; i < maxPlayers; i++) {
	// add dummy computer player
	Player* p = new Player;
	p->setName(i18n("Player %1", i + 1));

	// AB: all players can be controlled by the user in editor mode, so all
	// have a localplayer input.
	BosonLocalPlayerInput* input = new BosonLocalPlayerInput(false);
	p->addGameIO(input);
	if (!input->initializeIO()) {
		p->removeGameIO(input, true);

		boError() << k_funcinfo << "localplayer IO could not be initialized. fatal error - quitting" << endl;
		KMessageBox::sorry(0, i18n("internal error. could not initialize IO of local player. quitting now."));
		exit(1);
		return;
	}


	QColor color = availableTeamColors.first();
	availableTeamColors.pop_front();
	p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);
	boGame->bosonAddPlayer(p);
 }

 networkInterface()->addNeutralPlayer(true);
 networkInterface()->sendNewGame(preview, true, &newMap);
}


void BoUfoStartEditorWidget::slotNetPlayFieldChanged(BPFPreview* preview)
{
 boDebug() << k_funcinfo << endl;

 if (!preview) {
	// New map was selected and playfield was reset. Ignore it
	return;
 }

 // AB: this is a workaround.
 // actually selecting a map should be disabled when "edit existing map" is not
 // selected. however setEnabled(false) seems not to work properly with libufo,
 // so here we make sure that the correct radio button is selected.
 slotEditExistingToggled(true);

 QMap<int, QString>::Iterator it;
 int item = -1;
 for (it = d->mIndex2Map.begin(); it != d->mIndex2Map.end() && item < 0; ++it) {
	if (it.data() == preview->identifier()) {
		item = it.key();
	}
 }
 if (item < 0) {
	boError() << k_funcinfo << "Cannot find playfield item for " << preview->identifier() << endl;
 } else {
	mSelectMap->blockSignals(true);
	mSelectMap->setSelectedItem(item);
	mSelectMap->blockSignals(false);
 }


 BO_CHECK_NULL_RET(preview->description());
 boDebug() << k_funcinfo << "id: " << preview->identifier() << endl;
 QStringList list = boData->availablePlayFields();

 const BPFDescription* description = preview->description();
 BO_CHECK_NULL_RET(description);

 // AB: I am not fully sure if a text browser is the right choice for this
 // widget. but being able to use links in the description is surely a good
 // idea.
 if (description->comment().isEmpty()) {
	mMapDescription->setText(i18n("There is no comment for this map available"));
 } else {
	mMapDescription->setText(description->comment());
 }

 mMapWidth->setValue(preview->mapWidth());
 mMapHeight->setValue(preview->mapHeight());
 mGroundTheme->setCurrentItem(0); // TODO - we do not yet support more than one :(
 mMaxPlayers->setValue(preview->maxPlayers());

 d->mSelectedMap = preview;
}

void BoUfoStartEditorWidget::slotPlayFieldChanged(int, int)
{
 boDebug() << k_funcinfo << endl;
 int index = mSelectMap->selectedItem();
 if (index < 0) {
	boWarning() << k_funcinfo << "no playfield selected" << endl;
	return;
 }
 if (!d->mIndex2Map.contains(index)) {
	boWarning() << k_funcinfo << "invalid item" << endl;
	return;
 }
 networkInterface()->sendChangePlayField(d->mIndex2Map[index]);
}

void BoUfoStartEditorWidget::slotGroundThemeChanged(int)
{
 // we don't transmit over network.

 if (mGroundTheme->count() == 0) {
	// not yet initialized
	return;
 }

 mFilling->clear();
 QStringList groundThemes = BosonData::bosonData()->availableGroundThemes();
 int themeIndex = mGroundTheme->currentItem();
 if (themeIndex < 0 || themeIndex >= groundThemes.count()) {
	KMessageBox::sorry(0, i18n("Invalid groundTheme index %1", themeIndex));
	return;
 }
 QString themeId = groundThemes[themeIndex];
 BosonGroundTheme* theme = boData->groundTheme(themeId);
 if (!theme) {
	BO_NULL_ERROR(theme);
	KMessageBox::sorry(0, i18n("An error occured while loading the selected groundtheme"));
	return;
 }

 for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
	mFilling->insertItem(theme->groundType(i)->name);
 }
 if (mFilling->count() > 0) {
	mFilling->setCurrentItem(0);
 }
}

void BoUfoStartEditorWidget::slotMaxPlayersChanged(float m)
{
 slotMaxPlayersChanged((int)m);
}

void BoUfoStartEditorWidget::slotMaxPlayersChanged(int)
{
 // we don't transmit over network.
}

void BoUfoStartEditorWidget::slotWidthChanged(float w)
{
 slotWidthChanged((int)w);
}

void BoUfoStartEditorWidget::slotWidthChanged(int)
{
 // we don't transmit over network.
}

void BoUfoStartEditorWidget::slotHeightChanged(float h)
{
 slotHeightChanged((int)h);
}

void BoUfoStartEditorWidget::slotHeightChanged(int)
{
 // we don't transmit over network.
}

void BoUfoStartEditorWidget::slotMapSizeChanged(int index)
{
 int size = d->mMapSizes[index];

 bool custom = (size == -1);
 mMapHeight->setEnabled(custom);
 mMapWidth->setEnabled(custom);

 if (!custom) {
	mMapHeight->setValue(size);
	mMapWidth->setValue(size);
 }
}

void BoUfoStartEditorWidget::slotNewMapToggled(bool isNewMap)
{
 boDebug() << k_funcinfo << "isNewMap: " << isNewMap << endl;
 mMapName->setEnabled(isNewMap);
 mSelectMap->setEnabled(!isNewMap);

 if (isNewMap) {
	mMapSize->setCurrentItem(2);
	mMaxPlayers->setValue(2);
	mGroundTheme->setCurrentItem(0);
	mFilling->setCurrentItem(0);
	mMapDescription->setText(i18n("Enter description here"));
 } else if (d->mSelectedMap) {
	BPFPreview* preview = d->mSelectedMap;
	const BPFDescription* description = d->mSelectedMap->description();

	BO_CHECK_NULL_RET(description);

	if (description->comment().isEmpty()) {
		mMapDescription->setText(i18n("There is no comment for this map available"));
	} else {
		mMapDescription->setText(description->comment());
	}
	mMapSize->setCurrentItem(6);  // custom
	mMapWidth->setValue(preview->mapWidth());
	mMapHeight->setValue(preview->mapHeight());
	mGroundTheme->setCurrentItem(0); // TODO - we do not yet support more than one :(
	mMaxPlayers->setValue(preview->maxPlayers());
 }

 mMapSize->setEnabled(isNewMap);
 mMapHeight->setEnabled(isNewMap);
 mMapWidth->setEnabled(isNewMap);
 mMaxPlayers->setEnabled(isNewMap);
 mGroundTheme->setEnabled(isNewMap);
 mFilling->setEnabled(isNewMap);

 mMapDescription->setEditable(isNewMap);

 if (isNewMap) {
	// Reset playfield
	networkInterface()->sendChangePlayField(QString());
 }

 boDebug() << k_funcinfo << "DONE" << endl;
}

void BoUfoStartEditorWidget::slotCreateNewToggled(bool selected)
{
 mEditExistingMap->setSelected(!selected);
 slotNewMapToggled(selected);
}

void BoUfoStartEditorWidget::slotEditExistingToggled(bool selected)
{
 mCreateNewMap->setSelected(!selected);
 slotNewMapToggled(!selected);
}

QByteArray BoUfoStartEditorWidget::createNewMap()
{
 boDebug() << k_funcinfo << endl;
 unsigned int width = (unsigned int)mMapWidth->value();
 unsigned int height = (unsigned int)mMapHeight->value();

 QStringList groundThemes = BosonData::bosonData()->availableGroundThemes();
 int themeIndex = mGroundTheme->currentItem();
 if (themeIndex < 0 || themeIndex >= groundThemes.count()) {
	boError() << k_funcinfo << "invalid theme index " << themeIndex << endl;
	if (themeIndex < 0) {
		KMessageBox::sorry(0, i18n("Please select a ground theme first"));
	} else {
		KMessageBox::sorry(0, i18n("The selected groundTheme at index %1 could not be found. %2 themes are available", themeIndex, groundThemes.count()));
	}
	return QByteArray();
 }
 QString themeId = boData->availableGroundThemes()[themeIndex];
 BosonGroundTheme* theme = boData->groundTheme(themeId);
 if (!theme) {
	BO_NULL_ERROR(theme);
	KMessageBox::sorry(0, i18n("An error occured while loading the selected groundtheme"));
	return QByteArray();
 }

 unsigned int groundType = mFilling->currentItem();
 if (groundType >= theme->groundTypeCount()) {
	boError() << k_funcinfo << "invalid groundtype " << groundType << endl;
	KMessageBox::sorry(0, i18n("Could not fill the map with texture %1 - only %2 textures in groundTheme %3", groundType, theme->groundTypeCount(), themeId));
	return QByteArray();
 }

 int maxPlayers = (int)mMaxPlayers->value();
 if (maxPlayers < 2) {
	boError() << k_funcinfo << "maxPlayers < 2 does not make sense" << endl;
	KMessageBox::sorry(0, i18n("Max Players is an invalid value: %1", maxPlayers));
	return QByteArray();
 }
 if (maxPlayers > BOSON_MAX_PLAYERS) {
	boError() << k_funcinfo << "maxPlayers > " << BOSON_MAX_PLAYERS << " is not allowed" << endl;
	KMessageBox::sorry(0, i18n("Max Players is an invalid value: %1 (must be < %2)", maxPlayers, BOSON_MAX_PLAYERS));
	return QByteArray();
 }


 BoCreateNewMap newMap;
 newMap.setSize(width, height);
 newMap.setGroundTheme(theme);
 newMap.setGroundFilling(groundType);
 newMap.setPlayerCount(maxPlayers);
 newMap.setName(mMapName->text());

 return newMap.createNewMap();
}

void BoUfoStartEditorWidget::slotCancel()
{
 // AB: we use a timer, so that the widget can be deleted in the slot
 // (otherwise this would not be allowed, as we are in a pushbutton click)
 QTimer::singleShot(0, this, SIGNAL(signalCancelled()));
}

