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

#include "editorwidget.h"

#include "defines.h"
#include "boson.h"
#include "bosonmap.h"
#include "player.h"
#include "bosonplayfield.h"
#include "bosoncursor.h"
#include "bodisplaymanager.h"
#include "global.h"
#include "bodebug.h"
#include "bpfdescriptiondialog.h"
#include "optionsdialog.h"
#include "boaction.h"
#include "botexmapimportdialog.h"
#include "bosongroundtheme.h"
#include "bosoncommandframeinterface.h"

#include <kfiledialog.h>
#include <klocale.h>
#include <kaction.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>

#include <qptrlist.h>
#include <qvalidator.h>
#include <qimage.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "editorwidget.moc"

class EditorWidget::EditorWidgetPrivate
{
public:
	EditorWidgetPrivate()
	{
		mPlayerAction = 0;
		mPlaceAction = 0;
		mChangeHeight = 0;
	}

	KSelectAction* mPlayerAction;
	KSelectAction* mPlaceAction;
	KToggleAction* mChangeHeight;

	QPtrList<Player> mPlayers;
};

EditorWidget::EditorWidget(QWidget* parent)
    : BosonWidgetBase(parent)
{
 d = new EditorWidgetPrivate;
}

EditorWidget::~EditorWidget()
{
 boDebug() << k_funcinfo << endl;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void EditorWidget::initDisplayManager()
{
 BosonWidgetBase::initDisplayManager();
 // FIXME: do it with actions
 connect(cmdFrame(), SIGNAL(signalPlaceGround(unsigned int, unsigned char*)),
		displayManager(), SLOT(slotPlaceGround(unsigned int, unsigned char*)));
 connect(cmdFrame(), SIGNAL(signalPlaceUnit(unsigned long int, Player*)),
		displayManager(), SLOT(slotPlaceUnit(unsigned long int, Player*)));

 connect(displayManager(), SIGNAL(signalLockAction(bool)),
		this, SLOT(slotLockAction(bool)));
}

void EditorWidget::initConnections()
{
 BosonWidgetBase::initConnections();

 connect(boGame, SIGNAL(signalPlayerJoinedGame(KPlayer*)),
		this, SLOT(slotPlayerJoinedGame(KPlayer*)));
 connect(boGame, SIGNAL(signalPlayerLeftGame(KPlayer*)),
		this, SLOT(slotPlayerLeftGame(KPlayer*)));
 connect(boGame, SIGNAL(signalGameStarted()),
		this, SLOT(slotGameStarted()));
}

void EditorWidget::initMap()
{
 BosonWidgetBase::initMap();
 if (!boGame->playField() || !boGame->playField()->map()) {
	boError() << k_funcinfo << endl;
	return;
 }
 BosonMap* map = boGame->playField()->map();
 connect(map, SIGNAL(signalGroundThemeChanged(BosonGroundTheme*)),
		this, SLOT(slotGroundThemeChanged(BosonGroundTheme*)));

 connect(boGame, SIGNAL(signalChangeTexMap(int,int,unsigned int,unsigned int*,unsigned char*)),
		map, SLOT(slotChangeTexMap(int,int,unsigned int,unsigned int*,unsigned char*)));

 // AB: maybe we don't need the connect() above concerning groundtheme. it can't
 // be changed once the map is started
 if (!map->groundTheme()) {
	BO_NULL_ERROR(map->groundTheme());
 } else {
	slotGroundThemeChanged(map->groundTheme());
 }
}

void EditorWidget::initPlayer()
{
 BosonWidgetBase::initPlayer();

 // FIXME: GL minimap!
// minimap()->slotShowMap(true);
}

BosonCommandFrameInterface* EditorWidget::createCommandFrame(QWidget* parent)
{
 BosonCommandFrameInterface* frame = BosonCommandFrameInterface::createCommandFrame(parent, false);
// connect(boGame, SIGNAL(signalUpdateProduction(Unit*)),
//		frame, SLOT(slotUpdateProduction(Unit*)));

 return frame;
}

void EditorWidget::slotChangeCursor(int , const QString& )
{
 // editor mode
 changeCursor(new BosonKDECursor());
}

void EditorWidget::initKActions()
{
 BosonWidgetBase::initKActions();
// KStdAction::save(this, SLOT(slotSavePlayField()), actionCollection(), "file_save_playfield");
 KStdAction::saveAs(this, SLOT(slotSavePlayFieldAs()), actionCollection(), "file_save_playfield_as");
 KAction* close = KStdAction::close(this, SIGNAL(signalEndGame()), actionCollection());
 close->setText(i18n("&End Editor"));
 KStdAction::quit(this, SIGNAL(signalQuit()), actionCollection());
 (void)KStdAction::preferences(this, SLOT(slotEditorPreferences()), actionCollection());

 d->mPlayerAction = new KSelectAction(i18n("&Player"), KShortcut(), actionCollection(), "editor_player");
 connect(d->mPlayerAction, SIGNAL(activated(int)),
		this, SLOT(slotChangeLocalPlayer(int)));

 QStringList list;
 list.append(i18n("&Facilities"));
 list.append(i18n("&Mobiles"));
 list.append(i18n("&Ground"));
 d->mPlaceAction = new KSelectAction(i18n("Place"), KShortcut(), actionCollection(), "editor_place");
 connect(d->mPlaceAction, SIGNAL(activated(int)),
		this, SLOT(slotPlace(int)));
 d->mPlaceAction->setItems(list);

 KShortcut s;
 s.append(KKeySequence(QKeySequence(Qt::Key_Delete)));
 s.append(KKeySequence(QKeySequence(Qt::Key_D)));
 (void)new KAction(i18n("Delete selected unit"), KShortcut(s), displayManager(),
		SLOT(slotDeleteSelectedUnits()), actionCollection(),
		"editor_delete_selected_unit");

 (void)new KAction(i18n("Map &description"), KShortcut(), this,
		SLOT(slotEditMapDescription()), actionCollection(),
		"editor_map_description");
 (void)new KAction(i18n("Edit &Minerals"), KShortcut(), this,
		SLOT(slotEditPlayerMinerals()), actionCollection(),
		"editor_player_minerals");
 (void)new KAction(i18n("Edit &Oil"), KShortcut(), this,
		SLOT(slotEditPlayerOil()), actionCollection(),
		"editor_player_oil");
 d->mChangeHeight = new KToggleAction(i18n("Edit &Height"), "bo_edit_height", 
		KShortcut(),this, 0, actionCollection(), "editor_height"); 
 connect(d->mChangeHeight, SIGNAL(toggled(bool)),
		this, SLOT(slotEditHeight(bool)));
 (void)new KAction(i18n("&Import height map"), KShortcut(), this,
		SLOT(slotImportHeightMap()), actionCollection(),
		"editor_import_heightmap");
 (void)new KAction(i18n("&Export height map"), KShortcut(), this,
		SLOT(slotExportHeightMap()), actionCollection(),
		"editor_export_heightmap");
 (void)new KAction(i18n("I&mport texmap"), KShortcut(), this,
		SLOT(slotImportTexMap()), actionCollection(),
		"editor_import_texmap");
 (void)new KAction(i18n("E&xport texmap"), KShortcut(), this,
		SLOT(slotExportTexMap()), actionCollection(),
		"editor_export_texmap");
 (void)new KAction(i18n("Edit global conditions"), KShortcut(), this,
		SLOT(slotEditConditions()), actionCollection(),
		"editor_edit_conditions");

// KStdAction::preferences(bosonWidget(), SLOT(slotGamePreferences()), actionCollection()); // FIXME: slotEditorPreferences()
}

void EditorWidget::saveConfig()
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
 BosonWidgetBase::saveConfig();

// boConfig->save(editor); //FIXME
 boDebug() << k_funcinfo << "done" << endl;
}

void EditorWidget::slotSavePlayFieldAs()
{
 boDebug() << k_funcinfo << endl;
 QString startIn; // shall we provide this??
 QString fileName = KFileDialog::getSaveFileName(startIn, "*.bpf", this);
 if (fileName.isNull()) {
	return;
 }
 QFileInfo info(fileName);
 if (info.extension().isEmpty()) {
	fileName += ".bpf";
 }
 if (info.exists()) {
	int r = KMessageBox::warningYesNoCancel(this, i18n("The file \"%1\" already exists. Are you sure you want to overwrite it?").arg(info.fileName()), i18n("Overwrite File?"));
	if (r != KMessageBox::Yes) {
		return;
	}
 }
 savePlayField(fileName);
}

void EditorWidget::savePlayField(const QString& fileName)
{
 bool ret = boGame->savePlayFieldToFile(fileName);
 if (!ret) {
	KMessageBox::sorry(this, i18n("An error occurred while saving the playfield. Unable to save."));
 }
}

void EditorWidget::slotSavePlayField()
{
 boWarning() << k_funcinfo << "not yet implemented" << endl;
}

void EditorWidget::slotChangeLocalPlayer(int index)
{
 Player* p = 0;
 p = d->mPlayers.at(index);
 if (p) {
	emit signalChangeLocalPlayer(p);
	if (d->mPlaceAction->currentItem() >= 0) {
		slotPlace(d->mPlaceAction->currentItem());
	}
 } else {
	boWarning() << k_funcinfo << "NULL player for index " << index << endl;
 }
}

void EditorWidget::slotPlace(int index)
{
 boDebug() << k_funcinfo << "index: " << index << endl;
 BosonCommandFrameInterface* cmd = cmdFrame();
 if (!cmd) {
	boError() << k_funcinfo << "NULL cmd frame" << endl;
	return;
 }
 switch (index) {
	case 0:
		cmd->placeFacilities(localPlayer());
		break;
	case 1:
		cmd->placeMobiles(localPlayer());
		break;
	case 2:
		cmd->placeGround();
		break;
	default:
		boError() << k_funcinfo << "Invalid index " << index << endl;
		return;
 }
}

void EditorWidget::setBosonXMLFile()
{
 BosonWidgetBase::setBosonXMLFile();
 setXMLFile("editorui.rc", true);
}

void EditorWidget::slotPlayerJoinedGame(KPlayer* player)
{
 boDebug() << k_funcinfo << endl;
 if (!player) {
	return;
 }
 BosonWidgetBase::slotPlayerJoinedGame(player);
 Player* p = (Player*)player;
 QStringList players = d->mPlayerAction->items();
 d->mPlayers.append(p);
 players.append(p->name());
 d->mPlayerAction->setItems(players);
}

void EditorWidget::slotPlayerLeftGame(KPlayer* player)
{
 if (!player) {
	return;
 }
 boDebug() << k_funcinfo << player->id() << endl;
 BosonWidgetBase::slotPlayerLeftGame(player);
 Player* p = (Player*)p;
 if (d->mPlayers.count() == 0) {
	return;
 }
 int index = d->mPlayers.find((Player*)player);
 if (index < 0) {
	boError() << k_funcinfo << "player not found" << endl;
	return;
 }
 QStringList players = d->mPlayerAction->items();

 players.remove(players.at(index));
 d->mPlayers.removeRef((Player*)player);

 d->mPlayerAction->setItems(players);
}

void EditorWidget::slotGroundThemeChanged(BosonGroundTheme* theme)
{
 BO_CHECK_NULL_RET(cmdFrame());
 cmdFrame()->setGroundTheme(theme);
}

void EditorWidget::slotGameStarted()
{
 d->mPlaceAction->setCurrentItem(0);
 slotPlace(0);
}

void EditorWidget::startScenarioAndGame()
{
 BosonWidgetBase::startScenarioAndGame();
 slotChangeLocalPlayer(0);
 d->mPlayerAction->setCurrentItem(0);
}

void EditorWidget::slotEditMapDescription()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->description());


// TODO: non-modal might be fine. one could use that for translations (one
// dialog the original language, one the translated language)
 BPFDescriptionDialog* dialog = new BPFDescriptionDialog(this, true);
 dialog->setDescription(boGame->playField()->description());
 dialog->exec();

 delete dialog;
}

void EditorWidget::slotEditorPreferences()
{
 OptionsDialog* dlg = gamePreferences(true);
 if (!dlg) {
	boError() << k_funcinfo << "NULL options dialog" << endl;
	return;
 }
 dlg->show();
}

void EditorWidget::slotEditPlayerMinerals()
{
 BO_CHECK_NULL_RET(localPlayer());
 bool ok = false;
 QString value = QString::number(localPlayer()->minerals());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Minerals for player %1").arg(localPlayer()->name()), value, &ok, this, &val);
 if (!ok) {
	// cancel pressed
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer()->setMinerals(v);
}

void EditorWidget::slotEditPlayerOil()
{
 BO_CHECK_NULL_RET(localPlayer());
 bool ok = false;
 QString value = QString::number(localPlayer()->oil());
 QIntValidator val(this);
 val.setBottom(0);
 val.setTop(1000000); // we need to set a top, because int is limited. this should be enough, i hope (otherwise feel free to increase)
 value = KLineEditDlg::getText(i18n("Oil for player %1").arg(localPlayer()->name()), value, &ok, this, &val);
 if (!ok) {
	return;
 }
 boDebug() << k_funcinfo << value << endl;
 unsigned long int v = value.toULong(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "value " << value << " not valid" << endl;
	return;
 }
 localPlayer()->setOil(v);
}

void EditorWidget::slotEditHeight(bool on)
{
 if (on) {
	BoSpecificAction action;
  action.setType(ActionChangeHeight);
	displayManager()->slotAction(action);
 } else {
	displayManager()->unlockAction();
 }
}

void EditorWidget::slotLockAction(bool locked)
{
 if (locked) {
	// we might display something in the cmdframe or in the toolbar or so.
	// the cursor will get updated anyway.
	return;
 }
 if (d->mChangeHeight->isChecked()) {
	d->mChangeHeight->setChecked(false);
 }
}

void EditorWidget::slotImportHeightMap()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 boDebug() << k_funcinfo << endl;
 QString fileName = KFileDialog::getOpenFileName(QString::null, "*.png", this);
 if (fileName.isNull()) {
	return;
 }
 // first load the file as an image. We need it to be in greyscale png in boson,
 // and we can easily convert that image.
 QImage image(fileName);
 if (image.isNull()) {
	boError() << k_funcinfo << "unbable to load file " << fileName << endl;
	KMessageBox::sorry(this, i18n("Unable to load %1\nSeems not to be a valid image.").arg(fileName));
	return;
 }
 BosonMap* map = boGame->playField()->map();
 if ((unsigned int)image.width() != map->width() + 1 ||
		(unsigned int)image.height() != map->height() + 1) {
	KMessageBox::sorry(this, i18n("This image can't be used as height map for this map. The map is a %1x%2 map, meaning you need a %3x%4 image.\nThe image selected %5 was %6x%7").
			arg(map->width()).arg(map->height()).
			arg(map->width() + 1).arg(map->height() + 1).
			arg(fileName).
			arg(image.width()).arg(image.height()));
	return;
 }
 if (!image.isGrayscale()) {
	KMessageBox::sorry(this, i18n("%1 is not a greyscale image").arg(fileName));
	return;
 }
 boGame->playField()->importHeightMapImage(image);
 // TODO: update unit positions!
}

void EditorWidget::slotExportHeightMap()
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 boDebug() << k_funcinfo << endl;
 QString fileName = KFileDialog::getSaveFileName(QString::null, "*.png", this);
 if (fileName.isNull()) {
	return;
 }
 QByteArray buffer = boGame->playField()->exportHeightMap();
 if (buffer.size() == 0) {
	boError() << k_funcinfo << "Could not export heightMap" << endl;
	KMessageBox::sorry(this, i18n("Unable to export heightMap"));
	return;
 }
 QImage image(buffer);
 if (image.isNull()) {
	boError() << k_funcinfo << "an invalid image has been generated" << endl;
	KMessageBox::sorry(this, i18n("An invalid heightmop image has been generated."));
 }
 if (!image.save(fileName, "PNG")) {
	boError() << k_funcinfo << "unable to save image to " << fileName << endl;
	KMessageBox::sorry(this, i18n("Unable to save image to %1.").arg(fileName));
	return;
 }
}

void EditorWidget::slotImportTexMap()
{
 boDebug() << k_funcinfo << endl;
 BoTexMapImportDialog* dialog = new BoTexMapImportDialog(this);
 connect(dialog, SIGNAL(finished()),
		dialog, SLOT(deleteLater()));

 BosonMap* map = boGame->playField()->map();
 dialog->setMap(map);

 dialog->show();
 dialog->slotSelectTexMapImage();
}

void EditorWidget::slotExportTexMap()
{
 boDebug() << k_funcinfo << endl;
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->playField());
 BO_CHECK_NULL_RET(boGame->playField()->map());
 BO_CHECK_NULL_RET(boGame->playField()->map()->groundTheme());

 BosonMap* map = boGame->playField()->map();
 QStringList textures;
 for (unsigned int i = 0; i < map->groundTheme()->textureCount(); i++) {
	textures.append(map->groundTheme()->textureFileName(i));
 }

 QDialog* d = new QDialog(0, 0, true);
 QVBoxLayout* layout = new QVBoxLayout(d);
 QLabel* label = new QLabel(i18n("Select texture to export:"), d);
 QComboBox* combo = new QComboBox(d);
 QPushButton* button = new QPushButton(i18n("Ok"), d);
 connect(button, SIGNAL(clicked()), d, SLOT(accept()));
 combo->insertStringList(textures);
 layout->addWidget(label);
 layout->addWidget(combo);
 layout->addWidget(button);
 d->exec();
 unsigned int tex = (unsigned int)combo->currentItem();
 boDebug() << k_funcinfo << "tex: " << tex << endl;
 delete d;

 QString fileName = KFileDialog::getSaveFileName(QString::null, "*.png", this);
 if (fileName.isNull()) {
	return;
 }
 QByteArray buffer = boGame->playField()->exportTexMap(tex);
 if (buffer.size() == 0) {
	boError() << k_funcinfo << "Could not export texmap" << endl;
	KMessageBox::sorry(this, i18n("Unable to export texmap"));
	return;
 }
 QImage image(buffer);
 if (image.isNull()) {
	boError() << k_funcinfo << "an invalid image has been generated" << endl;
	KMessageBox::sorry(this, i18n("An invalid texmap image has been generated."));
 }
 if (!image.save(fileName, "PNG")) {
	boError() << k_funcinfo << "unable to save image to " << fileName << endl;
	KMessageBox::sorry(this, i18n("Unable to save image to %1.").arg(fileName));
	return;
 }
}

