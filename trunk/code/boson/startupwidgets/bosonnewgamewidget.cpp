/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "../player.h"
#include "../speciestheme.h"
#include "../bosoncomputerio.h"
#include "../boson.h"
#include "../bosoncampaign.h"
#include "../bosonplayfield.h"
#include "../bpfdescription.h"
#include "../bosonmap.h"
#include "../bosondata.h"
#include "../bosonwidgets/bosonplayfieldview.h"
#include "../bosonwidgets/bocolorchooser.h"
#include "../bosonwidgets/bogamechat.h"
#include "bosonstartupnetwork.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgameproperty.h>
#include <kgame/kgamechat.h>
#include <kgame/kgamemessage.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <klistbox.h>
#include <ktextbrowser.h>
#include <kcombobox.h> // can we use qcombobox instead?

#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qguardedptr.h>
#include <qtimer.h>


/**
 * @ref QListBoxText with changeable color
 * Used for displaying players in "Connected players" listbox
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class QListBoxPlayerText : public QListBoxText
{
public:
	QListBoxPlayerText(QListBox* listBox, const QString& text)
		: QListBoxText(listBox, text)
	{
		mIsLocalPlayer = false;
	}
	QListBoxPlayerText(const QString& text)
		: QListBoxText(text)
	{
		mIsLocalPlayer = false;
	}
	void setColor(const QColor& c)
	{
		mPlayerColor = c;
	}
	const QColor& color() const
	{
		return mPlayerColor;
	}
	void setPlayerName(const QString& name)
	{
		setText(name);
	}
	void setLocalPlayer(bool l)
	{
		mIsLocalPlayer = l;
	}

	virtual void paint(QPainter* p)
	{
		QFont f(p->font());
		f.setBold(mIsLocalPlayer);
		p->setFont(f);
		p->setPen(color());
		QListBoxText::paint(p);
	}

private:
	QColor mPlayerColor;
	bool mIsLocalPlayer;
};


class BosonNewGameWidgetPrivate
{
public:
	BosonNewGameWidgetPrivate()
	{
		mLocalPlayer = 0;
	}

	QPtrDict<KPlayer> mItem2Player;
	QMap<QListViewItem*, QString> mItem2Map;

	QMap<int, QString> mSpeciesIndex2Identifier;
	QMap<int, QString> mSpeciesIndex2Comment;

	QGuardedPtr<Player> mLocalPlayer;
};


BosonNewGameWidget::BosonNewGameWidget(BosonStartupNetwork* interface, QWidget* parent)
    : BosonNewGameWidgetBase(parent)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BosonNewGameWidgetPrivate;
 mNetworkInterface = interface;

 mSelectedPlayer = 0;
 mMaxPlayers = 0;
 mMinPlayers = 0;
 mInited = false; // Will become true once localplayer gets added

 initPlayFields();
 initSpecies();

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
 connect(boGame, SIGNAL(signalConnectionBroken()),
		this, SLOT(slotNetConnectionBroken()));
 connect(boGame, SIGNAL(signalAddChatSystemMessage(const QString&, const QString&, const Player*)),
		this, SLOT(slotAddChatSystemMessage(const QString&, const QString&, const Player*)));

 connect(networkInterface(), SIGNAL(signalSetAdmin(bool)),
		this, SLOT(slotNetSetAdmin(bool)));

 mPlayerColor->setColors(SpeciesTheme::defaultColors());

 slotNetSetAdmin(boGame->isAdmin());

 mChat->chatWidget()->setKGame(boGame);
}

BosonNewGameWidget::~BosonNewGameWidget()
{
 boDebug() << k_funcinfo << endl;
 // Save stuff like player name, color etc.
 if (boGame) {
	boConfig->saveLocalPlayerName(localPlayer()->name());
 } else {
	// FIXME: Boson object has already been deleted. This happens when closing
	//  window with Alt+F4, because then TopWidget dtor deletes Boson (by calling
	//  endGame()) before Qt deletes child widgets (including this one)
	boError() << k_funcinfo << "FIXME: Boson already deleted!" << endl;
 }
 boConfig->saveLocalPlayerColor(mLocalPlayerColor);
 QString playFieldIdentifier;
 if (mSelectMap->currentItem()) {
	if (d->mItem2Map.contains(mSelectMap->currentItem())) {
		playFieldIdentifier = d->mItem2Map[mSelectMap->currentItem()];
	}
 }
 if (!playFieldIdentifier.isEmpty()) {
	boConfig->saveLocalPlayerMap(playFieldIdentifier);
 }
 // TODO: save species. not yet useful as we support one species only.
 delete d;
}

void BosonNewGameWidget::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
 if (d->mLocalPlayer && !d->mLocalPlayer->speciesTheme()) {
	initLocalPlayer();
	mChat->chatWidget()->setFromPlayer(p);
 }
}

void BosonNewGameWidget::initLocalPlayer()
{
 boDebug() << k_funcinfo << endl;
 if (!localPlayer()) {
	boWarning() << k_funcinfo << "waiting for local player to join first" << endl;
	return;
 }
 // Load name and color
 localPlayer()->setName(boConfig->readLocalPlayerName());
 mPlayerName->setText(localPlayer()->name());
 if (localPlayer()->speciesTheme()) {
	boDebug() << k_funcinfo << "Player has speciesTheme already loaded, reloading" << endl;
 }
 mLocalPlayerColor = boConfig->readLocalPlayerColor();
 localPlayer()->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), mLocalPlayerColor);
 // TODO: load species. not yet useful as we support one species only.
}

void BosonNewGameWidget::initPlayFields()
{
 boDebug() << k_funcinfo << boData->availablePlayFields().count() << " playfields found" << endl;
 // Add root item for default-campaign maps
 QListViewItem* maps = new QListViewItem(mSelectMap);
 maps->setOpen(true);
 maps->setSelectable(false);

 // Get list of campaigns
 QStringList list = boData->availableCampaigns();
 // the default campaign _must_ be there (even if its empty!)
 if (list.count() == 0) {
	boError() << k_funcinfo << "no campaigns found. At least the default campaign (random maps) is mandatory!" << endl;
	return;
 }
 if (!list.contains(QString::fromLatin1(""))) {
	boError() << k_funcinfo  << "no default campaign found! (code bug)" << endl;
	return;
 }
 list.remove(QString::fromLatin1(""));

 // Add default campaign
 BosonCampaign* defaultCampaign = boData->campaign(QString::fromLatin1(""));
 BO_CHECK_NULL_RET(defaultCampaign);
 initPlayFields(defaultCampaign, maps);

 // Add other campaigns
 QListViewItem* campaigns = new QListViewItem(mSelectMap);
 campaigns->setOpen(true);
 campaigns->setSelectable(false);
 campaigns->setText(0, i18n("Campaigns"));
 for (unsigned int i = 0; i < list.count(); i++) {
	BosonCampaign* campaign = boData->campaign(list[i]);
	if (!campaign) {
		BO_NULL_ERROR(campaign);
		continue;
	}
	QListViewItem* item = new QListViewItem(campaigns);
	initPlayFields(campaign, item);
 }

 if (boGame->isAdmin()) {
	// load the map that was selected in a previous game
	QString mapId = boConfig->readLocalPlayerMap();
	if (mapId.isNull()) {
		mapId = BosonPlayField::defaultPlayField();
	}
	networkInterface()->sendChangePlayField(mapId);
 }
}

void BosonNewGameWidget::initPlayFields(BosonCampaign* campaign, QListViewItem* parent)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(campaign);
 BO_CHECK_NULL_RET(parent);
 parent->setOpen(true);
 parent->setSelectable(false);
 if (campaign->identifier().isEmpty()) {
	boDebug() << k_funcinfo << "default campaign" << endl;
	parent->setText(0, i18n("Maps")); // FIXME: better name. "misc maps" ? "other maps" ?
 } else {
	boDebug() << k_funcinfo << campaign->identifier() << endl;
	parent->setText(0, campaign->name());
 }
 QStringList list = campaign->playFields();
 for (unsigned int i = 0; i < list.count(); i++) {
	QListViewItem* item = new QListViewItem(parent);
	BosonPlayField* field = boData->playField(list[i]);
	if (!field) {
		BO_NULL_ERROR(field);
		continue;
	}
	item->setText(0, field->playFieldName());
	d->mItem2Map.insert(item, list[i]);
 }
}

void BosonNewGameWidget::initSpecies()
{
 boDebug() << k_funcinfo << endl;
 // update possible species:
 mPlayerSpecies->clear();
 d->mSpeciesIndex2Comment.clear();
 d->mSpeciesIndex2Identifier.clear();
  //TODO: some scenarios might not provide all species!
 int defaultIndex = 0;
 QStringList list = SpeciesTheme::availableSpecies();
 for (unsigned int i = 0; i < list.count(); i++) {
	KSimpleConfig cfg(list[i]);
	cfg.setGroup("Boson Species");
	QString identifier = cfg.readEntry("Identifier", "Unknown");
	if (identifier == QString::fromLatin1("Neutral")) {
		continue;
	}
	mPlayerSpecies->insertItem(cfg.readEntry("Name", i18n("Unknown")), i);
	// comments aren't used. Maybe remove this dict?
	d->mSpeciesIndex2Comment.insert(i, cfg.readEntry("Comment", i18n("None")));
	d->mSpeciesIndex2Identifier.insert(i, identifier);
	if (identifier == SpeciesTheme::defaultSpecies()) {
		defaultIndex = i;
	}
 }
 mPlayerSpecies->setCurrentItem(defaultIndex);
}

void BosonNewGameWidget::updateColors()
{
 boDebug() << k_funcinfo << endl;
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }
 if (!localPlayer()->speciesTheme()) {
	boError() << k_funcinfo << "local player has NULL species theme" << endl;
	return;
 }

 Player* p = mSelectedPlayer;
 if (!p) {
	p = localPlayer();
 }

 QValueList<QColor> availableColors = boGame->availableTeamColors();
 availableColors.prepend(p->teamColor());

 // first set all taken, then make those available, that are still available
 mPlayerColor->setAllTaken(true);
 for(unsigned int i = 0; i < availableColors.count(); i++) {
	mPlayerColor->setTaken(availableColors[i], false);
 }

 mPlayerColor->highlightColor(p->teamColor());
}

/*****  Slots for networks commands  *****/

void BosonNewGameWidget::slotNetStart()
{
 boDebug() << k_funcinfo << endl;
 if (!boGame->isAdmin()) {
	return;
 }
 // Check for map
 if (!mSelectMap->selectedItem()) {
	KMessageBox::sorry(this, i18n("No map selected. Select a map first!"));
	return;
 }
 // Check for valid players count
 if (boGame->playerCount() > mMaxPlayers) {
	KMessageBox::sorry(this, i18n("There are too many players in game.\n"
			"Current map supports only %1 players, currently, there are %2 players in the game.\n"
			"Please remove some players.").arg(mMaxPlayers).arg(boGame->playerCount()),
			i18n("Too many players"));
 } else if (boGame->playerCount() < mMinPlayers) {
	KMessageBox::sorry(this, i18n("There are too few players in game.\n"
			"Current map requires at least %1 players, currently, there are only %2 players in the game.\n"
			"Please add some players.").arg(mMinPlayers).arg(boGame->playerCount()),
			i18n("Too few players"));
 } else {
	// All good
	//slotPlayerNameChanged();
	// Check if each player has unique team color
	for (unsigned int i = 0; i < boGame->playerCount() - 1; i++) {
		Player* p = (Player*)boGame->playerList()->at(i);
		if (!p) {
			BO_NULL_ERROR(p);
			continue;
		}
		for (unsigned int j = i + 1; j < boGame->playerCount(); j++) {
			Player* p2 = (Player*)boGame->playerList()->at(j);
			if (!p2) {
				BO_NULL_ERROR(p2);
				continue;
			}
			if (p->teamColor() == p2->teamColor()) {
				KMessageBox::sorry(this, i18n("Cannot start game - player %1 (id=%2) and %3 (id=%4) have the same teamcolor.\nThis is not allowed!").arg(p->name()).arg(p->id()).arg(p2->name()).arg(p2->id()));
				return;
			}
		}
	}
	if (!mSelectMap->currentItem()) {
		KMessageBox::sorry(this, i18n("You have to select a map first"));
		return;
	}

	BosonPlayField* field = 0;
	if (!d->mItem2Map.contains(mSelectMap->currentItem())) {
		KMessageBox::sorry(this, i18n("The selected item seems not to be a valid map. Please select a different map.\n(if you believe you have selected a valid map, you might have encountered a bug)"));
		return;
	}
	QString identifier = d->mItem2Map[mSelectMap->currentItem()];
	field = boData->playField(identifier);

	if (!field) {
		KMessageBox::sorry(this, i18n("The selected item seems to be a map, but it cannot be found.\nYou have encountered a bug."));
		return;
	}

	// Send start message over net
	if (!networkInterface()->sendNewGame(field, false)) {
		KMessageBox::sorry(this, i18n("The selected map could not be saved to a network stream. This might be due to a broken map file, or due to a boson bug.\nAnyway - the game cannot be started."));
		return;
	} else {
		boDebug() << k_funcinfo << "newgame message sent successfully" << endl;
	}
 }
}


void BosonNewGameWidget::slotNetPlayerJoinedGame(KPlayer* p)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(p);
 if (((Player*)p)->isNeutralPlayer()) {
	// Don't show neutral player in players' list
	boDebug() << k_funcinfo << "Added player is neutral player. Returning" << endl;
	return;
 }
 QListBoxPlayerText* t = new QListBoxPlayerText(p->name());
 t->setColor(((Player*)p)->teamColor());
 d->mItem2Player.insert(t, p);
 mPlayers->insertItem(t);
 if (mPlayers->count() == 1) {
	// Localplayer joined the game. No chat message, but select it
	mPlayers->setSelected(t, true);
	mInited = true;
 } else if (mInited) {
	// Display chat message
	if (!p->isVirtual()) {
		// Localplayer added AI player
		boGame->slotAddChatSystemMessage("Boson", i18n("You added AI player %1 to the game").arg(p->name()));
	} else {
		int gameID = KGameMessage::rawGameId(p->id());  // gameID == clientID
		KPlayer* mainPlayer = boGame->findPlayer(KGameMessage::createPlayerId(1, gameID));
		if (mainPlayer == p) {
			// Another client connected
			boGame->slotAddChatSystemMessage("Boson", i18n("%1 joined the game").arg(p->name()));
			if (boGame->isAdmin()) {
				boDebug() << k_funcinfo << "new client connected - sending current playfield" << endl;
				QListViewItem* item = mSelectMap->currentItem();
				if (item) {
					networkInterface()->sendChangePlayField(d->mItem2Map[item]);
				}
			}
		} else if(!mainPlayer) {
			// first player has not ID 0. probably do a loop instead, searching for the
			//  player with the lowest KGameMessage::rawPlayerId() in this gameID.
			boGame->slotAddChatSystemMessage("Boson", i18n("%1 joined (?) the game").arg(p->name()));
		} else {
			// AI player was added
			boGame->slotAddChatSystemMessage("Boson", i18n("%1 added AI player %2 to the game").arg(mainPlayer->name()).arg(p->name()));
		}
	}
	for (int i = 0; i < (int)boGame->playerCount() - 1; i++) {
		Player* p2 = (Player*)boGame->playerList()->at(i);
		if (p2 == (Player*)p) {
			continue;
		}
		if (((Player*)p)->teamColor() == p2->teamColor()) {
			boDebug() << k_funcinfo << "new players' teamcolor already exists - chosing a different one" << endl;
			QColor color = boGame->availableTeamColors().first();
			networkInterface()->sendChangeTeamColor((Player*)p, color);
			break;
		}
	}
 }

 if (p == (KPlayer*)localPlayer()) {
	QPtrDictIterator<KPlayer> it(d->mItem2Player);
	while (it.current()) {
		if (it.current() == p) {
			((QListBoxPlayerText*)it.currentKey())->setLocalPlayer(true);
		} else {
			((QListBoxPlayerText*)it.currentKey())->setLocalPlayer(false);
		}
		++it;
	}
 }

 updateColors();
 playerCountChanged();
}

void BosonNewGameWidget::slotNetPlayerLeftGame(KPlayer* p)
{
 boDebug() << k_funcinfo << "there are " << boGame->playerList()->count() << " players in game now" << endl;

 if (((Player*)p)->isNeutralPlayer()) {
	// Neutral player shouldn't be removed because it's not added until the game
	//  starts.
	boError() << k_funcinfo << "Neutral player removed from game?!" << endl;
	boGame->slotAddChatSystemMessage(i18n("Internal error: Neutral player has been removed from the game"));
	return;
 }

 if (mInited) {
	boGame->slotAddChatSystemMessage("Boson", i18n("%1 left the game").arg(p->name()));
 }

 if (p == (KPlayer*)localPlayer() && boGame->isNetwork()) {
	// Localplayer was removed and w have a net game.
	// Either player was kicked by ADMIN or we're connecting to net game
	// If we're connecting, players are still stored in inactive list
	if (!boGame->inactivePlayerList()->containsRef(p)) {
		boDebug() << k_funcinfo << "We were kicked out!" << endl;
		boGame->slotAddChatSystemMessage("Boson", i18n("You were kicked from the game by ADMIN"));

		// disconnect from the network game
		QTimer::singleShot(0, this, SIGNAL(signalKickedOut()));
	}
 }

 removePlayer((Player*)p);
}

void BosonNewGameWidget::slotNetSpeciesChanged(Player* p)
{
 boDebug() << k_funcinfo << endl;
 if (mSelectedPlayer == p) {
	// Update species combobox
	QMap<int, QString>::Iterator it;
	for (it = d->mSpeciesIndex2Identifier.begin(); it != d->mSpeciesIndex2Identifier.end(); it++) {
		if (it.data() == p->speciesTheme()->identifier()) {
			mPlayerSpecies->setCurrentItem(it.key());
			break;
		}
	}
 }
}

void BosonNewGameWidget::slotNetColorChanged(Player* p)
{
 boDebug() << k_funcinfo << endl;
 // Update selected players' list
 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	if (it.current() == (KPlayer*)p) {
		QListBoxPlayerText* t = (QListBoxPlayerText*)it.currentKey();
		t->setColor(p->teamColor());
		break;
	}
	++it;
 }
 mPlayers->triggerUpdate(false);
 // Update color selector widget
 updateColors();
}

void BosonNewGameWidget::slotNetPlayFieldChanged(BosonPlayField* field)
{
 BO_CHECK_NULL_RET(field);
 boDebug() << k_funcinfo << "id: " << field->identifier() << endl;
 if (!field->isPreLoaded()) {
	// well, if that happens - something evil must have happened!
	boWarning() << k_funcinfo << "playfield not yet preloaded?!" << endl;
	field->preLoadPlayField(field->identifier());
 }
 QStringList list = boData->availablePlayFields();
 QMap<QListViewItem*, QString>::Iterator it;
 QListViewItem* item = 0;
 for (it = d->mItem2Map.begin(); it != d->mItem2Map.end() && !item; ++it) {
	if (it.data() == field->identifier()) {
		item = it.key();
		break;
	}
 }
 if (!item) {
	boError() << k_funcinfo << "Cannot find playfield item for " << field->identifier() << endl;
 } else {
	if (!boGame->isAdmin()) {
		// If client isn't admin, it can't select the map, but we still have to
		//  indicate selected map, so we temporarily set selection mode to single
		mSelectMap->setSelectionMode(QListView::Single);
		mSelectMap->blockSignals(true);
		mSelectMap->clearSelection();
		mSelectMap->setSelected(item, true);
		mSelectMap->blockSignals(false);
		mSelectMap->setSelectionMode(QListView::NoSelection);
	} else {
		mSelectMap->blockSignals(true);
		mSelectMap->setSelected(item, true);
		mSelectMap->blockSignals(false);
	}
	mSelectMap->ensureItemVisible(item);
 }
 mMinPlayers = field->information()->minPlayers();
 mMaxPlayers = field->information()->maxPlayers();
 boDebug() << "minPlayers: " << mMinPlayers << " ; maxPlayers: " << mMaxPlayers << endl;

 // Update general map info label
 QString info;
 info += i18n("Size: %1x%2\n").arg(field->information()->mapWidth()).arg(field->information()->mapHeight());
 if (mMinPlayers == mMaxPlayers) {
	info += i18n("Players: %1").arg(mMinPlayers);
 } else {
	info += i18n("Players: %1-%2").arg(mMinPlayers).arg(mMaxPlayers);
 }
 mMapInfo->setText(info);

 // Update description of the map
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
	mMapDescription->setText(i18n("There is no comment for this map available"));
 } else {
	mMapDescription->setText(description->comment());
 }
 playerCountChanged();
}

void BosonNewGameWidget::slotNetPlayerNameChanged(Player* p)
{
 BO_CHECK_NULL_RET(p);
 boDebug() << k_funcinfo << endl;

 if (p == mSelectedPlayer) {
	mPlayerName->setText(p->name());
 }

 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	if (it.current() == (KPlayer*)p) {
		QListBoxPlayerText* i = (QListBoxPlayerText*)it.currentKey();

		if (mInited) {
			if (p == localPlayer()) {
				boGame->slotAddChatSystemMessage("Boson", i18n("You are now known as %1").arg(p->name()));
			} else {
				boGame->slotAddChatSystemMessage("Boson", i18n("%1 is now known as %2").arg(i->text()).arg(p->name()));
			}
		}

		i->setPlayerName(p->name());
		mPlayers->triggerUpdate(false);

		// Setting local player's name is last step of initing when going to start
		//  game page. If name was set for localplayer and we wasn't inited yet,
		//  then now we are
		if (p == localPlayer() && !mInited) {
			mInited = true;
		}
		return;
	}
	++it;
 }
 boWarning() << k_funcinfo << "could not find player " << p->id() << endl;
}

void BosonNewGameWidget::slotNetSetLocalPlayer(Player* p)
{
}

void BosonNewGameWidget::slotNetSetAdmin(bool admin)
{
 boDebug() << k_funcinfo << endl;
 slotSetAdmin(admin);
}

void BosonNewGameWidget::slotNetConnectionBroken()
{
 boDebug() << k_funcinfo << endl;
 if (mInited) {
	// Connection to server was broken. Probably master quit ot something
	boGame->slotAddChatSystemMessage("Boson", i18n("Connection to server was broken!"));
 } else {
	// We tried to connect to server, but couldn't
	mInited = true;
 }
}

/*****  Slots called when user clicks somewhere  *****/

void BosonNewGameWidget::slotPlayerNameChanged()
{
 BO_CHECK_NULL_RET(localPlayer());
 boDebug() << k_funcinfo << endl;
 Player* p = mSelectedPlayer;
 if (!p) {
	p = localPlayer();
 }
 // Admin can change names of all players
 // Non-admin players can only change names of non-virtual players
 if (!boGame->isAdmin() && mSelectedPlayer->isVirtual()) {
	// This shouldn't happen, name lineedit should be read-only in this case
	boError() << k_funcinfo << "Can't set name for player " << p->id() << endl;
	mPlayerName->setText(p->name());
	return;
 }

 QString name = mPlayerName->text();
 if (name != p->name()) {
	networkInterface()->sendChangePlayerName(p, name);
 }
}

void BosonNewGameWidget::slotPlayerColorChanged(int index)
{
 boDebug() << k_funcinfo << endl;
 if (index < 0 || (unsigned int)index >= SpeciesTheme::defaultColors().count()) {
	boWarning() << k_funcinfo << "Invalid index: " << index << endl;
	return;
 }
 Player* p = mSelectedPlayer;
 if (!p) {
	p = localPlayer();
 }
 BO_CHECK_NULL_RET(p);
 if (!boGame->isAdmin() && mSelectedPlayer->isVirtual()) {
	boError() << k_funcinfo << "Can't set color for player " << mSelectedPlayer->id() << endl;
	KMessageBox::sorry(this, i18n("Only ADMIN can change color of other players!"));
	return;
 }

 QColor color = SpeciesTheme::defaultColors()[index];

 networkInterface()->sendChangeTeamColor(p, color);

 if (p == localPlayer()) {
	mLocalPlayerColor = color;
 }
}

void BosonNewGameWidget::slotPlayFieldSelected(QListViewItem* item)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(item);
 if (!d->mItem2Map.contains(item)) {
	boWarning() << k_funcinfo << "invalid item" << endl;
	return;
 }
 networkInterface()->sendChangePlayField(d->mItem2Map[item]);
}

void BosonNewGameWidget::slotPlayerSpeciesChanged(int index)
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(localPlayer());

 Player* p = mSelectedPlayer;
 if (!p) {
	p = localPlayer();
 }
 if (!boGame->isAdmin() && mSelectedPlayer->isVirtual()) {
	boError() << k_funcinfo << "Can't set species for player " << p->id() << endl;
	return;
 }

 if (index >= (int)d->mSpeciesIndex2Identifier.count()) {
	boError() << k_funcinfo << "invalid index " << index << endl;
	return;
 }
 networkInterface()->sendChangeSpecies(p, d->mSpeciesIndex2Identifier[index], p->speciesTheme()->teamColor());
}

void BosonNewGameWidget::slotAddComputerPlayer()
{
 BO_CHECK_NULL_RET(boGame);
 boDebug() << k_funcinfo << endl;
 // FIXME: maybe allow other players to add AIs?
 if (!boGame->isAdmin()) {
	KMessageBox::sorry(this, i18n("You must be ADMIN to do this"));
	return;
 }
 if (boGame->playerCount() >= mMaxPlayers) {
	KMessageBox::sorry(this, i18n("There are too many players in the game.\n"
			"Current map supports only %1 players.\n").arg(mMaxPlayers),
			i18n("Too many players"));
	return;
 }

 Player* p = new Player();

 // AB: we don't have access to libkdegames anymore. one day we will have
 // computer players with names that fit to the story (if we'll ever have one).
 // so the randomName() isn't required anyway.
// p->setName(KGameMisc::randomName());
 p->setName(i18n("Player %1").arg(boGame->playerCount()));

 // the color is dangerous concerning network and so!
 // it'd be better to first add the player and then change the color using a
 // network message. unfortunately we can't send a message, since we do not yet
 // have the ID of the new player.
 QColor color = boGame->availableTeamColors().first();
 p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);

 BosonComputerIO* io = new BosonComputerIO();
 p->addGameIO(io);
 boGame->bosonAddPlayer(p);
}

void BosonNewGameWidget::slotRemovePlayer()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(localPlayer());

 if (mSelectedPlayer != localPlayer()) {
	networkInterface()->removePlayer(mSelectedPlayer);
 }
}

void BosonNewGameWidget::slotPlayerSelected(QListBoxItem* item)
{
 boDebug() << k_funcinfo << endl;
 mSelectedPlayer = (Player*)d->mItem2Player[item];
 if (!mSelectedPlayer) {
	mRemovePlayer->setEnabled(false);
	return;
 }
 BO_CHECK_NULL_RET(mSelectedPlayer->speciesTheme());

 // Update name, color, species
 mPlayerName->setText(mSelectedPlayer->name());
 updateColors();
 QMap<int, QString>::Iterator it;
 for (it = d->mSpeciesIndex2Identifier.begin(); it != d->mSpeciesIndex2Identifier.end(); it++) {
	if (it.data() == mSelectedPlayer->speciesTheme()->identifier()) {
		mPlayerSpecies->setCurrentItem(it.key());
		break;
	}
 }

 // Enable/disable some stuff
 bool canchange = false;
 if (boGame->isAdmin() || !mSelectedPlayer->isVirtual()) {
	canchange = true;
 }
 mPlayerName->setReadOnly(!canchange);
 mPlayerColor->setEnabled(canchange);
 mPlayerSpecies->setEnabled(canchange);
 mRemovePlayer->setEnabled(canchange && (mSelectedPlayer != localPlayer()));
}

/*****  Other stuff  *****/

Player* BosonNewGameWidget::localPlayer() const
{
 return d->mLocalPlayer;
}

void BosonNewGameWidget::slotCancel()
{
 boDebug() << k_funcinfo << endl;
 boGame->disconnect();
 emit signalCancelled();
}

void BosonNewGameWidget::slotStartGame()
{
 boDebug() << k_funcinfo << endl;
 networkInterface()->sendStartGameClicked();
}

void BosonNewGameWidget::slotNetworkOptions()
{
 boDebug() << k_funcinfo << endl;
 emit signalShowNetworkOptions();
}

void BosonNewGameWidget::addAIPlayer()
{
 boDebug() << k_funcinfo << endl;
 // commandline interface for adding computer players
 slotAddComputerPlayer();
}

void BosonNewGameWidget::playersChanged()
{
 boDebug() << k_funcinfo << endl;
}

void BosonNewGameWidget::playerCountChanged()
{
 boDebug() << k_funcinfo << endl;
 if (mPlayers->count() >= mMaxPlayers) {
	mAddComputerPlayer->setEnabled(false);
 } else {
	// ATM only ADMIN can add AIs. Change this if that will be changed!
	mAddComputerPlayer->setEnabled(boGame->isAdmin());
 }
 // Maybe also disable/enable start button if player count is invalid?
}

void BosonNewGameWidget::slotSetAdmin(bool admin)
{
 boDebug() << k_funcinfo << endl;
 mStartButton->setEnabled(admin);
 mAddComputerPlayer->setEnabled(admin);
 if (admin) {
	mSelectMap->setSelectionMode(QListView::Single);
 } else {
	mSelectMap->setSelectionMode(QListView::NoSelection);
 }
}

void BosonNewGameWidget::slotOfferingConnections()
{
 boDebug() << k_funcinfo << endl;
 boGame->slotAddChatSystemMessage("Boson", i18n("You are now running as server and are offering connections"));
}

void BosonNewGameWidget::slotConnectingToServer()
{
 boDebug() << k_funcinfo << endl;
 mInited = false;
}

void BosonNewGameWidget::slotConnectedToServer()
{
 boDebug() << k_funcinfo << endl;
 boGame->slotAddChatSystemMessage("Boson", i18n("You are now connected to the server %1:%2").arg(boGame->bosonHostName()).arg(boGame->bosonPort()));
 mInited = true;
 // Select localplayer
 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	if (it.current() == (KPlayer*)localPlayer()) {
		mPlayers->setSelected((QListBoxItem*)it.currentKey(), true);
		break;
	}
	++it;
 }
 mPlayers->ensureCurrentVisible();
}

void BosonNewGameWidget::slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer)
{
 BO_CHECK_NULL_RET(mChat);
 BO_CHECK_NULL_RET(mChat->chatWidget());

 if (forPlayer && forPlayer != localPlayer()) {
	return;
 }
 mChat->chatWidget()->addSystemMessage(fromName, text);
}

void BosonNewGameWidget::removePlayer(KPlayer* p)
{
 this->disconnect(p);
 QPtrDictIterator<KPlayer> it(d->mItem2Player);
 while (it.current()) {
	if (it.current() == p) {
		QListBoxItem* i = (QListBoxItem*)it.currentKey();
		int index = mPlayers->index(i);
		if (mPlayers->selectedItem() == i) {
			// Selected item will be removed. Select another item
			if (index == (int)mPlayers->count() - 1) {
				// Last item in list
				mPlayers->setSelected(index - 1, true);
			} else {
				mPlayers->setSelected(index + 1, true);
			}
		}
		mPlayers->removeItem(index);
		d->mItem2Player.remove(i);
		// Update player info
		playerCountChanged();
		slotPlayerSelected(mPlayers->selectedItem());
		return;
	}
	++it;
 }
 boError() << k_funcinfo << "Player not found" << endl;
}

