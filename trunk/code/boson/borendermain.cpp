/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "borendermain.h"
#include "borendermain.moc"

#include "speciestheme.h"
#include "bosonconfig.h"
#include "bosonmodel.h"
#include "bosonprofiling.h"
#include "unitproperties.h"
#include "kgamemodeldebug.h"
#include "bodebug.h"
#include "sound/bosonmusic.h"
#include "boversion.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kdialogbase.h>

#include <qtimer.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <GL/glu.h>

#define NEAR 1.0
#define FAR 100.0

#define GL_UPDATE_TIMER 50 // sufficient for borender

static const char *description =
    I18N_NOOP("Rendering tester for Boson");

static const char *version = BOSON_VERSION_STRING;

static KCmdLineOptions options[] =
{
    { "m", 0, 0 },
    { "maximized", I18N_NOOP("Show maximized"), 0 },
    { "s", 0, 0 },
    { "species <identifier>", I18N_NOOP("Species Theme identifier"), 0 },
    { "u", 0, 0 },
    { "unit <unit>", I18N_NOOP("Unit"), 0 },
    { "i", 0, 0 },
    { "unit-id <typeid>", I18N_NOOP("Unit Type ID"), 0 },
    { "o", 0, 0 },
    { "object <file>", I18N_NOOP("Object"), 0 },
    { "cx", 0, 0 },
    { "camera-x <number>", I18N_NOOP("X-Position of the camera"), 0 },
    { "cy", 0, 0 },
    { "camera-y <number>", I18N_NOOP("Y-Position of the camera"), 0 },
    { "cz", 0, 0 },
    { "camera-z <number>", I18N_NOOP("Z-Position of the camera"), 0 },
    { "rx", 0, 0 },
    { "rotate-x <number>", I18N_NOOP("Rotation around the X-axis"), 0 },
    { "ry", 0, 0 },
    { "rotate-y <number>", I18N_NOOP("Rotation around the Y-axis"), 0 },
    { "rz", 0, 0 },
    { "rotate-z <number>", I18N_NOOP("Rotation around the Z-axis"), 0 },
    { "fovy <number>", I18N_NOOP("Field of view (zooming)"), "60.0" },
    { "f", 0, 0 },
    { "frame <number>", I18N_NOOP("Initially displayed frame"), 0 },
    { 0, 0, 0 }
};


ModelPreview::ModelPreview(QWidget* parent) : BosonGLWidget(parent)
{
 mUpdateTimer = new QTimer(this);
 connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateGL()));

 mCurrentFrame = 0;
 mModel = 0;

 mMouseDiffX = 0;
 mMouseDiffY = 0;

 mRotateX = mRotateY = mRotateZ = 0.0;
 mPlacementPreview = false;
 mDisallowPlacement = false;

 connect(this, SIGNAL(signalRotateXChanged(float)), this, SLOT(slotRotateXChanged(float)));
 connect(this, SIGNAL(signalRotateYChanged(float)), this, SLOT(slotRotateYChanged(float)));
 connect(this, SIGNAL(signalRotateZChanged(float)), this, SLOT(slotRotateZChanged(float)));
 connect(this, SIGNAL(signalCameraXChanged(float)), this, SLOT(slotCameraXChanged(float)));
 connect(this, SIGNAL(signalCameraYChanged(float)), this, SLOT(slotCameraYChanged(float)));
 connect(this, SIGNAL(signalCameraZChanged(float)), this, SLOT(slotCameraZChanged(float)));
 connect(this, SIGNAL(signalFovYChanged(float)), this, SLOT(slotFovYChanged(float)));
 connect(this, SIGNAL(signalFrameChanged(int)), this, SLOT(slotFrameChanged(int)));

 setMinimumSize(200, 200);
}

ModelPreview::~ModelPreview()
{
 resetModel();
 delete mUpdateTimer;
}

void ModelPreview::initializeGL()
{
 makeCurrent();
 glClearColor(0.0, 0.0, 0.0, 0.0);
 glShadeModel(GL_FLAT);
 glDisable(GL_DITHER);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 setUpdatesEnabled(false);
 mUpdateTimer->start(GL_UPDATE_TIMER);
 slotResetView();
}

void ModelPreview::resizeGL(int w, int h)
{
 makeCurrent();
 glViewport(0, 0, w, h);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluPerspective(mFovY, (float)w / (float)h, NEAR, FAR);
 glMatrixMode(GL_MODELVIEW);
}

void ModelPreview::paintGL()
{
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();

 glTranslatef(-mCameraX, -mCameraY, -mCameraZ);
 glRotatef(mRotateX, 1.0, 0.0, 0.0);
 glRotatef(mRotateY, 0.0, 1.0, 0.0);
 glRotatef(mRotateZ, 0.0, 0.0, 1.0);

 // AB: try to keep this basically similar to BosonBigDisplay::paintGL()
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 glColor3f(1.0, 1.0, 1.0);

 /*
 const float infinity = 10000; // damn how long should it be? is there a clean way in opengl for this?
 glBegin(GL_LINES);
 for (float x = 0; x < 10; x = x + 0.1) {
	glVertex3f(-x, -infinity, 0.0f);
	glVertex3f(-x, infinity, 0.0f);
	glVertex3f(x, -infinity, 0.0f);
	glVertex3f(x, infinity, 0.0f);
 }
 glEnd();
 */

 glEnable(GL_TEXTURE_2D);
 glEnable(GL_DEPTH_TEST);
 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#warning FIXME?
 // AB: fi these are enabled we can't use triangle strips by any reason.
// glEnable(GL_CULL_FACE);
// glCullFace(GL_BACK);

 if (mModel && mCurrentFrame >= 0) {
	BoFrame* f = mModel->frame(mCurrentFrame);
	if (f) {
		if (mPlacementPreview) {
			glEnable(GL_BLEND);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GLubyte r, g, b, a;
			a = PLACEMENTPREVIEW_ALPHA;
			r = 255;
			if (mDisallowPlacement) {
				g = PLACEMENTPREVIEW_DISALLOW_COLOR;
				b = PLACEMENTPREVIEW_DISALLOW_COLOR;
			} else {
				g = 255;
				b = 255;
			}
			glColor4ub(r, g, b, a);
		}

		// FIXME: this isn't good here...
		mModel->enablePointer();

		f->renderFrame(0);
		if (mPlacementPreview) {
			// AB: do not reset the actual color - if it will get
			// used it will be set again anyway.
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glColor4ub(255, 255, 255, 255);
		}
	} else {
		boError() << k_funcinfo << "NULL frame" << endl;
	}
 }

 glDisableClientState(GL_VERTEX_ARRAY);
 glDisableClientState(GL_TEXTURE_COORD_ARRAY);
 glDisable(GL_CULL_FACE);
 glDisable(GL_TEXTURE_2D);
 glDisable(GL_DEPTH_TEST);
 GLenum e = glGetError();
 if (e != GL_NO_ERROR) {
	boError() << k_funcinfo << "OpenGL error: " << (int)e << endl;
 }
}

void ModelPreview::load(SpeciesTheme* s, const UnitProperties* prop)
{
 resetModel();
 BO_CHECK_NULL_RET(s);
 BO_CHECK_NULL_RET(prop);
 s->loadUnitModel(prop);
 mModel = s->unitModel(prop->typeId());
 BO_CHECK_NULL_RET(mModel);
 emit signalMaxFramesChanged(mModel->frames() - 1);
}

void ModelPreview::loadObjectModel(SpeciesTheme* s, const QString& file)
{
 resetModel();
 BO_CHECK_NULL_RET(s);
 if (file.isEmpty()) {
	boError() << k_funcinfo << "empty filename" << endl;
	return;
 }
 s->loadObjects();
 mModel = s->objectModel(file);
 if (!mModel) {
	boError() << k_funcinfo << "NULL model" << endl;
	return;
 }
 emit signalMaxFramesChanged(mModel->frames() - 1);
}

void ModelPreview::slotResetView()
{
 emit signalFovYChanged(60.0);
 emit signalCameraXChanged(0.0);
 emit signalCameraYChanged(0.0);
 emit signalCameraZChanged(3.0);
 emit signalRotateXChanged(0.0);
 emit signalRotateYChanged(0.0);
 emit signalRotateZChanged(0.0);
 resizeGL(width(), height());
}

void ModelPreview::resetModel()
{
 mModel = 0;
 mCurrentFrame = 0;
}

void ModelPreview::mousePressEvent(QMouseEvent* e)
{
 delete mMouseDiffX;
 delete mMouseDiffY;
 mMouseDiffX = new int;
 mMouseDiffY = new int;
 *mMouseDiffX = e->x();
 *mMouseDiffY = e->y();
}

void ModelPreview::mouseReleaseEvent(QMouseEvent* )
{
 delete mMouseDiffX;
 delete mMouseDiffY;
 mMouseDiffX = 0;
 mMouseDiffY = 0;
}

void ModelPreview::mouseMoveEvent(QMouseEvent* e)
{
 if (!mMouseDiffX || !mMouseDiffY) {
	return;
 }
 if (e->state() & LeftButton) {
	if (e->state() & ControlButton) {
		int dx = e->x() - *mMouseDiffX;
		int dy = e->y() - *mMouseDiffY;
		emit signalRotateXChanged(mRotateX + (float)(dx) / 2);
		emit signalRotateYChanged(mRotateY + (float)(dy) / 2);
	} else {
	}
 }
}

void ModelPreview::slotFrameChanged(int f)
{
 if (f != 0) {
	if (!mModel || f < 0) {
		emit signalFrameChanged(0);
		return;
	}
	if ((unsigned int)f >= mModel->frames()) {
		emit signalFrameChanged(mModel->frames() - 1);
		return;
	}
  }
 mCurrentFrame = f;
}


RenderMain::RenderMain()
{
 mSpecies.setAutoDelete(true);
 QStringList list = SpeciesTheme::availableSpecies();
 for (unsigned int i = 0; i < list.count(); i++) {
	QString dir = list[i];
	dir = dir.left(dir.length() - QString("index.species").length());
	SpeciesTheme* s = new SpeciesTheme(dir, QColor());
	mSpecies.append(s);
	s->readUnitConfigs(false);
 }

 QWidget* w = new QWidget(this);
 QHBoxLayout* layout = new QHBoxLayout(w);

 mConfig = new PreviewConfig(w);
 layout->addWidget(mConfig, 0);
 mConfig->show();
 mPreview = new ModelPreview(w);
 layout->addWidget(mPreview, 1);
 mPreview->show();

 connectBoth(mConfig, mPreview, SIGNAL(signalFovYChanged(float)), SLOT(slotFovYChanged(float)));
 connectBoth(mConfig, mPreview, SIGNAL(signalRotateXChanged(float)), SLOT(slotRotateXChanged(float)));
 connectBoth(mConfig, mPreview, SIGNAL(signalRotateYChanged(float)), SLOT(slotRotateYChanged(float)));
 connectBoth(mConfig, mPreview, SIGNAL(signalRotateZChanged(float)), SLOT(slotRotateZChanged(float)));
 connectBoth(mConfig, mPreview, SIGNAL(signalCameraXChanged(float)), SLOT(slotCameraXChanged(float)));
 connectBoth(mConfig, mPreview, SIGNAL(signalCameraYChanged(float)), SLOT(slotCameraYChanged(float)));
 connectBoth(mConfig, mPreview, SIGNAL(signalCameraZChanged(float)), SLOT(slotCameraZChanged(float)));
 connectBoth(mConfig, mPreview, SIGNAL(signalFrameChanged(int)), SLOT(slotFrameChanged(int)));
 connect(mConfig, SIGNAL(signalResetDefaults()), mPreview, SLOT(slotResetView()));
 connect(mPreview, SIGNAL(signalMaxFramesChanged(int)), mConfig, SLOT(slotMaxFramesChanged(int)));
 connect(mConfig, SIGNAL(signalPlacementPreviewChanged(bool)), mPreview, SLOT(slotPlacementPreviewChanged(bool)));
 connect(mConfig, SIGNAL(signalDisallowPlacementChanged(bool)), mPreview, SLOT(slotDisallowPlacementChanged(bool)));

 mPreview->slotResetView();

 setCentralWidget(w);

 initKAction();

 mIface = new BoDebugDCOPIface();
}

RenderMain::~RenderMain()
{
 mSpecies.clear();
 delete mIface;
}

void RenderMain::connectBoth(QObject* o1, QObject* o2, const char* signal, const char* slot)
{
 connect(o1, signal, o2, slot);
 connect(o2, signal, o1, slot);
}

void RenderMain::slotUnitChanged(int index)
{
 if (!sender() || !sender()->inherits("KAction")) {
	boError() << k_funcinfo << "sender() must inherit KAction" << endl;
	return;
 }
 if (index < 0) {
	boError() << k_funcinfo << "index < 0" << endl;
	return;
 }
 KAction* p = (KAction*)sender();
 uncheckAllBut(p);
 SpeciesTheme* s = mAction2Species[p];
 boDebug() << k_funcinfo << index << endl;
 QValueList<const UnitProperties*> props = s->allUnits();
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

void RenderMain::slotObjectChanged(int index)
{
 if (!sender() || !sender()->inherits("KAction")) {
	boError() << k_funcinfo << "sender() must inherit KAction" << endl;
	return;
 }
 if (index < 0) {
	boError() << k_funcinfo << "index < 0" << endl;
	return;
 }
 KAction* a = (KAction*)sender();
 SpeciesTheme* s = mAction2Species[a];
 BO_CHECK_NULL_RET(s);
 boDebug() << k_funcinfo << index << endl;
 QString object = s->allObjects()[index];
 if (object.isEmpty()) {
	boError() << k_funcinfo << "Can't find " << object << " (==" << index << ")" << endl;
	return;
 }
 boDebug() << k_funcinfo << object << endl;
 changeObject(s, object);
}

void RenderMain::slotDebugModels()
{
 KDialogBase* dialog = new KDialogBase(KDialogBase::Plain, i18n("Debug Models"),
		KDialogBase::Cancel, KDialogBase::Cancel, this,
		"debugmodelsdialog", false, true);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 QWidget* w = dialog->plainPage();
 QVBoxLayout* l = new QVBoxLayout(w);
 KGameModelDebug* models = new KGameModelDebug(w);
 l->addWidget(models);

 QPtrListIterator<SpeciesTheme> it(mSpecies);
 for (; it.current(); ++it) {
	models->addTheme(it.current());
 }
 models->slotUpdate();

 dialog->show();
}


SpeciesTheme* RenderMain::findTheme(const QString& theme) const
{
 SpeciesTheme* s = 0;
 QPtrListIterator<SpeciesTheme> it(mSpecies);
 for (; it.current() && !s; ++it) {
	if (it.current()->identifier().lower() == theme.lower()) {
		s = it.current();
	}
 }
 return s;
}

void RenderMain::changeUnit(const QString& theme, const QString& unit)
{
 SpeciesTheme* s = 0;
 const UnitProperties* prop = 0;

 s = findTheme(theme);
 if (!s) {
	boError() << k_funcinfo << "Could not find theme " << theme << endl;
	return;
 }
 QValueList<const UnitProperties*> units = s->allUnits();
 QValueList<const UnitProperties*>::Iterator i = units.begin();
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

void RenderMain::changeUnit(const QString& theme, unsigned long int type)
{
 SpeciesTheme* s = 0;
 QPtrListIterator<SpeciesTheme> it(mSpecies);
 for (; it.current() && !s; ++it) {
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

void RenderMain::changeObject(const QString& theme, const QString& object)
{
 SpeciesTheme* s = 0;

 s = findTheme(theme);
 if (!s) {
	boError() << k_funcinfo << "Could not find theme " << theme << endl;
	return;
 }
 changeObject(s, object);
}

void RenderMain::changeUnit(SpeciesTheme* s, const UnitProperties* prop)
{
 BO_CHECK_NULL_RET(s);
 BO_CHECK_NULL_RET(prop);
 // TODO: check/uncheck the menu items!

 mPreview->load(s, prop);
}

void RenderMain::changeObject(SpeciesTheme* s, const QString& objectModel)
{
 BO_CHECK_NULL_RET(s);
 mPreview->loadObjectModel(s, objectModel);
}

void RenderMain::initKAction()
{
 KStdAction::quit(this, SLOT(close()), actionCollection());

 QPtrListIterator<SpeciesTheme> it(mSpecies);
 for (; it.current(); ++it) {
	SpeciesTheme* s = it.current();
	KActionMenu* menu = new KActionMenu(s->identifier(), actionCollection(),
			"view_species");
	KSelectAction* selectUnit = new KSelectAction(i18n("&Units"), 0, this, SLOT(slotUnitChanged(int)));
	KSelectAction* selectObject = new KSelectAction(i18n("&Objects"), 0, this, SLOT(slotUnitChanged(int)));
	menu->insert(selectUnit);
	menu->insert(selectObject);
	mAction2Species.insert(selectUnit, s);
	mAction2Species.insert(selectObject, s);

	connect(selectUnit, SIGNAL(activated(int)),
			this, SLOT(slotUnitChanged(int)));
	connect(selectObject, SIGNAL(activated(int)),
			this, SLOT(slotObjectChanged(int)));

	QValueList<const UnitProperties*> units = s->allUnits();
	QStringList list;
	for (unsigned int j = 0; j < units.count(); j++) {
		list.append(units[j]->name());
	}
	selectUnit->setItems(list);
	selectObject->setItems(s->allObjects());
 }

 (void)new KAction(i18n("Debug &Models"), 0, this, SLOT(slotDebugModels()),
		actionCollection(), "debug_models");

 createGUI(locate("data", "boson/borenderui.rc"));
}

void RenderMain::uncheckAllBut(KAction* action)
{
 if (!action || !action->isA("KSelectAction")) {
	boError() << k_funcinfo << "not a valid KSelectAction" << endl;
	return;
 }
 QPtrDictIterator<SpeciesTheme> it(mAction2Species);
 for (; it.current(); ++it) {
	KAction* a = (KAction*)it.currentKey();
	if (a == action) {
		continue;
 }
	if (!a->isA("KSelectAction")) {
		continue;
	}
	((KSelectAction*)a)->setCurrentItem(-1);
 }
}



PreviewConfig::PreviewConfig(QWidget* parent) : QWidget(parent)
{
 bool slider = false; // FIXME: there seems to be a bug with the slider. when you slide between e.g. -1.0->0.0 or so then there seem to be recursive setValue() calls which block the UI for some seconds
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 mFovY = new KMyFloatNumInput(this);
 mFovY->setRange(MIN_FOVY, MAX_FOVY, 1.0, slider);
 connect(mFovY, SIGNAL(valueChanged(float)), this, SIGNAL(signalFovYChanged(float)));
 mFovY->setLabel(i18n("FovY"));
 topLayout->addWidget(mFovY);
 topLayout->addStretch(1);


 mRotateX = new KMyIntNumInput(this);
 mRotateY = new KMyIntNumInput(this);
 mRotateZ = new KMyIntNumInput(this);
 mRotateX->setLabel(i18n("Rotation around X-axis"));
 mRotateY->setLabel(i18n("Rotation around Y-axis"));
 mRotateZ->setLabel(i18n("Rotation around Z-axis"));
 mRotateX->setRange(MIN_ROTATE, MAX_ROTATE, 1, true);
 mRotateY->setRange(MIN_ROTATE, MAX_ROTATE, 1, true);
 mRotateZ->setRange(MIN_ROTATE, MAX_ROTATE, 1, true);
 connect(mRotateX, SIGNAL(valueChanged(float)), this, SIGNAL(signalRotateXChanged(float)));
 connect(mRotateY, SIGNAL(valueChanged(float)), this, SIGNAL(signalRotateYChanged(float)));
 connect(mRotateZ, SIGNAL(valueChanged(float)), this, SIGNAL(signalRotateZChanged(float)));
 topLayout->addWidget(mRotateX);
 topLayout->addWidget(mRotateY);
 topLayout->addWidget(mRotateZ);
 topLayout->addStretch(1);

 mCameraX = new KMyFloatNumInput(this);
 mCameraY = new KMyFloatNumInput(this);
 mCameraZ = new KMyFloatNumInput(this);
 mCameraX->setLabel(i18n("Camera X Position"));
 mCameraY->setLabel(i18n("Camera Y Position"));
 mCameraZ->setLabel(i18n("Camera Z Position"));
 mCameraX->setRange(MIN_CAMERA_X, MAX_CAMERA_X, 0.2, slider);
 mCameraY->setRange(MIN_CAMERA_Y, MAX_CAMERA_Y, 0.2, slider);
 mCameraZ->setRange(MIN_CAMERA_Z, MAX_CAMERA_Z, 0.2, slider);
 connect(mCameraX, SIGNAL(valueChanged(float)), this, SIGNAL(signalCameraXChanged(float)));
 connect(mCameraY, SIGNAL(valueChanged(float)), this, SIGNAL(signalCameraYChanged(float)));
 connect(mCameraZ, SIGNAL(valueChanged(float)), this, SIGNAL(signalCameraZChanged(float)));
 topLayout->addWidget(mCameraX);
 topLayout->addWidget(mCameraY);
 topLayout->addWidget(mCameraZ);
 topLayout->addStretch(1);

 mFrame = new KIntNumInput(this);
 mFrame->setLabel(i18n("Frame"));
 mFrame->setRange(0, 0);
 connect(mFrame, SIGNAL(valueChanged(int)), this, SIGNAL(signalFrameChanged(int)));
 topLayout->addWidget(mFrame);
 topLayout->addStretch(1);

 QWidget* placement = new QWidget(this);
 QHBoxLayout* placementLayout = new QHBoxLayout(placement);
 mPlacementPreview = new QCheckBox(i18n("Show placement preview"), placement);
 connect(mPlacementPreview, SIGNAL(toggled(bool)), this, SIGNAL(signalPlacementPreviewChanged(bool)));
 placementLayout->addWidget(mPlacementPreview);
 mDisallowPlacement = new QCheckBox(i18n("Disallow placement"), placement);
 connect(mDisallowPlacement, SIGNAL(toggled(bool)), this, SIGNAL(signalDisallowPlacementChanged(bool)));
 placementLayout->addWidget(mDisallowPlacement);
 topLayout->addWidget(placement);

 QPushButton* defaults = new QPushButton(i18n("Reset Defaults"), this);
 connect(defaults, SIGNAL(clicked()), this, SIGNAL(signalResetDefaults()));
 topLayout->addStretch(1);
 topLayout->addSpacing(10);
 topLayout->addWidget(defaults);
}

PreviewConfig::~PreviewConfig()
{
}

int main(int argc, char **argv)
{
 KAboutData about("borender",
		I18N_NOOP("Boson Rendering tester"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 2002 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );

 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 KApplication app;
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

 BosonConfig::initBosonConfig();
 BosonProfiling::initProfiling();
 boConfig->setDisableSound(true);
 BosonMusic::initBosonMusic(); // TODO: completely disable music in BosonConfig

 RenderMain* main = new RenderMain();
 kapp->setMainWidget(main);
 main->show();

 QString theme;
 unsigned long int typeId = 0;
 QString unit;
 QString object;

 if (args->isSet("maximized")) {
	main->showMaximized();
 }
 if (args->isSet("species")) {
	theme = args->getOption("species");
 }
 if (args->isSet("unit")) {
	if (theme.isEmpty()) {
		boError() << k_funcinfo << "you have to specify both unit and species!" << endl;
		return 1;
	}
	unit = args->getOption("unit");
 } else if (args->isSet("unit-id")) {
	if (theme.isEmpty()) {
		boError() << k_funcinfo << "you have to specify both unit-id and species!" << endl;
		return 1;
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
		boError() << k_funcinfo << "you have to specify both object and species!" << endl;
		return 1;
	}
	object = args->getOption("object");
 }
 if (!theme.isEmpty()) {
	if (typeId == 0 && unit.isEmpty() && object.isEmpty()) {
		boError() << k_funcinfo << "you have to specify both species and unit/object!" << endl;
		return 1;
	} else if (typeId > 0) {
		main->changeUnit(theme, typeId);
	} else if (!unit.isEmpty()) {
		main->changeUnit(theme, unit);
	} else if (!object.isEmpty()) {
		main->changeObject(theme, object);
	} else {
		boError() << k_funcinfo << "you have to specify both unit and species!" << endl;
		return 1;
	}
 }

 // AB: currently this makes sense when unit is specified on command line, as
 // all view parameter are reset on unit loading
 if (args->isSet("camera-x")) {
	bool ok = false;
	float c = args->getOption("camera-x").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-x must be a number" << endl;
		return 1;
	}
	main->emitSignalCameraX(c);
 }
 if (args->isSet("camera-y")) {
	bool ok = false;
	float c = args->getOption("camera-y").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-y must be a number" << endl;
		return 1;
	}
	main->emitSignalCameraY(c);
 }
 if (args->isSet("camera-z")) {
	bool ok = false;
	float c = args->getOption("camera-z").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "camera-z must be a number" << endl;
		return 1;
	}
	main->emitSignalCameraZ(c);
 }
 if (args->isSet("rotate-x")) {
	bool ok = false;
	float r = args->getOption("rotate-x").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-x must be a number" << endl;
		return 1;
	}
	main->emitSignalRotateX(r);
 }
 if (args->isSet("rotate-y")) {
	bool ok = false;
	float r = args->getOption("rotate-y").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-y must be a number" << endl;
		return 1;
	}
	main->emitSignalRotateY(r);
 }
 if (args->isSet("rotate-z")) {
	bool ok = false;
	float r = args->getOption("rotate-z").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "rotate-z must be a number" << endl;
		return 1;
	}
	main->emitSignalRotateZ(r);
 }
 if (args->isSet("fovy")) {
	bool ok = false;
	float f = args->getOption("fovy").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "fovy must be a number" << endl;
		return 1;
	}
	main->emitSignalFovY(f);
 }
 if (args->isSet("frame")) {
	bool ok = false;
	int f = args->getOption("frame").toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "frame must be a number" << endl;
		return 1;
	}
	main->emitSignalFrame(f);
 }


 args->clear();
 return app.exec();
}


