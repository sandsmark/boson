
#include "bosonwidget.h"

#include "bosonbigdisplay.h"
#include "bosonminimap.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "unit.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "kspritetooltip.h"
#include "bosoncommandframe.h"
#include "editorinput.h"
#include "bosonmessage.h"
#include "kgamedialogbosonconfig.h"
#include "bosonmap.h"
#include "bosonscenario.h"
#include "bosonconfig.h"
#include "optionsdialog.h"
#include "kgamedialogcomputerconfig.h"

#include "defines.h"

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
		mScenario = 0;
	}
	
	BosonBigDisplay* mBigDisplay;
	BosonMiniMap* mMiniMap;
	BosonCommandFrame* mCommandFrame;

	BosonCanvas* mCanvas;
	
	Boson* mBoson;
	Player* mLocalPlayer;
	BosonMap* mMap;
	BosonScenario* mScenario;

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

 d->mBoson->setCanvas(d->mCanvas); // should not be stored here - but seems to be necessary :-(


// new code
 connect(d->mBigDisplay, SIGNAL(signalConstructUnit(int,int, int, Player*)),
		d->mBoson, SLOT(slotSendAddUnit(int, int, int, Player*)));


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
 d->mBoson->slotSetGameSpeed(BosonConfig::gameSpeed());
 connect(d->mBoson, SIGNAL(signalAdvance()),
		d->mCanvas, SLOT(advance()));
 connect(d->mBoson, SIGNAL(signalAddUnit(Unit*, int, int)),
		d->mCanvas, SLOT(slotAddUnit(Unit*, int, int))); // needs a QCanvas - we need to call Boson::setCanvas for this
 connect(d->mBoson, SIGNAL(signalAddUnit(Unit*, int, int)),
		d->mMiniMap, SLOT(slotAddUnit(Unit*, int, int)));
 connect(d->mBoson, SIGNAL(signalAddUnit(Unit*, int, int)),
		this, SLOT(slotAddUnit(Unit*, int, int)));
 connect(d->mBoson, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SIGNAL(signalPlayerJoinedGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SIGNAL(signalPlayerLeftGame(KPlayer*)));
 connect(d->mBoson, SIGNAL(signalInitMap(const QByteArray&)),
		this, SLOT(slotReceiveMap(const QByteArray&)));
 connect(d->mBoson, SIGNAL(signalStartScenario()),
		this, SLOT(slotStartScenario()));


 connect(d->mBigDisplay, SIGNAL(contentsMoving(int, int)),
		d->mMiniMap, SLOT(slotMoveRect(int, int)));
 connect(d->mBigDisplay, SIGNAL(signalSizeChanged(int, int)),
		d->mMiniMap, SLOT(slotResizeRect(int, int)));
 connect(d->mBigDisplay, SIGNAL(signalAddCell(int,int, int, unsigned char)),
		d->mCanvas, SLOT(slotAddCell(int, int, int, unsigned char)));
 connect(d->mBigDisplay, SIGNAL(signalAddCell(int,int, int, unsigned char)),
		d->mMiniMap, SLOT(slotAddCell(int, int, int, unsigned char)));
 connect(d->mBigDisplay, SIGNAL(signalAddCell(int,int, int, unsigned char)),
		this, SLOT(slotAddCell(int, int, int, unsigned char)));
		
 connect(d->mMiniMap, SIGNAL(signalReCenterView(const QPoint&)),
		d->mBigDisplay, SLOT(slotReCenterView(const QPoint&)));

 connect(d->mCanvas, SIGNAL(signalUnitMoved(Unit*, double, double)),
		d->mMiniMap, SLOT(slotMoveUnit(Unit*, double, double)));
// connect(d->mCanvas, SIGNAL(signalUnitDestroyed(Unit*)), 
//		d->mBigDisplay, SLOT(slotUnitDestroyed(Unit*)));
 connect(d->mCanvas, SIGNAL(signalUnitDestroyed(Unit*)), 
		d->mMiniMap, SLOT(slotUnitDestroyed(Unit*)));



// tooltips - added in slotAddUnit
 d->mUnitTips = new KSpriteToolTip(d->mBigDisplay);

 // 640*480 is probably not enough (KDE needs at least 800*600) but as a minimum
 // should be ok.
 setMinimumWidth(640);
 setMinimumHeight(480);

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
 if (d->mScenario) {
	delete d->mScenario;
 }
 delete d;
}

void BosonWidget::addLocalPlayer()
{
 if (d->mLocalPlayer) {
	delete d->mLocalPlayer;
 }
 Player* p = new Player;
 p->setName(BosonConfig::localPlayerName());
 KGameMouseIO* bigDisplayIO = new KGameMouseIO(d->mBigDisplay, true);
 connect(bigDisplayIO, SIGNAL(signalMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)),
		d->mBigDisplay, SLOT(slotMouseEvent(KGameIO*, QDataStream&, QMouseEvent*, bool*)));
 p->addGameIO(bigDisplayIO);
 d->mBoson->addPlayer(p);

 changeLocalPlayer(p);
}

void BosonWidget::addDummyComputerPlayer(const QString& name)
{
 Player* p = new Player;
 p->setName(name);
 d->mBoson->addPlayer(p);
 p->loadTheme(SpeciesTheme::defaultSpecies(), SpeciesTheme::defaultColor());// FIXME - should be selectable in new game dialog
}

void BosonWidget::slotPlayerJoinedGame(KPlayer* p)
{
 if (!p) {
	kdError() << k_funcinfo << ": NULL player" << endl;
	return;
 }
 // BosonBigDisplay knows whether a unit was selected. If a unit changed forward
 // the signal to the big display and let it decide whether the
 // signalSingleUnitSelected should be emitted
 connect(p, SIGNAL(signalUnitChanged(Unit*)), 
		d->mBigDisplay, SLOT(slotUnitChanged(Unit*)));
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
// kdDebug() << k_funcinfo << endl;
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
// connect(bosonConfig, SIGNAL(signalAddComputerPlayer()),
//		this, SLOT(slotAddComputerPlayer()));
 connect(bosonConfig, SIGNAL(signalMapChanged(const QString&)),
		this, SLOT(slotLoadMap(const QString&)));
 connect(bosonConfig, SIGNAL(signalScenarioChanged(const QString&)),
		this, SLOT(slotLoadScenario(const QString&)));
 connect(bosonConfig, SIGNAL(signalSpeciesChanged(const QString&)),
		this, SLOT(slotChangeSpecies(const QString&)));

 // add a connection and a chat widget
 dialog->addGameConfig(bosonConfig);
 QVBox* page = dialog->configPage(KGameDialog::GameConfig);
 dialog->addConfigWidget(new KGameDialogConnectionConfig(), page);
 dialog->addConfigWidget(new KGameDialogChatConfig(BosonMessage::IdChat), page);

 KGameDialogComputerConfig* computerConfig = new KGameDialogComputerConfig(0);
 connect(computerConfig, SIGNAL(signalAddComputerPlayer(Player*)), 
		this, SLOT(slotAddComputerPlayer(Player*)));
 QVBox* computerPage = dialog->addConfigPage(computerConfig, i18n("&Computer Player"));
 dialog->addConnectionList(new KGameDialogConnectionConfig(0), computerPage);

 // add a network config
 dialog->addNetworkConfig(new KGameDialogNetworkConfig(0));

 // a connection list - aka "ban this player" - also in game page (to see
 // the number of the players when selecting a map)
 QVBox* networkPage = dialog->configPage(KGameDialog::NetworkConfig);
 dialog->addConnectionList(new KGameDialogConnectionConfig(0), networkPage);

 // add a msgserver config
// dialog->addMsgServerConfig(new KGameDialogMsgServerConfig(0)); // FIXME: do
// we use this?

 // show the dialog
 dialog->show();
 bosonConfig->slotMapChanged(0);
}

void BosonWidget::slotStartGame()
{
 if (!d->mBoson->isAdmin()) {
	KMessageBox::sorry(this, i18n("Only ADMIN can start the game"));
	kdWarning() << "not admin" << endl;
	return;
 }
 if (d->mBoson->isRunning()) {
	KMessageBox::sorry(this, i18n("The game is already running"));
	kdWarning() << "game already running" << endl;
	return;
 }
 if (d->mBoson->playerCount() < d->mBoson->minPlayers()) {
	KMessageBox::sorry(this, i18n("Need at least %1 players").arg(d->mBoson->minPlayers()));
	kdError() << "not enough players" << endl;
	return;
 }
 if (d->mBoson->maxPlayers() > 0 && (int)d->mBoson->playerCount() > d->mBoson->maxPlayers()) {
	KMessageBox::sorry(this, i18n("Maximal %1 players").arg(d->mBoson->maxPlayers()));
	kdError() << "too many players" << endl;
	return;
 }

 d->mBoson->sendMessage(0, BosonMessage::IdStartScenario);
}

void BosonWidget::slotStartScenario()
{
// kdDebug() << k_funcinfo << endl;
 if (!d->mScenario) {
	kdError() << k_funcinfo << ": NULL scenario" << endl;
	return;
 }
 d->mScenario->startScenario(d->mBoson);

 // TODO as soon as it is implemented the map file should also contain the
 // species of the player. The NewGameDialog should enable the player to choose
 // the desired species and especially the color. This selection should be
 // stored as a KGameProperty. When this (slotReceiveMap()) is called we can
 // read (1) the map-specie and (2) the player's selection. And according to
 // these settings the correct theme should be loaded. 
 // We have to ensure that the files are actually available *before* the game 
 // is started!!
 // UPDATE (01/11/19): should be in scenario file!
 for (unsigned int i = 0; i < d->mBoson->playerCount(); i++) {
//	QRgb color = SpeciesTheme::defaultColor();
//	QString species = "human";
//	((Player*)d->mBoson->playerList()->at(i))->loadTheme(species, color);
 }
 d->mBoson->startGame(); // correct here? should be so.
}

void BosonWidget::slotGamePreferences()
{

 OptionsDialog* dlg = new OptionsDialog(this);
 connect(dlg, SIGNAL(finished()), dlg, SLOT(slotDelayedDestruct())); // seems not to be called if you quit with "cancel"!
 dlg->setGameSpeed(d->mBoson->gameSpeed());
 dlg->setArrowScrollSpeed(d->mArrowKeyStep);

 connect(dlg, SIGNAL(signalArrowScrollChanged(int)),
		this, SLOT(slotArrowScrollChanged(int)));
 connect(dlg, SIGNAL(signalSpeedChanged(int)),
		d->mBoson, SLOT(slotSetGameSpeed(int)));
 dlg->show();
}

void BosonWidget::slotAddUnit(Unit* unit, int, int)
{
 if (!unit) {
	kdError() << k_funcinfo << ": NULL unit" << endl;
	return;
 }
 Player* player = unit->owner();
 if (!player) {
	kdError() << k_funcinfo << ": NULL owner" << endl;
	return;
 }
 d->mUnitTips->add(unit->type(), player->speciesTheme()->unitProperties(unit)->name()); // doesn't add if this tip was added before
}

void BosonWidget::quitGame()
{
// TODO: set SpeciesTheme::defaultColor() back!! is a static variable!!
 d->mBoson->quitGame();
 
 d->mLocalPlayer = 0;

 if (d->mMap) {
	delete d->mMap;
	d->mMap = 0;
 }

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
// kdDebug() << k_funcinfo << endl;
 if (d->mLocalPlayer) {
	kdDebug() << "local player: " << d->mLocalPlayer->id() << endl;
 }
 d->mCommandFrame->slotEditorConstruction(index, d->mLocalPlayer);
}

void BosonWidget::recreateMap()
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

 connect(d->mCommandFrame, SIGNAL(signalUnitSelected(int,Unit*, Player*)), 
		d->mBigDisplay, SLOT(slotWillConstructUnit(int, Unit*, Player*))); // in addEditorCommandFrame()
 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(Unit*)), 
		d->mCommandFrame, SLOT(slotShowSingleUnit(Unit*)));
 connect(d->mCommandFrame, SIGNAL(signalCellSelected(int,unsigned char)), 
		d->mBigDisplay, SLOT(slotWillPlaceCell(int, unsigned char))); // in addEditorCommandFrame()


}
void BosonWidget::addGameCommandFrame()
{ // remember to call this *after* init() - otherwise connect()s won't work
 d->mCommandFrame = new BosonCommandFrame(this, false);

 connect(d->mCommandFrame, SIGNAL(signalUnitSelected(int,Unit*, Player*)),
		d->mBigDisplay, SLOT(slotWillConstructUnit(int, Unit*, Player*))); // in addEditorCommandFrame()
 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(Unit*)),
		d->mCommandFrame, SLOT(slotShowSingleUnit(Unit*)));
 connect(d->mBigDisplay, SIGNAL(signalSingleUnitSelected(Unit*)),
		d->mCommandFrame, SLOT(slotSetConstruction(Unit*)));
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

 addDummyComputerPlayer(i18n("Computer 1"));
 addDummyComputerPlayer(i18n("Computer 2"));

 // load default map
 // FIXME: should be loaded by a dialog!
 slotLoadMap(BosonMap::defaultMap());
 slotLoadScenario(BosonScenario::defaultScenario()); // perhaps this should load the map as well - as it depends on the map...


 d->mBoson->sendMessage(0, BosonMessage::IdStartScenario);
}

void BosonWidget::slotLoadMap(const QString& map)
{
// the map is first loaded locally from the file. Then the data is sent over
// network. It is initialized (i.e. the cells are shown in the canvas) when the
// data is received from network, in slotReceiveMap()
 if (!d->mBoson->isAdmin()) {
	kdWarning() << k_funcinfo << ": not ADMIN" << endl;
	return;
 }
 recreateMap();
 // load the map into d->mMap
 kdDebug() << "load map " << map << endl;
 if (!d->mMap->loadMap(map)) {
	return;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 d->mMap->saveMapGeo(stream);
 d->mMap->saveCells(stream);

 // send the loaded map via network. It will be initialized in slotReceiveMap
 d->mBoson->sendMessage(stream, BosonMessage::InitMap);

 delete d->mMap; // we create it again when InitMap is received again.
 d->mMap = 0;
}

void BosonWidget::slotLoadScenario(const QString& scenario)
{
 if (!d->mBoson->isAdmin()) {
	kdWarning() << k_funcinfo << ": not ADMIN" << endl;
	return;
 }
 if (d->mScenario) {
	delete d->mScenario;
 }
 d->mScenario = new BosonScenario;
 d->mScenario->loadScenario(scenario);
 d->mBoson->setMinPlayers(d->mScenario->minPlayers());
 d->mBoson->setMaxPlayers(d->mScenario->maxPlayers());
}

void BosonWidget::slotReceiveMap(const QByteArray& buffer)
{
 QDataStream stream(buffer, IO_ReadOnly);
 recreateMap();
 QString tiles = "earth.png"; // TODO: should be selectable
 d->mMap->loadMapGeo(stream);
 d->mMap->loadCells(stream);

 // load tiles if in editor mode
 kdDebug() << "Editor mode: load tiles" << endl;
 d->mCommandFrame->slotEditorLoadTiles(tiles); // FIXME: do not load if not in editor mode

 kdDebug() << "init map" << endl;
 d->mMiniMap->initMap();
 d->mCanvas->initMap(tiles);

 kdDebug() << k_funcinfo << " done" << endl;
}

void BosonWidget::slotChangeLocalPlayer(int index)
{
// kdDebug() << k_funcinfo << endl;
 Player* p = (Player*)d->mBoson->playerList()->at(index);
 changeLocalPlayer(p);
 d->mCommandFrame->slotEditorConstruction(-1, d->mLocalPlayer);
}

void BosonWidget::changeLocalPlayer(Player* localPlayer)
{
 d->mLocalPlayer = localPlayer; // is this used?
 d->mBigDisplay->setLocalPlayer(d->mLocalPlayer);
 d->mCommandFrame->setLocalPlayer(d->mLocalPlayer);
}

void BosonWidget::slotAddComputerPlayer(Player* computer)
{
 d->mBoson->addPlayer(computer);
}

void BosonWidget::slotEditorSaveMap(const QString& fileName)
{
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 // TODO: let the user choose - binary or XML.
 d->mMap->saveMap(fileName, false);
}

void BosonWidget::slotEditorSaveScenario(const QString& fileName)
{
 if (!d->mScenario) {
	kdError() << k_funcinfo << ": NULL scenario" << endl;
	return;
 }
 // TODO: let the user choose - binary or XML? XML is far better here. binary is
 // probably useless (scenario files are not that big).
 d->mScenario->saveScenario(fileName, false);
}

void BosonWidget::slotAddCell(int x, int y, int type, unsigned char b)
{
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 d->mMap->changeCell(x, y, type, b);
}

void BosonWidget::saveConfig()
{
 // note: the game is *not* saved here! just general settings like game speed,
 // player name, ...
 if (!d->mBoson) {
	kdError() << k_funcinfo << ": NULL game" << endl;
	return;
 }
 if (!d->mLocalPlayer) {
	kdError() << k_funcinfo << ": NULL local player" << endl;
	return;
 }
 BosonConfig::saveLocalPlayerName(d->mLocalPlayer->name());
 BosonConfig::saveGameSpeed(d->mBoson->gameSpeed());
}

void BosonWidget::slotChangeSpecies(const QString& directory)
{
 if (!d->mLocalPlayer) {
	kdError() << k_funcinfo << ": no local player!" << endl;
	return;
 }
 d->mLocalPlayer->loadTheme(directory, SpeciesTheme::defaultColor());
}

void BosonWidget::zoom(const QWMatrix& m)
{
 d->mBigDisplay->setWorldMatrix(m);
}
