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

#include <qintdict.h>
#include <qwmatrix.h>

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
 about.addAuthor("Thomas Capricelli", I18N_NOOP("Game Design & Coding"), "orzel@yalbi.com", "http://aquila.rezel.enst.fr/thomas/");
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
		mPlayerAction = 0;
		mBuildAction = 0;
		mCellsAction = 0;
	}

	KSelectAction* mPlayerAction;
	KSelectAction* mBuildAction;
	KSelectAction* mCellsAction;

	QIntDict<Player> mPlayers;
};

Editor::Editor() : TopBase()
{
 d = new EditorPrivate;

 bosonWidget()->addEditorCommandFrame();
 connect(bosonWidget(), SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(bosonWidget(), SIGNAL(signalPlayerLeftGame(KPlayer*)), 
		this, SLOT(slotPlayerLeftGame(KPlayer*)));

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

 (void)new KAction(i18n("Save &Map as..."), QKeySequence(), this,
		  SLOT(slotSaveMapAs()), actionCollection(),
		  "file_save_map_as");
 (void)new KAction(i18n("Save &Scenario as..."), QKeySequence(), this,
		  SLOT(slotSaveScenarioAs()), actionCollection(), 
		  "file_save_scenario_as");

 d->mPlayerAction = new KSelectAction(i18n("&Player"), QKeySequence(), actionCollection(), "editor_player");
 connect(d->mPlayerAction, SIGNAL(activated(int)), 
		bosonWidget(), SLOT(slotChangeLocalPlayer(int)));
 d->mBuildAction = new KSelectAction(i18n("&Units"), QKeySequence(), actionCollection(), "editor_build");
 connect(d->mBuildAction, SIGNAL(activated(int)), 
		bosonWidget(), SLOT(slotEditorConstructionChanged(int)));

// quite complex - but this way we are mostly independant from changes in
// BosonCommandFrame::OrderType
 QStringList buildList;
 for (int i = 0; i < BosonCommandFrame::OrderLast; i++) {
	switch ((BosonCommandFrame::OrderType)i) {
		case BosonCommandFrame::Facilities:
			buildList.append(i18n("Facilities"));
			break;
		case BosonCommandFrame::Mobiles:
			buildList.append(i18n("Mobile Units"));
			break;
		case BosonCommandFrame::PlainTiles:
			buildList.append(i18n("Plain Tiles"));
			break;
		case BosonCommandFrame::Small:
			buildList.append(i18n("Small"));
			break;
		case BosonCommandFrame::Big1:
			buildList.append(i18n("Big 1"));
			break;
		case BosonCommandFrame::Big2:
			buildList.append(i18n("Big 2"));
			break;
		default:
			kdError() << "unknown order " << i << endl;
			break;
	}
 }
 d->mBuildAction->setItems(buildList);

 (void)new KAction(i18n("&Create Custom Unit"), QKeySequence(), this,
		  SLOT(slotCreateUnit()), actionCollection(),
		  "editor_create_unit");
 (void)new KAction(i18n("&Generate Custom Tiles Ground"), QKeySequence(), this,
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

void Editor::slotChangePlayer(int index)
{
kdWarning() << k_funcinfo << "is obsolete" << endl;
// Player* currentPlayer = d->mPlayers[index];

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

void Editor::slotChangeUnitConstruction(int index)
{
 kdWarning() << k_funcinfo << "is obsolete" << endl;
// bosonWidget()->editorConstructionChanged(index, d->mPlayers[d->mPlayerAction->currentItem()]);
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

