
#include "bosonwidget.h"

#include "bosonbigdisplay.h"
#include "bosonminimap.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "bosonmap.h"
#include "player.h"
#include "visualunit.h"
#include "defines.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "kspritetooltip.h"
#include "bosoncommandframe.h"
#include "editorinput.h"
#include "bosonmessage.h"
#include "kgamedialogbosonconfig.h"

#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kgame/kgameio.h>
#include <kgame/kgamedebugdialog.h>
#include <kmessagebox.h>
#include <kgame/kgamedialog.h>

#include <qlayout.h>
#include <qevent.h>
#include <qtimer.h>
#include <qvbox.h>

#include "bosonwidget.moc"

class BosonWidgetPrivate
{
public:
	BosonWidgetPrivate()
	{
		mBigDisplay = 0;
		mMiniMap = 0;
		mCommandFrame = 0;

		mCanvas = 0;

		mBoson = 0;
		mLocalPlayer = 0;
		mMap = 0;
	}
	
	BosonBigDisplay* mBigDisplay;
	BosonMiniMap* mMiniMap;
	BosonCommandFrame* mCommandFrame;

	BosonCanvas* mCanvas;
	
	Boson* mBoson;
	Player* mLocalPlayer;
	BosonMap* mMap;

	KSpriteToolTip* mUnitTips;

	
	// options:
	int mArrowKeyStep;
};

BosonWidget::BosonWidget(QWidget* parent, bool editor)
    : QWidget( parent, "BosonWidget" )
{
 init();

 if (editor) {
	addEditorCommandFrame();
 } else {
	addGameCommandFrame();
 }

 QHBoxLayout* topLayout = new QHBoxLayout(this, 5); // FIXME: 5 is hardcoded
 QVBoxLayout* layout = new QVBoxLayout();
 topLayout->addWidget(d->mBigDisplay);
 topLayout->addLayout(layout);
 layout->addWidget(d->mMiniMap, 0, AlignHCenter);
 layout->addWidget(d->mCommandFrame);

// the map is also found here. This is currently only used on startup to load
// the cells (aka map - they contain the groundtypes) and the initial units.
// do not call in init()  -  connects to commandframe
// clearMap(); // note: also creates a new map!

 d->mBoson->setCanvas(d->mCanvas); // should not be stored here - but seems to be necessary :-(


// new code
 connect(d->mBigDisplay, SIGNAL(signalConstructUnit(int,VisualUnit*, Player*)),
		d->mBoson, SLOT(slotConstructUnit(int, VisualUnit*, Player*)));
 connect(d->mBigDisplay, SIGNAL(signalConstructUnit(int,int, int, Player*)),
		d->mBoson, SLOT(slotConstructUnit(int, int, int, Player*)));


}

void BosonWidget::init()
{
 d = new BosonWidgetPrivate;
 d->mArrowKeyStep = ARROW_KEY_STEP;

 d->mCanvas = new BosonCanvas(this);

// this widget contains at least 3 widgets:
// - BosonBigDisplay - the actual game view. Inherits QCanvasView
// - BosonMiniMap - the mini map. what else ;-)
// - CommandFrame - the frame on the right side e.g. for producing units/cells, data
// about units,
 d->mBigDisplay = new BosonBigDisplay(d->mCanvas, this);
 d->mMiniMap = new BosonMiniMap(this);
// d->mCommandFrame is now in addEditorCommandFrame() or addGameCommandFrame()


// beside the three main widgets this widget contains the game itself. Boson is
// derived from KGame and should manage the game stuff. BosonWidget also
// connects Boson to the three main widget (see above) so that they can communicate
// together.
// Boson should (!) not contain any pointer to BosonCanvas or so as of
// readability.
// AB: but nevertheless this might be necessary - e.g. Boson::signalAddUnit()
// sends a request to add a unit but Boson should rather create it itself.
 d->mBoson = new Boson(this);
 connect(d->mBoson, SIGNAL(signalAdvance()), 
		d->mCanvas, SLOT(advance()));
 connect(d->mBoson, SIGNAL(signalAddUnit(VisualUnit*, int, int)), 
		d->mCanvas, SLOT(slotAddUnit(VisualUnit*, int, int))); // needs a QCanvas - we need to call Boson::setCanvas for this
 connect(d->mBoson, SIGNAL(signalAddUnit(VisualUnit*, int, int)), 
		d->mMiniMap, SLOT(slotAddUnit(VisualUnit*, int, int)));
 connect(d->mBoson, SIGNAL(signalAddUnit(VisualUnit*, int, int)), 
		this, SLOT(slotAddUnit(VisualUnit*, int, int)));
 connect(d->mBoson, SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
		this, SIGNAL(signalPlayerJoinedGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalPlayerJoinedGame(KPlayer*)), 
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalPlayerLeftGame(KPlayer*)), 
		this, SIGNAL(signalPlayerLeftGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalInitMap(const QByteArray&)), 
		this, SLOT(slotReceiveMap(const QByteArray&)));


 connect(d->mBigDisplay, SIGNAL(contentsMoving(int, int)), 
		d->mMiniMap, SLOT(slotMoveRect(int, int)));
 connect(d->mBigDisplay, SIGNAL(signalSizeChanged(int, int)), 
		d->mMiniMap, SLOT(slotResizeRect(int, int)));
 connect(d->mBigDisplay, SIGNAL(signalEditorAddUnit(int, int, int, int)), 
		d->mBoson, SLOT(slotAddUnit(int, int, int, int)));
 connect(d->mBigDisplay, SIGNAL(signalAddCell(int,int, int, unsigned char)),
		d->mCanvas, SLOT(slotAddCell(int, int, int, unsigned char)));
 connect(d->mBigDisplay, SIGNAL(signalAddCell(int,int, int, unsigned char)),
		d->mMiniMap, SLOT(slotAddCell(int, int, int, unsigned char)));
		
 connect(d->mMiniMap, SIGNAL(signalReCenterView(const QPoint&)), 
		d->mBigDisplay, SLOT(slotReCenterView(const QPoint&)));

 connect(d->mCanvas, SIGNAL(signalUnitMoved(VisualUnit*, double, double)), 
		d->mMiniMap, SLOT(slotMoveUnit(VisualUnit*, double, double)));
// connect(d->mCanvas, SIGNAL(signalUnitDestroyed(VisualUnit*)), 
//		d->mBigDisplay, SLOT(slotUnitDestroyed(VisualUnit*)));
 connect(d->mCanvas, SIGNAL(signalUnitDestroyed(VisualUnit*)), 
		d->mMiniMap, SLOT(slotUnitDestroyed(VisualUnit*)));



// tooltips - added in slotAddUnit
 d->mUnitTips = new KSpriteToolTip(d->mBigDisplay);

 setFocusPolicy(StrongFocus); // accept key event
 setFocus();
 d->mBoson->slotSetGameSpeed(DEFAULT_GAME_SPEED);
}

BosonWidget::~BosonWidget()
{
 delete d->mUnitTips;

// first delete all KGame related stuff - will also remove players and therefore
// units. Otherwise this is deleted later, when all units are already cleared by
// QCanvas (->crash)
 delete d->mBoson;
 if (d->mMap) {
	delete d->mMap;
 }
 delete d;
}

void BosonWidget::addLocalPlayer()
{
 if (d->mLocalPlayer) {
	delete d->mLocalPlayer;
 }
 Player* p = new Player;
 p->setName(i18n("You"));
 KGameMouseIO* bigDisplayIO = new KGameMouseIO(d->mBigDisplay, true);
 connect(bigDisplayIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)),
		d->mBigDisplay, SLOT(slotMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)));
 p->addGameIO(bigDisplayIO);
 d->mBoson->addPlayer(p);

 changeLocalPlayer(p);
}

void BosonWidget::addComputerPlayer(const QString& name)
{
 Player* p = new Player;
 p->setName(name);
 d->mBoson->addPlayer(p);
}

void BosonWidget::slotPlayerJoinedGame(KPlayer* p)
{
 if (!p) {
	kdError() << "NULL player" << endl;
	return;
 }
 // BosonBigDisplay knows whether a unit was selected. If a unit changed forward
 // the signal to the big display and let it decide whether the
 // signalSingleUnitSelected should be emitted
 connect(p, SIGNAL(signalUnitChanged(VisualUnit*)), 
		d->mBigDisplay, SLOT(slotUnitChanged(VisualUnit*)));
}

void BosonWidget::keyReleaseEvent(QKeyEvent* e)
{
 switch (e->key()) {
	case Key_Left:
		d->mBigDisplay->scrollBy(-d->mArrowKeyStep, 0);
		break;
	case Key_Right:
		d->mBigDisplay->scrollBy(d->mArrowKeyStep, 0);
		break;
	case Key_Up:
		d->mBigDisplay->scrollBy(0, -d->mArrowKeyStep);
		break;
	case Key_Down:
		d->mBigDisplay->scrollBy(0, d->mArrowKeyStep);
		break;
	default:
		break;
 }
}

void BosonWidget::slotDebug()
{
 KGameDebugDialog* dlg = new KGameDebugDialog(d->mBoson, this);
 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct()));
 dlg->show();
}

void BosonWidget::slotArrowScrollChanged(int speed)
{
 d->mArrowKeyStep = speed;
}

void BosonWidget::slotNewGame()
{
 kdDebug() << "BosonWidget::slotNewGame()" << endl;
 if (d->mBoson->isRunning()) {
	if (KMessageBox::questionYesNo(this, i18n("Quit the running game?"))
			!= KMessageBox::Yes) {
		return;
	}
 }
 quitGame();
 KGameDialog* dialog = new KGameDialog(d->mBoson, d->mLocalPlayer, i18n("New Game"),
		this, false);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(slotDelayedDestruct())); // is this called when "cancel"ed?

 // add our costum game config widget
 KGameDialogBosonConfig* bosonConfig = new KGameDialogBosonConfig(0);
 connect(bosonConfig, SIGNAL(signalStartGame()), this, SLOT(slotStartGame()));
 connect(bosonConfig, SIGNAL(signalAddComputerPlayer()), 
		this, SLOT(slotAddComputerPlayer()));

 // add a chat widget
 dialog->addGameConfig(bosonConfig);
 QVBox* page = dialog->configPage(KGameDialog::GameConfig);
 dialog->addConfigWidget(new KGameDialogChatConfig(BosonMessage::IdChat), page);

 // add a network config
 dialog->addNetworkConfig(new KGameDialogNetworkConfig(0));
 
 // a connection list - aka "ban this player"
 page = dialog->configPage(KGameDialog::NetworkConfig);
 dialog->addConnectionList(new KGameDialogConnectionConfig(0), page);

 // add a msgserver config
// dialog->addMsgServerConfig(new KGameDialogMsgServerConfig(0)); // FIXME: do
// we use this?

 // show the dialog
 dialog->show();
}

void BosonWidget::slotStartGame()
{
 if (!d->mBoson->isServer()) {
	kdWarning() << "not server" << endl;
	return;
 }
 // it would be great this way:
 // - new game dialog gives the possibility to select a map. The selected map is
 // first shown as a preview
 // - as soon as a map is actually selected (by the admin aka server) it is also
 // loaded. The dialog is not yet closed (non-modal!).
 // - the player can now scroll on the final map and decide whether he actually
 // wants to play this map
 // - then he selects "start game" which calls this slot. Here the selected map
 // is transmitted over network (we assume that the tileset is available on all
 // clients!!)
 // - as soon as all clients receive the map they load it. The local
 // (admin/server) client should not load the map again - but it wouldn't hurt
 // as long as the already loaded one is being unloaded.
 // - then the game is actually started.
 // load default map

 // first step - load the map. We don't provide several maps to be selected in
 // our newgame dialog so this here must be enough. Should be replaced by a
 // signal in newgamedialog.
 kdDebug() << "BosonWidget::slotStartGame(): load default map" << endl;
 slotLoadMap(BosonMap::defaultMap()); // also sends over network

}

void BosonWidget::slotPreferences()
{
/*
 OptionsDialog* dlg = new OptionsDialog(this);
 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct())); // seems not to be called if you quit with "cancel"!
 dlg->setGameSpeed(d->mBoson->gameSpeed());
 dlg->setArrowScrollSpeed(d->mArrowKeyStep);

 connect(dlg, SIGNAL(signalArrowScrollChanged(int)),
		this, SLOT(slotArrowScrollChanged(int)));
 connect(dlg, SIGNAL(signalSpeedChanged(int)),
		d->mBoson, SLOT(slotSetGameSpeed(int)));
 dlg->show();*/
 kdDebug() << "options disabled" << endl;
}

void BosonWidget::slotAddUnit(VisualUnit* unit, int, int)
{
 if (!unit) {
	kdError() << "NULL unit" << endl;
	return;
 }
 Player* player = unit->owner();
 if (!player) {
	kdError() << "NULL owner" << endl;
	return;
 }
 d->mUnitTips->add(unit->type(), player->speciesTheme()->unitProperties(unit)->name()); // doesn't add if this tip was added before

}

void BosonWidget::quitGame()
{
// TODO: set SpeciesTheme::defaultColor() back!! is a static variable!!
 d->mBoson->quitGame();
 
 d->mLocalPlayer = 0;

// clearMap();

 // now re-add the local player
 addLocalPlayer();
}


void BosonWidget::slotEndGame()
{
 // the game has ended but don't quit boson
 quitGame();
}

void BosonWidget::slotEditorConstructionChanged(int index)
{ // called by the map editor only
// FIXME
 kdDebug() << "slotEditorConstructionChanged()" << endl;
 if (d->mLocalPlayer) {
	kdDebug() << d->mLocalPlayer->id() << endl;
 }
 d->mCommandFrame->slotEditorConstruction(index, d->mLocalPlayer);
}

void BosonWidget::clearMap()
{
 if (d->mMap) {
	delete d->mMap;
 }
 d->mMap = new BosonMap;
 d->mCanvas->setMap(d->mMap);
 d->mMiniMap->setMap(d->mMap);
}

void BosonWidget::addEditorCommandFrame()
{ // remember to call this *after* init() - otherwise connect()s won't work
 d->mCommandFrame = new BosonCommandFrame(this, true);

 connect(d->mCommandFrame, SIGNAL(signalUnitSelected(int,VisualUnit*, Player*)), 
		d->mBigDisplay, SLOT(slotWillConstructUnit(int, VisualUnit*, Player*))); // in addEditorCommandFrame()
 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(VisualUnit*)), 
		d->mCommandFrame, SLOT(slotShowSingleUnit(VisualUnit*)));
 connect(d->mCommandFrame, SIGNAL(signalCellSelected(int,unsigned char)), 
		d->mBigDisplay, SLOT(slotWillPlaceCell(int, unsigned char))); // in addEditorCommandFrame()


}
void BosonWidget::addGameCommandFrame()
{ // remember to call this *after* init() - otherwise connect()s won't work
 d->mCommandFrame = new BosonCommandFrame(this, false);

 connect(d->mCommandFrame, SIGNAL(signalUnitSelected(int,VisualUnit*, Player*)), 
		d->mBigDisplay, SLOT(slotWillConstructUnit(int, VisualUnit*, Player*))); // in addEditorCommandFrame()
 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(VisualUnit*)), 
		d->mCommandFrame, SLOT(slotShowSingleUnit(VisualUnit*)));
 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(VisualUnit*)), 
		d->mCommandFrame, SLOT(slotSetConstruction(VisualUnit*)));
}

void BosonWidget::startGame()
{
//TODO
 kdDebug() << "BosonWidget::startGame()" << endl;
 addLocalPlayer();

}

void BosonWidget::startEditor()
{
 // this manages the mouse input for bosonBigDisplay. In non-editor mode this is
 // done by KGameMouseIO
 EditorInput* input = new EditorInput(d->mBigDisplay);
 connect(input, SIGNAL(signalMouseEvent(QMouseEvent*, bool*)), 
		d->mBigDisplay, SLOT(slotEditorMouseEvent(QMouseEvent*, bool*)));


// FIXME - the stuff below should be replaced by a proper dialog and config
// implementation

 addComputerPlayer(i18n("Computer 1"));
 addComputerPlayer(i18n("Computer 2"));

 // load default map
 // FIXME: should be loaded by a dialog!
 slotLoadMap(BosonMap::defaultMap());
 
}

void BosonWidget::slotLoadMap(const QString& map)
{
 if (!d->mBoson->isServer()) {
	kdWarning() << "BosonWidget::slotLoadMap(): not server" << endl;
	return;
 }
 if (!d->mMap) {
	kdWarning() << "NULL map" << endl;
	clearMap();
 }
 // load the map into d->mMap
 kdDebug() << "load default map" << endl;
 d->mMap->loadMap(map);

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << QString("earth.png"); // FIXME: don't send a QString!! -> i18n!!
 d->mMap->saveMapGeo(stream);

 // send the loaded map via network. It will be initialized in slotReceiveMap
 d->mBoson->sendMessage(stream, BosonMessage::InitMap);
}

void BosonWidget::slotReceiveMap(const QByteArray& buffer)
{
 QDataStream stream(buffer, IO_ReadOnly);
 QString tiles;
 stream >> tiles; // FIXME: don't send as QString!
 if (!d->mMap) {
	clearMap();
 }
 d->mMap->loadMapGeo(stream);

 // load tiles if in editor mode
 kdDebug() << "Editor mode: load tiles" << endl;
 d->mCommandFrame->slotEditorLoadTiles(tiles); // FIXME: do not load if not in editor mode

 // TODO as soon as it is implemented the map file should also contain the
 // species of the player. The NewGameDialog should enable the player to choose
 // the desired species and especially the color. This selection should be
 // stored as a KGameProperty. When this (slotReceiveMap()) is called we can
 // read (1) the map-specied and (2) the player's selection. And according to
 // these settings the correct theme should be loaded. 
 // We have to ensure that the files are actually available *before* the game 
 // is started!!
 for (unsigned int i = 0; i < d->mBoson->playerCount(); i++) {
	QRgb color = SpeciesTheme::defaultColor();
	((Player*)d->mBoson->playerList()->at(i))->loadTheme("human", color);
 }

 kdDebug() << "init map" << endl;
 d->mMiniMap->initMap();
 d->mCanvas->initMap(tiles);

 if (d->mBoson->isServer()) {
	// add player units
	kdDebug() << "start map" << endl;
	d->mMap->startMap(d->mBoson);

	// FIXME: we have too many functions called "startGame"!
	// now start the game
	d->mBoson->startGame();
 }
 kdDebug() << "slotReceiveMap done" << endl;
}

void BosonWidget::slotChangeLocalPlayer(int index)
{
 kdDebug() << "slotChangeLocalPlayer " << index << endl;
 Player* p = (Player*)d->mBoson->playerList()->at(index);
 changeLocalPlayer(p);
 d->mCommandFrame->slotEditorConstruction(-1, d->mLocalPlayer);
}

void BosonWidget::changeLocalPlayer(Player* localPlayer)
{
 d->mLocalPlayer = localPlayer; // is this used?
 d->mBigDisplay->setLocalPlayer(d->mLocalPlayer);
}

void BosonWidget::slotAddComputerPlayer()
{
 // TODO: name, difficulty, ...
 addComputerPlayer(i18n("Computer"));
}
