#include "editor.h"

#include "bosonunitdialog.h"
#include "bosonwidget.h"
#include "player.h"
#include "bosontiles.h"

#include "bosoncommandframe.h" // necessary cause of BosonCommandFrame::OrderType

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kconfig.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <qintdict.h>

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
		I18N_NOOP("Boson game"),
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



class EditorPrivate
{
public:
	EditorPrivate()
	{
		mBosonWidget = 0;
		mToolbarAction = 0;
		mStatusbarAction = 0;

		mPlayerAction = 0;
	}

	BosonWidget* mBosonWidget;
	KToggleAction* mToolbarAction;
	KToggleAction* mStatusbarAction;

	KSelectAction* mPlayerAction;
	KSelectAction* mBuildAction;
	KSelectAction* mCellsAction;

	QIntDict<Player> mPlayers;
};

Editor::Editor()
    : KMainWindow( 0 )
{
 d = new EditorPrivate;

 d->mBosonWidget= new BosonWidget(this, true);
 connect(d->mBosonWidget, SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(d->mBosonWidget, SIGNAL(signalPlayerLeftGame(KPlayer*)), 
		this, SLOT(slotPlayerLeftGame(KPlayer*)));

 // tell the KMainWindow that this is indeed the main widget
 setCentralWidget(d->mBosonWidget);

 // then, setup our actions
 setupActions();

 // and a status bar
 setupStatusBar();

 showMaximized();

 d->mBosonWidget->startEditor();
}

Editor::~Editor()
{
 delete d;
}

void Editor::setupActions()
{
 KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
// KStdAction::save(this, SLOT(fileSave()), actionCollection());
// KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
 KStdAction::quit(kapp, SLOT(quit()), actionCollection());

 (void)new KAction(i18n("Save &Map as..."), QKeySequence(), this,
		  SLOT(slotSaveMapAs()), actionCollection(),
		  "file_save_map_as");
 (void)new KAction(i18n("Save &Scenario as..."), QKeySequence(), this,
		  SLOT(slotSaveScenarioAs()), actionCollection(), 
		  "file_save_scenario_as");
// Debug - no i18n!
 (void)new KAction("Debug", QKeySequence(), d->mBosonWidget, SLOT(slotDebug()), actionCollection(), "game_debug");


 d->mPlayerAction = new KSelectAction(i18n("&Player"), QKeySequence(), actionCollection(), "editor_player");
 connect(d->mPlayerAction, SIGNAL(activated(int)), 
		d->mBosonWidget, SLOT(slotChangeLocalPlayer(int)));
 d->mBuildAction = new KSelectAction(i18n("&Units"), QKeySequence(), actionCollection(), "editor_build");
 connect(d->mBuildAction, SIGNAL(activated(int)), 
		d->mBosonWidget, SLOT(slotEditorConstructionChanged(int)));

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

 d->mToolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
 d->mStatusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

 KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
 KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
 KStdAction::preferences(d->mBosonWidget, SLOT(slotGamePreferences()), actionCollection()); // FIXME: slotEditorPreferences()

 createGUI("bosoneditorui.rc");
}

void Editor::setupStatusBar()
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
	d->mBosonWidget->slotEditorSaveMap(fileName);
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
	d->mBosonWidget->slotEditorSaveScenario(fileName);
 }
}

void Editor::optionsShowToolbar()
{
 if (d->mToolbarAction->isChecked()) {
	toolBar()->show();
 } else {
	toolBar()->hide();
 }
}

void Editor::optionsShowStatusbar()
{
 if (d->mStatusbarAction->isChecked()) {
        statusBar()->show();
 } else {
        statusBar()->hide();
 }
}

void Editor::optionsConfigureKeys()
{
 KKeyDialog::configureKeys(actionCollection(), "bosonui.rc");
}

void Editor::optionsConfigureToolbars()
{
 // use the standard toolbar editor
 KEditToolbar dlg(actionCollection());
 if (dlg.exec()) {
	// recreate our GUI
	createGUI();
 } 
}

void Editor::slotFileNew()
{
// d->mBosonWidget->startEditor();
}

void Editor::slotChangePlayer(int index)
{
kdWarning() << "obsolete" << endl;
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
 d->mBosonWidget->slotChangeLocalPlayer(d->mPlayerAction->currentItem());
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
 kdWarning() << "obsolete" << endl;
// d->mBosonWidget->editorConstructionChanged(index, d->mPlayers[d->mPlayerAction->currentItem()]);
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

