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

#include "boufonewgamewidget.h"
#include "boufonewgamewidget.moc"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "../bosonconfig.h"
#include "../gameengine/player.h"
#include "../gameengine/speciestheme.h"
#include "../gameengine/bosoncomputerio.h"
#include "../gameengine/boson.h"
#include "../gameengine/bosoncampaign.h"
#include "../gameengine/bosonplayfield.h"
#include "../gameengine/bpfdescription.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/bpfloader.h" // BPFPreview. FIXME: dedicated file!
#include "../bosondata.h"
#include "bosonstartupnetwork.h"
#include "boufocolorchooser.h"
#include "boufomappreview.h"
#include "bodebug.h"

#include <klocale.h>
#include <kgame/kgameproperty.h>
#include <kgame/kgamemessage.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>

#include <qpointer.h>
#include <qtimer.h>
#include <qimage.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>

#warning TODO: implement!
#define HAVE_CHAT_WIDGET 0

class PlayFieldSelection
{
public:
  PlayFieldSelection(BoUfoListBox* campaign, BoUfoListBox* playField)
  {
    BO_CHECK_NULL_RET(campaign);
    BO_CHECK_NULL_RET(playField);
    mSelectCampaign = campaign;
    mSelectPlayField = playField;
    mIsAdmin = true;
  }

  void addCampaign(const BosonCampaign* campaign)
  {
    BO_CHECK_NULL_RET(campaign);
    QString name = campaign->name();
    if (name.isEmpty()) {
      boWarning() << k_funcinfo << "empty campaign name - using <unnamed>" << endl;
      name = i18n("<unnamed>");
    }
    mIndex2Campaign.insert(mSelectCampaign->count(), campaign);
    mSelectCampaign->insertItem(name);

    // AB: WARNING: we leave the current item uninitialized!
    //     we expect the caller to also set the current campaign after adding
    //     all campaigns!
  }

  void setAdmin(bool isAdmin)
  {
    mIsAdmin = isAdmin;
    if (mIsAdmin) {
       mSelectCampaign->setSelectionMode(BoUfoListBox::SingleSelection);
       mSelectPlayField->setSelectionMode(BoUfoListBox::SingleSelection);
       mSelectCampaign->setEnabled(true);
       mSelectPlayField->setEnabled(true);
    } else {
       mSelectCampaign->setSelectionMode(BoUfoListBox::NoSelection);
       mSelectPlayField->setSelectionMode(BoUfoListBox::NoSelection);
       mSelectCampaign->setEnabled(false);
       mSelectPlayField->setEnabled(false);
    }
  }
  void updateCurrentPlayField()
  {
    int campaign = mSelectCampaign->selectedItem();
    int playField = mSelectPlayField->selectedItem();

    campaign = qMax(campaign, 0);
    campaign = qMin(campaign, (int)(mIndex2Campaign.count() - 1));
    setCurrentCampaignWithIndex(campaign);

    playField = qMax(playField, 0);
    playField = qMin(playField, (int)(mIndex2PlayFieldIdentifier.count() - 1));
    setCurrentPlayFieldWithIndex(playField);
  }

  void setCurrentCampaignWithIndex(int index)
  {
    if (index < 0) {
      boError() << k_funcinfo << "invalid index " << index << endl;
      return;
    }
    if ((unsigned int)index >= mIndex2Campaign.count()) {
      boError() << k_funcinfo << "index " << index << " exceeds count " << mIndex2Campaign.count() << endl;
      return;
    }
    if (mIndex2Campaign.count() != mSelectCampaign->count()) {
      boError() << k_funcinfo << "internal error" << endl;
      return;
    }
    const BosonCampaign* campaign = mIndex2Campaign[index];
    BO_CHECK_NULL_RET(campaign);
    int old = mSelectCampaign->selectedItem();
    if (old != index) {
      if (!mIsAdmin) {
          // If client isn't admin, it can't select the campaign, but we still have to
          //  indicate selected campaign, so we temporarily set selection mode to single
          mSelectCampaign->setSelectionMode(BoUfoListBox::SingleSelection);
          mSelectCampaign->blockSignals(true);
          mSelectCampaign->unselectAll();
          mSelectCampaign->setItemSelected(index, true);
          mSelectCampaign->blockSignals(false);
          mSelectCampaign->setSelectionMode(BoUfoListBox::NoSelection);
      } else {
          mSelectCampaign->blockSignals(true);
          mSelectCampaign->setItemSelected(index, true);
          mSelectCampaign->blockSignals(false);
      }
    }
    // AB: note that we have to update mSelectPlayField even if the new index
    // already was selected - setCurrentCampaignWithIndex() is called by the
    // slot that changes the listbox selection for the campaign!

    mIndex2PlayFieldIdentifier.clear();
    mSelectPlayField->clear();
    QStringList list = campaign->playFields();
    QStringList items;
    for (unsigned int i = 0; i < list.count(); i++) {
      BPFPreview* preview = boData->playFieldPreview(list[i]);
      if (!preview) {
        BO_NULL_ERROR(preview);
        continue;
      }
      if (!preview->description()) {
        BO_NULL_ERROR(preview->description());
        continue;
      }
      int index = items.count();
      items.append(preview->description()->name());
      mIndex2PlayFieldIdentifier.insert(index, list[i]);
    }

    mSelectPlayField->setItems(items);

    setCurrentPlayFieldWithIndex(0);
  }

  void setCurrentPlayFieldWithIndex(int index)
  {
    if (index < 0) {
      boError() << k_funcinfo << "invalid index " << index << endl;
      return;
    }
    if ((unsigned int)index >= mIndex2PlayFieldIdentifier.count()) {
      boError() << k_funcinfo << "index " << index << " exceeds count " << mIndex2PlayFieldIdentifier.count() << endl;
      return;
    }
    if (mIndex2PlayFieldIdentifier.count() != mSelectPlayField->count()) {
      boError() << k_funcinfo << "internal error" << endl;
      return;
    }

    int old = mSelectPlayField->selectedItem();
    if (old != index) {
      if (!mIsAdmin) {
          // If client isn't admin, it can't select the map, but we still have to
          //  indicate selected map, so we temporarily set selection mode to single
          mSelectPlayField->setSelectionMode(BoUfoListBox::SingleSelection);
          mSelectPlayField->blockSignals(true);
          mSelectPlayField->unselectAll();
          mSelectPlayField->setItemSelected(index, true);
          mSelectPlayField->blockSignals(false);
          mSelectPlayField->setSelectionMode(BoUfoListBox::NoSelection);
      } else {
          mSelectPlayField->blockSignals(true);
          mSelectPlayField->setItemSelected(index, true);
          mSelectPlayField->blockSignals(false);
      }
    }
  }

  void setCurrentPlayFieldWithIdentifier(const QString& identifier)
  {
    int campaignIndex = -1;
    const BosonCampaign* campaign = 0;
    for (QMap<int, const BosonCampaign*>::iterator it = mIndex2Campaign.begin(); it != mIndex2Campaign.end() && !campaign; ++it) {
      const BosonCampaign* c = it.data();
      if (!c) {
        BO_NULL_ERROR(c);
        continue;
      }
      if (c->playFields().contains(identifier)) {
        campaignIndex = it.key();
        campaign = c;
      }
    }
    if (!campaign || campaignIndex < 0) {
      boError() << k_funcinfo << "cannot find campaign for playfield " << identifier << endl;
      return;
    }

    setCurrentCampaignWithIndex(campaignIndex);

    int playFieldIndex = -1;
    for (QMap<int, QString>::iterator it = mIndex2PlayFieldIdentifier.begin(); it != mIndex2PlayFieldIdentifier.end() && playFieldIndex < 0; ++it) {
      if (it.data() == identifier) {
        playFieldIndex = it.key();
      }
    }
    if (playFieldIndex < 0) {
      boError() << k_funcinfo << "could not find playfield " << identifier << endl;
      return;
    }
    setCurrentPlayFieldWithIndex(playFieldIndex);
  }

  QString playFieldIdentifier() const
  {
    if (mSelectPlayField->selectedItem() < 0) {
      return QString::null;
    }
    if (!mIndex2PlayFieldIdentifier.contains(mSelectPlayField->selectedItem())) {
      return QString::null;
    }
    QString identifier = mIndex2PlayFieldIdentifier[mSelectPlayField->selectedItem()];
    return identifier;
  }

private:
  bool mIsAdmin;
  BoUfoListBox* mSelectCampaign;
  BoUfoListBox* mSelectPlayField;

  QMap<int, const BosonCampaign*> mIndex2Campaign;
  QMap<int, QString> mIndex2PlayFieldIdentifier;
};

class BoUfoNewGameWidgetPrivate
{
public:
    BoUfoNewGameWidgetPrivate()
    {
      mPlayFieldSelection = 0;
      mLocalPlayer = 0;

      mPlayerColor = 0;
    }

    PlayFieldSelection* mPlayFieldSelection;
    QMap<int, KPlayer*> mItem2Player;
    QMap<int, QString> mItem2Map; // "item" is an index in the listbox now

    QMap<int, QString> mSpeciesIndex2Identifier;
    QMap<int, QString> mSpeciesIndex2Comment;

    QMap<int, int> mSideIndex2Side; // index in combobox -> Player::bosonId() of the side

    QPointer<Player> mLocalPlayer;

    BoUfoMapPreview* mMapPreview;
    BoUfoColorChooser* mPlayerColor;

    int mComputerPlayerNumber;
};


BoUfoNewGameWidget::BoUfoNewGameWidget(BosonStartupNetwork* interface)
    : BosonUfoNewGameWidgetBase()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(interface);
 d = new BoUfoNewGameWidgetPrivate;
 mNetworkInterface = interface;

 mSelectedPlayer = 0;
 mMaxPlayers = 0;
 mMinPlayers = 0;
 mInited = false; // Will become true once localplayer gets added
 d->mComputerPlayerNumber = 1;

 d->mPlayFieldSelection = new PlayFieldSelection(mSelectCampaign, mSelectMap);

 d->mPlayerColor = new BoUfoColorChooser();
 mPlayerColorContainer->setLayoutClass(BoUfoWidget::UHBoxLayout);
 mPlayerColorContainer->addWidget(d->mPlayerColor);
 connect(d->mPlayerColor, SIGNAL(signalColorSelected(int)),
        this, SLOT(slotPlayerColorChanged(int)));

 // TODO: use a fixed size
 d->mMapPreview = new BoUfoMapPreview();
 mMapPreviewContainer->setLayoutClass(BoUfoWidget::UHBoxLayout);
 mMapPreviewContainer->addWidget(d->mMapPreview);
// d->mMapPreview->setVerticalAlignment(BoUfoWidget::Qt::AlignVCenter);
// d->mMapPreview->setHorizontalAlignment(BoUfoWidget::Qt::AlignHCenter);

 initPlayFields();
 d->mPlayFieldSelection->setCurrentCampaignWithIndex(0);
 d->mPlayFieldSelection->setCurrentPlayFieldWithIndex(0);
 initSpecies();

 connect(networkInterface(), SIGNAL(signalStartGameClicked()),
        this, SLOT(slotNetStart()));
 connect(networkInterface(), SIGNAL(signalPlayerJoinedGame(KPlayer*)),
        this, SLOT(slotNetPlayerJoinedGame(KPlayer*)));
 connect(networkInterface(), SIGNAL(signalPlayerLeftGame(KPlayer*)),
        this, SLOT(slotNetPlayerLeftGame(KPlayer*)));
 connect(networkInterface(), SIGNAL(signalSpeciesChanged(Player*)),
        this, SLOT(slotNetSpeciesChanged(Player*)));
 connect(networkInterface(), SIGNAL(signalSideChanged(Player*)),
        this, SLOT(slotNetSideChanged(Player*)));
 connect(networkInterface(), SIGNAL(signalTeamColorChanged(Player*)),
        this, SLOT(slotNetColorChanged(Player*)));
 connect(networkInterface(), SIGNAL(signalPlayFieldChanged(BPFPreview*)),
        this, SLOT(slotNetPlayFieldChanged(BPFPreview*)));
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

 d->mPlayerColor->setColors(SpeciesTheme::defaultColors());

 slotNetSetAdmin(boGame->isAdmin());

#if HAVE_CHAT_WIDGET
 mChat->chatWidget()->setKGame(boGame);
#endif
}

BoUfoNewGameWidget::~BoUfoNewGameWidget()
{
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
 QString playFieldIdentifier = d->mPlayFieldSelection->playFieldIdentifier();
 if (!playFieldIdentifier.isEmpty()) {
    boConfig->saveLocalPlayerMap(playFieldIdentifier);
 }
 // TODO: save species. not yet useful as we support one species only.
 delete d;
}

void BoUfoNewGameWidget::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
 if (d->mLocalPlayer && !d->mLocalPlayer->speciesTheme()) {
    initLocalPlayer();
#if HAVE_CHAT_WIDGET
    mChat->chatWidget()->setFromPlayer(p);
#endif
 }
}

void BoUfoNewGameWidget::initLocalPlayer()
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

void BoUfoNewGameWidget::initPlayFields()
{
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
 d->mPlayFieldSelection->addCampaign(defaultCampaign);

 // Add other campaigns
 for (unsigned int i = 0; i < list.count(); i++) {
    BosonCampaign* campaign = boData->campaign(list[i]);
    if (!campaign) {
        BO_NULL_ERROR(campaign);
        continue;
    }
    d->mPlayFieldSelection->addCampaign(campaign);
 }
}

void BoUfoNewGameWidget::initSpecies()
{
 // update possible species:
 mPlayerSpecies->clear();
 d->mSpeciesIndex2Comment.clear();
 d->mSpeciesIndex2Identifier.clear();
  //TODO: some scenarios might not provide all species!
 int defaultIndex = 0;
 QStringList list = SpeciesTheme::availableSpecies();
 int index = 0;
 for (unsigned int i = 0; i < list.count(); i++) {
    KSimpleConfig cfg(list[i]);
    cfg.setGroup("Boson Species");
    QString identifier = cfg.readEntry("Identifier", "Unknown");
    if (identifier == QString::fromLatin1("Neutral")) {
        continue;
    }
    QStringList playerSpeciesItems = mPlayerSpecies->items();
    playerSpeciesItems.insert(playerSpeciesItems.at(index), cfg.readEntry("Name", i18n("Unknown")));
    mPlayerSpecies->setItems(playerSpeciesItems);
    // comments aren't used. Maybe remove this dict?
    d->mSpeciesIndex2Comment.insert(index, cfg.readEntry("Comment", i18n("None")));
    d->mSpeciesIndex2Identifier.insert(index, identifier);
    if (identifier == SpeciesTheme::defaultSpecies()) {
        defaultIndex = index;
    }
    index++;
 }
 mPlayerSpecies->blockSignals(true); // localPlayer() is not yet available, so dont emit signal
 mPlayerSpecies->setCurrentItem(defaultIndex);
 mPlayerSpecies->blockSignals(false);
}

void BoUfoNewGameWidget::updateColors()
{
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

 Q3ValueList<QColor> availableColors = boGame->availableTeamColors();
 availableColors.prepend(p->teamColor());

 // first set all taken, then make those available, that are still available
 d->mPlayerColor->setAllTaken(true);
 for(unsigned int i = 0; i < availableColors.count(); i++) {
    d->mPlayerColor->setTaken(availableColors[i], false);
 }

 d->mPlayerColor->highlightColor(p->teamColor());
}

void BoUfoNewGameWidget::initInitialPlayField()
{
 if (boGame->isAdmin()) {
    // load the map that was selected in a previous game
    QString mapId = boConfig->readLocalPlayerMap();
    if (mapId.isNull()) {
        mapId = BosonPlayField::defaultPlayField();
    }
    networkInterface()->sendChangePlayField(mapId);
 }
}

/*****  Slots for networks commands  *****/

void BoUfoNewGameWidget::slotNetStart()
{
 if (!boGame->isAdmin()) {
    return;
 }
 // Check for map
 if (d->mPlayFieldSelection->playFieldIdentifier().isEmpty()) {
    KMessageBox::sorry(0, i18n("No map selected. Select a map first!"));
    return;
 }
 // Check for valid players count
 if (boGame->gamePlayerCount() > mMaxPlayers) {
    KMessageBox::sorry(0, i18n("There are too many players in game.\n"
            "Current map supports only %1 players, currently, there are %2 players in the game.\n"
            "Please remove some players.").arg(mMaxPlayers).arg(boGame->gamePlayerCount()),
            i18n("Too many players"));
 } else if (boGame->gamePlayerCount() < mMinPlayers) {
    KMessageBox::sorry(0, i18n("There are too few players in game.\n"
            "Current map requires at least %1 players, currently, there are only %2 players in the game.\n"
            "Please add some players.").arg(mMinPlayers).arg(boGame->gamePlayerCount()),
            i18n("Too few players"));
 } else {
    // All good
    //slotPlayerNameChanged();
    // Check if each player has unique team color
    Q3PtrList<Player> gamePlayerList = boGame->gamePlayerList();
    for (unsigned int i = 0; i < gamePlayerList.count() - 1; i++) {
        Player* p = gamePlayerList.at(i);
        if (!p) {
            BO_NULL_ERROR(p);
            continue;
        }
        for (unsigned int j = i + 1; j < gamePlayerList.count(); j++) {
            Player* p2 = gamePlayerList.at(j);
            if (!p2) {
                BO_NULL_ERROR(p2);
                continue;
            }
            if (p->teamColor() == p2->teamColor()) {
                KMessageBox::sorry(0, i18n("Cannot start game - player %1 (id=%2) and %3 (id=%4) have the same teamcolor.\nThis is not allowed!").arg(p->name()).arg(p->bosonId()).arg(p2->name()).arg(p2->bosonId()));
                return;
            }
        }
    }

    QString identifier = d->mPlayFieldSelection->playFieldIdentifier();
    if (identifier.isNull()) {
        KMessageBox::sorry(0, i18n("You have to select a map first"));
        return;
    }
    BPFPreview* preview = boData->playFieldPreview(identifier);

    if (!preview) {
        KMessageBox::sorry(0, i18n("The selected item seems to be a map, but it cannot be found.\nYou have encountered a bug."));
        return;
    }

    bool ret = true;
    if (ret) {
      if (boGame->isAdmin()) {
        ret = networkInterface()->addNeutralPlayer(false);
      }
    }
    boConfig->addDynamicEntryBool("ExploreMapOnStartup");
    boConfig->setBoolValue("ExploreMapOnStartup", mStartExplored->checked());
    if (ret) {
      ret = networkInterface()->sendNewGame(preview, false);
    }
    if (!ret) {
        KMessageBox::sorry(0, i18n("The selected map could not be saved to a network stream. This might be due to a broken map file, or due to a boson bug.\nAnyway - the game cannot be started."));
        return;
    } else {
        boDebug() << k_funcinfo << "newgame message sent successfully" << endl;
    }
 }
}


void BoUfoNewGameWidget::slotNetPlayerJoinedGame(KPlayer* p)
{
 BO_CHECK_NULL_RET(p);
 if (((Player*)p)->isNeutralPlayer()) {
    // Don't show neutral player in players' list
    boDebug() << k_funcinfo << "Added player is neutral player. Returning" << endl;
    return;
 }

 QStringList connectedPlayersItems = mConnectedPlayersList->items();
 int indexOfNewPlayerInList = connectedPlayersItems.count();
 d->mItem2Player.insert(indexOfNewPlayerInList, p);
 connectedPlayersItems.append(p->name());
 mConnectedPlayersList->setItems(connectedPlayersItems);

 if (mConnectedPlayersList->count() == 1) {
    // Localplayer joined the game. No chat message, but select it
    mConnectedPlayersList->setItemSelected(indexOfNewPlayerInList, true);
    mInited = true;
 } else if (mInited) {
    // Display chat message
    if (!p->isVirtual()) {
        // Localplayer added AI player
        boGame->slotAddChatSystemMessage("Boson", i18n("You added AI player %1 to the game").arg(p->name()));
    } else {
        int gameID = KGameMessage::rawGameId(p->kgameId());  // gameID == clientID
        KPlayer* mainPlayer = boGame->findPlayerByKGameId(KGameMessage::createPlayerId(1, gameID));
        if (mainPlayer == p) {
            // Another client connected
            boGame->slotAddChatSystemMessage("Boson", i18n("%1 joined the game").arg(p->name()));
            if (boGame->isAdmin()) {
                boDebug() << k_funcinfo << "new client connected - sending current playfield" << endl;
                QString identifier = d->mPlayFieldSelection->playFieldIdentifier();
                if (!identifier.isEmpty()) {
                    networkInterface()->sendChangePlayField(identifier);
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
    Q3PtrList<Player> gamePlayerList = boGame->gamePlayerList();
    for (int i = 0; i < (int)gamePlayerList.count() - 1; i++) {
        Player* p2 = gamePlayerList.at(i);
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

 updateColors();
 playerCountChanged();
 possibleSidesChanged();
}

void BoUfoNewGameWidget::slotNetPlayerLeftGame(KPlayer* p)
{
 boDebug() << k_funcinfo << "there are " << boGame->allPlayerCount() << " players in game now with " << boGame->gamePlayerCount() << " game players" << endl;

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

void BoUfoNewGameWidget::slotNetSpeciesChanged(Player* p)
{
 if (mSelectedPlayer == p) {
    // Update species combobox
    QMap<int, QString>::Iterator it;
    for (it = d->mSpeciesIndex2Identifier.begin(); it != d->mSpeciesIndex2Identifier.end(); it++) {
        if (it.data() == p->speciesTheme()->identifier()) {
            mPlayerSpecies->blockSignals(true);
            mPlayerSpecies->setCurrentItem(it.key());
            mPlayerSpecies->blockSignals(false);
            break;
        }
    }
 }
}

void BoUfoNewGameWidget::slotNetSideChanged(Player* p)
{
 possibleSidesChanged();
}

void BoUfoNewGameWidget::slotNetColorChanged(Player* p)
{
 // Update color selector widget
 updateColors();
}

void BoUfoNewGameWidget::slotNetPlayFieldChanged(BPFPreview* preview)
{
 BO_CHECK_NULL_RET(preview);
 if (!preview->isLoaded()) {
   boError() << k_funcinfo << "playfieldpreview not yet loaded (all playfieldpreviews should be loaded already at this point" << endl;
   return;
 }
 d->mPlayFieldSelection->setCurrentPlayFieldWithIdentifier(preview->identifier());
 if (d->mPlayFieldSelection->playFieldIdentifier() != preview->identifier()) {
   boError() << k_funcinfo << "could not set correct playfield " << preview->identifier() << endl;
   BPFPreview* p = boData->playFieldPreview(d->mPlayFieldSelection->playFieldIdentifier());
   BO_CHECK_NULL_RET(p);
   preview = p;
   if (!preview->isLoaded()) {
     boError() << k_funcinfo << "fallback playfieldpreview is not loaded (all previews should be loaded at this point)" << endl;
     return;
   }
 }
 mMinPlayers = preview->minPlayers();
 mMaxPlayers = preview->maxPlayers();

 // Update general map info
 mMapSize->setText(i18n("%1x%2\n").arg(preview->mapWidth()).arg(preview->mapHeight()));
 if (mMinPlayers == mMaxPlayers) {
    mMapPlayers->setText(i18n("%1").arg(mMinPlayers));
 } else {
    mMapPlayers->setText(i18n("%1-%2").arg(mMinPlayers).arg(mMaxPlayers));
 }

 // Update description of the map
 const BPFDescription* description = preview->description();
 if (!description) {
   boError() << k_funcinfo << "unable to load the description" << endl;
   return;
 }
 // AB: I am not fully sure if a text browser is the right choice for this
 // widget. but being able to use links in the description is surely a good
 // idea.
 if (description->comment().isEmpty()) {
    mMapDescription->setText(i18n("There is no comment for this map available"));
 } else {
    mMapDescription->setText(description->comment());
 }
 d->mMapPreview->setPlayField(*preview);
 playerCountChanged();
 possibleSidesChanged();
}

void BoUfoNewGameWidget::slotNetPlayerNameChanged(Player* p)
{
 BO_CHECK_NULL_RET(p);

 if (p == mSelectedPlayer) {
    mPlayerName->setText(p->name());
 }

 QMap<int, KPlayer*>::iterator it;
 for (it = d->mItem2Player.begin(); it != d->mItem2Player.end(); ++it) {
    if ((*it) == (KPlayer*)p) {
        if (mInited) {
            if (p == localPlayer()) {
                boGame->slotAddChatSystemMessage("Boson", i18n("You are now known as %1").arg(p->name()));
            } else {
                QString oldName = mConnectedPlayersList->itemText(it.key());
                boGame->slotAddChatSystemMessage("Boson", i18n("%1 is now known as %2").arg(oldName).arg(p->name()));
            }
        }

        mConnectedPlayersList->setItemText(it.key(), p->name());

        // Setting local player's name is last step of initing when going to start
        //  game page. If name was set for localplayer and we wasn't inited yet,
        //  then now we are
        if (p == localPlayer() && !mInited) {
            mInited = true;
        }
        return;
    }
 }
 boWarning() << k_funcinfo << "could not find player " << p->bosonId() << endl;
}

void BoUfoNewGameWidget::slotNetSetLocalPlayer(Player* p)
{
}

void BoUfoNewGameWidget::slotNetSetAdmin(bool admin)
{
 slotSetAdmin(admin);
}

void BoUfoNewGameWidget::slotNetConnectionBroken()
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

void BoUfoNewGameWidget::slotPlayerNameChanged()
{
 BO_CHECK_NULL_RET(localPlayer());
 Player* p = mSelectedPlayer;
 if (!p) {
    p = localPlayer();
 }
 // Admin can change names of all players
 // Non-admin players can only change names of non-virtual players
 if (!boGame->isAdmin() && mSelectedPlayer->isVirtual()) {
    // This shouldn't happen, name lineedit should be read-only in this case
    boError() << k_funcinfo << "Can't set name for player " << p->bosonId() << endl;
    mPlayerName->setText(p->name());
    return;
 }

 QString name = mPlayerName->text();
 if (name != p->name()) {
    networkInterface()->sendChangePlayerName(p, name);
 }
}

void BoUfoNewGameWidget::slotPlayerColorChanged(int index)
{
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
    boError() << k_funcinfo << "Can't set color for player " << mSelectedPlayer->bosonId() << endl;
    KMessageBox::sorry(0, i18n("Only ADMIN can change color of other players!"));
    return;
 }

 QColor color = SpeciesTheme::defaultColors()[index];

 networkInterface()->sendChangeTeamColor(p, color);

 if (p == localPlayer()) {
    mLocalPlayerColor = color;
 }
}

void BoUfoNewGameWidget::slotPlayFieldSelected(int first, int last)
{
 // AB: first and last are barely usable. there is no way to find out _which_
 // one of these is selected _currently_. better just ignore them.
 Q_UNUSED(first);
 Q_UNUSED(last);
 d->mPlayFieldSelection->updateCurrentPlayField();
 QString identifier = d->mPlayFieldSelection->playFieldIdentifier();
 networkInterface()->sendChangePlayField(identifier);
}

void BoUfoNewGameWidget::slotCampaignSelected(int first, int last)
{
 // AB: first and last are barely usable. there is no way to find out _which_
 // one of these is selected _currently_. better just ignore them.
 Q_UNUSED(first);
 Q_UNUSED(last);
 d->mPlayFieldSelection->updateCurrentPlayField();
 QString identifier = d->mPlayFieldSelection->playFieldIdentifier();
 networkInterface()->sendChangePlayField(identifier);
}

void BoUfoNewGameWidget::slotPlayerSpeciesChanged(int index)
{
 BO_CHECK_NULL_RET(localPlayer());

 Player* p = mSelectedPlayer;
 if (!p) {
    p = localPlayer();
 }
 if (!boGame->isAdmin() && mSelectedPlayer->isVirtual()) {
    boError() << k_funcinfo << "Can't set species for player " << p->bosonId() << endl;
    return;
 }

 if (index >= (int)d->mSpeciesIndex2Identifier.count()) {
    boError() << k_funcinfo << "invalid index " << index << endl;
    return;
 }
 networkInterface()->sendChangeSpecies(p, d->mSpeciesIndex2Identifier[index], p->speciesTheme()->teamColor());
}

void BoUfoNewGameWidget::slotPlayerSideChanged(int index)
{
 BO_CHECK_NULL_RET(localPlayer());

 Player* p = mSelectedPlayer;
 if (!p) {
    p = localPlayer();
 }
 if (!boGame->isAdmin() && mSelectedPlayer->isVirtual()) {
    boError() << k_funcinfo << "Can't set side for player " << p->bosonId() << endl;
    return;
 }

 if (index >= (int)d->mSideIndex2Side.count()) {
    boError() << k_funcinfo << "invalid index " << index << endl;
    return;
 }
 networkInterface()->sendChangeSide(p, d->mSideIndex2Side[index]);
}

void BoUfoNewGameWidget::slotAddComputerPlayer()
{
 BO_CHECK_NULL_RET(boGame);
 // FIXME: maybe allow other players to add AIs?
 if (!boGame->isAdmin()) {
    KMessageBox::sorry(0, i18n("You must be ADMIN to do this"));
    return;
 }

 Player* p = new Player();

 // AB: we don't have access to libkdegames anymore. one day we will have
 // computer players with names that fit to the story (if we'll ever have one).
 // so the randomName() isn't required anyway.
// p->setName(KGameMisc::randomName());
 p->setName(i18n("Computer %1").arg(d->mComputerPlayerNumber));
 d->mComputerPlayerNumber++;

 // the color is dangerous concerning network and so!
 // it'd be better to first add the player and then change the color using a
 // network message. unfortunately we can't send a message, since we do not yet
 // have the ID of the new player.
 QColor color = boGame->availableTeamColors().first();
 p->loadTheme(SpeciesTheme::speciesDirectory(SpeciesTheme::defaultSpecies()), color);

 BosonComputerIO* io = new BosonComputerIO();
 p->addGameIO(io);
 if (!io->initializeIO()) {
   boError() << k_funcinfo << "could not initialize IO" << endl;
   KMessageBox::sorry(0, i18n("An error occured while initializing computer IO. Could not add the player."));
   delete p;
   return;
 }
 boGame->bosonAddPlayer(p);
}

void BoUfoNewGameWidget::slotRemovePlayer()
{
 BO_CHECK_NULL_RET(localPlayer());

 if (mSelectedPlayer != localPlayer()) {
    networkInterface()->removePlayer(mSelectedPlayer);
 }
}

void BoUfoNewGameWidget::slotPlayerSelected(int first, int last)
{
 // AB: first and last are barely usable. there is no way to find out _which_
 // one of these is selected _currently_. better just ignore them.
 Q_UNUSED(first);
 Q_UNUSED(last);
 int selected = mConnectedPlayersList->selectedItem();
 slotPlayerSelected(selected);
}

void BoUfoNewGameWidget::slotPlayerSelected(int index)
{
 if (d->mItem2Player.contains(index)) {
   mSelectedPlayer = (Player*)d->mItem2Player[index];
 } else {
   mSelectedPlayer = 0;
 }
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
        mPlayerSpecies->blockSignals(true);
        mPlayerSpecies->setCurrentItem(it.key());
        mPlayerSpecies->blockSignals(false);
        break;
    }
 }
 possibleSidesChanged();

 // Enable/disable some stuff
 bool canChange = false;
 if (boGame->isAdmin() || !mSelectedPlayer->isVirtual()) {
    canChange = true;
 }
 mPlayerName->setEditable(canChange);
 d->mPlayerColor->setEnabled(canChange);
 mPlayerSpecies->setEnabled(canChange);
 mPlayerDesiredSide->setEnabled(canChange);
 mRemovePlayer->setEnabled(canChange && (mSelectedPlayer != localPlayer()));
}

/*****  Other stuff  *****/

Player* BoUfoNewGameWidget::localPlayer() const
{
 return d->mLocalPlayer;
}

void BoUfoNewGameWidget::slotCancel()
{
 // AB: we use a timer, so that the newgame widget can be deleted in the slot
 // (otherwise this would not be allowed, as we are in a pushbutton click)
 QTimer::singleShot(0, this, SIGNAL(signalCancelled()));
}

void BoUfoNewGameWidget::slotStartGame()
{
 boDebug() << k_funcinfo << endl;
 networkInterface()->sendStartGameClicked();
}

void BoUfoNewGameWidget::addAIPlayer()
{
 // commandline interface for adding computer players
 slotAddComputerPlayer();
}

void BoUfoNewGameWidget::playersChanged()
{
}

void BoUfoNewGameWidget::playerCountChanged()
{
 if (mConnectedPlayersList->count() >= mMaxPlayers) {
    mAddAIPlayer->setEnabled(false);
 } else {
    // ATM only ADMIN can add AIs. Change this if that will be changed!
    mAddAIPlayer->setEnabled(boGame->isAdmin());
 }
 // Maybe also disable/enable start button if player count is invalid?
}

void BoUfoNewGameWidget::possibleSidesChanged()
{
 BO_CHECK_NULL_RET(boGame);
 mPlayerDesiredSide->clear();
 d->mSideIndex2Side.clear();

 if (!mSelectedPlayer) {
   boDebug() << k_funcinfo << "no player selected" << endl;
   return;
 }

 QStringList sides;

#warning TODO: read available sides from map
 // TODO: insert possible sides of the map
 unsigned int maxPlayers = BOSON_MAX_PLAYERS;
 maxPlayers = qMin(maxPlayers, mMaxPlayers);
 maxPlayers = qMax(maxPlayers, mMinPlayers);
 for (unsigned int i = 0; i < maxPlayers; i++) {
   int id = 128 + i;
   Player* p = (Player*)boGame->findPlayerByUserId(id);
   if (p && p != mSelectedPlayer) {
     continue;
   }
   sides.append(i18n("Player %1 (Id=%2)").arg(i + 1).arg(id));
   d->mSideIndex2Side.insert(sides.count() - 1, id);
 }

 int watchId = 1; // ID of a player who watches the game only (but won't be able to play)
 {
   bool isTaken = false;
   watchId = 1;
   do {
     isTaken = false;
     Q3PtrListIterator<Player> it(boGame->allPlayerList());
     for (; it.current(); ++it) {
       Player* p = it.current();
       if (p->bosonId() == watchId && p != mSelectedPlayer) {
         isTaken = true;
       }
     }
     if (isTaken) {
       watchId++;
     }
   } while (isTaken && watchId <= 127);
 }

 // AB: by definition a "watch" player must have an Id >= 1 and <= 127.
 //     See http://boson.freehackers.org/wiki/Main/PlayerIds
 if (watchId >= 1 && watchId <= 127) {
   sides.append(i18n("Watch game only (Id = %1)").arg(watchId));
   d->mSideIndex2Side.insert(sides.count() - 1, watchId);
 }

 mPlayerDesiredSide->setItems(sides);

 if (d->mSideIndex2Side.count() != mPlayerDesiredSide->count()) {
   boError() << k_funcinfo << "invalid side count" << endl;
   d->mSideIndex2Side.clear();
   mPlayerDesiredSide->clear();
   return;
 }

 BO_CHECK_NULL_RET(mSelectedPlayer);
 int selectedPlayerSide = mSelectedPlayer->bosonId();
 for (QMap<int, int>::iterator it = d->mSideIndex2Side.begin(); it != d->mSideIndex2Side.end(); it++) {
   if (it.data() == selectedPlayerSide) {
     mPlayerDesiredSide->blockSignals(true);
     mPlayerDesiredSide->setCurrentItem(it.key());
     mPlayerDesiredSide->blockSignals(false);
     break;
   }
 }
}

void BoUfoNewGameWidget::slotSetAdmin(bool admin)
{
 mStartGame->setEnabled(admin);
 mAddAIPlayer->setEnabled(admin);
 d->mPlayFieldSelection->setAdmin(admin);
}

void BoUfoNewGameWidget::slotOfferingConnections()
{
 boGame->slotAddChatSystemMessage("Boson", i18n("You are now running as server and are offering connections"));
}

void BoUfoNewGameWidget::slotConnectingToServer()
{
 mInited = false;
}

void BoUfoNewGameWidget::slotConnectedToServer()
{
 boGame->slotAddChatSystemMessage("Boson", i18n("You are now connected to the server %1:%2").arg(boGame->bosonHostName()).arg(boGame->bosonPort()));
 mInited = true;
 // Select localplayer
 QMap<int, KPlayer*>::iterator it;
 for (it = d->mItem2Player.begin(); it != d->mItem2Player.end(); ++it) {
    if ((*it) == (KPlayer*)localPlayer()) {
        mConnectedPlayersList->setItemSelected(it.key(), true);
        break;
    }
 }
}

void BoUfoNewGameWidget::slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer)
{
#if HAVE_CHAT_WIDGET
 BO_CHECK_NULL_RET(mChat);
 BO_CHECK_NULL_RET(mChat->chatWidget());

 if (forPlayer && forPlayer != localPlayer()) {
    return;
 }
 mChat->chatWidget()->addSystemMessage(fromName, text);
#endif
}

void BoUfoNewGameWidget::removePlayer(KPlayer* p)
{
 this->disconnect(p);
 QMap<int, KPlayer*>::iterator it;
 for (it = d->mItem2Player.begin(); it != d->mItem2Player.end(); ++it) {
    if ((*it) == p) {
        int index = it.key();
        if (mConnectedPlayersList->selectedItem() == index) {
            // Selected item will be removed. Select another item
            if (index == (int)mConnectedPlayersList->count() - 1) {
                // Last item in list
                mConnectedPlayersList->setItemSelected(index - 1, true);
            } else {
                mConnectedPlayersList->setItemSelected(index + 1, true);
            }
        }
        mConnectedPlayersList->removeItem(index);
        d->mItem2Player.remove(index);

        {
          // fix indices
          QMap<int, KPlayer*> fix;
          QMap<int, KPlayer*>::iterator it2;
          for (it2 = d->mItem2Player.begin(); it2 != d->mItem2Player.end(); ++it2) {
            if (it2.key() > index) {
              fix.insert(it2.key(), it2.data());
            }
          }
          for (it2 = fix.begin(); it2 != fix.end(); ++it2) {
            d->mItem2Player.remove(it2.key());
            int i = it2.key() - 1;
            d->mItem2Player.insert(i, it2.data());
          }
        }

        // Update player info
        playerCountChanged();
        possibleSidesChanged();
        slotPlayerSelected(mConnectedPlayersList->selectedItem());
        return;
    }
 }
 boError() << k_funcinfo << "Player not found" << endl;
}

/*
 * vim: et sw=2
 */
