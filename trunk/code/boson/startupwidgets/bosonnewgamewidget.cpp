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

#include "bosonnewgamewidget.h"
#include "bosonnewgamewidget.moc"

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
#include <kcombobox.h> // can we use qcombobox instead?
#include <kgamemisc.h>

#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>

class BosonNewGameWidgetPrivate
{
public:
	BosonNewGameWidgetPrivate()
	{
	}

	QPtrDict<KPlayer> mItem2Player;
	QMap<QListViewItem*, QString> mItem2Map;

	QMap<int, QString> mSpeciesIndex2Identifier;
	QMap<int, QString> mSpeciesIndex2Comment;

};


BosonNewGameWidget::BosonNewGameWidget(BosonStartupNetwork* interface, QWidget* parent)
    : BosonNewGameWidgetBase(parent)
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BosonNewGameWidgetPrivate;
 mNetworkInterface = interface;

 mHighlightedPlayer = 0;

 initSpecies();
 initPlayFields();

 connect(networkInterface(), SIGNAL(signalStartGameClicked()),
		this, SLOT(slotNetStart()));
 connect(networkInterface(), SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotNetPlayerJoinedGame(KPlayer*)));
 connect(networkInterface(), SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SLOT(slotNetPlayerLeftGame(KPlayer*)));
 connect(networkInterface(), SIGNAL(signalSpeciesChanged(Player*)),
		this, SLOT(slotNetSpeciesChanged(Player*)));
 connect(networkInterface(), SIGNAL(signalTeamColorChanged(Player*)),
		this, SLOT(slotNetColorChanged(Player*)));
 connect(networkInterface(), SIGNAL(signalPlayFieldChanged(BosonPlayField*)),
		this, SLOT(slotNetPlayFieldChanged(BosonPlayField*)));
 connect(networkInterface(), SIGNAL(signalPlayerNameChanged(Player*)),
		this, SLOT(slotNetPlayerNameChanged(Player*)));
 connect(networkInterface(), SIGNAL(signalSetLocalPlayer(Player*)),
		this, SLOT(slotNetSetLocalPlayer(Player*)));

 connect(networkInterface(), SIGNAL(signalSetAdmin(bool)),
		this, SLOT(slotNetSetAdmin(bool)));

 slotNetSetAdmin(boGame->isAdmin());
}

BosonNewGameWidget::~BosonNewGameWidget()
{
 // Save stuff like player name, color etc.
 boConfig->saveLocalPlayerName(mLocalPlayerName->text());
 boConfig->saveLocalPlayerColor(mPlayerColor);
 QString playFieldIdentifier;
 if (mChooseBosonMap->currentItem()) {
	if (d->mItem2Map.contains(mChooseBosonMap->currentItem())) {
		playFieldIdentifier = d->mItem2Map[mChooseBosonMap->currentItem()];
	}
 }
 if (!playFieldIdentifier.isEmpty()) {
	boConfig->saveLocalPlayerMap(playFieldIdentifier);
 }
// TODO: save species. not yet useful as we support one species only.
 delete d;
}

void BosonNewGameWidget::initPlayer()
{
 boDebug() << k_funcinfo << "playerCount(): " << boGame->playerCount() << endl;
 if (!localPlayer()) {
	boWarning() << k_funcinfo << "waiting for local player to join first" << endl;
	return;
 }
 localPlayer()->setName(boConfig->readLocalPlayerName());
 mLocalPlayerName->setText(localPlayer()->name());
 if (localPlayer()->speciesTheme()) {
	boDebug() << k_funcinfo << "Player has speciesTheme already loaded, reloading" << endl;
 }
 mPlayerColor = boConfig->readLocalPlayerColor();
 localPlayer()->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), mPlayerColor);
}

void BosonNewGameWidget::initPlayFields()
{
 QStringList list = BosonPlayField::availablePlayFields();
 boDebug() << k_funcinfo << list.count() << endl;
 for (unsigned int i = 0; i < list.count(); i++) {
	QListViewItem* item = new QListViewItem(mChooseBosonMap);
	item->setText(0, BosonPlayField::playFieldName(list[i]));
	mChooseBosonMap->insertItem(item);
	d->mItem2Map.insert(item, list[i]);
 }
 if (boGame->isAdmin()) {
	// load the map that was selected in a previous game
	QString mapId = boConfig->readLocalPlayerMap();
	if (mapId.isNull()) {
		mapId = BosonPlayField::defaultPlayField();
	}
	int index = list.findIndex(mapId);
	if (index < 0) {
		index = list.findIndex(BosonPlayField::defaultPlayField());
	}
	networkInterface()->sendChangePlayField(index);
 }
}

void BosonNewGameWidget::initSpecies()
{
  // update possible species:
 mChangeSpecies->clear();
 d->mSpeciesIndex2Comment.clear();
 d->mSpeciesIndex2Identifier.clear();
  //TODO: some scenarios might not provide all species!
 QStringList list = SpeciesTheme::availableSpecies();
 for (unsigned int i = 0; i < list.count(); i++) {
	KSimpleConfig cfg(list[i]);
	cfg.setGroup("Boson Species");
	mChangeSpecies->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
	d->mSpeciesIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
	d->mSpeciesIndex2Identifier.insert(i, cfg.readEntry("Identifier", "Unknown"));
 }
 mChangeSpecies->setCurrentItem(0);
 if (localPlayer()) {
	slotLocalPlayerSpeciesChanged(0);
 }
}

void BosonNewGameWidget::initColors()
{
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 if (!localPlayer()->speciesTheme()) {
	boError() << k_funcinfo << "local player has NULL species theme" << endl;
	return;
 }
 mAvailableColors.clear();
 mLocalColor->clear();
 mAvailableColors = boGame->availableTeamColors();
 mAvailableColors.prepend(localPlayer()->speciesTheme()->teamColor());
 for(unsigned int i = 0; i < mAvailableColors.count(); i++) {
	QPainter painter;
	QRect rect(0, 0, mLocalColor->width(), QFontMetrics(painter.font()).height() + 4);
	QPixmap pixmap(rect.width(), rect.height());
	painter.begin(&pixmap);
	painter.fillRect(rect, QBrush(mAvailableColors[i]));
	painter.end();
	mLocalColor->insertItem(pixmap);
 }
}

void BosonNewGameWidget::addDummyComputerPlayer()
{
 // commandline interface for adding computer players
 slotLocalPlayerAddedComputerPlayer();
}

void BosonNewGameWidget::slotNetStart()
{
 if (!boGame->isAdmin()) {
	return;
 }
 if ((int)boGame->playerCount() > mMaxPlayers) {
	KMessageBox::sorry(this, i18n("There are too many players in game.\n"
			"Current map supports only %1 players, currently, there are %2 players in the game.\n"
			"Please remove some players.").arg(mMaxPlayers).arg(boGame->playerCount()),
			i18n("Too many players"));
 } else if ((int)boGame->playerCount() < mMinPlayers) {
	KMessageBox::sorry(this, i18n("There are too few players in game.\n"
			"Current map requires at least %1 players, currently, there are only %2 players in the game.\n"
			"Please add some players.").arg(mMinPlayers).arg(boGame->playerCount()),
			i18n("Too few players"));
 } else {
	slotLocalPlayerNameChanged();
	networkInterface()->sendNewGame(false);
 }
}


void BosonNewGameWidget::slotNetPlayerJoinedGame(KPlayer* p)
{
 QListBoxText* t = new QListBoxText(p->name());
 d->mItem2Player.insert(t, p);
 mConnectPlayers->insertItem(t);

 initColors();
}

void BosonNewGameWidget::slotNetPlayerLeftGame(KPlayer* p)
{
 boDebug() << k_funcinfo << "there are " << boGame->playerList()->count() << " players in game now" << endl;
 this->disconnect(p);
 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	if (it.current() == p) {
		mConnectPlayers->removeItem(mConnectPlayers->index((QListBoxItem*)it.currentKey()));
		initColors();
		return;
	}
	++it;
 }
}

void BosonNewGameWidget::slotNetSpeciesChanged(Player* p)
{
 boWarning() << k_funcinfo << "TODO" << endl;
}

void BosonNewGameWidget::slotNetColorChanged(Player*)
{
 initColors();
}

void BosonNewGameWidget::slotNetPlayFieldChanged(BosonPlayField* field)
{
 BO_CHECK_NULL_RET(field);
 BO_CHECK_NULL_RET(field->scenario());
 boDebug() << k_funcinfo << "id: " << field->identifier() << endl;
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
 mMinPlayers = field->scenario()->minPlayers();
 mMaxPlayers = field->scenario()->maxPlayers();
 boDebug() << "minPlayers: " << mMinPlayers << " ; maxPlayers: " << mMaxPlayers << endl;

 BPFDescription* description = field->description();
 if (!description) {
	boWarning() << k_funcinfo << "NULL description" << endl;
	BosonPlayField::preLoadAllPlayFields();
	if (!description) {
		boError() << k_funcinfo << "unable to load the description" << endl;
		return;
	}
 }

 // AB: I am not fully sure if a text browser is the right choice for this
 // widget. but being able to use links in the description is surely a good
 // idea.
 if (description->comment().isEmpty()) {
	mMapPropertiesTextBrowser->setText(i18n("There is no comment for this map available"));
 } else {
	mMapPropertiesTextBrowser->setText(description->comment());
 }
}

void BosonNewGameWidget::slotNetPlayerNameChanged(Player* p)
{
 BO_CHECK_NULL_RET(p);
 boDebug() << k_funcinfo << endl;
 if (p == localPlayer()) {
	mLocalPlayerName->setText(p->name());
 }

 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	if (it.current() == (KPlayer*)p) {
		QListBoxText* old = (QListBoxText*)it.currentKey();
		int index = mConnectPlayers->index(old);
		if (index < 0 || (unsigned int)index >= mConnectPlayers->count()) {
			boError() << k_funcinfo << "invalid index " << index << endl;
			return;
		}
		QListBoxItem* t = new QListBoxText(p->name());
		d->mItem2Player.remove(old);
		mConnectPlayers->changeItem(t, index);
		d->mItem2Player.insert(t, p);
		return;
	}
	++it;
 }
 boWarning() << k_funcinfo << "could not find player " << p->id() << endl;
}

void BosonNewGameWidget::slotNetSetLocalPlayer(Player* p)
{
 boDebug() << k_funcinfo << endl;
 if (!p) {
	boDebug() << k_funcinfo << "unset local player" << endl;
	return;
 }
 initPlayer();
}

void BosonNewGameWidget::slotNetSetAdmin(bool admin)
{
 if (admin) {
	mChooseBosonMap->setEnabled(true);
//	mStartGameButton->setEnabled(true);
 } else {
	mChooseBosonMap->setEnabled(false);
//	mStartGameButton->setEnabled(false);
 }
}

void BosonNewGameWidget::slotLocalPlayerNameChanged()
{
 BO_CHECK_NULL_RET(localPlayer());
 boDebug() << k_funcinfo << endl;
 QString name = mLocalPlayerName->text();
 if (name != localPlayer()->name()) {
	networkInterface()->sendChangePlayerName(localPlayer(), name);
 }
}

void BosonNewGameWidget::slotLocalPlayerColorChanged(int index)
{
 if (index < 0 || (unsigned int)index >= mAvailableColors.count()) {
	boWarning() << k_funcinfo << "Invalid index: " << index << endl;
	return;
 }
 BO_CHECK_NULL_RET(localPlayer());
 mPlayerColor = mAvailableColors[index];

 networkInterface()->sendChangeTeamColor(localPlayer(), mPlayerColor);
}

void BosonNewGameWidget::slotLocalPlayerPlayFieldChanged(QListViewItem* item)
{
 BO_CHECK_NULL_RET(item);
 if (!d->mItem2Map.contains(item)) {
	boWarning() << k_funcinfo << "invalid item" << endl;
	return;
 }
 networkInterface()->sendChangePlayField(d->mItem2Map[item]);
}

void BosonNewGameWidget::slotLocalPlayerSpeciesChanged(int index)
{
 BO_CHECK_NULL_RET(localPlayer());

 if (index >= (int)d->mSpeciesIndex2Identifier.count()) {
	boError() << k_funcinfo << "invalid index " << index << endl;
	return;
 }
 networkInterface()->sendChangeSpecies(localPlayer(), d->mSpeciesIndex2Identifier[index], mPlayerColor);
}

void BosonNewGameWidget::slotLocalPlayerAddedComputerPlayer()
{
 BO_CHECK_NULL_RET(boGame);
 boDebug() << k_funcinfo << endl;
 if (!boGame->isAdmin()) {
	KMessageBox::sorry(this, i18n("You must be ADMIN to do this"));
	return;
 }
 if ((int)boGame->playerCount() >= mMaxPlayers) {
	KMessageBox::sorry(this, i18n("There are too many players in the game.\n"
			"Current map supports only %1 players.\n").arg(mMaxPlayers),
			i18n("Too many players"));
	return;
 }

 Player* p = new Player();

#warning TODO: lineedit for AI name
// p->setName(mAddAIName->text());
 p->setName(KGameMisc::randomName());

 // the color is dangerous concerning network and so!
 // it'd be better to first add the player and then change the color using a
 // network message. unfortunately we can't send a message, since we do not yet
 // have the ID of the new player.
 QColor color = boGame->availableTeamColors().first();
 p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);

 BosonComputerIO* io = new BosonComputerIO();
 p->addGameIO(io);
 boGame->addPlayer(p);
}

void BosonNewGameWidget::slotLocalPlayerRemovedPlayer()
{
 BO_CHECK_NULL_RET(localPlayer());

 if (mHighlightedPlayer != localPlayer()) {
	networkInterface()->removePlayer(mHighlightedPlayer);
 }
}

void BosonNewGameWidget::slotLocalPlayerHighlightedPlayer(QListBoxItem* item)
{
 mHighlightedPlayer = d->mItem2Player[item];
 if (!mHighlightedPlayer) {
	mRemovePlayer->setEnabled(false);
	return;
 }
 if (boGame->isAdmin() || !mHighlightedPlayer->isVirtual()) {
	mRemovePlayer->setEnabled(true);
 } else {
	mRemovePlayer->setEnabled(false);
 }
}

Player* BosonNewGameWidget::localPlayer() const
{
 // AB: I don't like to using boGame->localPlayer(). i consider this unclean,
 // but it definitely saves a lot of trouble, as we set the localplayer in
 // boGame anyway. no need to duplicate that stuff here.
 return boGame->localPlayer();
}


