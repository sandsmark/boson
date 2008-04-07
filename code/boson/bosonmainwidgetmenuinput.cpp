/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonmainwidgetmenuinput.h"
#include "bosonmainwidgetmenuinput.moc"

#include "../bomemory/bodummymemory.h"
#include "boufo/boufoaction.h"
#include "bodebug.h"
#include "bodebuglogdialog.h"
#include "bosonprofiling.h"
#include "bosonprofilingdialog.h"
#include "bosonconfig.h"
#include "bosondata.h"
#include "boson.h"
#include "bosonmessageids.h"
#include "sound/bosonaudiointerface.h"
#include "kgameunitdebug.h"
#include "kgameplayerdebug.h"
#include "kgameadvancemessagesdebug.h"
#include "boglstatewidget.h"
#include "bofullscreen.h"

#include <kgame/kgamedebugdialog.h>

#include <klocale.h>
#include <kshortcut.h>
#include <KVBox>

#include <qinputdialog.h>
#include <q3vbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <QPixmap>
#include <QCoreApplication>

#include <unistd.h>


static QString findSaveFileName(const QString& prefix, const QString& suffix)
{
 QString file;
 for (int i = 0; i < 1000; i++) {
	file.sprintf("%s-%03d.%s", prefix.latin1(), i, suffix.latin1());
	if (!QFile::exists(file)) {
		return QFileInfo(file).absoluteFilePath();
	}
 }
 return QString::null;
}



class BosonMainWidgetMenuInputPrivate
{
public:
	BosonMainWidgetMenuInputPrivate()
	{
		mActionCollection = 0;
	}
	BoUfoActionCollection* mActionCollection;
};

BosonMainWidgetMenuInput::BosonMainWidgetMenuInput(BoUfoActionCollection* parentCollection, QObject* parent)
		: QObject(parent)
{
 init(parentCollection);
}

BosonMainWidgetMenuInput::~BosonMainWidgetMenuInput()
{
 delete d->mActionCollection;
 delete d;
}

void BosonMainWidgetMenuInput::init(BoUfoActionCollection* parentCollection)
{
 d = new BosonMainWidgetMenuInputPrivate;
 d->mActionCollection = new BoUfoActionCollection(parentCollection, this, "mainwidget_actioncollection");

 if (!qobject_cast<QWidget*>(parent()) || qobject_cast<QWidget*>(parent())->parent() != 0) {
	boError() << k_funcinfo << "parent() is not a toplevel QWidget! slotGrabScreenshot() relies on that!";
 }

 initUfoActions();
}

void BosonMainWidgetMenuInput::initUfoActions()
{
 BO_CHECK_NULL_RET(actionCollection());

 // TODO: help menu


 // Settings
 (void)BoUfoStdAction::preferences(this, SIGNAL(signalPreferences()), actionCollection());
 BoUfoToggleAction* sound = new BoUfoToggleAction(i18n("Soun&d"),
		KShortcut(), this, SLOT(slotToggleSound()),
		actionCollection(), "options_sound");
 sound->setChecked(boConfig->boolValue("Sound"));
 BoUfoToggleAction* music = new BoUfoToggleAction(i18n("M&usic"), KShortcut(),
		this, SLOT(slotToggleMusic()),
		actionCollection(), "options_music");
 music->setChecked(boConfig->boolValue("Music"));
 (void)new BoUfoAction(i18n("Maximal entries..."), KShortcut(), this,
		SLOT(slotChangeMaxProfilingEntries()), actionCollection(), "options_profiling_max_event_entries");
 (void)new BoUfoAction(i18n("Maximal advance call entries..."), KShortcut(), this,
		SLOT(slotChangeMaxProfilingAdvanceEntries()), actionCollection(), "options_profiling_max_advance_entries");
 (void)new BoUfoAction(i18n("Maximal rendering entries..."), KShortcut(), this,
		SLOT(slotChangeMaxProfilingGLEntries()), actionCollection(), "options_profiling_max_rendering_entries");


 // Debug
 (void)new BoUfoAction(i18n("&Profiling..."), KShortcut(), this,
		SLOT(slotProfiling()), actionCollection(), "debug_profiling");
 (void)new BoUfoAction(i18n("Clear profiling data"), KShortcut(), this,
		SLOT(slotClearProfilingData()), actionCollection(), "debug_clear_profiling_data");
 (void)new BoUfoAction(i18n("&Debug KGame..."), KShortcut(), this,
		SLOT(slotDebugKGame()), actionCollection(), "debug_kgame");
 (void)new BoUfoAction(i18n("Debug &BoDebug log..."), KShortcut(), this,
		SLOT(slotBoDebugLogDialog()), actionCollection(), "debug_bodebuglog");
 (void)new BoUfoAction(i18n("sleep() 1s"), KShortcut(), this,
		SLOT(slotSleep1s()), actionCollection(), "debug_sleep_1s");
 (void)new BoUfoAction(i18n("Crash boson"), KShortcut(), this,
		SLOT(slotCrashBoson()), actionCollection(),
		"debug_crash_boson");
 (void)new BoUfoAction(i18n("OpenGL states..."), KShortcut(), this,
		SLOT(slotShowGLStates()), actionCollection(),
		"debug_show_opengl_states");
 (void)new BoUfoAction(i18n("Debug &ufo widgets..."), KShortcut(),
		this, SIGNAL(signalDebugUfoWidgets()), actionCollection(), "debug_ufo_widgets");
 (void)new BoUfoAction(i18n("Debug &Textures..."), KShortcut(),
		this, SIGNAL(signalDebugTextures()), actionCollection(), "debug_textures");
 (void)new BoUfoAction(i18n("Debug &Models..."), KShortcut(),
		this, SIGNAL(signalDebugModels()), actionCollection(), "debug_models");
// (void)new BoUfoAction(i18n("Grab &Profiling data"), KShortcut(Qt::CTRL + Qt::Key_P),
//		this, SLOT(slotGrabProfiling()), actionCollection(), "game_grab_profiling");
 (void)new BoUfoAction(i18n("&Grab Screenshot"), KShortcut(Qt::CTRL + Qt::Key_G),
		this, SLOT(slotGrabScreenshot()), actionCollection(), "game_grab_screenshot");


 // Display
 BoUfoToggleAction* fullScreen = BoUfoStdAction::fullScreen(0,
		0, actionCollection());
 connect(fullScreen, SIGNAL(signalToggled(bool)),
		this, SLOT(slotToggleFullScreen(bool)));
 fullScreen->setChecked(false);


 QStringList files;
 files.append(boData->locateDataFile("boson/topui.rc"));
 actionCollection()->setGUIFiles(files);
}

BoUfoActionCollection* BosonMainWidgetMenuInput::actionCollection() const
{
 return d->mActionCollection;
}

void BosonMainWidgetMenuInput::slotToggleSound()
{
 boAudio->setSound(!boAudio->sound());
 boConfig->setBoolValue("Sound", boAudio->sound());
}

void BosonMainWidgetMenuInput::slotToggleMusic()
{
 boAudio->setMusic(!boAudio->music());
 boConfig->setBoolValue("Music", boAudio->music());
}

void BosonMainWidgetMenuInput::slotChangeMaxProfilingEntries()
{
 bool ok = true;
 unsigned int max = boConfig->uintValue("MaxProfilingEntries");
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling entries"),
		i18n("Maximal number of profiling entries"),
		(int)max, 0, 100000, 1, &ok, 0);
 if (ok) {
	boConfig->setUIntValue("MaxProfilingEntries", max);
	boProfiling->setMaximalEntries("Default", boConfig->uintValue("MaxProfilingEntries"));
 }
}

void BosonMainWidgetMenuInput::slotChangeMaxProfilingAdvanceEntries()
{
 bool ok = true;
 unsigned int max = boConfig->uintValue("MaxProfilingEntriesAdvance");
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling advance entries"),
		i18n("Maximal number of profiled advance calls"),
		(int)max, 0, 100000, 1, &ok, 0);
 if (ok) {
	boConfig->setUIntValue("MaxProfilingEntriesAdvance", max);
	boProfiling->setMaximalEntries("Advance", boConfig->uintValue("MaxProfilingEntriesAdvance"));
 }
}

void BosonMainWidgetMenuInput::slotChangeMaxProfilingGLEntries()
{
 bool ok = true;
 unsigned int max = boConfig->uintValue("MaxProfilingEntriesGL");
 max = (unsigned int)QInputDialog::getInteger(i18n("Profiling rendering entries"),
		i18n("Maximal number of profiled frames"),
		(int)max, 0, 100000, 1, &ok, 0);
 if (ok) {
	boConfig->setUIntValue("MaxProfilingEntriesGL", max);
	boProfiling->setMaximalEntries("GL", boConfig->uintValue("MaxProfilingEntriesGL"));
 }
}

void BosonMainWidgetMenuInput::slotProfiling()
{
 BosonProfilingDialog* dialog = new BosonProfilingDialog(0, false);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 dialog->show();
}

void BosonMainWidgetMenuInput::slotClearProfilingData()
{
 boProfiling->clearAllStorages();
}

void BosonMainWidgetMenuInput::slotDebugKGame()
{
 if (!boGame) {
	boError() << k_funcinfo << "NULL game" << endl;
	return;
 }
 KGameDebugDialog* dlg = new KGameDebugDialog(boGame, 0, false);

 KGameUnitDebug* units = new KGameUnitDebug(0);
 dlg->addPage(units, i18n("Debug &Units"));
 units->setBoson(boGame);

 KGamePlayerDebug* player = new KGamePlayerDebug(0);
 dlg->addPage(player, i18n("Debug &Boson Players"));
 player->setBoson(boGame);

 KGameAdvanceMessagesDebug* messages = new KGameAdvanceMessagesDebug(0);
 dlg->addPage(messages, i18n("Debug &Advance messages"));
 messages->setBoson(boGame);

#if 0
 if (boGame->playField()) {
	BosonMap* map = boGame->playField()->map();
	if (!map) {
		boError() << k_funcinfo << "NULL map" << endl;
		return;
	}

	// AB: this hardly does anything atm (04/04/23), but it takes a lot of
	// time and memory to be initialized on big maps (on list item per cell,
	// on a 500x500 map thats a lot)
	KGameCellDebug* cells = new KGameCellDebug(b);
	dlg->addPage(cells, i18n("Debug &Cells"));
	cells->setMap(map);
 }
#endif

 connect(dlg, SIGNAL(signalRequestIdName(int,bool,QString&)),
		this, SLOT(slotDebugRequestIdName(int,bool,QString&)));

 connect(dlg, SIGNAL(finished()), dlg, SLOT(deleteLater()));
 dlg->show();
}

void BosonMainWidgetMenuInput::slotDebugRequestIdName(int msgid, bool , QString& name)
{
 // we don't use i18n() for debug messages... not worth the work
 switch (msgid) {
	case BosonMessageIds::ChangeSpecies:
		name = "Change Species";
		break;
	case BosonMessageIds::ChangePlayField:
		name = "Change PlayField";
		break;
	case BosonMessageIds::ChangeTeamColor:
		name = "Change TeamColor";
		break;
	case BosonMessageIds::AdvanceN:
		name = "Advance";
		break;
	case BosonMessageIds::IdChat:
		name = "Chat Message";
		break;
	case BosonMessageIds::IdGameIsStarted:
		name = "Game is started";
		break;
	case BosonMessageIds::MoveMove:
		name = "PlayerInput: Move";
		break;
	case BosonMessageIds::MoveAttack:
		name = "PlayerInput: Attack";
		break;
	case BosonMessageIds::MoveBuild:
		name = "PlayerInput: Build";
		break;
	case BosonMessageIds::MoveProduce:
		name = "PlayerInput: Produce";
		break;
	case BosonMessageIds::MoveProduceStop:
		name = "PlayerInput: Produce Stop";
		break;
	case BosonMessageIds::MoveMine:
		name = "PlayerInput: Mine";
		break;
	case BosonMessageIds::UnitPropertyHandler:
	default:
		// a unit property was changed
		// all ids > UnitPropertyHandler will be a unit property. we
		// don't check further...
		break;
 }
// boDebug() << name << endl;
}

void BosonMainWidgetMenuInput::slotBoDebugLogDialog()
{
 BoDebugLogDialog* dialog = new BoDebugLogDialog(0);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 dialog->slotUpdate();
 dialog->show();
#if 0
 BoUfoDebugLogDialog* dialog = new BoUfoDebugLogDialog(ufoManager());
 dialog->slotUpdate();
 dialog->show();
#endif
}

void BosonMainWidgetMenuInput::slotSleep1s()
{
 sleep(1);
}

void BosonMainWidgetMenuInput::slotGrabScreenshot()
{
 boDebug() << k_funcinfo << "Taking screenshot!" << endl;

 QWidget* topWidget = qobject_cast<QWidget*>(parent());
 if (!topWidget) {
	boError() << k_funcinfo << "parent() is not a QWidget. cannot take screenshot.";
	return;
 }
 if (topWidget->parent()) {
	boError() << k_funcinfo << "parent() is not a tiplevel widget. screenshot may be incomplete!";
	// do not return
 }

 QPixmap shot = QPixmap::grabWindow(topWidget->winId());
 if (shot.isNull()) {
	boError() << k_funcinfo << "NULL image returned" << endl;
	return;
 }
 QString file = findSaveFileName("boson", "jpg");
 if (file.isNull()) {
	boWarning() << k_funcinfo << "Can't find free filename???" << endl;
	return;
 }
 boDebug() << k_funcinfo << "Saving screenshot to " << file << endl;
 bool ok = shot.save(file, "JPEG", 90);
 if (!ok) {
	boError() << k_funcinfo << "Error saving screenshot to " << file << endl;
	boGame->slotAddChatSystemMessage(i18n("An error occured while saving screenshot to %1").arg(file));
 } else {
	boGame->slotAddChatSystemMessage(i18n("Screenshot saved to %1").arg(file));
 }
}

void BosonMainWidgetMenuInput::slotGrabProfiling()
{
	boWarning() << k_funcinfo << "obsolete feature" << endl;
#if 0
 QString file = findSaveFileName("boprofiling", "boprof");
 if (file.isNull()) {
	boWarning() << k_funcinfo << "Can't find free filename???" << endl;
	return;
 }
 // TODO: chat message about file location!
 boDebug() << k_funcinfo << "Saving profiling to " << file << endl;
 bool ok = boProfiling->saveToFile(file);
 if (!ok) {
	boError() << k_funcinfo << "Error saving profiling to " << file << endl;
	boGame->slotAddChatSystemMessage(i18n("An error occured while saving profiling log to %1").arg(file));
 } else {
	boGame->slotAddChatSystemMessage(i18n("Profiling log saved to %1").arg(file));
 }
#endif
}

void BosonMainWidgetMenuInput::slotShowGLStates()
{
 boDebug() << k_funcinfo << endl;
 BoGLStateWidget* w = new BoGLStateWidget(0);
 w->setAttribute(Qt::WA_DeleteOnClose);
 w->show();
}

void BosonMainWidgetMenuInput::slotCrashBoson()
{
 ((QObject*)0)->name();
}

void BosonMainWidgetMenuInput::slotToggleFullScreen(bool fullScreen)
{
 if (fullScreen) {
	BoFullScreen::enterMode(-1);
 } else {
	BoFullScreen::leaveFullScreen();
 }
}



