/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosoncommandframe.h" // necessary cause of BosonCommandFrame::OrderType

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
#if HAVE_KSHORTCUT
#include <kshortcut.h>
#endif

#include <qintdict.h>
#include <qwmatrix.h>
#include <qvbox.h>

#include "editor.moc"


static const char *description =
    I18N_NOOP("A realtime strategy game for KDE");

static const char *version = "v0.6";

static KCmdLineOptions options[] =
{
//    { "e", 0, 0 },
//    { "editor", I18N_NOOP( "Map editor"), 0 },
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
//FIXME:
 KAboutData about("boson",
		I18N_NOOP("Boson Map Editor"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 1999-2000,2001 The Boson team",
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

    // see if we are starting with session management
 if (app.isRestored()) {
 //FIXME: do we use this at all?? probably not...
	RESTORE(Editor)
 } else {
// no session.. just start up normally
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	Editor *widget = new Editor;
	widget->show();
	args->clear();
 }
 return app.exec();
}



class Editor::EditorPrivate
{
public:
	EditorPrivate()
	{
		mEdit = 0;

		mPlayerAction = 0;
		mCellsAction = 0;

		mActionFacilities = 0;
		mActionMobiles = 0;
		mActionCellPlain = 0;
		mActionCellSmall = 0;
		mActionCellBig1 = 0;
		mActionCellBig2 = 0;
	}

	KSelectAction* mEdit;

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

 bosonWidget()->addEditorCommandFrame(commandFrame());
 connect(bosonWidget(), SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(bosonWidget(), SIGNAL(signalPlayerLeftGame(KPlayer*)), 
		this, SLOT(slotPlayerLeftGame(KPlayer*)));
 connect(bosonWidget(), SIGNAL(signalGameStarted()), 
		this, SLOT(slotGameStarted()));

 initKAction();
 initStatusBar();

 showMaximized();

 bosonWidget()->startEditor();

}

Editor::~Editor()
{
 delete d;
}

void Editor::initKAction()
{
 KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
 KStdAction::quit(kapp, SLOT(quit()), actionCollection());

#if HAVE_KSHORTCUT
 (void)new KAction(i18n("Save &Map as..."), KShortcut(), this,
#else
 (void)new KAction(i18n("Save &Map as..."), QKeySequence(), this,
#endif
		SLOT(slotSaveMapAs()), actionCollection(),
		"file_save_map_as");

#if HAVE_KSHORTCUT
 (void)new KAction(i18n("Save &Scenario as..."), KShortcut(), this,
#else
 (void)new KAction(i18n("Save &Scenario as..."), QKeySequence(), this,
#endif
		SLOT(slotSaveScenarioAs()), actionCollection(), 
		"file_save_scenario_as");

#if HAVE_KSHORTCUT
 d->mEdit = new KSelectAction(i18n("&Edit"), KShortcut(), actionCollection(), "editor_edit");
#else
 d->mEdit = new KSelectAction(i18n("&Edit"), QKeySequence(), actionCollection(), "editor_edit");
#endif
 connect(d->mEdit, SIGNAL(activated(int)), this, SLOT(slotChangeEdit(int)));
 QStringList list;
 list.append(i18n("&Map"));
 list.append(i18n("&Scenario"));
 d->mEdit->setItems(list);
 list.clear();
 
#if HAVE_KSHORTCUT
 d->mPlayerAction = new KSelectAction(i18n("&Player"), KShortcut(), actionCollection(), "editor_player");
#else
 d->mPlayerAction = new KSelectAction(i18n("&Player"), QKeySequence(), actionCollection(), "editor_player");
#endif
 connect(d->mPlayerAction, SIGNAL(activated(int)), 
		bosonWidget(), SLOT(slotChangeLocalPlayer(int)));
#if HAVE_KSHORTCUT
 d->mActionFacilities = new KRadioAction(i18n("&Facilities"), KShortcut(),
#else
 d->mActionFacilities = new KRadioAction(i18n("&Facilities"), QKeySequence(),
#endif
		this, SLOT(slotPlaceFacilities()), actionCollection(),
		"editor_place_facilities");
 d->mActionFacilities->setExclusiveGroup("Place");
#if HAVE_KSHORTCUT
 d->mActionMobiles = new KRadioAction(i18n("&Mobiles"), KShortcut(),
#else
 d->mActionMobiles = new KRadioAction(i18n("&Mobiles"), QKeySequence(),
#endif
		this, SLOT(slotPlaceMobiles()), actionCollection(),
		"editor_place_mobiles");
 d->mActionMobiles->setExclusiveGroup("Place");
#if HAVE_KSHORTCUT
 d->mActionCellSmall = new KRadioAction(i18n("&Small"), KShortcut(),
#else
 d->mActionCellSmall = new KRadioAction(i18n("&Small"), QKeySequence(),
#endif
		this, SLOT(slotPlaceCellSmall()), actionCollection(),
		"editor_place_cell_small");
 d->mActionCellSmall->setExclusiveGroup("Place");
#if HAVE_KSHORTCUT
 d->mActionCellPlain = new KRadioAction(i18n("&Plain"), KShortcut(), 
#else
 d->mActionCellPlain = new KRadioAction(i18n("&Plain"), QKeySequence(), 
#endif
		this, SLOT(slotPlaceCellPlain()), actionCollection(),
		"editor_place_cell_plain");
 d->mActionCellPlain->setExclusiveGroup("Place");
#if HAVE_KSHORTCUT
 d->mActionCellBig1 = new KRadioAction(i18n("&Big1"), KShortcut(), 
#else
 d->mActionCellBig1 = new KRadioAction(i18n("&Big1"), QKeySequence(), 
#endif
		this, SLOT(slotPlaceCellBig1()),actionCollection(),
		"editor_place_cell_big1");
 d->mActionCellBig1->setExclusiveGroup("Place");
#if HAVE_KSHORTCUT
 d->mActionCellBig2 = new KRadioAction(i18n("B&ig2"), KShortcut(),
#else
 d->mActionCellBig2 = new KRadioAction(i18n("B&ig2"), QKeySequence(),
#endif
		this, SLOT(slotPlaceCellBig2()),actionCollection(),
		"editor_place_cell_big2");
 d->mActionCellBig2->setExclusiveGroup("Place");

#if HAVE_KSHORTCUT
 (void)new KAction(i18n("&Create Custom Unit"), KShortcut(), this,
#else
 (void)new KAction(i18n("&Create Custom Unit"), QKeySequence(), this,
#endif
		  SLOT(slotCreateUnit()), actionCollection(),
		  "editor_create_unit");
#if HAVE_KSHORTCUT
 (void)new KAction(i18n("&Generate Custom Tiles Ground"), KShortcut(), this,
#else
 (void)new KAction(i18n("&Generate Custom Tiles Ground"), QKeySequence(), this,
#endif
		  SLOT(slotCreateTiles()), actionCollection(),
		  "editor_create_tiles");

 KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
 KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection()); // FIXME: slotEditorPreferences()

 createGUI("bosoneditorui.rc");
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

void Editor::slotSaveMapAs()
{
 QString startIn; // shall we provide this??
 QString fileName = KFileDialog::getSaveFileName(startIn, "*.bpf", this);
 if (fileName != QString::null) {
	if (QFileInfo(fileName).extension().isEmpty()) {
		fileName += ".bpf";
	}
	bosonWidget()->slotEditorSaveMap(fileName);
 }
}

void Editor::slotSaveScenarioAs()
{
 // FIXME: you need an already saved map here!!
 // check for isChanged() or so!

 QString startIn; // shall we provide this??
 QString fileName = KFileDialog::getSaveFileName(startIn, "*.bsc", this);
 if (fileName != QString::null) {
	if (QFileInfo(fileName).extension().isEmpty()) {
		fileName += ".bsc";
	}
	bosonWidget()->slotEditorSaveScenario(fileName);
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
 QString dir = KFileDialog::getExistingDirectory();
 if (dir.isNull()) {
	return;
 }
 BosonTiles newTiles;
 if (dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }
 if (!newTiles.loadTiles(dir)) {
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

void Editor::slotChangeEdit(int e)
{
 kdDebug() << k_funcinfo << endl;
 // e == 0 is map editor
 // e == 1 is scenario editor
 if (bosonWidget()->isModified()) {
	KMessageBox::sorry(this, i18n("Please save the current file first"));
	if (e == 0) {
		d->mEdit->setCurrentItem(1);
	} else {
		d->mEdit->setCurrentItem(1);
	}
	return;
 }
 d->mActionFacilities->setEnabled(e == 1);
 d->mActionMobiles->setEnabled(e == 1);
 d->mActionCellSmall->setEnabled(e == 0);
 d->mActionCellPlain->setEnabled(e == 0);
 d->mActionCellBig1->setEnabled(e == 0);
 d->mActionCellBig2->setEnabled(e == 0);
 d->mMapMode = (e == 0);
 if (e == 0) {
	d->mActionCellSmall->activate();
 } else {
	d->mActionFacilities->activate();
 }
 bosonWidget()->displayAllItems(e == 1);
}

void Editor::slotPlaceFacilities()
{
 if (!d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(BosonCommandFrame::Facilities);
 }
}

void Editor::slotPlaceMobiles()
{
 if (!d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(BosonCommandFrame::Mobiles);
 }
}

void Editor::slotPlaceCellSmall()
{
 if (d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(BosonCommandFrame::Small);
 }
}

void Editor::slotPlaceCellPlain()
{
 if (d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(BosonCommandFrame::Plain);
 }
}

void Editor::slotPlaceCellBig1()
{
 if (d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(BosonCommandFrame::Big1);
 }
}

void Editor::slotPlaceCellBig2()
{
 if (d->mMapMode) {
	bosonWidget()->slotEditorConstructionChanged(BosonCommandFrame::Big2);
 }
}

void Editor::slotGameStarted()
{
 bosonWidget()->setModified(false);
 d->mEdit->setCurrentItem(0); // TODO: use BosonConfig
 slotChangeEdit(0); 
}
