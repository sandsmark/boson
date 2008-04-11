/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "borendermain.h"
#include "borendermain.moc"

#include "../defines.h"
#include "../../bomemory/bodummymemory.h"
#include "../gameengine/speciestheme.h"
#include "../speciesdata.h"
#include "../bosonconfig.h"
#include "../modelrendering/bosonmodel.h"
#include "../modelrendering/bosonmodeltextures.h"
#include "../modelrendering/bomesh.h"
#include "../bomaterial.h"
#include "../bosonprofiling.h"
#include "../gameengine/unitproperties.h"
#include "../kgame3dsmodeldebug.h"
#include "../kgamespeciesdebug.h"
#include "bodebug.h"
#include "../boversion.h"
#include "../boapplication.h"
#include "../bocamera.h"
#include "../bocamerawidget.h"
#include "../bomaterialwidget.h"
#include "../bolight.h"
#include "../modelrendering/bomeshrenderermanager.h"
#include "../boglstatewidget.h"
#include "../botexture.h"
#include "../bomousemovediff.h"
#include "../bosonviewdata.h"
#include "boufo.h"
#include "../boufo/boufoaction.h"
#include "../info/boinfo.h"
#include "borendergui.h"
#include "boeditturretpropertiesdialog.h"
#ifdef BOSON_USE_BOMEMORY
#include "../../bomemory/bomemorydialog.h"
#endif
#define BOSONFONT 0
#if BOSONFONT
#include "../boufo/bosonfont/bosonglfont.h"
#include "../boufo/bosonfont/bosonglfontchooser.h"
#endif
#include "borenderrendermodel.h"
#include "../bopixmaprenderer.h"
#include <bogl.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kaction.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <KDialog>
#include <kcolordialog.h>
#include <kmessagebox.h>

#include <qtimer.h>
#include <qlayout.h>
#include <qinputdialog.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <Q3ValueList>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3CString>
#include <Q3PtrList>

#include <math.h>
#include <stdlib.h>

#define NEAR 1.0
#define FAR 100.0

#define GL_UPDATE_TIMER 40

#define BORENDER_DEFAULT_LIGHT_ENABLED true
#define BORENDER_DEFAULT_MATERIALS_ENABLED true

static KLocalizedString description =
    ki18n("Rendering tester for Boson");

static const char *version = BOSON_VERSION_STRING;

void postBosonConfigInit();



class BoRenderGLWidgetPrivate
{
public:
	BoRenderGLWidgetPrivate()
	{
		mGUI = 0;
		mModelDisplay = 0;

		mMaterialWidget = 0;
		mLightWidget = 0;
	}
	BoRenderGUI* mGUI;
	ModelDisplay* mModelDisplay;
	Q3PtrList<SpeciesTheme> mSpecies;
	Q3PtrDict<SpeciesTheme> mAction2Species;

	// TODO: use a BoUfo widget. don't use a separate window.
	BoMaterialWidget* mMaterialWidget;
	BoLightCameraWidget1* mLightWidget;
};

BoRenderGLWidget::BoRenderGLWidget(QWidget* parent, bool direct)
	: BosonUfoGLWidget(parent, direct)
{
 d = new BoRenderGLWidgetPrivate();
}

void BoRenderGLWidget::initWidget()
{
 d->mMaterialWidget = 0;
 d->mSpecies.setAutoDelete(true);
 QStringList list = SpeciesTheme::availableSpecies();
 for (int i = 0; i < list.count(); i++) {
	QString dir = list[i];
	dir = dir.left(dir.length() - QString("index.species").length());
	SpeciesTheme* s = new SpeciesTheme();
	s->setThemePath(dir);
	s->setTeamColor(QColor());
	d->mSpecies.append(s);
	s->readUnitConfigs();
 }

 glInit();
}

BoRenderGLWidget::~BoRenderGLWidget()
{
 delete d->mLightWidget;
 delete d->mMaterialWidget;
 d->mSpecies.setAutoDelete(true);
 d->mSpecies.clear();
 delete d;
}

bool BoRenderGLWidget::parseCmdLineArgs(KCmdLineArgs* args)
{
 QString theme;
 unsigned long int typeId = 0;
 QString unit;
 QString object;

 if (args->isSet("species")) {
	theme = args->getOption("species");
 }
 if (args->isSet("unit")) {
	if (theme.isEmpty()) {
		theme = QString::fromLatin1("human");
	}
	unit = args->getOption("unit");
 } else if (args->isSet("unit-id")) {
	if (theme.isEmpty()) {
		theme = QString::fromLatin1("human");
	}
	QString arg = args->getOption("unit-id");
	bool ok = false;
	typeId = arg.toULong(&ok);
	if (!ok) {
		boError() << k_funcinfo << "unit-id must be a number" << endl;
		return 1;
	}
 } else if (args->isSet("object")) {
	if (theme.isEmpty()) {
		theme = QString::fromLatin1("human");
	}
	object = args->getOption("object");
 }
 if (!theme.isEmpty()) {
	if (typeId == 0 && unit.isEmpty() && object.isEmpty()) {
		boError() << k_funcinfo << "you have to specify both species and unit/object!" << endl;
		return 1;
	} else if (typeId > 0) {
		changeUnit(theme, typeId);
	} else if (!unit.isEmpty()) {
		changeUnit(theme, unit);
	} else if (!object.isEmpty()) {
		changeObject(theme, object);
	} else {
		boError() << k_funcinfo << "you have to specify both unit and species!" << endl;
		return 1;
	}
 }



 if (!loadCamera(args)){
	return false;
 }


 if (args->isSet("fovy")) {
	bool ok = false;
	float f = args->getOption("fovy").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "fovy must be a number" << endl;
		return 1;
	}
	d->mGUI->mFrame->setValue(f);
 }
 if (args->isSet("frame")) {
	bool ok = false;
	int f = args->getOption("frame").toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "frame must be a number" << endl;
		return 1;
	}
	d->mGUI->mFrame->setValue(f);
 }
 if (args->isSet("lod")) {
	bool ok = false;
	int l = args->getOption("lod").toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "lod must be a number" << endl;
		return 1;
	}
	d->mGUI->mLOD->setValue(l);
 }
 return true;
}

bool BoRenderGLWidget::loadCamera(KCmdLineArgs* args)
{
 BoQuaternion quat = d->mModelDisplay->camera()->quaternion();
 BoVector3Float cameraPos = d->mModelDisplay->camera()->cameraPos();

 // in borender we use a first translate, then rotate approach, whereas the
 // camera does it the other way round. we need to transform the vector first.
 quat.transform(&cameraPos, &cameraPos);

 if (args->isSet("camera-x")) {
	bool ok = false;
	float c = args->getOption("camera-x").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-x must be a number" << endl;
		return false;
	}
	cameraPos.setX(c);
 }
 if (args->isSet("camera-y")) {
	bool ok = false;
	float c = args->getOption("camera-y").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-y must be a number" << endl;
		return false;
	}
	cameraPos.setY(c);
 }
 if (args->isSet("camera-z")) {
	bool ok = false;
	float c = args->getOption("camera-z").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-z must be a number" << endl;
		return false;
	}
	cameraPos.setZ(c);
 }
 if (args->isSet("rotate-x")) {
	bool ok = false;
	float r = args->getOption("rotate-x").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-x must be a number" << endl;
		return false;
	}
	BoQuaternion q;
	q.setRotation(r, 0.0f, 0.0f);
	quat.multiply(q);
 }
 if (args->isSet("rotate-y")) {
	bool ok = false;
	float r = args->getOption("rotate-y").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-y must be a number" << endl;
		return false;
	}
	BoQuaternion q;
	q.setRotation(0.0f, r, 0.0f);
	quat.multiply(q);
 }
 if (args->isSet("rotate-z")) {
	bool ok = false;
	float r = args->getOption("rotate-z").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-z must be a number" << endl;
		return false;
	}
	BoQuaternion q;
	q.setRotation(0.0f, 0.0f, r);
	quat.multiply(q);
 }
 // re-transform to gluLookAt() values
 quat.inverse().transform(&cameraPos, &cameraPos);

 if (args->isSet("lookAtCenter")) {
	if (args->isSet("rotate-x") ||
			args->isSet("rotate-y") ||
			args->isSet("rotate-z")) {
		boWarning() << "--rotate-x, --rotate-y and --rotate-z are ignored when --lookAtCenter was specified!" << endl;
	}
	BoVector3Float lookAt = BoVector3Float(0, 0, 0);
	BoVector3Float up = BoVector3Float(0, 0, 1);
	quat.setRotation(cameraPos, lookAt, up);
 }

 BoVector3Float lookAt, up;
 quat.matrix().toGluLookAt(&lookAt, &up, BoVector3Float(0, 0, 0));

 d->mModelDisplay->updateCamera(cameraPos, lookAt, up);

 return true;
}

void BoRenderGLWidget::initializeGL()
{
 boDebug() << k_funcinfo << endl;
 makeCurrent();
 BoInfo::boInfo()->update(this);
 setUpdatesEnabled(false);

 BoTextureManager::initStatic();
 BoLightManager::initStatic();
 BoMeshRendererManager::initStatic();
 boTextureManager->initOpenGL();

 // in case we are rendering to a pixmap:
 boTextureManager->reloadTextures();
 // TODO: reload light?

 glClearColor(0.0, 0.0, 0.0, 0.0);
 glShadeModel(GL_FLAT);
 glDisable(GL_DITHER);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 initUfoGUI();
}

void BoRenderGLWidget::initUfoGUI()
{
 static bool initialized = false;
 if (initialized) {
	boWarning() << k_funcinfo << "called twice" << endl;
	return;
 }
 initialized = true;

 initUfo();

 d->mGUI = new BoRenderGUI();
 ufoManager()->contentWidget()->addWidget(d->mGUI);

 d->mModelDisplay = new ModelDisplay();
 d->mGUI->mModelView->addWidget(d->mModelDisplay);
 d->mModelDisplay->show();

 connect(d->mModelDisplay, SIGNAL(signalLODChanged(float)),
		d->mGUI->mLOD, SLOT(setValue(float)));
 connect(d->mModelDisplay, SIGNAL(signalFovYChanged(float)),
		d->mGUI->mFovY, SLOT(setValue(float)));
 connect(d->mModelDisplay, SIGNAL(signalFrameChanged(float)),
		d->mGUI->mFrame, SLOT(setValue(float)));
 connect(d->mModelDisplay, SIGNAL(signalShowSelectedMeshLabel(bool)),
		d->mGUI->mSelectedMeshLabel, SLOT(setVisible(bool)));
 connect(d->mModelDisplay, SIGNAL(signalSelectedMeshLabel(const QString&)),
		d->mGUI->mSelectedMeshLabel, SLOT(setText(const QString&)));
 connect(d->mModelDisplay, SIGNAL(signalMeshUnderMouseLabel(const QString&)),
		d->mGUI->mMeshUnderMouseLabel, SLOT(setText(const QString&)));

 connect(d->mGUI->mFovY, SIGNAL(signalValueChanged(float)),
		d->mModelDisplay, SLOT(slotFovYChanged(float)));
 connect(d->mModelDisplay, SIGNAL(signalMaxLODChanged(float)),
		d->mGUI->mLOD, SLOT(slotSetMaxValue(float)));
 connect(d->mGUI->mLOD, SIGNAL(signalValueChanged(float)),
		d->mModelDisplay, SLOT(slotLODChanged(float)));
 connect(d->mModelDisplay, SIGNAL(signalMaxFramesChanged(float)),
		d->mGUI->mFrame, SLOT(slotSetMaxValue(float)));
 connect(d->mGUI->mFrame, SIGNAL(signalValueChanged(float)),
		d->mModelDisplay, SLOT(slotFrameChanged(float)));
 d->mGUI->mLOD->setRange(0.0f, 0.0f);
 d->mGUI->mFrame->setRange(0.0f, 0.0f);

 connect(d->mGUI->mHideButton, SIGNAL(signalClicked()),
		d->mModelDisplay, SLOT(slotHideSelectedMesh()));
 connect(d->mGUI->mHideOthersButton, SIGNAL(signalClicked()),
		d->mModelDisplay, SLOT(slotHideUnSelectedMeshes()));
 connect(d->mGUI->mUnhideAllButton, SIGNAL(signalClicked()),
		d->mModelDisplay, SLOT(slotUnHideAllMeshes()));



 connect(d->mGUI->mPlacement, SIGNAL(signalToggled(bool)),
		d->mModelDisplay, SLOT(slotPlacementPreviewChanged(bool)));
 connect(d->mGUI->mDisallowPlacement, SIGNAL(signalToggled(bool)),
		d->mModelDisplay, SLOT(slotDisallowPlacementChanged(bool)));
 connect(d->mGUI->mEnableWireframe, SIGNAL(signalToggled(bool)),
		d->mModelDisplay, SLOT(slotWireFrameChanged(bool)));
 connect(d->mGUI->mShowAxis, SIGNAL(signalToggled(bool)),
		d->mModelDisplay, SLOT(slotRenderAxisChanged(bool)));
 connect(d->mGUI->mShowGrid, SIGNAL(signalToggled(bool)),
		d->mModelDisplay, SLOT(slotRenderGridChanged(bool)));
 connect(d->mGUI->mEnableLight, SIGNAL(signalToggled(bool)),
		d->mModelDisplay, SLOT(slotEnableLight(bool)));
 connect(d->mGUI->mEnableMaterials, SIGNAL(signalToggled(bool)),
		d->mModelDisplay, SLOT(slotEnableMaterials(bool)));
 connect(d->mGUI->mDebugTurret, SIGNAL(signalToggled(bool)),
		this, SLOT(slotShowTurretToggled(bool)));
 connect(d->mGUI->mDebugTurretMode, SIGNAL(signalButtonActivated(BoUfoRadioButton*)),
		this, SLOT(slotChangeTurretMode(BoUfoRadioButton*)));
 connect(d->mGUI->mTurretRotation, SIGNAL(signalValueChanged(float)),
		d->mModelDisplay, SLOT(slotSetTurretRotationAngle(float)));
 connect(d->mGUI->mModelRotationZ, SIGNAL(signalValueChanged(float)),
		d->mModelDisplay, SLOT(slotSetModelRotationZ(float)));
 connect(d->mGUI->mModelRotationX, SIGNAL(signalValueChanged(float)),
		d->mModelDisplay, SLOT(slotSetModelRotationX(float)));
 connect(d->mGUI->mModelRotationY, SIGNAL(signalValueChanged(float)),
		d->mModelDisplay, SLOT(slotSetModelRotationY(float)));
 connect(d->mModelDisplay, SIGNAL(signalTurretRotation(float)),
		d->mGUI->mTurretRotation, SLOT(setValue(float)));
 connect(d->mGUI->mEditTurretProperties, SIGNAL(signalClicked()),
		this, SLOT(slotEditTurretProperties()));



#define UFO_CAMERA_WIDGET 0
#if UFO_CAMERA_WIDGET
 // AB: atm this causes problems with the camera.
 BoUfoCameraWidget* camera = new BoUfoCameraWidget();
 camera->setCamera(d->mRenderModel->camera());
 connect(this, SIGNAL(signalCameraChanged()), camera, SLOT(slotUpdateFromCamera()));
 d->mGUI->mCameraContainer->addWidget(camera);
#endif
 connect(d->mGUI->mDefaultsButton, SIGNAL(signalClicked()),
		d->mModelDisplay, SLOT(slotResetView()));

 initUfoAction();

 d->mGUI->mEnableLight->setChecked(boConfig->boolValue("UseLight", true));
 slotShowTurretToggled(false);
 d->mModelDisplay->slotResetView();
}

void BoRenderGLWidget::initUfoAction()
{
 BoUfoActionCollection::initActionCollection(ufoManager());
 BoUfoActionCollection* actionCollection = ufoManager()->actionCollection();
 BO_CHECK_NULL_RET(actionCollection);
 actionCollection->setAccelWidget(this);

 (void)new BoUfoAction(i18n("Toggle GUI"), KShortcut(Qt::CTRL + Qt::Key_G),
		this, SLOT(slotToggleShowGUI()),
		actionCollection, "options_toggle_show_gui");
 BoUfoStdAction::showMenubar(this, SLOT(slotApplyShowMenubar()), actionCollection);
 ((BoUfoToggleAction*)actionCollection->action("options_show_menubar"))->setChecked(true);

 BoUfoActionMenu* modelMenu = new BoUfoActionMenu(i18n("&Model"),
		 actionCollection, "model");
 for (Q3PtrListIterator<SpeciesTheme> it(d->mSpecies); it.current(); ++it) {
	SpeciesTheme* s = it.current();
	BoUfoActionMenu* menu = new BoUfoActionMenu(s->identifier(), actionCollection,
			QString("model_species_%1").arg(s->identifier()));
	modelMenu->insert(menu);

	BoUfoSelectAction* selectUnit = new BoUfoSelectAction(i18n("&Units"), this, SLOT(slotUnitChanged(int)), 0, 0);
	BoUfoSelectAction* selectObject = new BoUfoSelectAction(i18n("&Objects"), this, SLOT(slotUnitChanged(int)), 0, 0);
	menu->insert(selectUnit);
	menu->insert(selectObject);

	d->mAction2Species.insert(selectUnit, s);
	d->mAction2Species.insert(selectObject, s);


	connect(selectUnit, SIGNAL(signalActivated(int)),
			this, SLOT(slotUnitChanged(int)));
	connect(selectObject, SIGNAL(signalActivated(int)),
			this, SLOT(slotObjectChanged(int)));

	Q3ValueList<const UnitProperties*> units = s->allUnits();
	QStringList list;
	for (int j = 0; j < units.count(); j++) {
		list.append(units[j]->name());
	}
	selectUnit->setItems(list);
	selectObject->setItems(s->allObjects());
 }


 BoUfoToggleAction* vertexPoints = new BoUfoToggleAction(i18n("Show vertex points"),
		KShortcut(), 0, 0,
		actionCollection, "options_show_vertex_points");
 connect(vertexPoints, SIGNAL(signalToggled(bool)),
		this, SLOT(slotShowVertexPoints(bool)));
 vertexPoints->setChecked(boConfig->boolValue("ShowVertexPoints"));

 // TODO BoUfoStdAction
 (void)new BoUfoAction(i18n("Quit"), KShortcut(),
		kapp, SLOT(closeAllWindows()),
		actionCollection, "file_quit");
 (void)new BoUfoAction(i18n("Vertex point size..."), KShortcut(),
		this, SLOT(slotChangeVertexPointSize()),
		actionCollection, "options_vertex_point_size");
 (void)new BoUfoAction(i18n("Grid unit size..."), KShortcut(),
		this, SLOT(slotChangeGridUnitSize()),
		actionCollection, "options_grid_unit_size");
 (void)new BoUfoAction(i18n("Background color..."), KShortcut(),
		this, SLOT(slotChangeBackgroundColor()),
		actionCollection, "options_background_color");
 (void)new BoUfoAction(i18n("Light..."), KShortcut(),
		this, SLOT(slotShowLightWidget()),
		actionCollection, "options_light"); // AB: actually thisis NOT a setting
 (void)new BoUfoAction(i18n("Materials..."), KShortcut(),
		this, SLOT(slotShowMaterialsWidget()),
		actionCollection, "options_materials"); // AB: actually this is NOT a setting
 (void)new BoUfoAction(i18n("Font..."), KShortcut(),
		this, SLOT(slotShowChangeFont()),
		actionCollection, "options_font"); // AB: actually this is NOT a setting


 (void)new BoUfoAction(i18n("Debug &Models"), KShortcut(),
		this, SLOT(slotDebugModels()),
		actionCollection, "debug_models");
#ifdef BOSON_USE_BOMEMORY
 (void)new BoUfoAction(i18n("Debug M&emory"), KShortcut(),
		this, SLOT(slotDebugMemory()),
		actionCollection, "debug_memory");
#endif
 (void)new BoUfoAction(i18n("Debug &Species"), KShortcut(),
		this, SLOT(slotDebugSpecies()),
		actionCollection, "debug_species");
 (void)new BoUfoAction(i18n("Show OpenGL states"), KShortcut(),
		this, SLOT(slotShowGLStates()),
		actionCollection, "debug_show_opengl_states");
 (void)new BoUfoAction(i18n("&Reload model textures"), KShortcut(),
		this, SLOT(slotReloadModelTextures()),
		actionCollection, "debug_lazy_reload_model_textures");
 (void)new BoUfoAction(i18n("Reload &meshrenderer plugin"), KShortcut(),
		this, SLOT(slotReloadMeshRenderer()),
		actionCollection, "debug_lazy_reload_meshrenderer");


 if (!actionCollection->createGUI(KStandardDirs::locate("data", "boson/borenderui.rc"))) {
	boError() << k_funcinfo << "createGUI() failed" << endl;
 }
}

void BoRenderGLWidget::paintGL()
{
 QColor background = boConfig->colorValue("BoRenderBackgroundColor", Qt::black);
 glClearColor((GLfloat)background.red() / 255.0f, (GLfloat)background.green() / 255.0f, background.blue() / 255.0f, 0.0f);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 boTextureManager->clearStatistics();

 if (ufoManager()) {
	boTextureManager->invalidateCache();
	glColor3ub(255, 255, 255);

	ufoManager()->dispatchEvents();
	ufoManager()->render(false);
 }
}

void BoRenderGLWidget::slotToggleShowGUI()
{
 bool v = d->mGUI->mGUI->isVisible();
 boDebug() << k_funcinfo << !v << endl;
 slotSetGUIVisible(!v);
}

void BoRenderGLWidget::slotSetGUIVisible(bool v)
{
 d->mGUI->mGUI->setVisible(v);
}

void BoRenderGLWidget::slotApplyShowMenubar()
{
 BO_CHECK_NULL_RET(ufoManager());
 BO_CHECK_NULL_RET(ufoManager()->menuBarData());
 BoUfoActionCollection* c = ufoManager()->actionCollection();
 BO_CHECK_NULL_RET(c);
 BoUfoToggleAction* t = (BoUfoToggleAction*)c->action("options_show_menubar");
 BO_CHECK_NULL_RET(t);
 ufoManager()->menuBarData()->setVisible(t->isChecked());
}

void BoRenderGLWidget::slotSetShowMenubar(bool v)
{
 BO_CHECK_NULL_RET(ufoManager());
 BoUfoActionCollection* c = ufoManager()->actionCollection();
 BO_CHECK_NULL_RET(c);
 BoUfoToggleAction* t = (BoUfoToggleAction*)c->action("options_show_menubar");
 BO_CHECK_NULL_RET(t);
 t->setChecked(v);
}

void BoRenderGLWidget::slotShowTurretToggled(bool show)
{
 if (show) {
	d->mGUI->mEditTurretProperties->setEnabled(true);
	d->mModelDisplay->setTurretMeshesEnabled(true);
	d->mGUI->mDebugTurretDetailsWidget->setVisible(true);
 } else {
	d->mGUI->mEditTurretProperties->setEnabled(false);
	d->mModelDisplay->setTurretMeshesEnabled(false);
	d->mGUI->mDebugTurretDetailsWidget->setVisible(false);
 }
}

void BoRenderGLWidget::slotChangeTurretMode(BoUfoRadioButton* button)
{
 if (button == d->mGUI->mDebugTurretRotating) {
	d->mModelDisplay->setTurretTimerRotation(true);
 } else if (button == d->mGUI->mDebugTurretKeepPosition) {
	d->mModelDisplay->setTurretTimerRotation(false);
 } else {
	boWarning() << k_funcinfo << "unknown mode selected" << endl;
 }
}

void BoRenderGLWidget::slotEditTurretProperties()
{
 if (!d->mModelDisplay->model()) {
	KMessageBox::information(this, i18n("You need to load a model first"));
	return;
 }
 BoEditTurretPropertiesDialog* dialog = new BoEditTurretPropertiesDialog(this);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 dialog->setModelFile(d->mModelDisplay->model()->file());
 dialog->setTurretMeshes(d->mModelDisplay->turretMeshes());
 connect(dialog, SIGNAL(signalApply(BoEditTurretPropertiesDialog*)),
	this, SLOT(slotApplyTurretProperties(BoEditTurretPropertiesDialog*)));
 dialog->show();
}

void BoRenderGLWidget::slotApplyTurretProperties(BoEditTurretPropertiesDialog* dialog)
{
 BO_CHECK_NULL_RET(dialog);
 d->mModelDisplay->setTurretMeshes(dialog->turretMeshes());
 d->mModelDisplay->setTurretInitialZRotation(dialog->initialZRotation());
}

void BoRenderGLWidget::slotShowVertexPoints(bool s)
{
 boConfig->setBoolValue("ShowVertexPoints", s);
}

// TODO: prevent displaying two dialogs of the same type at once!
void BoRenderGLWidget::slotChangeVertexPointSize()
{
 unsigned int size = boConfig->uintValue("VertexPointSize");
 BoUfoInputDialog::getIntegerWidget(ufoManager(), this, SLOT(slotSetVertexPointSize(int)),
		i18n("Vertex point size (in pixels)"), i18n("Vertex point size"),

		(int)size, 0, 500, 1);
// emit signalChangeVertexPointSize(); // obsolete
}

// TODO: prevent displaying two dialogs of the same type at once!
void BoRenderGLWidget::slotChangeGridUnitSize()
{
 float size = (float)boConfig->doubleValue("GridUnitSize", 0.1);
 BoUfoInputDialog::getFloatWidget(ufoManager(), this, SLOT(slotSetGridUnitSize(float)),
		i18n("Grid unit size"), i18n("Grid unit size"),

		size, 0.0, 1.0, 0.1);
// emit signalChangeVertexPointSize(); // obsolete
}

void BoRenderGLWidget::slotChangeBackgroundColor()
{
 QColor color = boConfig->colorValue("BoRenderBackgroundColor", QColor(183, 183, 183));
 int result = KColorDialog::getColor(color, color, this);
 if (result == KColorDialog::Accepted) {
	boConfig->setColorValue("BoRenderBackgroundColor", color);
 }
}

void BoRenderGLWidget::slotDebugModels()
{
 KDialog* dialog = new KDialog(0);
 dialog->setWindowTitle(KDialog::makeStandardCaption(i18n("Debug Models")));
 dialog->setButtons(KDialog::Cancel);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 QWidget* w = dialog->mainWidget();
 Q3VBoxLayout* l = new Q3VBoxLayout(w);
 KGame3DSModelDebug* models = new KGame3DSModelDebug(w);
 l->addWidget(models);

 Q3PtrListIterator<SpeciesTheme> it(d->mSpecies);
 for (; it.current(); ++it) {
	SpeciesTheme* theme = it.current();

	// units
	Q3ValueList<const UnitProperties*> prop = it.current()->allUnits();
	Q3ValueList<const UnitProperties*>::Iterator propIt;
	for (propIt = prop.begin(); propIt != prop.end(); ++propIt) {
		QStringList fileNames = SpeciesData::unitModelFiles();
		bool found = false;
		QString file;
		for (QStringList::Iterator fit = fileNames.begin(); fit != fileNames.end(); ++fit) {
			if (KStandardDirs::exists((*propIt)->unitPath() + *fit)) {
				file = *fit;
				found = true;
				break;
			}
		}
		if (!found) {
			boError() << k_funcinfo << "Cannot find model file file for " << (*propIt)->typeId() << endl;
			continue;
		}
		file = (*propIt)->unitPath() + file;
		models->addFile(file, QString("%1/%2").arg(theme->identifier()).arg((*propIt)->name()));
	}

	// objects
	QStringList objectFiles;
	QStringList objects = theme->allObjects(&objectFiles);
	for (int i = 0; i < objects.count(); i++) {
		QString file = theme->themePath() + QString::fromLatin1("objects/") + objectFiles[i];
		models->addFile(file, objects[i]);
	}

	// add any files that we might have missed
	models->addFiles(theme->themePath());
 }
 models->slotUpdate();

 dialog->show();
}

void BoRenderGLWidget::slotDebugSpecies()
{
 KDialog* dialog = new KDialog(0);
 dialog->setWindowTitle(KDialog::makeStandardCaption(i18n("Debug Species")));
 dialog->setButtons(KDialog::Cancel);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 QWidget* w = dialog->mainWidget();
 Q3VBoxLayout* l = new Q3VBoxLayout(w);
 KGameSpeciesDebug* species = new KGameSpeciesDebug(w);
 l->addWidget(species);
 species->loadSpecies();

 dialog->show();
}

void BoRenderGLWidget::slotShowMaterialsWidget()
{
 if (!d->mMaterialWidget) {
	d->mMaterialWidget = new BoMaterialWidget(0);
 }
 d->mMaterialWidget->clearMaterials();
if (!d->mModelDisplay->model()) {
	return;
 }
 const BosonModel* model = d->mModelDisplay->model();
 for (unsigned int i = 0; i < model->materialCount(); i++) {
	d->mMaterialWidget->addMaterial(model->material(i));
 }
 d->mMaterialWidget->show();
}

void BoRenderGLWidget::slotShowLightWidget()
{
 if (!d->mLightWidget) {
	d->mLightWidget = new BoLightCameraWidget1(0, true);
	d->mLightWidget->setLight(d->mModelDisplay->light(), const_cast<QGLContext*>(context()));
 }
 d->mLightWidget->show();
}

void BoRenderGLWidget::slotReloadMeshRenderer()
{
 bool unusable = false;
 bool r = BoMeshRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << "meshrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(0, i18n("Reloading meshrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(0, i18n("Reloading meshrenderer failed but library should still be usable"));
 }
}

void BoRenderGLWidget::slotReloadModelTextures()
{
 BO_CHECK_NULL_RET(BosonModelTextures::modelTextures());
#warning TODO! (reloading textures)
 //BosonModelTextures::modelTextures()->reloadTextures();
}

void BoRenderGLWidget::slotShowGLStates()
{
 boDebug() << k_funcinfo << endl;
 BoGLStateWidget* w = new BoGLStateWidget(0);
 w->setAttribute(Qt::WA_DeleteOnClose);
 w->show();
}

void BoRenderGLWidget::slotShowChangeFont()
{
#if BOSONFONT
 BoFontInfo f;
 f = mDefaultFont->fontInfo();
 int result = BosonGLFontChooser::getFont(f, this);
 if (result == QDialog::Accepted) {
	setFont(f);
	boConfig->setStringValue("GLFont", fontInfo().toString());
 }
#endif
}

void BoRenderGLWidget::load(SpeciesTheme* s, const UnitProperties* prop)
{
 BO_CHECK_NULL_RET(s);
 BO_CHECK_NULL_RET(prop);
 boViewData->addSpeciesTheme(s);
 SpeciesData* speciesData = boViewData->speciesData(s);
 speciesData->loadUnitModel(prop, s->teamColor());
 BosonModel* model = speciesData->unitModel(prop->typeId());
 if (!model) {
	BO_NULL_ERROR(model);
 }
 d->mModelDisplay->setModel(model);
}

void BoRenderGLWidget::loadObjectModel(SpeciesTheme* s, const QString& file)
{
 BO_CHECK_NULL_RET(s);
 if (file.isEmpty()) {
	boError() << k_funcinfo << "empty filename" << endl;
	return;
 }
 boViewData->addSpeciesTheme(s);
 SpeciesData* speciesData = boViewData->speciesData(s);
 speciesData->loadObjects(s->teamColor());
 BosonModel* model = speciesData->objectModel(file);
 if (!model) {
	BO_NULL_ERROR(model);
 }
 d->mModelDisplay->setModel(model);
}

SpeciesTheme* BoRenderGLWidget::findTheme(const QString& theme) const
{
 SpeciesTheme* s = 0;
 for (Q3PtrListIterator<SpeciesTheme> it(d->mSpecies); it.current() && !s; ++it) {
	if (it.current()->identifier().lower() == theme.lower()) {
		s = it.current();
	}
 }
 return s;
}

void BoRenderGLWidget::changeUnit(SpeciesTheme* s, const UnitProperties* prop)
{
 BO_CHECK_NULL_RET(s);
 BO_CHECK_NULL_RET(prop);
 // TODO: check/uncheck the menu items!

 if (d->mMaterialWidget) {
	d->mMaterialWidget->clearMaterials();
	d->mMaterialWidget->hide();
 }
 load(s, prop);
}

void BoRenderGLWidget::changeObject(SpeciesTheme* s, const QString& objectModel)
{
 BO_CHECK_NULL_RET(s);
 if (d->mMaterialWidget) {
	d->mMaterialWidget->clearMaterials();
	d->mMaterialWidget->hide();
 }
 loadObjectModel(s, objectModel);
}

void BoRenderGLWidget::changeUnit(const QString& theme, const QString& unit)
{
 SpeciesTheme* s = 0;
 const UnitProperties* prop = 0;

 s = findTheme(theme);
 if (!s) {
	boError() << k_funcinfo << "Could not find theme " << theme << endl;
	return;
 }
 Q3ValueList<const UnitProperties*> units = s->allUnits();
 Q3ValueList<const UnitProperties*>::Iterator i = units.begin();
 for (; i != units.end() && !prop; ++i) {
	const UnitProperties* p = *i;
	// warning: this might cause trouble once we start translations! but we
	// also have a way to use IDs directly so this is just a minor problem
	if (p->name().lower() == unit.lower()) {
		prop = p;
	}
 }
 if (!prop) {
	boError() << "Could not find unit " << unit << endl;
 }
 changeUnit(s, prop);
}

void BoRenderGLWidget::changeUnit(const QString& theme, unsigned long int type)
{
 SpeciesTheme* s = 0;
 for (Q3PtrListIterator<SpeciesTheme> it(d->mSpecies); it.current() && !s; ++it) {
	if (it.current()->identifier().lower() == theme.lower()) {
		s = it.current();
	}
 }
 if (!s) {
	boError() << k_funcinfo << "Could not find theme " << theme << endl;
	return;
 }
 const UnitProperties* prop = s->unitProperties(type);
 if (!prop) {
	boError() << k_funcinfo << "Could not find unit " << type << endl;
	return;
 }
 changeUnit(s, prop);
}

void BoRenderGLWidget::changeObject(const QString& theme, const QString& object)
{
 SpeciesTheme* s = 0;

 s = findTheme(theme);
 if (!s) {
	boError() << k_funcinfo << "Could not find theme " << theme << endl;
	return;
 }
 changeObject(s, object);
}

void BoRenderGLWidget::slotSetGridUnitSize(float size)
{
 boConfig->setDoubleValue("GridUnitSize", size);
}

void BoRenderGLWidget::uncheckAllBut(BoUfoAction* action)
{
 if (!action || !action->isA("BoUfoSelectAction")) {
	boError() << k_funcinfo << "not a valid BoUfoSelectAction" << endl;
	return;
 }
 Q3PtrDictIterator<SpeciesTheme> it(d->mAction2Species);
 for (; it.current(); ++it) {
	BoUfoAction* a = (BoUfoAction*)it.currentKey();
	if (a == action) {
		continue;
	}
	if (!a->isA("BoUfoSelectAction")) {
		continue;
	}
	((BoUfoSelectAction*)a)->setCurrentItem(-1);
 }
}

void BoRenderGLWidget::slotUnitChanged(int index)
{
 if (!sender() || !sender()->inherits("BoUfoAction")) {
	boError() << k_funcinfo << "sender() must inherit BoUfoAction" << endl;
	return;
 }
 BoUfoAction* p = (BoUfoAction*)sender();
 uncheckAllBut(p);
 SpeciesTheme* s = d->mAction2Species[p];
 BO_CHECK_NULL_RET(s);

 Q3ValueList<const UnitProperties*> props = s->allUnits();
 if (index >= (int)props.count()) {
	boError() << k_funcinfo << "index " << index << " out of range" << endl;
	return;
 }
 unsigned long int type = props[index]->typeId();
 const UnitProperties* prop = s->unitProperties(type);
 if (!prop) {
	boError() << k_funcinfo << "could not find unitproperties for index=" << index << " type=" << type << endl;
	return;
 }

 changeUnit(s, prop);
}

void BoRenderGLWidget::slotObjectChanged(int index)
{
 if (!sender() || !sender()->inherits("BoUfoAction")) {
	boError() << k_funcinfo << "sender() must inherit BoUfoAction" << endl;
	return;
 }
 BoUfoAction* p = (BoUfoAction*)sender();
 uncheckAllBut(p);
 SpeciesTheme* s = d->mAction2Species[p];
 BO_CHECK_NULL_RET(s);

 QString object = s->allObjects()[index];
 if (object.isEmpty()) {
	boError() << k_funcinfo << "Can't find " << object << " (==" << index << ")" << endl;
	return;
 }
 boDebug() << k_funcinfo << object << endl;
 changeObject(s, object);
}







class ModelDisplayPrivate
{
public:
	ModelDisplayPrivate()
	{
		mRenderModel = 0;
	}
	BoRenderRenderModel* mRenderModel;
};

ModelDisplay::ModelDisplay()
	: BoUfoCustomWidget()
{
 d = new ModelDisplayPrivate();
 d->mRenderModel = 0;

 qApp->setGlobalMouseTracking(true);

 mMeshUnderMouse = -1;
 mSelectedMesh = -1;
 mViewData = new BosonViewData(this);
 BosonViewData::setGlobalViewData(mViewData);


 mMouseMoveDiff = new BoMouseMoveDiff;

 mRenderAxis = false;
 mRenderGrid = false;

#if BOSONFONT
 mDefaultFont = 0;
#endif

 mFovY = 60.0f;

 boConfig->addDynamicEntry(new BoConfigBoolEntry(boConfig, "ShowVertexPoints", true));
 boConfig->addDynamicEntry(new BoConfigUIntEntry(boConfig, "VertexPointSize", 3));
 boConfig->addDynamicEntry(new BoConfigColorEntry(boConfig, "BoRenderBackgroundColor", QColor(183, 183, 183)));
 boConfig->addDynamicEntry(new BoConfigDoubleEntry(boConfig, "GridUnitSize", 0.1));

 connect(this, SIGNAL(signalMousePressed(QMouseEvent*)),
		this, SLOT(slotMousePressEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseMoved(QMouseEvent*)),
		this, SLOT(slotMouseMoveEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseDragged(QMouseEvent*)),
		this, SLOT(slotMouseMoveEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseReleased(QMouseEvent*)),
		this, SLOT(slotMouseReleaseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseWheel(QWheelEvent*)),
		this, SLOT(slotWheelEvent(QWheelEvent*)));

 setMouseEventsEnabled(true, true);

 initializeGL();
}

ModelDisplay::~ModelDisplay()
{
 delete mViewData;
 qApp->setGlobalMouseTracking(false);
 BoMeshRendererManager::manager()->unsetCurrentRenderer();
 delete mMouseMoveDiff;
 delete d->mRenderModel;
 delete d;
 BoTextureManager::deleteStatic();
}

void ModelDisplay::initializeGL()
{
#if BOSONFONT
 delete mDefaultFont;
 BoFontInfo defaultFontInfo;
 defaultFontInfo.fromString(boConfig->stringValue("GLFont", QString::null));
 mDefaultFont = new BosonGLFont(defaultFontInfo);
#endif

 if (d->mRenderModel) {
	return;
 }
 d->mRenderModel = new BoRenderRenderModel(this);
 connect(d->mRenderModel, SIGNAL(signalMaxFramesChanged(float)),
		this, SIGNAL(signalMaxFramesChanged(float)));
 connect(d->mRenderModel, SIGNAL(signalMaxLODChanged(float)),
		this, SIGNAL(signalMaxLODChanged(float)));
 connect(d->mRenderModel, SIGNAL(signalFrameChanged(float)),
		this, SIGNAL(signalFrameChanged(float)));
 connect(d->mRenderModel, SIGNAL(signalLODChanged(float)),
		this, SIGNAL(signalLODChanged(float)));
 connect(d->mRenderModel, SIGNAL(signalCameraChanged()),
		this, SIGNAL(signalCameraChanged()));
 connect(d->mRenderModel, SIGNAL(signalResetModel()),
		this, SLOT(slotResetModel()));
 connect(d->mRenderModel, SIGNAL(signalTurretRotation(float)),
		this, SIGNAL(signalTurretRotation(float)));
}

const BosonModel* ModelDisplay::model() const
{
 return d->mRenderModel->model();
}

BoCamera* ModelDisplay::camera() const
{
 return d->mRenderModel->camera();
}

BoLight* ModelDisplay::light() const
{
 return d->mRenderModel->light();
}

void ModelDisplay::setTurretMeshes(const QStringList& meshes)
{
 d->mRenderModel->setTurretMeshes(meshes);
}

const QStringList& ModelDisplay::turretMeshes() const
{
 return d->mRenderModel->turretMeshes();
}

void ModelDisplay::setTurretMeshesEnabled(bool e)
{
 d->mRenderModel->setTurretMeshesEnabled(e);
}

void ModelDisplay::setTurretInitialZRotation(float r)
{
 d->mRenderModel->setTurretInitialZRotation(r);
}

void ModelDisplay::setTurretTimerRotation(bool timer)
{
 d->mRenderModel->setTurretTimerRotation(timer);
}

void ModelDisplay::slotSetTurretRotationAngle(float rot)
{
 d->mRenderModel->slotSetTurretRotationAngle(rot);
}

void ModelDisplay::slotSetModelRotationZ(float rot)
{
 d->mRenderModel->slotSetModelRotationZ(rot);
}

void ModelDisplay::slotSetModelRotationX(float rot)
{
 d->mRenderModel->slotSetModelRotationX(rot);
}

void ModelDisplay::slotSetModelRotationY(float rot)
{
 d->mRenderModel->slotSetModelRotationY(rot);
}

void ModelDisplay::setFont(const BoFontInfo& font)
{
#if BOSONFONT
 delete mDefaultFont;
 mDefaultFont = new BosonGLFont(font);
#endif
}

void ModelDisplay::slotPlacementPreviewChanged(bool on)
{
 d->mRenderModel->slotPlacementPreviewChanged(on);
}

void ModelDisplay::slotDisallowPlacementChanged(bool on)
{
 d->mRenderModel->slotDisallowPlacementChanged(on);
}

void ModelDisplay::slotWireFrameChanged(bool on)
{
 d->mRenderModel->slotWireFrameChanged(on);
}

void ModelDisplay::paintWidget()
{
 BO_CHECK_NULL_RET(camera());

 glPushAttrib(GL_ALL_ATTRIB_BITS);

 glViewport(0, 0, width(), height());

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluPerspective(mFovY, (float)width() / (float)height(), NEAR, FAR);
 glMatrixMode(GL_MODELVIEW);
 glColor3ub(255, 255, 255);


 glPushMatrix();
 glLoadIdentity();
 camera()->applyCameraToScene();
 if (mRenderGrid) {
	renderGrid();
 }
 if (mRenderAxis) {
	renderAxii();
 }
 glPopMatrix();

 boTextureManager->invalidateCache();
 d->mRenderModel->render();

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glPopAttrib();
}

void ModelDisplay::renderAxii()
{
 glPushAttrib(GL_ENABLE_BIT);
 glDisable(GL_TEXTURE_2D);
 glDisable(GL_BLEND);
 glColor3ub(255, 0, 0);

 // FIXME: we _cannot_ use really high numbers (has strange effects on the lines
 // - if you make the numbers high enough, they even won't appear at all)
 // i guess we should get the size of the bounding box of the model, add a
 // certain value to that and use that size as maximum.
 // -> the axii would be big enough for the model then
 // 10.0 is high enough for us atm, as we scale every model to 1.0 anyway.
 float max = 10.0f;
 glBegin(GL_LINES);
	glVertex3f(-max, 0.0f, 0.0f);
	glVertex3f(max, 0.0f, 0.0f);

	glVertex3f(0.0f, -max, 0.0f);
	glVertex3f(0.0f, max, 0.0f);

	glVertex3f(0.0f, 0.0f, -max);
	glVertex3f(0.0f, 0.0f, max);

	const float offset = 1.1f;
	const float distance = 0.1f; // distance of the letter to the line

	// "X"
	glVertex3f(offset, distance, 0.0f);
	glVertex3f(offset + 0.666f, distance + 1.0f, 0.0f);
	glVertex3f(offset, distance + 1.0f, 0.0f);
	glVertex3f(offset + 0.666f, distance, 0.0f);

	// "Y"
	glVertex3f(distance, offset + 1.0f, 0.0f);
	glVertex3f(distance + 0.5f, offset + 0.5f, 0.0f);
	glVertex3f(distance + 0.5f, offset + 0.5f, 0.0f);
	glVertex3f(distance + 1.0f, offset + 1.0f, 0.0f);
	glVertex3f(distance + 0.5f, offset + 0.5f, 0.0f);
	glVertex3f(distance + 0.5f, offset, 0.0f);

	// "Z"
	glVertex3f(0.0f, distance + 1.0f, -offset);
	glVertex3f(0.0f, distance + 1.0f, -offset - 1.0f);
	glVertex3f(0.0f, distance + 1.0f, -offset - 1.0f);
	glVertex3f(0.0f, distance, -offset);
	glVertex3f(0.0f, distance, -offset);
	glVertex3f(0.0f, distance, -offset - 1.0f);
 glEnd();

 glColor3ub(255, 255, 255);
 glPopAttrib();
}

void ModelDisplay::renderGrid()
{
#warning FIXME!!!
#if 0
 glPushAttrib(GL_ENABLE_BIT);
 glDisable(GL_TEXTURE_2D);
 glEnable(GL_DEPTH_TEST);
 float size = 2.0f;
 if (mModel) {
	size = fmaxf(mModel->width(), mModel->height());
 }
 size = ceilf(size);
 glColor3ub(0, 255, 0);
 const float unit = (float)boConfig->doubleValue("GridUnitSize");

 // the XY plane
 int lines = (int)((2 * size) / unit);
 // fix rounding errors...
 if (unit * lines < 2 * size) {
	lines++;
 }
 glBegin(GL_LINES);
	// AB: we dont use for (float x = -size; x <= size; x +=unit) due to
	// rounding errors
	for (int i = 0; i <= lines; i++) {
		float x = -size + i * unit;
		glVertex3f(x, -size, 0.0f);
		glVertex3f(x, size, 0.0f);

		float y = -size + i * unit;
		glVertex3f(-size, y, 0.0f);
		glVertex3f(size, y, 0.0f);
	}
 glEnd();


 glColor3ub(255, 255, 255);
 glPopAttrib();
#endif
}

void ModelDisplay::setModel(BosonModel* model)
{
 boDebug() << k_funcinfo << endl;
 d->mRenderModel->setModel(model);
}


void ModelDisplay::slotDebugMemory()
{
#ifdef BOSON_USE_BOMEMORY
 boDebug() << k_funcinfo << endl;
 BoMemoryDialog* dialog = new BoMemoryDialog(this);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 boDebug() << k_funcinfo << "update data" << endl;
 dialog->slotUpdate();
 dialog->show();
 boDebug() << k_funcinfo << "done" << endl;
#endif
}




void ModelDisplay::slotSetVertexPointSize(int size)
{
 boDebug() << k_funcinfo << size << endl;
 boConfig->setUIntValue("VertexPointSize", size);
}

void ModelDisplay::slotResetModel()
{
 mMeshUnderMouse = -1;
 mSelectedMesh = -1;
 updateMeshUnderMouse();
}

void ModelDisplay::updateMeshUnderMouse()
{
 QString meshUnderCursor;
 QString selectedMeshText;
 BoMesh* mesh = 0;
 if (mMeshUnderMouse >= 0) {
	mesh = d->mRenderModel->meshWithIndex(mMeshUnderMouse);
	if (mesh) {
		meshUnderCursor = mesh->name();
	}
 }
 if (meshUnderCursor.isNull()) {
	meshUnderCursor = i18n("(no mesh under cursor)");
 }
 if (mSelectedMesh >= 0) {
	mesh = d->mRenderModel->meshWithIndex(mSelectedMesh);
	if (mesh) {
		QString name = mesh->name();
		QString material;
		if (mesh->material()) {
			material = mesh->material()->name();
		} else {
			material = i18n("(None)");
		}
		selectedMeshText = i18n("Selected mesh: %1 points: %2 material: %3",
				 name,
				 mesh->pointCount(),
				 material);
	}
 }
 emit signalMeshUnderMouseLabel(i18n("Mesh under cursor: %1", meshUnderCursor));
 if (selectedMeshText.isEmpty()) {
	emit signalShowSelectedMeshLabel(false);
	emit signalSelectedMeshLabel("");
 } else {
	emit signalShowSelectedMeshLabel(true);
	emit signalSelectedMeshLabel(selectedMeshText);
 }
}

void ModelDisplay::updateCursorDisplay(const QPoint& pos)
{
 mMeshUnderMouse = -1;
 if (!d->mRenderModel->haveModel()) {
	return;
 }
 int picked = d->mRenderModel->pickObject(pos, mFovY, NEAR, FAR);
 mMeshUnderMouse = picked;
 updateMeshUnderMouse();
}

void ModelDisplay::slotMousePressEvent(QMouseEvent* e)
{
 if (e->isAccepted()) {
	return;
 }
 e->accept();
 switch (e->button()) {
	case Qt::LeftButton:
		break;
	case Qt::RightButton:
		mMouseMoveDiff->moveEvent(e->pos());
		break;
	default:
		break;
 }
}

void ModelDisplay::slotMouseReleaseEvent(QMouseEvent* e)
{
 if (e->isAccepted()) {
	return;
 }
 e->accept();
 switch (e->button()) {
	case Qt::LeftButton:
		selectMesh(mMeshUnderMouse);
		break;
	case Qt::RightButton:
		break;
	default:
		break;
 }
}

void ModelDisplay::selectMesh(int mesh)
{
 d->mRenderModel->setSelectedMesh(mesh);
 mSelectedMesh = mesh;
 updateMeshUnderMouse();
 emit signalMeshSelected(mesh);
}

void ModelDisplay::slotMouseMoveEvent(QMouseEvent* e)
{
 updateCursorDisplay(e->pos());

 if (e->isAccepted()) {
	return;
 }
 e->accept();
 mMouseMoveDiff->moveEvent(e);


 if (e->state() & Qt::LeftButton) {
 } else if (e->state() & Qt::RightButton) {
	// dx == rot. around x axis. this is equal to d_y_ mouse movement.
	int dx = mMouseMoveDiff->dy();
	int dy = mMouseMoveDiff->dx(); // rotation around y axis

	BoQuaternion q = camera()->quaternion();
	BoVector3Float cameraPos;
	q.transform(&cameraPos, &camera()->cameraPos());

	BoQuaternion q2;
	q2.setRotation(dx, dy, 0);

	// AB: we want a rotation around x/y axii in the global, fixed
	// coordinate system, whereas OpenGL would usually do rotations around
	// the local (to the objects) coordinate systems. Remember: you can
	// think about coordinate systems as "fixed", if you simple inverse the
	// order of all OpenGL operations (B,A instead of A,B).
	// So we simply need to _pre_multiply the rotation diff, instead of post
	// multiply.
	q2.multiply(q);

	// AB: usually q2 should be normalized already, but sometimes we have
	// pretty big rounding errors (2nd and 3rd digit after the decimal point
	// are pretty usual already).
	// so we re-normalize here, to avoid trouble
	q2.normalize();

	BoQuaternion inv = q2.conjugate(); // equal to inverse() (as q2 is normalized) but faster
	inv.transform(&cameraPos, &cameraPos);

	d->mRenderModel->updateCamera(cameraPos, q2);
 }
}

void ModelDisplay::slotWheelEvent(QWheelEvent* e)
{
 BO_CHECK_NULL_RET(camera());
 if (e->isAccepted()) {
	return;
 }
 e->accept();
 float delta = e->delta() / 120;
 if (e->orientation() == Qt::Horizontal) {
 } else {
 }
 BoVector3Float lookAt = camera()->lookAt();
 BoVector3Float eye = camera()->cameraPos();
 BoVector3Float up = camera()->up();

 BoVector3Float orientation = eye - lookAt;
 orientation.normalize();
 if (orientation.isNull()) {
	// FIXME: this should be disallowed!
	orientation = BoVector3Float(1.0f, 0.0f, 0.0f);
 }
 // TODO: we should check whether all meshes are still in front of the NEAR plane!

 eye.add(orientation * -delta);
 lookAt.add(orientation * -delta);
 d->mRenderModel->updateCamera(eye, lookAt, up);
}

void ModelDisplay::slotFovYChanged(float f)
{
 mFovY = f;
}

void ModelDisplay::slotFrameChanged(int f)
{
 d->mRenderModel->slotFrameChanged(f);
}

void ModelDisplay::slotLODChanged(int l)
{
 d->mRenderModel->slotLODChanged(l);
}

void ModelDisplay::slotHideSelectedMesh()
{
 d->mRenderModel->slotHideSelectedMesh();
}

void ModelDisplay::slotHideUnSelectedMeshes()
{
 d->mRenderModel->slotHideUnSelectedMeshes();
}

void ModelDisplay::slotUnHideAllMeshes()
{
 d->mRenderModel->slotUnHideAllMeshes();
}

void ModelDisplay::slotEnableLight(bool e)
{
 boConfig->setBoolValue("UseLight", e);
}

void ModelDisplay::slotEnableMaterials(bool e)
{
 boConfig->setBoolValue("UseMaterials", e);
}

void ModelDisplay::slotResetView()
{
 BoVector3Float cameraPos(0.0f, 0.0f, 2.0f);
 BoVector3Float lookAt(0.0f, 0.0f, 0.0f);
 BoVector3Float up(0.0f, 50.0f, 0.0f);
 d->mRenderModel->updateCamera(cameraPos, lookAt, up);
 emit signalFovYChanged(60.0f);
}

void ModelDisplay::updateCamera(const BoVector3Float& cameraPos, const BoQuaternion& q)
{
 d->mRenderModel->updateCamera(cameraPos, q);
}

void ModelDisplay::updateCamera(const BoVector3Float& cameraPos, const BoMatrix& rotationMatrix)
{
 d->mRenderModel->updateCamera(cameraPos, rotationMatrix);
}

void ModelDisplay::updateCamera(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up)
{
 d->mRenderModel->updateCamera(cameraPos, lookAt, up);
}




RenderMain::RenderMain() : KMainWindow()
{
 QWidget* w = new QWidget(this);
 Q3HBoxLayout* layout = new Q3HBoxLayout(w);

 mGLWidget = new BoRenderGLWidget(w, boConfig->boolValue("ForceWantDirect", true));
 mGLWidget->initWidget();
 mGLWidget->setMinimumSize(400, 400);
 layout->addWidget(mGLWidget, 1);
 mGLWidget->setMouseTracking(true);
 mGLWidget->setFocusPolicy(Qt::StrongFocus);

 setCentralWidget(w);

 mUpdateTimer = new QTimer(this);
 connect(mUpdateTimer, SIGNAL(timeout()), mGLWidget, SLOT(updateGL()));
 mUpdateTimer->start(GL_UPDATE_TIMER);
}

RenderMain::~RenderMain()
{
 boConfig->save(false);
 delete mUpdateTimer;
}

bool RenderMain::parseCmdLineArgs(KCmdLineArgs* args)
{
 if (!mGLWidget->parseCmdLineArgs(args)) {
	return false;
 }
 if (args->isSet("pixmap")) {
	// AB: lights are not reloaded by initializeGL(), so we need to disable
	// it .. for now
	boConfig->setBoolValue("UseLight", false);

	mGLWidget->slotSetGUIVisible(false);
	mGLWidget->slotSetShowMenubar(false);
	BoPixmapRenderer r;
	r.setWidget(mGLWidget);
	QPixmap p = r.getPixmap();
	mGLWidget->slotSetShowMenubar(true);
	mGLWidget->slotSetGUIVisible(true);

	if (p.isNull()) {
		boError() << k_funcinfo << "rendering to pixmap failed" << endl;
	}
	QString file = args->getOption("pixmap");
	if (p.save(file, "JPEG")) {
		boDebug() << k_funcinfo << "saving pixmap to " << file << endl;
	} else {
		boError() << k_funcinfo << "saving pixmap to " << file << " failed" << endl;
	}
 }
 return true;
}


int main(int argc, char **argv)
{
 KAboutData about("borender",
		QByteArray(),
		ki18n("Boson Rendering tester"),
		version,
		description,
		KAboutData::License_GPL,
		ki18n("(C) 2002-2005 The Boson team"),
		KLocalizedString(),
		"http://boson.eu.org");
 about.addAuthor( ki18n("Andreas Beckermann"), ki18n("Coding & Current Maintainer"), "b_mann@gmx.de" );

 KCmdLineOptions options;
 options.add("m");
 options.add("maximized", ki18n("Show maximized"));
 options.add("s");
 options.add("species <identifier>", ki18n("Species Theme identifier"));
 options.add("u");
 options.add("unit <unit>", ki18n("Unit"));
 options.add("i");
 options.add("unit-id <typeid>", ki18n("Unit Type ID"));
 options.add("o");
 options.add("object <file>", ki18n("Object"));
 options.add("cx");
 options.add("camera-x <number>", ki18n("X-Position of the camera"));
 options.add("cy");
 options.add("camera-y <number>", ki18n("Y-Position of the camera"));
 options.add("cz");
 options.add("camera-z <number>", ki18n("Z-Position of the camera"));
 options.add("rx");
 options.add("rotate-x <number>", ki18n("Rotation around the X-axis"));
 options.add("ry");
 options.add("rotate-y <number>", ki18n("Rotation around the Y-axis"));
 options.add("rz");
 options.add("rotate-z <number>", ki18n("Rotation around the Z-axis"));
 options.add("lookAtCenter", ki18n("Rotate the camera so, that it looks at the center, i.e. (0,0,0)"));
 options.add("fovy <number>", ki18n("Field of view (zooming)"), "60.0");
 options.add("f");
 options.add("frame <number>", ki18n("Initially displayed frame"));
 options.add("l");
 options.add("lod <number>", ki18n("Initially displayed LOD"));
 options.add("indirect", ki18n("Use Indirect rendering (sloooow!!). debugging only."));
 options.add("pixmap <filename>", ki18n("Render to pixmap."));

 // we need to do extra stuff after BosonConfig's initialization
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 Q3CString argv0(argv[0]);
 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 BoApplication app(argv0);
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 if (args->isSet("indirect") || args->isSet("pixmap")) {
	boWarning() << k_funcinfo << "use indirect rendering (slow!)" << endl;
	boConfig->setBoolValue("ForceWantDirect", false);
 }

 RenderMain* main = new RenderMain();
 kapp->setMainWidget(main);
 main->show();
 bool renderToPixmap = false;
 if (args->isSet("pixmap")) {
	boDebug() << k_funcinfo << "rendering to pixmap" << endl;
	main->hide();
	renderToPixmap = true;
 }
 if (args->isSet("maximized")) {
	main->showMaximized();
 }

 if (!main->parseCmdLineArgs(args)) {
	return 1;
 }

 args->clear();

 if (renderToPixmap) {
	return 0;
 }
 return app.exec();
}


void postBosonConfigInit()
{
 boConfig->setBoolValue("ForceDisableSound", true);
}
