/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonwidgetbase.h"
#include "bosonwidgetbase.moc"

#include "defines.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "bosonsaveload.h"
#include "player.h"
#include "unit.h"
#include "bosonmessage.h"
#include "bosonplayfield.h"
#include "bosonmap.h"
#include "bosonconfig.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "bosonbigdisplaybase.h"
#include "bosonbigdisplayinput.h"
#include "editorbigdisplayinput.h"
#include "boselection.h"
#include "global.h"
#include "bodebug.h"
#include "bosonprofiling.h"
#include "optionsdialog.h"
#include "boaction.h"
#include "bosonlocalplayerinput.h"
#include "bosoncomputerio.h"
#include "bosonmodeltextures.h"
#include "sound/bosonaudiointerface.h"
#include "script/bosonscript.h"
#include "bosonwidgets/bogamechat.h"
#include "bosonpath.h"
#include "bomeshrenderermanager.h"
#include "bogroundrenderermanager.h"
#include "boglstatewidget.h"
#include "boconditionwidget.h"
#include "bocamerawidget.h"
#include "boeventlistener.h"
#include "bowater.h"
#include "bo3dtools.h"

#include <kapplication.h>
#include <klocale.h>
#include <kaction.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kgame/kgamedebugdialog.h>
#include <kgame/kgamepropertyhandler.h>
#include <klineedit.h>
#include <kdockwidget.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qptrdict.h>
#include <qsignalmapper.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>
#include <qdom.h>
#include <qimage.h>

#include <stdlib.h>

class BosonWidgetBase::BosonWidgetBasePrivate
{
public:
	BosonWidgetBasePrivate()
	{
		mChat = 0;

		mCanvas = 0;
	}

	BoGameChatWidget* mChat;

	QPtrDict<KPlayer> mPlayers; // needed for debug only

	bool mInitialized;

	BosonCanvas* mCanvas;
};

BosonWidgetBase::BosonWidgetBase(QWidget* parent)
    : QWidget( parent, "BosonWidgetBase" )
{
 d = new BosonWidgetBasePrivate;
 d->mInitialized = false;

 mDisplayManager = 0;
 mCursor = 0;
 mLocalPlayer = 0;
 mLocalPlayerInput = 0;
}

BosonWidgetBase::~BosonWidgetBase()
{
 boDebug() << k_funcinfo << endl;
 if (factory()) {
	// remove the bosonwidget-specific menus from the XML GUI (menubar,
	// toolbar, ...)
	factory()->removeClient(this);
 }
 if (displayManager()) {
	// we do NOT delete the display manager here
	setDisplayManager(0);
 }
 d->mPlayers.clear();

 delete mCursor;
 delete d->mChat;

 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidgetBase::setDisplayManager(BoDisplayManager* displayManager)
{
 if (mDisplayManager) {
	mDisplayManager->hide();
	mDisplayManager->reparent(0, QPoint(0, 0)); // we do NOT own the display manager!
 }
 mDisplayManager = displayManager;
 if (mDisplayManager) {
	if (mDisplayManager->parent()) {
		boError() << k_funcinfo << "the displaymanager already has a parent - reparenting..." << endl;
	}
	mDisplayManager->reparent(this, QPoint(0, 0));
	mDisplayManager->show();

	changeToConfigCursor();
 }
}

BosonCanvas* BosonWidgetBase::canvas() const
{
 return d->mCanvas;
}

#include <kstandarddirs.h> //locate()
void BosonWidgetBase::init(KDockWidget* chatDock)
{
 // NOTE: order of init* methods is very important here, so don't change it,
 //  unless you know what you're doing!
 if (d->mInitialized) {
	return;
 }
 d->mInitialized = true;
 initChat(chatDock);
 initDisplayManager();

 initConnections();
 actionCollection()->setWidget(this); // needs to be called *before* initKActions()
 initKActions();
 // XMLClient stuff. needs to be called *after* initKActions().
 setBosonXMLFile();
// setXML(top()->domDocument().toString());
 setXMLGUIBuildDocument(QDomDocument());
 // XMLClient stuff ends. note that there is a factory()->addClient() in
 // TopWidget!

 setFocusPolicy(StrongFocus); // accept key event
// setFocus(); // nonsense, since its still hidden

 BosonScript::setGame(boGame);
}

void BosonWidgetBase::initConnections()
{
 connect(boGame, SIGNAL(signalLoadExternalStuffFromXML(const QDomElement&)),
		this, SLOT(slotLoadExternalStuffFromXML(const QDomElement&)));
 connect(boGame, SIGNAL(signalSaveExternalStuffAsXML(QDomElement&)),
		this, SLOT(slotSaveExternalStuffAsXML(QDomElement&)));

 connect(boGame, SIGNAL(signalAddChatSystemMessage(const QString&, const QString&, const Player*)),
		this, SLOT(slotAddChatSystemMessage(const QString&, const QString&, const Player*)));
}

void BosonWidgetBase::initDisplayManager()
{
 BO_CHECK_NULL_RET(displayManager());
 BosonBigDisplayBase* display = displayManager()->activeDisplay();
 BO_CHECK_NULL_RET(display);
 // dont do the connect()s here, as some objects might not be deleted and
 // therefore we do the same connect twice if an endgame() occurs!
 connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
		displayManager(), SLOT(slotAdvance(unsigned int, bool)));
 connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
		this, SLOT(slotAdvance(unsigned int, bool)));

 display->setLocalPlayerIO(localPlayerIO()); // this does nothing.

 connect(localPlayer(), SIGNAL(signalUnitChanged(Unit*)),
		display, SLOT(slotUnitChanged(Unit*)));
}


void BosonWidgetBase::initChat(KDockWidget* chatDock)
{
 // note: we can use the chat widget even for editor mode, e.g. for status
 // messages!
 d->mChat = new BoGameChatWidget(chatDock, "chatwidget", boGame, BosonMessage::IdChat);
 connect(d->mChat->chatWidget(), SIGNAL(signalScriptCommand(const QString&)),
		this, SLOT(slotRunScriptLine(const QString&)));
 chatDock->setWidget(d->mChat);
}

void BosonWidgetBase::initPlayer()
{
 boDebug() << k_funcinfo << endl;
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

 connect(localPlayer(), SIGNAL(signalPropertyChanged(KGamePropertyBase*, KPlayer*)),
		this, SLOT(slotPlayerPropertyChanged(KGamePropertyBase*, KPlayer*)));

 // Needed for loading game
 emit signalMineralsUpdated(localPlayer()->minerals());
 emit signalOilUpdated(localPlayer()->oil());
 slotUnitCountChanged(localPlayer());
}

void BosonWidgetBase::initGameMode()//FIXME: rename! we don't have a difference to initEditorMode anymore. maybe just initGame() or so??
{
 BO_CHECK_NULL_RET(displayManager());
 BO_CHECK_NULL_RET(displayManager()->activeDisplay());

 // Init all bigdisplays // AB: there is only one now
 initBigDisplay(displayManager()->activeDisplay());

 initLayout();
 startScenarioAndGame();

 initScripts();

#ifdef PATHFINDER_TNG
 // FIXME: this isn't correct I suppose. But atm it's only used for debugging
 //  anyway (and I don't intend to use it for anything else)
 boDebug() << k_funcinfo << "Trying searching sample path" << endl;
 BosonPathInfo i;
 i.start = BoVector2Fixed(5, 5);
 i.dest = BoVector2Fixed(45, 35);
 boDebug() << k_funcinfo << "Let's go!" << endl;
 canvas()->pathfinder()->findPath(&i);
 boDebug() << k_funcinfo << "sample path searching complete" << endl;
#endif
}

void BosonWidgetBase::initBigDisplay(BosonBigDisplayBase* b)
{
 if (!b) {
	boError() << k_funcinfo << "NULL display" << endl;
	return;
 }
 if (b->isInputInitialized()) {
	// Already initialized
	return;
 }
 BO_CHECK_NULL_RET(boGame);
 if (boGame->gameMode()) {
	BosonBigDisplayInput* i = new BosonBigDisplayInput(b);
	b->setDisplayInput(i);
 } else {
	EditorBigDisplayInput* i = new EditorBigDisplayInput(b);
	b->setDisplayInput(i);
 }
 connect(b->displayInput(), SIGNAL(signalLockAction(bool)),
		displayManager(), SIGNAL(signalLockAction(bool)));
 connect(b, SIGNAL(signalUnfogAll()),
		this, SLOT(slotUnfogAll()));
 connect(b, SIGNAL(signalSetGrabMovie(bool)),
		displayManager(), SLOT(slotSetGrabMovie(bool)));
 b->setCanvas(canvas());

 // FIXME: this should be done by this->setLocalPlayer(), NOT here!
 // (setLocalPlayer() is also called when changing player in editor mode)
 b->setLocalPlayerIO(localPlayer()->playerIO()); // AB: this will also add the mouseIO!

 b->setCursor(mCursor);
 b->setKGameChat(d->mChat->chatWidget());

 b->show();

 b->setInputInitialized(true);
}

void BosonWidgetBase::initLayout()
{
 boDebug() << k_funcinfo << endl;

 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addWidget(displayManager());

 emit signalLoadBosonGameDock();
}

void BosonWidgetBase::changeCursor(BosonCursor* cursor)
{
 if (!cursor) {
	boError() << k_funcinfo << "NULL cursor" << endl;
	return;
 }
 delete mCursor;
 mCursor = cursor;
 displayManager()->activeDisplay()->setCursor(mCursor);
}

void BosonWidgetBase::slotHack1()
{
 BosonBigDisplayBase* display = displayManager()->activeDisplay();
 QSize size = display->size();
 display->resize(size.width() - 1, size.height() - 1);
 display->resize(size);
}

void BosonWidgetBase::slotItemAdded(BosonItem* item)
{
 if (!item) {
	boError() << k_funcinfo << "NULL item" << endl;
	return;
 }
 if (!RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* unit = (Unit*)item;
 Player* p = unit->owner();
 if (!p) {
	boError() << k_funcinfo << "NULL owner" << endl;
	return;
 }
 if (p != localPlayer()) {
	return;
 }

 slotUnitCountChanged(p);
}

void BosonWidgetBase::slotUnitRemoved(Unit* unit)
{
 if (unit->owner() != localPlayer()) {
	return;
 }

 slotUnitCountChanged(unit->owner());
}

void BosonWidgetBase::slotPlayerPropertyChanged(KGamePropertyBase* prop, KPlayer* p)
{
 if (p != localPlayer()) {
	// not yet used
	return;
 }
 switch (prop->id()) {
	case Player::IdMinerals:
		emit signalMineralsUpdated(localPlayer()->minerals());
		break;
	case Player::IdOil:
		emit signalOilUpdated(localPlayer()->oil());
		break;
	default:
		break;
 }
}

bool BosonWidgetBase::sound() const
{
 return boAudio->sound();
}

bool BosonWidgetBase::music() const
{
 return boAudio->music();
}

void BosonWidgetBase::slotAddChatSystemMessage(const QString& fromName, const QString& text, const Player* forPlayer)
{
 if (forPlayer && forPlayer != localPlayer()) {
	return;
 }
 // add a chat system-message *without* sending it over network (makes no sense
 // for system messages)
 d->mChat->chatWidget()->addSystemMessage(fromName, text);

 displayManager()->activeDisplay()->addChatMessage(i18n("--- %1: %2").arg(fromName).arg(text));
}

void BosonWidgetBase::slotUnfogAll(Player* pl)
{
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!boGame->playField()) {
	boError() << k_funcinfo << "NULL playField" << endl;
	return;
 }
 BosonMap* map = boGame->playField()->map();
 if (!map) {
	boError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 QPtrList<KPlayer> list;
 if (!pl) {
	list = *boGame->playerList();
 } else {
	list.append(pl);
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	Player* p = (Player*)list.at(i);
	for (unsigned int x = 0; x < map->width(); x++) {
		for (unsigned int y = 0; y < map->height(); y++) {
			p->unfog(x, y);
		}
	}
	boGame->slotAddChatSystemMessage(i18n("Debug"), i18n("Unfogged player %1 - %2").arg(p->id()).arg(p->name()));
 }
}

void BosonWidgetBase::slotCmdBackgroundChanged(const QString& file)
{
 // AB: this has been disabled for now. once the commandframe has been ported to
 // OpenGL, we will support backgrounds again (I hope), but it will work in a
 // different way, we won't be able to use this code anymore.
#if 0
 if (file.isNull()) {
	cmdFrame()->unsetPalette();
	return;
 }
 QPixmap p(file);
 if (p.isNull()) {
	boError() << k_funcinfo << "Could not load " << file << endl;
	cmdFrame()->unsetPalette();
	return;
 }
 cmdFrame()->setPaletteBackgroundPixmap(p);
#endif
}

void BosonWidgetBase::initKActions()
{
}

void BosonWidgetBase::quitGame()
{
// this needs to be done first, before the players are removed
 boDebug() << k_funcinfo << endl;
 boGame->quitGame();
 d->mCanvas = 0;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidgetBase::saveConfig()
{
 // note: the game is *not* saved here! just general settings like game speed,
 // player name, ...
 boDebug() << k_funcinfo << endl;
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 if (!localPlayer()) {
	boError() << k_funcinfo << "NULL local player" << endl;
	return;
 }

// boConfig->save(editor); //FIXME - what is this for?
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonWidgetBase::startScenarioAndGame()
{
 boDebug() << k_funcinfo << endl;
 // Center home base if new game was started. If game is loaded, camera was
 //  already loaded as well
 // FIXME: this is hackish but I don't know any other way of checking if game
 //  is loaded or new one here. Feel free to improve
 if (boGame->loadingStatus() != BosonSaveLoad::LoadingCompleted) {
	slotCenterHomeBase();
 }
}

void BosonWidgetBase::setBosonXMLFile()
{
 QString file = locate("config", "ui/ui_standards.rc", instance());
 setXMLFile(file);
 setXMLFile("bosonbaseui.rc", true);
}

void BosonWidgetBase::setLocalPlayer(Player* p)
{
 if (mLocalPlayer) {
	KGameIO* oldIO = mLocalPlayer->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
	if (oldIO) {
		mLocalPlayer->removeGameIO(oldIO);
	}
	mLocalPlayerInput = 0;
 }
 mLocalPlayer = p;
 if (mLocalPlayer) {
	mLocalPlayerInput = new BosonLocalPlayerInput();
	connect(mLocalPlayerInput, SIGNAL(signalAction(const BoSpecificAction&)),
			displayManager(), SLOT(slotAction(const BoSpecificAction&)));
	mLocalPlayer->addGameIO(mLocalPlayerInput);
 }
 if (displayManager()) {
	displayManager()->activeDisplay()->setLocalPlayerIO(localPlayer()->playerIO());
 }
 if (!boGame) {
	boError() << k_funcinfo << "NULL game object" << endl;
	return;
 }
 if (d->mChat) {
	d->mChat->chatWidget()->setFromPlayer(localPlayer());
 }
 boWaterManager->setLocalPlayerIO(localPlayer()->playerIO());
}

void BosonWidgetBase::slotUnitCountChanged(Player* p)
{
 emit signalMobilesCount(p->mobilesCount());
 emit signalFacilitiesCount(p->facilitiesCount());
}

void BosonWidgetBase::slotLoadExternalStuffFromXML(const QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 // TODO: load camera
 // TODO: load unitgroups
 displayManager()->loadFromXML(root);
}

void BosonWidgetBase::slotSaveExternalStuffAsXML(QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 // TODO: save camera  (BosonBigDisplayBase?)
 // TODO: save unitgroups  (BoDisplayManager?)
 displayManager()->saveAsXML(root);
}

OptionsDialog* BosonWidgetBase::gamePreferences(bool editor)
{
 OptionsDialog* dlg = new OptionsDialog(editor, this);
 dlg->setGame(boGame);
 dlg->setPlayer(localPlayer());
 dlg->slotLoad();

 connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater())); // seems not to be called if you quit with "cancel"!

 connect(dlg, SIGNAL(signalCursorChanged(int, const QString&)),
		this, SLOT(slotChangeCursor(int, const QString&)));
 connect(dlg, SIGNAL(signalCmdBackgroundChanged(const QString&)),
		this, SLOT(slotCmdBackgroundChanged(const QString&)));
 connect(dlg, SIGNAL(signalOpenGLSettingsUpdated()),
		displayManager(), SLOT(slotUpdateOpenGLSettings()));
 connect(dlg, SIGNAL(signalApply()),
		this, SLOT(slotApplyOptions()));
 connect(dlg, SIGNAL(signalFontChanged(const BoFontInfo&)),
		displayManager(), SLOT(slotChangeFont(const BoFontInfo&)));

 return dlg;
}

void BosonWidgetBase::slotApplyOptions()
{
 // apply all options from boConfig to boson, that need to be applied. all
 // options that are stored in boConfig only don't need to be touched.
 // AB: cursor is still a special case and not handled here.
 // AB: FIXME: cmdbackground is not yet stored in boConfig! that option should
 // be managed here!
 boDebug() << k_funcinfo << endl;
 displayManager()->slotUpdateIntervalChanged(boConfig->updateInterval()); // FIXME: no slot anymore
 displayManager()->setToolTipCreator(boConfig->toolTipCreator());
 displayManager()->setToolTipUpdatePeriod(boConfig->toolTipUpdatePeriod());
}

void BosonWidgetBase::slotRunScriptLine(const QString& line)
{
 mLocalPlayerInput->eventListener()->script()->execLine(line);
}

void BosonWidgetBase::slotAdvance(unsigned int, bool)
{
}

void BosonWidgetBase::initScripts()
{
 displayManager()->activeDisplay()->setLocalPlayerScript(mLocalPlayerInput->eventListener()->script());
}

void BosonWidgetBase::changeToConfigCursor()
{
 slotChangeCursor(boConfig->cursorMode(), boConfig->cursorDir());
}

void BosonWidgetBase::setCanvas(BosonCanvas* canvas)
{
 d->mCanvas = canvas;
 connect(d->mCanvas, SIGNAL(signalUnitRemoved(Unit*)),
		this, SLOT(slotUnitRemoved(Unit*)));
 connect(d->mCanvas, SIGNAL(signalItemAdded(BosonItem*)),
		this, SLOT(slotItemAdded(BosonItem*)));

 BosonScript::setCanvas(d->mCanvas);
}

void BosonWidgetBase::initMap()
{
 // implemented by EditorWidget
}

PlayerIO* BosonWidgetBase::localPlayerIO() const
{
 if (localPlayer()) {
	return localPlayer()->playerIO();
 }
 return 0;
}

void BosonWidgetBase::slotCenterHomeBase()
{
 BO_CHECK_NULL_RET(displayManager());
 BosonBigDisplayBase* display = displayManager()->activeDisplay();
 BO_CHECK_NULL_RET(display);
 display->slotCenterHomeBase();
}

