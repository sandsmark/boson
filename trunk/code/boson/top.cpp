#include "top.h"

#include "bosonwidget.h"

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kconfig.h>

#include <kedittoolbar.h>

#include <kstatusbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstdgameaction.h>
#include <kdebug.h>

#include "top.moc"

class TopPrivate
{
public:
	TopPrivate()
	{
		mBosonWidget = 0;
		mToolbarAction = 0;
		mStatusbarAction = 0;
	}

	BosonWidget* mBosonWidget;
	KToggleAction* mToolbarAction;
	KToggleAction* mStatusbarAction;
};

Top::Top()
    : KMainWindow( 0 )
{
 d = new TopPrivate;

 d->mBosonWidget= new BosonWidget(this);

 // tell the KMainWindow that this is indeed the main widget
 setCentralWidget(d->mBosonWidget);

 // then, setup our actions
 setupActions();

 // and a status bar
 setupStatusBar();

 // a local player is always needed for a game (not for editor)
 d->mBosonWidget->addLocalPlayer();

}

Top::~Top()
{
 delete d;
}

void Top::setupActions()
{
 KStdGameAction::gameNew(d->mBosonWidget, SLOT(slotNewGame()), actionCollection());
// KStdGameAction::save(this, SLOT(fileSave()), actionCollection());
// KStdGameAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
 KStdGameAction::quit(kapp, SLOT(quit()), actionCollection());
 KStdGameAction::end(d->mBosonWidget, SLOT(slotEndGame()), actionCollection());
// (void)new KAction(i18n("Connect To localhost"), 0, d->mBosonWidget, SLOT(slotConnect()), actionCollection(), "game_connect");

// Debug - no i18n!
 (void)new KAction("Debug", 0, d->mBosonWidget, SLOT(slotDebug()), actionCollection(), "game_debug");

 d->mToolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
 d->mStatusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

 KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
 KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
 KStdAction::preferences(d->mBosonWidget, SLOT(slotPreferences()), actionCollection());

 createGUI();
}

void Top::setupStatusBar()
{
 statusBar()->show();
}

void Top::saveProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  anything you write here will be available
 // later when this app is restored
 if (!config) {
	return;
 }
    
}

void Top::readProperties(KConfig *config)
{
 // the 'config' object points to the session managed
 // config file.  this function is automatically called whenever
 // the app is being restored.  read in here whatever you wrote
 // in 'saveProperties'
 if (!config) {
	return;
 }
}

void Top::slotGameNew()
{
 d->mBosonWidget->slotNewGame();
}

void Top::fileSave()
{
}

void Top::fileSaveAs()
{
}

void Top::optionsShowToolbar()
{
 if (d->mToolbarAction->isChecked()) {
	toolBar()->show();
 } else {
	toolBar()->hide();
 }
}

void Top::optionsShowStatusbar()
{
 if (d->mStatusbarAction->isChecked()) {
        statusBar()->show();
 } else {
        statusBar()->hide();
 }
}

void Top::optionsConfigureKeys()
{
 KKeyDialog::configureKeys(actionCollection(), "bosonui.rc");
}

void Top::optionsConfigureToolbars()
{
 // use the standard toolbar editor
 KEditToolbar dlg(actionCollection());
 if (dlg.exec()) {
	// recreate our GUI
	createGUI();
 } 
}

