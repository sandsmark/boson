/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "editor.h"

#include "bosonunitdialog.h"
#include "bosonwidget.h"
#include "player.h"
#include "bosontiles.h"
#include "bosonconfig.h"
#include "global.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <kkeydialog.h>
#include <kconfig.h>
#include <kstatusbar.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kshortcut.h>

#include <qintdict.h>

#include "editor.moc"


static const char *description =
    I18N_NOOP("A realtime strategy game for KDE");

static const char *version = "v0.6pre";

static KCmdLineOptions options[] =
{
    { "sound", I18N_NOOP("Enable Sounds"), 0 },
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
//FIXME:
 KAboutData about("boeditor",
		I18N_NOOP("Boson Map Editor"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001-2002 The Boson team",
		0,
		"http://boson.eu.org",
		"b_mann@gmx.de");
 about.addAuthor("Thomas Capricelli", I18N_NOOP("Initial Game Design & Coding"), "orzel@kde.org", "http://orzel.freehackers.org");
 about.addAuthor("Benjamin Adler", I18N_NOOP("Graphics & Homepage Design"), "benadler@bigfoot.de");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding"), "b_mann@gmx.de" );

 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KApplication app;
 KGlobal::locale()->insertCatalogue("libkdegames");

    // register ourselves as a dcop client
//    app.dcopClient()->registerAs(app.name(), false);

 BosonConfig::initBosonConfig();
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
 if (!args->isSet("sound")) {
	boConfig->setDisableSound(true);
 }
 Editor *widget = new Editor;

 bool showMaximized = true;
 if (showMaximized) {
	widget->showMaximized();
 } else {
	widget->show();
 }
	
 args->clear();
 return app.exec();
}



class Editor::EditorPrivate
{
public:
	EditorPrivate()
	{
		mPlayerAction = 0;
		mCellsAction = 0;

		mActionFacilities = 0;
		mActionMobiles = 0;
		mActionCellPlain = 0;
		mActionCellSmall = 0;
		mActionCellBig1 = 0;
		mActionCellBig2 = 0;
	}

	KSelectAction* mPlayerAction;
	KSelectAction* mCellsAction;

	KRadioAction* mActionFacilities;
	KRadioAction* mActionMobiles;
	KRadioAction* mActionCellPlain;
	KRadioAction* mActionCellSmall;
	KRadioAction* mActionCellBig1;
	KRadioAction* mActionCellBig2;

	QIntDict<Player> mPlayers;
	bool mMapMode;
};

Editor::Editor() : TopBase()
{
 d = new EditorPrivate;

 initKAction();
 initCommandFrame();
 initStatusBar();

 bosonWidget()->addEditorCommandFrame(commandFrame());
 connect(bosonWidget(), SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(bosonWidget(), SIGNAL(signalPlayerLeftGame(KPlayer*)), 
		this, SLOT(slotPlayerLeftGame(KPlayer*)));
 connect(bosonWidget(), SIGNAL(signalGameStarted()), 
		this, SLOT(slotGameStarted()));

 showMaximized();

 bosonWidget()->startEditor();

}

Editor::~Editor()
{
 bosonWidget()->saveConfig(true);
 delete d;
}

void Editor::initKAction()
{
 KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
 KStdAction::quit(kapp, SLOT(quit()), actionCollection());

 (void)new KAction(i18n("Save &PlayField as..."), KShortcut(), this,
		SLOT(slotSavePlayFieldAs()), actionCollection(),
		"file_save_playfield_as");

 d->mPlayerAction = new KSelectAction(i18n("&Player"), KShortcut(), actionCollection(), "editor_player");
 connect(d->mPlayerAction, SIGNAL(activated(int)), 
		bosonWidget(), SLOT(slotChangeLocalPlayer(int)));
 d->mActionFacilities = new KRadioAction(i18n("&Facilities"), KShortcut(),
		this, SLOT(slotPlaceFacilities()), actionCollection(),
		"editor_place_facilities");
 d->mActionFacilities->setExclusiveGroup("Place");
 d->mActionMobiles = new KRadioAction(i18n("&Mobiles"), KShortcut(),
		this, SLOT(slotPlaceMobiles()), actionCollection(),
		"editor_place_mobiles");
 d->mActionMobiles->setExclusiveGroup("Place");
 d->mActionCellSmall = new KRadioAction(i18n("&Small"), KShortcut(),
		this, SLOT(slotPlaceCellSmall()), actionCollection(),
		"editor_place_cell_small");
 d->mActionCellSmall->setExclusiveGroup("Place");
 d->mActionCellPlain = new KRadioAction(i18n("&Plain"), KShortcut(), 
		this, SLOT(slotPlaceCellPlain()), actionCollection(),
		"editor_place_cell_plain");
 d->mActionCellPlain->setExclusiveGroup("Place");
 d->mActionCellBig1 = new KRadioAction(i18n("&Big1"), KShortcut(), 
		this, SLOT(slotPlaceCellBig1()),actionCollection(),
		"editor_place_cell_big1");
 d->mActionCellBig1->setExclusiveGroup("Place");
 d->mActionCellBig2 = new KRadioAction(i18n("B&ig2"), KShortcut(),
		this, SLOT(slotPlaceCellBig2()),actionCollection(),
		"editor_place_cell_big2");
 d->mActionCellBig2->setExclusiveGroup("Place");

 (void)new KAction(i18n("&Create Custom Unit"), KShortcut(), this,
		  SLOT(slotCreateUnit()), actionCollection(),
		  "editor_create_unit");
 (void)new KAction(i18n("&Generate Custom Tiles Ground"), KShortcut(), this,
		  SLOT(slotCreateTiles()), actionCollection(),
		  "editor_create_tiles");

 (void)new KAction(i18n("&Generate Custom Tiles Ground (Debug mode)"), KShortcut(), this,
		  SLOT(slotCreateDebugTiles()), actionCollection(),
		  "editor_create_debug_tiles");

 KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
 KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection()); // FIXME: slotEditorPreferences()

 createGUI("boson/bosoneditorui.rc");
}

void Editor::initStatusBar()
{
 statusBar()->show();
}

void Editor::saveProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  anything you write here will be available
 // later when this app is restored
 if (!config) {
	return;
 }
    
}

void Editor::readProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  this function is automatically called whenever
 // the app is being restored.  read in here whatever you wrote
 // in 'saveProperties'
 if (!config) {
	return;
 }
}

void Editor::slotSavePlayFieldAs()
{
 QString startIn; // shall we provide this??
 QString fileName = KFileDialog::getSaveFileName(startIn, "*.bpf", this);
 if (fileName != QString::null) {
	if (QFileInfo(fileName).extension().isEmpty()) {
		fileName += ".bpf";
	}
	bosonWidget()->editorSavePlayField(fileName);
 }
}

void Editor::slotConfigureKeys()
{
 KKeyDialog::configureKeys(actionCollection(), "bosoneditorui.rc");
}

void Editor::slotFileNew()
{
// bosonWidget()->startEditor();
}

void Editor::slotPlayerJoinedGame(KPlayer* player)
{
 if (!player) {
	return;
 }
 Player* p = (Player*)player;
 QStringList players = d->mPlayerAction->items();
 d->mPlayers.insert(players.count(), p);
 players.append(p->name());
 d->mPlayerAction->setItems(players);

 // dunno if this makes sense - but currently one cannot add more players so we
 // just activate the player that was added last.
 d->mPlayerAction->setCurrentItem(players.count() - 1);
 bosonWidget()->slotChangeLocalPlayer(d->mPlayerAction->currentItem());
}

void Editor::slotPlayerLeftGame(KPlayer* player)
{
 if (!player) {
	return;
 }
 Player* p = (Player*)p;
 QIntDictIterator<Player> it(d->mPlayers);
 while (it.current() && it.current() != player) {
	++it;
 }
 if (!it.current()) {
	kdError() << k_funcinfo << ": player not found" << endl;
	return;
 }
 QStringList players = d->mPlayerAction->items();
 
 players.remove(players.at(it.currentKey()));
 d->mPlayers.remove(it.currentKey());

 d->mPlayerAction->setItems(players);
}

void Editor::slotCreateUnit()
{
 BosonUnitDialog* dlg = new BosonUnitDialog(this);
 dlg->exec();
 delete dlg;
}

void Editor::slotCreateTiles()
{
 createTiles(false);
}

void Editor::slotCreateDebugTiles()
{
 createTiles(true);
}

void Editor::createTiles(bool debug)
{
 QString dir = KFileDialog::getExistingDirectory();
 if (dir.isNull()) {
	return;
 }
 BosonTiles newTiles;
 if (dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }
 if (!newTiles.loadTiles(dir, debug)) {
	kdError() << "Could not load tiles from " << dir << endl;
	KMessageBox::sorry(this, i18n("Error loading tiles from %1").arg(dir));
	return;
 }
 //TODO: display progress widget/window

 QString fileName = KFileDialog::getSaveFileName(QString::null, "*.png", this);
 if (fileName.isNull()) {
	return;
 }
 if (QFileInfo(fileName).extension().isEmpty()) {
	fileName += ".png";
 }
 newTiles.save(fileName);
}

void Editor::slotPlaceFacilities()
{
 if (!d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(OrderFacilities);
 }
}

void Editor::slotPlaceMobiles()
{
 if (!d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(OrderMobiles);
 }
}

void Editor::slotPlaceCellSmall()
{
 if (d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(OrderSmall);
 }
}

void Editor::slotPlaceCellPlain()
{
 if (d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(OrderPlainTiles);
 }
}

void Editor::slotPlaceCellBig1()
{
 if (d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(OrderBig1);
 }
}

void Editor::slotPlaceCellBig2()
{
 if (d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(OrderBig2);
 }
}

void Editor::slotGameStarted()
{
}
