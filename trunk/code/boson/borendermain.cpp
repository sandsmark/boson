/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonmodeltextures.h"
#include "bomesh.h"
#include "bomaterial.h"
#include "bosonfont/bosonglfont.h"
#include "bosonprofiling.h"
#include "unitproperties.h"
#include "kgamemodeldebug.h"
#include "kgamespeciesdebug.h"
#include "bodebug.h"
#include "boversion.h"
#include "boapplication.h"
#include "bocamera.h"
#include "bocamerawidget.h"
#include "bomaterialwidget.h"
#include "bolight.h"
#include "bomeshrenderermanager.h"
#include "boglstatewidget.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kdialogbase.h>
#include <kcolordialog.h>
#include <kmessagebox.h>

#include <qtimer.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qinputdialog.h>

#include <GL/glu.h>
#include <math.h>
#include <stdlib.h>

#define NEAR 1.0
#define FAR 100.0

#define GL_UPDATE_TIMER 40

#define BORENDER_DEFAULT_LIGHT_ENABLED true
#define BORENDER_DEFAULT_MATERIALS_ENABLED true

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
    { "l", 0, 0 },
    { "lod <number>", I18N_NOOP("Initially displayed LOD"), 0 },
    { "indirect", I18N_NOOP("Use Indirect rendering (sloooow!!). debugging only."), 0 },
    { 0, 0, 0 }
};

void postBosonConfigInit();


ModelPreview::ModelPreview(QWidget* parent) : BosonGLWidget(parent)
{
 mUpdateTimer = new QTimer(this);
 connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateGL()));
 qApp->setGlobalMouseTracking(true);
 qApp->installEventFilter(this);
 setFocusPolicy(StrongFocus);

 mCurrentFrame = 0;
 mModel = 0;
 mCurrentLOD = 0;
 mMeshUnderMouse = -1;
 mSelectedMesh = -1;

 mMouseMoveDiff = new BoMouseMoveDiff;

 mPlacementPreview = false;
 mDisallowPlacement = false;
 mWireFrame = false;
 mConstruction = false;
 mRenderAxis = false;
 mRenderGrid = false;

 mDefaultFont = 0;

 mCamera = new BoCamera;
 mLight = 0;


 connect(this, SIGNAL(signalFovYChanged(float)), this, SLOT(slotFovYChanged(float)));
 connect(this, SIGNAL(signalFrameChanged(int)), this, SLOT(slotFrameChanged(int)));
 connect(this, SIGNAL(signalLODChanged(int)), this, SLOT(slotLODChanged(int)));

 setMinimumSize(200, 200);

 boConfig->addDynamicEntry(new BoConfigBoolEntry(boConfig, "ShowVertexPoints", true));
 boConfig->addDynamicEntry(new BoConfigUIntEntry(boConfig, "VertexPointSize", 3));
 boConfig->addDynamicEntry(new BoConfigColorEntry(boConfig, "BoRenderBackgroundColor", Qt::black));
 boConfig->addDynamicEntry(new BoConfigDoubleEntry(boConfig, "GridUnitSize", 0.1));
}

ModelPreview::~ModelPreview()
{
 qApp->setGlobalMouseTracking(false);
 resetModel();
 BoMeshRendererManager::manager()->unsetCurrentRenderer();
 delete mUpdateTimer;
 delete mMouseMoveDiff;
 delete mLight;
 delete mCamera;
}

void ModelPreview::initializeGL()
{
 if (isInitialized()) {
	return;
 }
 static bool recursive = false;
 if (recursive) {
	return;
 }
 recursive = true;
 makeCurrent();
 delete mDefaultFont;
 mDefaultFont = new BosonGLFont(QString::fromLatin1("fixed"));
 glClearColor(0.0, 0.0, 0.0, 0.0);
 glShadeModel(GL_FLAT);
 glDisable(GL_DITHER);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 delete mLight;
 mLight = new BoLight;
 if (mLight->id() < 0) {
	boError() << k_funcinfo << "light NOT created" << endl;
	delete mLight;
	mLight = 0;
 } else {
	BoVector4 lightDif(1.0f, 1.0f, 1.0f, 1.0f);
	BoVector4 lightAmb(0.5f, 0.5f, 0.5f, 1.0f);
	BoVector3 lightPos(-6000.0, 3000.0, 10000.0);
	mLight->setAmbient(lightAmb);
	mLight->setDiffuse(lightDif);
	mLight->setSpecular(lightDif);
	mLight->setDirectional(true);
	mLight->setEnabled(true);
 }

 setUpdatesEnabled(false);
 mUpdateTimer->start(GL_UPDATE_TIMER);


 recursive = false;
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
 BO_CHECK_NULL_RET(camera());
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();

 QColor background = boConfig->colorValue("BoRenderBackgroundColor", Qt::black);
 glClearColor((GLfloat)background.red() / 255.0f, (GLfloat)background.green() / 255.0f, background.blue() / 255.0f, 0.0f);

 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 glColor3ub(255, 255, 255);

 camera()->applyCameraToScene();

 if (mRenderGrid) {
	renderGrid();
 }
 if (mRenderAxis) {
	renderAxii();
 }

 if (boConfig->useLight()) {
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_NORMALIZE);
 }

 renderModel();
 renderMeshSelection();

 glDisable(GL_LIGHTING);
 glDisable(GL_NORMALIZE);

 glDisable(GL_DEPTH_TEST);

 renderText();
}

void ModelPreview::renderAxii()
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

void ModelPreview::renderModel(int mode)
{
 if (!haveModel()) {
	return;
 }

 glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT); // doesnt include client states

 if (mWireFrame) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 } else {
	glEnable(GL_TEXTURE_2D);
 }
 glEnable(GL_DEPTH_TEST);
 glEnableClientState(GL_VERTEX_ARRAY);
 glEnableClientState(GL_TEXTURE_COORD_ARRAY);

 // AB: if these are enabled we can't use triangle strips by any reason.
 // AB: we don't use triangle strips atm (and there are ways so that we still
 // can use backface culling). but i leave this out, as borender rendering speed
 // isn't critical and it may be easier to debug certain things.
// glEnable(GL_CULL_FACE);
// glCullFace(GL_BACK);

 if (mModel && mCurrentFrame >= 0) {
	BoFrame* f = frame(mCurrentFrame);
	if (f) {
		if (mPlacementPreview) {
			glEnable(GL_BLEND);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // default anyway - redundant call
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

		if (mode == GL_SELECT) {
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
		}
		BosonModel::startModelRendering();
		mModel->prepareRendering();
		f->renderFrame(0, mCurrentLOD, mode);
		BosonModel::stopModelRendering();
		if (mPlacementPreview) {
			// AB: do not reset the actual color - if it will get
			// used it will be set again anyway.
			glColor4ub(255, 255, 255, 255);
		}
	} else {
		boError() << k_funcinfo << "NULL frame" << endl;
	}
 }

 glPopAttrib();
 glDisableClientState(GL_VERTEX_ARRAY);
 glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void ModelPreview::renderGrid()
{
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

}

void ModelPreview::renderMeshSelection()
{
 if (!haveModel()) {
	return;
 }
 if (mSelectedMesh < 0) {
	return;
 }
 if (mCurrentFrame < 0) {
	return;
 }
 if ((unsigned int)mCurrentFrame >= mModel->frames()) {
	return;
 }
 BoMesh* mesh = 0;
 BoFrame* f = frame(mCurrentFrame);
 if ((unsigned int)mSelectedMesh >= f->meshCount()) {
	f = 0;
	mesh = 0;
 } else {
	mesh = f->mesh(mSelectedMesh);
 }
 if (!mesh) {
	return;
 }
 BoMatrix* matrix = f->matrix(mSelectedMesh);
 if (!matrix) {
	return;
 }
 glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
 glPushMatrix();
 glMultMatrixf(matrix->data());
 glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 glColor3ub(0, 255, 0);
 glDisable(GL_TEXTURE_2D);
 mesh->renderBoundingObject();
 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

 if (boConfig->boolValue("ShowVertexPoints")) {
	glColor3ub(0, 255, 0);
	float size = (float)boConfig->uintValue("VertexPointSize", 3);
	glPointSize(size);
	glDisable(GL_DEPTH_TEST);
	mesh->renderVertexPoints();
 }

 glPopMatrix();
 glPopAttrib();
 glColor3ub(255, 255, 255);
}

void ModelPreview::renderText()
{
 BO_CHECK_NULL_RET(mDefaultFont);

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0f, (GLfloat)width(), 0.0f, (GLfloat)height());
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 mDefaultFont->begin();
 const int border = 5;
 int y = height() - border;

 glColor3ub(255, 255, 255);
 QString meshName;
 if (mMeshUnderMouse >= 0) {
	BoMesh* mesh = 0;
	if (mModel && mCurrentFrame >= 0) {
		BoFrame* f = frame(mCurrentFrame);
		if (f) {
			if ((unsigned int)mMeshUnderMouse >= f->meshCount()) {
				f = 0;
			} else {
				mesh = f->mesh(mMeshUnderMouse);
			}
		}
	}
	if (mesh) {
		meshName = mesh->name();
	}
 }
 if (meshName.isNull()) {
	meshName = i18n("(no mesh under cursor)");
 }
 QString text = i18n("Mesh under cursor: %1").arg(meshName);
 y -= mDefaultFont->renderText(border, y, text, width() - 2 * border);

 if (mSelectedMesh >= 0) {
	BoMesh* mesh = 0;
	if (mModel && mCurrentFrame >= 0) {
		BoFrame* f = frame(mCurrentFrame);
		if ((unsigned int)mSelectedMesh < f->meshCount()) {
			mesh = f->mesh(mSelectedMesh);
		}
	}
	if (mesh) {
		QString name = mesh->name();
		QString material;
		if (mesh->material()) {
			material = mesh->material()->name();
		} else {
			material = i18n("(None)");
		}
		QString text = i18n("Selected mesh: %1 points: %2 material: %3")
				.arg(name)
				.arg(mesh->points())
				.arg(material);
		y -= mDefaultFont->renderText(border, y, text,
				width() - 2 * border);
	}
 }

 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();

}

BoFrame* ModelPreview::frame(unsigned int f) const
{
 if (!haveModel()) {
	return 0;
 }
 BoFrame* frame = 0;
 if (mConstruction) {
	frame = mModel->constructionStep(f);
 } else {
	frame = mModel->frame(f);
 }
 return frame;
}

void ModelPreview::load(SpeciesTheme* s, const UnitProperties* prop)
{
 resetModel();
 BO_CHECK_NULL_RET(s);
 BO_CHECK_NULL_RET(prop);
 makeCurrent();
 s->loadUnitModel(prop);
 BosonModel* model = s->unitModel(prop->typeId());
 if (!model) {
	BO_NULL_ERROR(model);
 }
 setModel(model);
}

void ModelPreview::loadObjectModel(SpeciesTheme* s, const QString& file)
{
 resetModel();
 BO_CHECK_NULL_RET(s);
 if (file.isEmpty()) {
	boError() << k_funcinfo << "empty filename" << endl;
	return;
 }
 makeCurrent();
 s->loadObjects();
 BosonModel* model = s->objectModel(file);
 if (!model) {
	BO_NULL_ERROR(model);
 }
 setModel(model);
}

void ModelPreview::setModel(BosonModel* model)
{
 mModel = model;
 if (mModel) {
	emit signalMaxFramesChanged(mModel->frames() - 1);
	emit signalMaxLODChanged(mModel->lodCount() - 1);
 }
}

void ModelPreview::slotResetView()
{
 BoVector3 cameraPos(0.0f, 0.0f, 2.0f);
 BoVector3 lookAt(0.0f, 0.0f, 0.0f);
 BoVector3 up(0.0f, 50.0f, 0.0f);
 updateCamera(cameraPos, lookAt, up);
 emit signalFovYChanged(60.0);
 resizeGL(width(), height());
}

void ModelPreview::slotConstructionChanged(bool on)
{
 mConstruction = on;
 int max = 0;
 if (mModel) {
	if (mConstruction) {
		max = mModel->constructionSteps() - 1;
	} else {
		max = mModel->frames() - 1;
	}
 }
 max = QMAX(0, max);
 boDebug() << k_funcinfo << max << endl;
 emit signalMaxFramesChanged(max);
}

void ModelPreview::resetModel()
{
 mModel = 0;
 mCurrentFrame = 0;
 mCurrentLOD = 0;
 mMeshUnderMouse = -1;
 mSelectedMesh = -1;
}

void ModelPreview::updateCursorDisplay(const QPoint& pos)
{
 mMeshUnderMouse = -1;
 if (!haveModel()) {
	return;
 }
 int picked = pickObject(pos);
 if (picked < 0) {
	return;
 }
 BoFrame* f = frame(mCurrentFrame);
 if (!f) {
	return;
 }
 if ((unsigned int)picked >= f->meshCount()) {
	boError() << k_funcinfo << "invalid mesh number: " << picked << endl;
	return;
 }
 mMeshUnderMouse = picked;
}

int ModelPreview::pickObject(const QPoint& cursor)
{
 BoFrame* f = 0;
 if (!haveModel()) {
	return -1;
 }
 f = frame(mCurrentFrame);
 if (!f) {
	return -1;
 }
 const int bufferSize = 256;
 unsigned int buffer[bufferSize];
 int viewport[4];

 glGetIntegerv(GL_VIEWPORT, viewport);

 glSelectBuffer(bufferSize, buffer);


 glRenderMode(GL_SELECT);
 glInitNames();
 glPushName(0);

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluPickMatrix((GLdouble)cursor.x(), (GLdouble)(viewport[3] - cursor.y()), 1.0, 10, viewport);
 gluPerspective(mFovY, (float)viewport[2] / (float)viewport[3], NEAR, FAR);
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 camera()->applyCameraToScene();
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 glColor3f(1.0f, 1.0f, 1.0f);


 renderModel(GL_SELECT);


 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 int hits = glRenderMode(GL_RENDER);
 int pos = 0;
 unsigned int closestZ = 0xFFFFFFFF;
 int mesh = -1;
 for (int i = 0; i < hits; i++) {
	unsigned int names = buffer[pos];
	pos++;
	unsigned int minZ = buffer[pos]; // note: depth is [0;1] and multiplied by 2^32-1
	pos++;
//	unsigned int maxZ = buffer[pos];
	pos++;
	if (names != 1) {
		boWarning() << k_funcinfo << "more than 1 name - not supported!" << endl;
		for (unsigned int j = 0; j < names; j++) {
			pos++;
		}
		continue;
	}
	unsigned int name = buffer[pos];
	BoMesh* m = 0;
	if (name < f->meshCount()) {
		m = f->mesh(name);
	}
	if (m) {
//		boDebug() << k_funcinfo << m->name() << endl;
		if (minZ < closestZ) {
			mesh = name;
			closestZ = minZ;
		}
	} else {
		boWarning() << k_funcinfo << "no such mesh: " << name << endl;
	}
	pos++;
 }

 return mesh;
}

bool ModelPreview::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseMove:
		if (hasMouse()) {
			updateCursorDisplay(((QMouseEvent*)e)->pos());
		}
		break;
	default:
		break;
 }
 return QObject::eventFilter(o, e);
}

void ModelPreview::mousePressEvent(QMouseEvent* e)
{
 switch (e->button()) {
	case QMouseEvent::LeftButton:
		break;
	case QMouseEvent::RightButton:
		mMouseMoveDiff->moveEvent(e->pos());
		break;
	default:
		break;
 }
}

void ModelPreview::mouseReleaseEvent(QMouseEvent* e)
{
 switch (e->button()) {
	case QMouseEvent::LeftButton:
		selectMesh(mMeshUnderMouse);
		break;
	case QMouseEvent::RightButton:
		break;
	default:
		break;
 }
}

void ModelPreview::selectMesh(int mesh)
{
 mSelectedMesh = mesh;
 emit signalMeshSelected(mesh);
}

void ModelPreview::mouseMoveEvent(QMouseEvent* e)
{
 mMouseMoveDiff->moveEvent(e);
 if (e->state() & LeftButton) {
 } else if (e->state() & RightButton) {
	// dx == rot. around x axis. this is equal to d_y_ mouse movement.
	int dx = mMouseMoveDiff->dy();
	int dy = mMouseMoveDiff->dx(); // rotation around y axis

	BoQuaternion q = camera()->quaternion();
	BoVector3 cameraPos;
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

	updateCamera(cameraPos, q2);
 }
}

void ModelPreview::wheelEvent(QWheelEvent* e)
{
 BO_CHECK_NULL_RET(camera());
 float delta = e->delta() / 120;
 if (e->orientation() == Horizontal) {
 } else {
 }
 BoVector3 lookAt = camera()->lookAt();
 BoVector3 eye = camera()->cameraPos();
 BoVector3 up = camera()->up();

 BoVector3 orientation = eye - lookAt;
 orientation.normalize();
 if (orientation.isNull()) {
	// FIXME: this should be disallowed!
	orientation = BoVector3(1.0f, 0.0f, 0.0f);
 }
 // TODO: we should check whether all meshes are still in front of the NEAR plane!

 eye.add(orientation * -delta);
 lookAt.add(orientation * -delta);
 updateCamera(eye, lookAt, up);
}

void ModelPreview::slotFrameChanged(int f)
{
 if (f != 0) {
	if (!mModel || f < 0) {
		emit signalFrameChanged(0);
		return;
	}
	int frames = 0;
	if (mConstruction) {
		frames = mModel->constructionSteps();
	} else {
		frames = mModel->frames();
	}
	if (f >= frames) {
		emit signalFrameChanged(frames - 1);
		return;
	}
  }
 mCurrentFrame = f;
}

void ModelPreview::slotLODChanged(int l)
{
 if (l != 0) {
	if (!mModel || l < 0) {
		emit signalLODChanged(0);
		return;
	}
	if ((unsigned int)l + 1 > mModel->lodCount()) {
		emit signalLODChanged(mModel->lodCount() - 1);
		return;
	}
  }
 mCurrentLOD = l;
}

void ModelPreview::updateCamera(const BoVector3& cameraPos, const BoQuaternion& q)
{
 updateCamera(cameraPos, q.matrix());
}

void ModelPreview::updateCamera(const BoVector3& cameraPos, const BoMatrix& rotationMatrix)
{
 BoVector3 lookAt;
 BoVector3 up;
 rotationMatrix.toGluLookAt(&lookAt, &up, cameraPos);
 updateCamera(cameraPos, lookAt, up);
}

void ModelPreview::updateCamera(const BoVector3& cameraPos, const BoVector3& lookAt, const BoVector3& up)
{
 BO_CHECK_NULL_RET(camera());
 camera()->setGluLookAt(cameraPos, lookAt, up);
 emit signalCameraChanged();
}

void ModelPreview::hideMesh(unsigned int mesh, bool hide)
{
 if (!haveModel()) {
	return;
 }
 for (unsigned int i = 0; i < mModel->frames(); i++) {
	BoFrame* f = mModel->frame(i);
	if (mesh >= f->meshCount()) {
		boWarning() << k_funcinfo << "mesh does not exist: " << mesh << " meshes in frame: " << f->meshCount() << endl;
		return;
	}
	if (!f) {
		boWarning() << k_funcinfo << "NULL frame " << i << endl;
		continue;
	}
	f->setHidden(mesh, hide);
 }
 if (isSelected(mesh)) {
	selectMesh(-1);
 }
}

bool ModelPreview::isSelected(unsigned int mesh) const
{
 // AB: one day we will use a list of selected meshes
 if (mSelectedMesh < 0) {
	return false;
 }
 if ((unsigned int)mSelectedMesh == mesh) {
	return true;
 }
 return false;
}

void ModelPreview::slotHideSelectedMesh()
{
 if (!haveModel()) {
	return;
 }
 if (mSelectedMesh < 0) {
	return;
 }
 hideMesh((unsigned int)mSelectedMesh);
}

void ModelPreview::slotHideUnSelectedMeshes()
{
 if (!haveModel()) {
	return;
 }
 if (mSelectedMesh < 0) {
	return;
 }
 if (mCurrentFrame < 0) {
	return;
 }
 BoFrame* f = frame(mCurrentFrame);
 BO_CHECK_NULL_RET(f);
 for (unsigned int i = 0; i < f->meshCount(); i++) {
	if (i == (unsigned int)mSelectedMesh) {
		continue;
	}
	hideMesh(i);
 }
}

void ModelPreview::slotUnHideAllMeshes()
{
 if (!haveModel()) {
	return;
 }
 if (mCurrentFrame < 0) {
	return;
 }
 BoFrame* f = frame(mCurrentFrame);
 BO_CHECK_NULL_RET(f);
 for (unsigned int i = 0; i < f->meshCount(); i++) {
	hideMesh(i, false);
 }
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

 connect(mPreview, SIGNAL(signalCameraChanged()), mConfig, SLOT(slotCameraChanged()));
 mConfig->setCamera(mPreview->camera());

 mPreview->initGL();
 mLightWidget = new BoLightCameraWidget(0, true);
 mLightWidget->hide();
 mLightWidget->setLight(mPreview->light(), mPreview->context());

 mMaterialWidget = new BoMaterialWidget(0);
 mMaterialWidget->hide();


 connectBoth(mConfig, mPreview, SIGNAL(signalFovYChanged(float)), SLOT(slotFovYChanged(float)));
 connectBoth(mConfig, mPreview, SIGNAL(signalFrameChanged(int)), SLOT(slotFrameChanged(int)));
 connectBoth(mConfig, mPreview, SIGNAL(signalLODChanged(int)), SLOT(slotLODChanged(int)));
 connect(mConfig, SIGNAL(signalResetDefaults()), mPreview, SLOT(slotResetView()));
 connect(mPreview, SIGNAL(signalMaxFramesChanged(int)), mConfig, SLOT(slotMaxFramesChanged(int)));
 connect(mPreview, SIGNAL(signalMaxLODChanged(int)), mConfig, SLOT(slotMaxLODChanged(int)));
 connect(mConfig, SIGNAL(signalPlacementPreviewChanged(bool)), mPreview, SLOT(slotPlacementPreviewChanged(bool)));
 connect(mConfig, SIGNAL(signalDisallowPlacementChanged(bool)), mPreview, SLOT(slotDisallowPlacementChanged(bool)));
 connect(mConfig, SIGNAL(signalWireFrameChanged(bool)), mPreview, SLOT(slotWireFrameChanged(bool)));
 connect(mConfig, SIGNAL(signalConstructionChanged(bool)), mPreview, SLOT(slotConstructionChanged(bool)));
 connect(mConfig, SIGNAL(signalRenderAxisChanged(bool)), mPreview, SLOT(slotRenderAxisChanged(bool)));
 connect(mConfig, SIGNAL(signalRenderGridChanged(bool)), mPreview, SLOT(slotRenderGridChanged(bool)));
 connect(mConfig, SIGNAL(signalHideMesh()), mPreview, SLOT(slotHideSelectedMesh()));
 connect(mConfig, SIGNAL(signalHideOthers()), mPreview, SLOT(slotHideUnSelectedMeshes()));
 connect(mConfig, SIGNAL(signalUnHideAll()), mPreview, SLOT(slotUnHideAllMeshes()));
 connect(mPreview, SIGNAL(signalMeshSelected(int)), mConfig, SLOT(slotMeshSelected(int)));


 mPreview->slotResetView();
 setCentralWidget(w);

 initKAction();

 mIface = new BoDebugDCOPIface();
}

RenderMain::~RenderMain()
{
 boConfig->save(false);
 mSpecies.clear();
 delete mIface;
}

void RenderMain::connectBoth(QObject* o1, QObject* o2, const char* signal, const char* slot)
{
 connect(o1, signal, o2, slot);
 connect(o2, signal, o1, slot);
}

bool RenderMain::loadCamera(KCmdLineArgs* args)
{
 BoQuaternion quat = mPreview->camera()->quaternion();
 BoVector3 cameraPos = mPreview->camera()->cameraPos();

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

 BoVector3 lookAt, up;
 quat.matrix().toGluLookAt(&lookAt, &up, cameraPos);

 mPreview->camera()->setGluLookAt(cameraPos, lookAt, up);
 mConfig->slotCameraChanged();

 return true;
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
	SpeciesTheme* theme = it.current();

	// units
	QValueList<const UnitProperties*> prop = it.current()->allUnits();
	QValueList<const UnitProperties*>::Iterator propIt;
	for (propIt = prop.begin(); propIt != prop.end(); ++propIt) {
		QStringList fileNames = SpeciesTheme::unitModelFiles();
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
	for (unsigned int i = 0; i < objects.count(); i++) {
		QString file = theme->themePath() + QString::fromLatin1("objects/") + objectFiles[i];
		models->addFile(file, objects[i]);
	}

	// add any files that we might have missed
	models->addFiles(theme->themePath());
 }
 models->slotUpdate();

 dialog->show();
}

void RenderMain::slotDebugSpecies()
{
 KDialogBase* dialog = new KDialogBase(KDialogBase::Plain, i18n("Debug Species"),
		KDialogBase::Cancel, KDialogBase::Cancel, this,
		"debugspeciesdialog", false, true);
 connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
 QWidget* w = dialog->plainPage();
 QVBoxLayout* l = new QVBoxLayout(w);
 KGameSpeciesDebug* species = new KGameSpeciesDebug(w);
 l->addWidget(species);
 species->loadSpecies();

 dialog->show();
}

void RenderMain::slotVertexPointSize()
{
 bool ok = false;
 unsigned int size = boConfig->uintValue("VertexPointSize");
 size = (unsigned int)QInputDialog::getInteger(i18n("Vertex point size"),
		i18n("Vertex point size (in pixels)"),
		(int)size, 0, 500, 1, &ok, this);
 if (ok) {
	boConfig->setUIntValue("VertexPointSize", size);
 }
}

void RenderMain::slotGridUnitSize()
{
 bool ok = false;
 double size = boConfig->doubleValue("GridUnitSize", 0.1);
 size = QInputDialog::getDouble(i18n("Grid unit size"),
		i18n("Grid unit size"),
		size, 0.0, 1.0, 2, &ok, this);
 if (ok) {
	boConfig->setDoubleValue("GridUnitSize", size);
 }
}

void RenderMain::slotShowVertexPoints(bool s)
{
 boConfig->setBoolValue("ShowVertexPoints", s);
}

void RenderMain::slotBackgroundColor()
{
 QColor color = boConfig->colorValue("BoRenderBackgroundColor", Qt::black);
 int result = KColorDialog::getColor(color, color, this);
 if (result == KColorDialog::Accepted) {
	boConfig->setColorValue("BoRenderBackgroundColor", color);
 }
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

 mMaterialWidget->clearMaterials();
 mMaterialWidget->hide();
 mPreview->load(s, prop);
}

void RenderMain::changeObject(SpeciesTheme* s, const QString& objectModel)
{
 BO_CHECK_NULL_RET(s);
 mMaterialWidget->clearMaterials();
 mMaterialWidget->hide();
 mPreview->loadObjectModel(s, objectModel);
}

void RenderMain::initKAction()
{
 KStdAction::quit(this, SLOT(close()), actionCollection());

 KActionMenu* modelMenu = new KActionMenu(i18n("&Model"),
		 actionCollection(), "model");
 QPtrListIterator<SpeciesTheme> it(mSpecies);
 for (; it.current(); ++it) {
	SpeciesTheme* s = it.current();
	KActionMenu* menu = new KActionMenu(s->identifier(), actionCollection(),
			QString("model_species_%1").arg(s->identifier()));
	modelMenu->insert(menu);

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

 KToggleAction* vertexPoints = new KToggleAction(i18n("Show vertex points"),
		0, 0, 0,
		actionCollection(), "options_show_vertex_points");
 connect(vertexPoints, SIGNAL(toggled(bool)), this, SLOT(slotShowVertexPoints(bool)));
 vertexPoints->setChecked(boConfig->boolValue("ShowVertexPoints"));
 (void)new KAction(i18n("Vertex point size..."), 0, this,
		SLOT(slotVertexPointSize()),
		actionCollection(), "options_vertex_point_size");
 (void)new KAction(i18n("Grid unit size..."), 0, this,
		SLOT(slotGridUnitSize()),
		actionCollection(), "options_grid_unit_size");
 (void)new KAction(i18n("Background color..."), 0, this,
		SLOT(slotBackgroundColor()),
		actionCollection(), "options_background_color");
 (void)new KAction(i18n("Light..."), 0, this, SLOT(slotShowLightWidget()),
		actionCollection(), "options_light"); // AB: actually this is NOT a setting
 (void)new KAction(i18n("Materials..."), 0, this, SLOT(slotShowMaterialsWidget()),
		actionCollection(), "options_materials"); // AB: actually this is NOT a setting


 (void)new KAction(i18n("Debug &Models"), 0, this, SLOT(slotDebugModels()),
		actionCollection(), "debug_models");
 (void)new KAction(i18n("Debug &Species"), 0, this, SLOT(slotDebugSpecies()),
		actionCollection(), "debug_species");
 (void)new KAction(i18n("Show OpenGL states"), KShortcut(), this,
		SLOT(slotShowGLStates()), actionCollection(),
		"debug_show_opengl_states");
 (void)new KAction(i18n("&Reload model textures"), KShortcut(), this,
		SLOT(slotReloadModelTextures()), actionCollection(), "debug_lazy_reload_model_textures");
 (void)new KAction(i18n("Reload &meshrenderer plugin"), KShortcut(), this,
		SLOT(slotReloadMeshRenderer()), actionCollection(),
		"debug_lazy_reload_meshrenderer");

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

void RenderMain::slotShowLightWidget()
{
 mLightWidget->show();
}

void RenderMain::slotShowMaterialsWidget()
{
 mMaterialWidget->clearMaterials();
 if (!mPreview || !mPreview->model()) {
	return;
 }
 BosonModel* model = mPreview->model();
 for (unsigned int i = 0; i < model->materialCount(); i++) {
	mMaterialWidget->addMaterial(model->material(i));
 }
 mMaterialWidget->show();
}

void RenderMain::slotShowGLStates()
{
 boDebug() << k_funcinfo << endl;
 BoGLStateWidget* w = new BoGLStateWidget(0, 0, WDestructiveClose);
 w->show();
}

void RenderMain::slotReloadMeshRenderer()
{
 bool unusable = false;
 bool r = BoMeshRendererManager::manager()->reloadPlugin(&unusable);
 if (r) {
	return;
 }
 boError() << "meshrenderer reloading failed" << endl;
 if (unusable) {
	KMessageBox::sorry(this, i18n("Reloading meshrenderer failed, library is now unusable. quitting."));
	exit(1);
 } else {
	KMessageBox::sorry(this, i18n("Reloading meshrenderer failed but library should still be usable"));
 }
}

void RenderMain::slotReloadModelTextures()
{
 BO_CHECK_NULL_RET(BosonModelTextures::modelTextures());
 BosonModelTextures::modelTextures()->reloadTextures();
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

 mFrame = new KIntNumInput(this);
 mFrame->setLabel(i18n("Frame"));
 mFrame->setRange(0, 0);
 connect(mFrame, SIGNAL(valueChanged(int)), this, SIGNAL(signalFrameChanged(int)));
 topLayout->addWidget(mFrame);
 topLayout->addStretch(1);

 mLOD = new KIntNumInput(this);
 mLOD->setLabel(i18n("LOD"));
 mLOD->setRange(0, 0);
 connect(mLOD, SIGNAL(valueChanged(int)), this, SIGNAL(signalLODChanged(int)));
 topLayout->addWidget(mLOD);
 topLayout->addStretch(1);

 QHBox* hideBox = new QHBox(this);
 mHideMesh = new QPushButton(i18n("Hide"), hideBox);
 mHideMesh->setEnabled(false);
 mHideOthers = new QPushButton(i18n("Hide others"), hideBox);
 mHideOthers->setEnabled(false);
 mUnHideAll = new QPushButton(i18n("UnHide all"), hideBox);
 connect(mHideMesh, SIGNAL(clicked()), this, SIGNAL(signalHideMesh()));
 connect(mHideOthers, SIGNAL(clicked()), this, SIGNAL(signalHideOthers()));
 connect(mUnHideAll, SIGNAL(clicked()), this, SIGNAL(signalUnHideAll()));
 topLayout->addWidget(hideBox);
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

 mWireFrame = new QCheckBox(i18n("Show wireframe"), this);
 connect(mWireFrame, SIGNAL(toggled(bool)), this, SIGNAL(signalWireFrameChanged(bool)));
 topLayout->addWidget(mWireFrame);

 mConstruction = new QCheckBox(i18n("Show construction"), this);
 connect(mConstruction, SIGNAL(toggled(bool)), this, SIGNAL(signalConstructionChanged(bool)));
 topLayout->addWidget(mConstruction);

 mRenderAxis = new QCheckBox(i18n("Render axis"), this);
 connect(mRenderAxis, SIGNAL(toggled(bool)), this, SIGNAL(signalRenderAxisChanged(bool)));
 topLayout->addWidget(mRenderAxis);

 mRenderGrid = new QCheckBox(i18n("Render grid"), this);
 connect(mRenderGrid, SIGNAL(toggled(bool)), this, SIGNAL(signalRenderGridChanged(bool)));
 topLayout->addWidget(mRenderGrid);

 mEnableLight = new QCheckBox(i18n("Enable Light"), this);
 connect(mEnableLight, SIGNAL(toggled(bool)), this, SLOT(slotEnableLightChanged(bool)));
 topLayout->addWidget(mEnableLight);
 mEnableLight->setChecked(BORENDER_DEFAULT_LIGHT_ENABLED);
 slotEnableLightChanged(mEnableLight->isChecked());

 mEnableMaterials = new QCheckBox(i18n("Enable Materials"), this);
 connect(mEnableMaterials, SIGNAL(toggled(bool)), this, SLOT(slotEnableMaterialsChanged(bool)));
 topLayout->addWidget(mEnableMaterials);
 mEnableMaterials->setChecked(BORENDER_DEFAULT_MATERIALS_ENABLED);
 slotEnableMaterialsChanged(mEnableMaterials->isChecked());

 mCameraWidget = new BoCameraWidget(this, "bocamerawidget");
 topLayout->addWidget(mCameraWidget);
 mCameraWidget->show();

 QPushButton* defaults = new QPushButton(i18n("Reset Defaults"), this);
 connect(defaults, SIGNAL(clicked()), this, SIGNAL(signalResetDefaults()));
 topLayout->addStretch(1);
 topLayout->addSpacing(10);
 topLayout->addWidget(defaults);
}

PreviewConfig::~PreviewConfig()
{
}

void PreviewConfig::setCamera(BoCamera* camera)
{
 mCameraWidget->setCamera(camera);
}

void PreviewConfig::slotCameraChanged()
{
 mCameraWidget->slotUpdateFromCamera();
}

void PreviewConfig::slotMeshSelected(int mesh)
{
 if (mesh < 0) {
	mHideMesh->setEnabled(false);
	mHideOthers->setEnabled(false);
 } else {
	mHideMesh->setEnabled(true);
	mHideOthers->setEnabled(true);
 }
}

void PreviewConfig::slotEnableLightChanged(bool e)
{
 boConfig->setUseLight(e);
}

void PreviewConfig::slotEnableMaterialsChanged(bool e)
{
 boConfig->setUseMaterials(e);
}

int main(int argc, char **argv)
{
 KAboutData about("borender",
		I18N_NOOP("Boson Rendering tester"),
		version,
		description,
		KAboutData::License_GPL,
		"(C) 2002-2003 The Boson team",
		0,
		"http://boson.eu.org");
 about.addAuthor( "Andreas Beckermann", I18N_NOOP("Coding & Current Maintainer"), "b_mann@gmx.de" );

 // we need to do extra stuff after BosonConfig's initialization
 BosonConfig::setPostInitFunction(&postBosonConfigInit);

 KCmdLineArgs::init(argc, argv, &about);
 KCmdLineArgs::addCmdLineOptions(options);
 KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

 BoApplication app;
 QObject::connect(kapp, SIGNAL(lastWindowClosed()), kapp, SLOT(quit()));

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
 if (args->isSet("indirect")) {
	boWarning() << k_funcinfo << "use indirect rendering (slow!)" << endl;
	boConfig->setWantDirect(false);
 }
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

 if (!main->loadCamera(args)) {
	return 1;
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
 if (args->isSet("lod")) {
	bool ok = false;
	int l = args->getOption("lod").toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "lod must be a number" << endl;
		return 1;
	}
	main->emitSignalLOD(l);
 }


 args->clear();
 return app.exec();
}


void postBosonConfigInit()
{
 boConfig->setDisableSound(true);
}
