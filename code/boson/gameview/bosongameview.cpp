/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <ufo/ufo.hpp>

#include "bosongameview.h"
#include "bosongameview.moc"

#include "../no_player.h"
#include "../defines.h"
#include "../bosoncanvas.h"
#include "../bosoncanvasstatistics.h"
#include "../bosonmap.h"
#include "../cell.h"
#include "../boitemlist.h"
#include "../rtti.h"
#include "../unit.h"
#include "../unitproperties.h"
#include "../speciestheme.h"
#include "../playerio.h"
#include "../bosoncursor.h"
#include "../boselection.h"
#include "../bosonconfig.h"
#include "../bosonprofiling.h"
#include "../bosoneffect.h"
#include "../bosoneffectparticle.h"
#include "../boson.h"
#include "bodebug.h"
#include "../items/bosonshot.h"
#include "../items/bosonitemrenderer.h"
#include "../unitplugins.h"
#include "../bosonmodel.h"
#include "../bo3dtools.h"
#include "bosongameviewinputbase.h"
#include "../bogltooltip.h"
#include "../bosongroundtheme.h"
#include "../bocamera.h"
#include "../boautocamera.h"
#include "../bogroundrenderer.h"
#include "../bogroundrenderermanager.h"
#include "../bomeshrenderermanager.h"
#include "../bolight.h"
#include "../bosonglminimap.h"
#include "../bomaterial.h"
#include "../info/boinfo.h"
#include "../speciesdata.h"
#include "../bowater.h"
#include "../bosonpath.h"
#include "../botexture.h"
#include "../boufo/boufoaction.h"
#include "../bosonufominimap.h"
#include "bosonufogamegui.h"
#include "bosonlocalplayerinput.h"
#include "../bosonplayfield.h"
#include "../bosondata.h"
#include "../bocamerawidget.h"
#include "../bosonmessage.h"
#include "../boaction.h"
#include "../boeventlistener.h"
#include "bosonmenuinput.h"
#include "../boshader.h"
#include "bosonufogamewidgets.h"
#include "../script/bosonscript.h"
#include "../script/bosonscriptinterface.h"
#include "../bomousemovediff.h"
#include "../bosonfpscounter.h"

#include <kgame/kgameio.h>
#include <kgame/kplayer.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include "../boeventloop.h"

#include <qtimer.h>
#include <qcursor.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qdir.h>
#include <qdom.h>
#include <qvaluevector.h>

/**
 * Calculate the maximum and minimum world coordinates from the
 * specified rectangles.
 *
 * The rect @p rect is in window coordinates (e.g. the selection rect).
 **/
static void calculateWorldRect(const BoGLMatrices& matrices, const QRect& rect, float* minX, float* minY, float* maxX, float* maxY)
{
 BO_CHECK_NULL_RET(boGame);
 BO_CHECK_NULL_RET(boGame->canvas());
 const BosonMap* map = boGame->canvas()->map();
 BO_CHECK_NULL_RET(map);
 GLfloat posX, posY;
 GLfloat posZ;
 Bo3dTools::mapCoordinates(matrices, rect.topLeft(), &posX, &posY, &posZ);
 *maxX = *minX = posX;
 *maxY = *minY = -posY;
 Bo3dTools::mapCoordinates(matrices, rect.topRight(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);
 Bo3dTools::mapCoordinates(matrices, rect.bottomLeft(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);
 Bo3dTools::mapCoordinates(matrices, rect.bottomRight(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);

 *maxX = QMAX(0, *maxX);
 *maxY = QMAX(0, *maxY);
 *minX = QMAX(0, *minX);
 *minY = QMAX(0, *minY);
 *maxX = QMIN((map->width() - 1), *maxX);
 *minX = QMIN((map->width() - 1), *minX);
 *maxY = QMIN((map->height() - 1), *maxY);
 *minY = QMIN((map->height() - 1), *minY);
 *minY *= -1;
 *maxY *= -1;
}

SelectionRect::SelectionRect() : QObject(0)
{
 mVisible = false;
}

void SelectionRect::widgetRect(QRect* rect) const
{
 QRect r(mStartPos, mEndPos);
 *rect = r.normalize();
}

void SelectionRect::setStartWidgetPos(const QPoint& pos)
{
 mStartPos = pos;
 setEndWidgetPos(mStartPos);

 // AB: no need to emit signalChanged(), as setEndWidgetPos() does so
}

void SelectionRect::setEndWidgetPos(const QPoint& pos)
{
 mEndPos = pos;

 QRect rect;
 widgetRect(&rect);
 emit signalChanged(rect);
}

void SelectionRect::setVisible(bool v)
{
 mVisible = v;
 emit signalVisible(isVisible());
}

void SelectionRect::quitGame()
{
 setVisible(false);
 setEndWidgetPos(startPos());
}


BoItemList* SelectionRect::items(const PlayerIO* localPlayerIO, const BosonCanvas* canvas, const BoGLMatrices& gameGLMatrices) const
{
 if (!canvas) {
	BO_NULL_ERROR(canvas);
	return new BoItemList();
 }
 if (!localPlayerIO) {
	BO_NULL_ERROR(localPlayerIO);
	return new BoItemList();
 }
 const BosonMap* map = canvas->map();
 if (!map) {
	BO_NULL_ERROR(map);
	return new BoItemList();
 }

 QRect widgetRect_;
 widgetRect(&widgetRect_);

 GLfloat maxX, maxY;
 GLfloat minX, minY;
 calculateWorldRect(gameGLMatrices, widgetRect_, &minX, &minY, &maxX, &maxY);
 maxY /= -1.0f;
 minY /= -1.0f;

 // now the really ugly part starts. we really should improve this.
 // the rect (pos1, pos2) is the smallest possible 2d rect that contains all
 // cells (with 2d rect i mean: parallel to x and y world-axis).
 // but we we need an actual world rect, as the view is most probably rotated
 // and so a lot of cells that are in the parallel rect are not in the actual
 // rect.
 // to achieve this we loop through all cells in the parallel rect and check
 // whether they are in the actual rect.
 // someone with some maths knowledge and/or some time can surely speed this up
 // by using some proper formulas!

 QRect cellRect = QRect(QPoint((int)minX, (int)minY), QPoint((int)maxX, (int)maxY)).normalize();
 QPoint win;
 QValueList<const Cell*> cells;
 for (int x = cellRect.left(); x <= cellRect.right(); x++) {
	for (int y = cellRect.top(); y <= cellRect.bottom(); y++) {
		const Cell* c = canvas->cell(x, y);
		if (!c) {
			boDebug() << k_funcinfo << "NULL cell at " << x << "," << y << endl;
			continue;
		}
		// AB: we use z==0.0, that is not really nice here. anyway,
		// let's wait until it causes trouble.
		// (read: I don't know which kind of trouble it will cause)
		GLfloat glx, gly, glz;

		// top left corner of cell
		glx = x;
		gly = -y;
		glz = map->heightAtCorner(x, y);
		Bo3dTools::boProject(gameGLMatrices, glx, gly, glz, &win);
		if (widgetRect_.contains(win)) {
			cells.append(c);
			continue;
		}
		// top right corner of cell
		glx = (x + 1);
		gly = -y;
		glz = map->heightAtCorner(x + 1, y);
		Bo3dTools::boProject(gameGLMatrices, glx, gly, glz, &win);
		if (widgetRect_.contains(win)) {
			cells.append(c);
			continue;
		}
		// bottom left corner of cell
		glx = x;
		gly = -(y + 1);
		glz = map->heightAtCorner(x, y + 1);
		Bo3dTools::boProject(gameGLMatrices, glx, gly, glz, &win);
		if (widgetRect_.contains(win)) {
			cells.append(c);
			continue;
		}
		// bottom right corner of cell
		glx = (x + 1);
		gly = -(y + 1);
		glz = map->heightAtCorner(x + 1, y + 1);
		Bo3dTools::boProject(gameGLMatrices, glx, gly, glz, &win);
		if (widgetRect_.contains(win)) {
			cells.append(c);
			continue;
		}
	}
 }

 // another ugly part of this function...
 QPtrVector<const Cell> cellVector(cells.count());
 QValueList<const Cell*>::Iterator it;
 int i = 0;
 for (it = cells.begin(); it != cells.end(); ++it) {
	cellVector.insert(i, *it);
	i++;
 }

 return localPlayerIO->unitsAtCells(&cellVector);
}


BoCursorEdgeScrolling::BoCursorEdgeScrolling(QObject* parent) : QObject(parent)
{
 mCursorEdgeCounter = 0;
 mCursorEdgeTimer = new QTimer(this);
 connect(mCursorEdgeTimer, SIGNAL(timeout()),
		this, SLOT(slotCursorEdgeTimeout()));
 qApp->installEventFilter(this);
}

BoCursorEdgeScrolling::~BoCursorEdgeScrolling()
{
}

void BoCursorEdgeScrolling::quitGame()
{
 mCursorEdgeTimer->stop();
 mCursorEdgeCounter = 0;
}

bool BoCursorEdgeScrolling::eventFilter(QObject* o, QEvent* e)
{
 switch (e->type()) {
	case QEvent::MouseMove:
		if (!mCursorEdgeTimer->isActive()) {
			slotCursorEdgeTimeout();
		}
		break;
	default:
		break;
 }
 return QObject::eventFilter(o, e);
}

void BoCursorEdgeScrolling::slotCursorEdgeTimeout()
{
 BO_CHECK_NULL_RET(camera());
 if (!boGame) {
	return;
 }
 if (boGame->gameStatus() == KGame::Init) {
	// probably startup screen visible
	return;
 }
 float x = 0;
 float y = 0;
 const int sensity = boConfig->uintValue("CursorEdgeSensity");

#warning FIXME: dont use mainWidget() here, use the ufo widget instead
 QWidget* w = qApp->mainWidget();
 BO_CHECK_NULL_RET(w);
 QPoint pos = w->mapFromGlobal(QCursor::pos());

 const int move = 20; // FIXME hardcoded - use BosonConfig instead
 if (pos.x() <= sensity && pos.x() > -1) {
	x = -move;
 } else if (pos.x() >= w->width() - sensity && pos.x() <= w->width()) {
	x = move;
 }
 if (pos.y() <= sensity && pos.y() > -1) {
	y = -move;
 } else if (pos.y() >= w->height() - sensity && pos.y() <= w->height()) {
	y = move;
 }
 if (!x && !y || !sensity) {
	mCursorEdgeTimer->stop();
	mCursorEdgeCounter = 0;
 } else {
	GLfloat dx, dy;
	Bo3dTools::mapDistance(matrices()->modelviewMatrix(), matrices()->projectionMatrix(), matrices()->viewport(),
			(int)x, (int)y, &dx, &dy);
	if (!mCursorEdgeTimer->isActive()) {
		mCursorEdgeTimer->start(20);
	}
	mCursorEdgeCounter++;
	if (mCursorEdgeCounter > 30) {
		camera()->changeLookAt(BoVector3Float(dx, dy, 0));
	}
 }
}







class BosonGameViewPrivate
{
public:
	BosonGameViewPrivate()
	{
		mUfoCanvasWidget = 0;
		mUfoGameGUI = 0;
		mToolTipLabel = 0;
		mUfoCursorWidget = 0;
		mUfoSelectionRectWidget = 0;

		mActionCollection = 0;
		mToolTips = 0;
		mGLMiniMap = 0;
		mLocalPlayerIO = 0;
		mSelectionRect = 0;
		mScriptConnector = 0;
		mInput = 0;
		mMouseIO = 0;
		mCursorEdgeScrolling = 0;

		mSelectionGroups = 0;

		mGameGLMatrices = 0;

		mLightWidget = 0;
	}
	BosonUfoCanvasWidget* mUfoCanvasWidget;
	BosonUfoPlacementPreviewWidget* mUfoPlacementPreviewWidget;
	BosonUfoLineVisualizationWidget* mUfoLineVisualizationWidget;
	BosonUfoGameGUI* mUfoGameGUI;
	BoUfoLabel* mToolTipLabel;
	BosonUfoCursorWidget* mUfoCursorWidget;
	BosonUfoSelectionRectWidget* mUfoSelectionRectWidget;

	BosonGameFPSCounter* mFPSCounter;
	BoUfoActionCollection* mActionCollection;
	BoGLToolTip* mToolTips;
	BosonGLMiniMap* mGLMiniMap;
	PlayerIO* mLocalPlayerIO;
	BosonGameViewScriptConnector* mScriptConnector;
	SelectionRect* mSelectionRect;
	KGameMouseIO* mMouseIO;
	BosonGameViewInputBase* mInput;
	bool mInputInitialized;
	BoCursorEdgeScrolling* mCursorEdgeScrolling;
	BoMouseMoveDiff mMouseMoveDiff;

	QPoint mCursorQtWidgetPos;
	QPoint mCursorWidgetPos;
	BoVector3Fixed mCursorCanvasVector;
	BoGameCamera mCamera;
	BoSelectionGroup* mSelectionGroups;

	GLint mViewport[4];
	BoMatrix mProjectionMatrix;
	BoMatrix mModelviewMatrix;
	GLfloat mViewFrustum[6 * 4];
	BoGLMatrices* mGameGLMatrices;
	GLfloat mFovY; // see gluPerspective
	GLfloat mAspect; // see gluPerspective

	BoLightCameraWidget1* mLightWidget;
};

BosonGameView::BosonGameView()
		: BoUfoCustomWidget()
{
 boDebug() << k_funcinfo << endl;
 init();
}

BosonGameView::~BosonGameView()
{
 boDebug() << k_funcinfo << endl;

 quitGame();
 delete d->mSelectionRect;
 delete d->mScriptConnector;
 delete d->mSelectionGroups;
 delete mSelection;
 delete d->mGameGLMatrices;
 delete d->mUfoGameGUI;
 delete d->mGLMiniMap;
 delete d->mToolTips;
 BoMeshRendererManager::manager()->unsetCurrentRenderer();
 BoGroundRendererManager::manager()->unsetCurrentRenderer();
 SpeciesData::clearSpeciesData();
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonGameView::init()
{
 d = new BosonGameViewPrivate;
 mCanvas = 0;
 d->mInputInitialized = false;
 d->mFPSCounter = 0;

 d->mFovY = 60.0f;

 for (int i = 0; i < 4; i++) {
	d->mViewport[i] = 0;
 }
 for (int i = 0; i < 6; i++) {
	for (int j = 0; j < 4; j++) {
		d->mViewFrustum[i * 4 + j] = 0.0;
	}
 }

 BoMeshRendererManager::manager()->makeRendererCurrent(QString::null);
 BoGroundRendererManager::manager()->makeRendererCurrent(QString::null);
 boWaterManager->setViewFrustum(d->mViewFrustum);


 d->mGameGLMatrices = new BoGLMatrices(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewFrustum, d->mViewport, d->mFovY, d->mAspect);
 BoGroundRendererManager::manager()->setMatrices(&d->mModelviewMatrix, &d->mProjectionMatrix, d->mViewport);
 BoGroundRendererManager::manager()->setViewFrustum(d->mViewFrustum);

 d->mToolTips = new BoGLToolTip(this);
 connect(this, SIGNAL(signalCursorCanvasVectorChanged(const BoVector3Fixed&)),
		d->mToolTips, SLOT(slotSetCursorCanvasVector(const BoVector3Fixed&)));
 d->mGLMiniMap = new BosonGLMiniMap(this);

 d->mSelectionRect = new SelectionRect();
 d->mSelectionRect->setMatrices(d->mGameGLMatrices);

 d->mCursorEdgeScrolling = new BoCursorEdgeScrolling(this);
 d->mCursorEdgeScrolling->setCamera(camera());
 d->mCursorEdgeScrolling->setMatrices(d->mGameGLMatrices);

 mSelection = new BoSelection(this);
 connect(mSelection, SIGNAL(signalSelectionChanged(BoSelection*)),
		this, SIGNAL(signalSelectionChanged(BoSelection*)));
 d->mSelectionGroups = new BoSelectionGroup(10, this);
 d->mSelectionGroups->setSelection(selection());

 d->mScriptConnector = new BosonGameViewScriptConnector(this);



 BoVector4Float lightDif(0.644f, 0.644f, 0.644f, 1.0f);
 BoVector4Float lightAmb(0.502f, 0.502f, 0.502f, 1.0f);
 BoVector3Float lightPos(1545.0, 4755.0, 2600.0);

 BoLight* l = newLight();
 // This is the "main" light, i.e. the Sun. It should always have id 0
 if (l->id() != 0) {
	boWarning() << k_funcinfo << "Main light has id " << l->id() << endl;
 }
 l->setAmbient(lightAmb);
 l->setDiffuse(lightDif);
 l->setSpecular(lightDif);
 l->setDirectional(true);
 l->setPosition3(lightPos);
 l->setEnabled(true);
 boWaterManager->setSun(l);
 BoShader::setSun(l);


 boTextureManager->initOpenGL();
 boWaterManager->initOpenGL();
 boConfig->setBoolValue("TextureFOW", boTextureManager->textureUnits() > 1);


 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << endl;
 }




 initUfoGUI();

 connect(this, SIGNAL(signalWidgetResized()),
		this, SLOT(slotWidgetResized()));
 connect(this, SIGNAL(signalWidgetShown(ufo::UWidgetEvent*)),
		this, SLOT(slotWidgetShown()));
 connect(this, SIGNAL(signalMouseWheel(QWheelEvent*)),
		this, SLOT(slotWheelEvent(QWheelEvent*)));
 connect(this, SIGNAL(signalMouseMoved(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseDragged(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMousePressed(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseReleased(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));

 setMouseEventsEnabled(true, true);
 setKeyEventsEnabled(true);
 setFocusEventsEnabled(true);

 updateOpenGLSettings();
}

void BosonGameView::quitGame()
{
 boDebug() << k_funcinfo << endl;

 d->mMouseMoveDiff.stop();
 d->mCursorEdgeScrolling->quitGame();
 delete d->mMouseIO;
 d->mMouseIO = 0;
 delete d->mInput,
 d->mInput = 0;
 d->mSelectionRect->quitGame();
 d->mSelectionGroups->clearGroups();
 selection()->clear();
 d->mUfoCanvasWidget->quitGame();
 d->mToolTips->hideTip();

 if (d->mGLMiniMap) {
	d->mGLMiniMap->quitGame();
 }

 setLocalPlayerIO(0);
 setCanvas(0);

// setCamera(BoGameCamera()); do not do this! it calls cameraChanged() which generates cell list and all that stuff
 d->mCamera = BoGameCamera(canvas());
 d->mCamera.setCameraChanged(false);  // to prevent generating cell list and all that stuff

 setInputInitialized(false);

 delete d->mLightWidget;
 d->mLightWidget = 0;
}

void BosonGameView::slotWidgetResized()
{
 int w = width();
 int h = height();
 boDebug() << k_funcinfo << w << " " << h << endl;


 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();

 // IMO we don't need zooming. changing posY should be equally .. and even
 // better. no distortion
 GLfloat fovY = d->mFovY; // * d->mCamera.zoomFactor();
 d->mAspect = (float)w / (float)h;
 gluPerspective(fovY, d->mAspect, BO_GL_NEAR_PLANE, BO_GL_FAR_PLANE);

 // cache the composed projection matrix. we'll need it very often in
 // mapCoordinates()
 d->mProjectionMatrix.loadMatrix(GL_PROJECTION_MATRIX);
 extractFrustum(); // projection matrix changed

 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);


 glClearDepth(1.0f);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if 0
 // AB: this does not work dependable :(
 float depth = 1.0f;
 glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
 if (fabsf(depth) > 0.001f && fabsf(depth - 1.0f) > 0.001f && isVisible()) {
	// i really cannot  imagine why this happens - but it does for me.
	// Radeon 9200 with ATI proprietary drivers, version 3.2.8
	boWarning() << k_funcinfo << "clearing depth buffer with 1.0 did caused depth buffer values of " << depth << ", but not 1.0! enabling workaround" << endl;
	Bo3dTools::enableReadDepthBufferWorkaround(depth);
 }
#endif


 if (canvas()) {
	// update the minimap
	camera()->setCameraChanged(true);
 }

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << endl;
 }
}

void BosonGameView::setViewport(int x, int y, GLsizei w, GLsizei h)
{
 glViewport(x, y, w, h);
 // AB: we could use glGetIntegerv(GL_VIEWPORT, d->mViewport); here. But our own
 // version should be the same
 d->mViewport[0] = x;
 d->mViewport[1] = y;
 d->mViewport[2] = w;
 d->mViewport[3] = h;
}

void BosonGameView::extractFrustum()
{
 // modelview or projection matrix was changed (and therefore the frustum).
 GLfloat t;

 // Combine the two matrices (multiply projection by modelview)
 BoMatrix clip(d->mProjectionMatrix);
 clip.multiply(&d->mModelviewMatrix);


 // Extract the numbers for the RIGHT plane
 d->mViewFrustum[0 * 4 + 0] = clip[3] - clip[0];
 d->mViewFrustum[0 * 4 + 1] = clip[7] - clip[4];
 d->mViewFrustum[0 * 4 + 2] = clip[11] - clip[8];
 d->mViewFrustum[0 * 4 + 3] = clip[15] - clip[12];

 // Normalize the result
 // ( AB: normalizing means to make a unit vector, i.e. a vector with length 1! )
 // ( AB: the length of a vector v is |v| == sqrt(v[0]^2 + v[1]^2 + v[2]^2) )
 // ( AB: you can normalize a vector by doing v / |v| )
 t = sqrt(d->mViewFrustum[0 * 4 + 0] * d->mViewFrustum[0 * 4 + 0] +
		d->mViewFrustum[0 * 4 + 1] * d->mViewFrustum[0 * 4 + 1] +
		d->mViewFrustum[0 * 4 + 2] * d->mViewFrustum[0 * 4 + 2]);
 d->mViewFrustum[0 * 4 + 0] /= t;
 d->mViewFrustum[0 * 4 + 1] /= t;
 d->mViewFrustum[0 * 4 + 2] /= t;
 d->mViewFrustum[0 * 4 + 3] /= t;

 // Extract the numbers for the LEFT plane
 d->mViewFrustum[1 * 4 + 0] = clip[3] + clip[0];
 d->mViewFrustum[1 * 4 + 1] = clip[7] + clip[4];
 d->mViewFrustum[1 * 4 + 2] = clip[11] + clip[8];
 d->mViewFrustum[1 * 4 + 3] = clip[15] + clip[12];

 // Normalize the result
 t = sqrt(d->mViewFrustum[1 * 4 + 0] * d->mViewFrustum[1 * 4 + 0] +
		d->mViewFrustum[1 * 4 + 1] * d->mViewFrustum[1 * 4 + 1] +
		d->mViewFrustum[1 * 4 + 2] * d->mViewFrustum[1 * 4 + 2]);
 d->mViewFrustum[1 * 4 + 0] /= t;
 d->mViewFrustum[1 * 4 + 1] /= t;
 d->mViewFrustum[1 * 4 + 2] /= t;
 d->mViewFrustum[1 * 4 + 3] /= t;

 // Extract the BOTTOM plane
 d->mViewFrustum[2 * 4 + 0] = clip[3] + clip[1];
 d->mViewFrustum[2 * 4 + 1] = clip[7] + clip[5];
 d->mViewFrustum[2 * 4 + 2] = clip[11] + clip[9];
 d->mViewFrustum[2 * 4 + 3] = clip[15] + clip[13];

 // Normalize the result
 t = sqrt(d->mViewFrustum[2 * 4 + 0] * d->mViewFrustum[2 * 4 + 0] +
		d->mViewFrustum[2 * 4 + 1] * d->mViewFrustum[2 * 4 + 1] +
		d->mViewFrustum[2 * 4 + 2] * d->mViewFrustum[2 * 4 + 2]);
 d->mViewFrustum[2 * 4 + 0] /= t;
 d->mViewFrustum[2 * 4 + 1] /= t;
 d->mViewFrustum[2 * 4 + 2] /= t;
 d->mViewFrustum[2 * 4 + 3] /= t;

 // Extract the TOP plane
 d->mViewFrustum[3 * 4 + 0] = clip[3] - clip[1];
 d->mViewFrustum[3 * 4 + 1] = clip[7] - clip[5];
 d->mViewFrustum[3 * 4 + 2] = clip[11] - clip[9];
 d->mViewFrustum[3 * 4 + 3] = clip[15] - clip[13];

 // Normalize the result
 t = sqrt(d->mViewFrustum[3 * 4 + 0] * d->mViewFrustum[3 * 4 + 0] +
		d->mViewFrustum[3 * 4 + 1] * d->mViewFrustum[3 * 4 + 1] +
		d->mViewFrustum[3 * 4 + 2] * d->mViewFrustum[3 * 4 + 2]);
 d->mViewFrustum[3 * 4 + 0] /= t;
 d->mViewFrustum[3 * 4 + 1] /= t;
 d->mViewFrustum[3 * 4 + 2] /= t;
 d->mViewFrustum[3 * 4 + 3] /= t;

 // Extract the FAR plane
 d->mViewFrustum[4 * 4 + 0] = clip[3] - clip[2];
 d->mViewFrustum[4 * 4 + 1] = clip[7] - clip[6];
 d->mViewFrustum[4 * 4 + 2] = clip[11] - clip[10];
 d->mViewFrustum[4 * 4 + 3] = clip[15] - clip[14];

 // Normalize the result
 t = sqrt(d->mViewFrustum[4 * 4 + 0] * d->mViewFrustum[4 * 4 + 0] +
		d->mViewFrustum[4 * 4 + 1] * d->mViewFrustum[4 * 4 + 1] +
		d->mViewFrustum[4 * 4 + 2] * d->mViewFrustum[4 * 4 + 2]);
 d->mViewFrustum[4 * 4 + 0] /= t;
 d->mViewFrustum[4 * 4 + 1] /= t;
 d->mViewFrustum[4 * 4 + 2] /= t;
 d->mViewFrustum[4 * 4 + 3] /= t;

 // Extract the NEAR plane
 d->mViewFrustum[5 * 4 + 0] = clip[3] + clip[2];
 d->mViewFrustum[5 * 4 + 1] = clip[7] + clip[6];
 d->mViewFrustum[5 * 4 + 2] = clip[11] + clip[10];
 d->mViewFrustum[5 * 4 + 3] = clip[15] + clip[14];

 // Normalize the result
 t = sqrt(d->mViewFrustum[5 * 4 + 0] * d->mViewFrustum[5 * 4 + 0] +
		d->mViewFrustum[5 * 4 + 1] * d->mViewFrustum[5 * 4 + 1] +
		d->mViewFrustum[5 * 4 + 2] * d->mViewFrustum[5 * 4 + 2]);
 d->mViewFrustum[5 * 4 + 0] /= t;
 d->mViewFrustum[5 * 4 + 1] /= t;
 d->mViewFrustum[5 * 4 + 2] /= t;
 d->mViewFrustum[5 * 4 + 3] /= t;
}

const BoVector3Fixed& BosonGameView::cursorCanvasVector() const
{
 return d->mCursorCanvasVector;
}

const QPoint& BosonGameView::cursorWidgetPos() const
{
 return d->mCursorWidgetPos;
}

void BosonGameView::updateCursorCanvasVector(const QPoint& qtWidgetPos, const QPoint& widgetPos)
{
 GLfloat x = 0.0, y = 0.0, z = 0.0;
 d->mCursorQtWidgetPos = qtWidgetPos;
 d->mCursorWidgetPos = widgetPos;
 mapCoordinates(d->mCursorWidgetPos, &x, &y, &z);
 d->mCursorCanvasVector = BoVector3Fixed(x, -y, z); // AB: are these already real z coordinates?
 emit signalCursorCanvasVectorChanged(d->mCursorCanvasVector);
}

void BosonGameView::setGameFPSCounter(BosonGameFPSCounter* counter)
{
 d->mFPSCounter = counter;
 d->mUfoGameGUI->setGameFPSCounter(gameFPSCounter());
}

BosonGameFPSCounter* BosonGameView::gameFPSCounter() const
{
 return d->mFPSCounter;
}

void BosonGameView::setCamera(const BoGameCamera& camera)
{
 d->mCamera = camera;
}

BoGameCamera* BosonGameView::camera() const
{
 return &d->mCamera;
}

BoAutoGameCamera* BosonGameView::autoCamera() const
{
 return camera()->autoGameCamera();
}

void BosonGameView::cameraChanged()
{
 camera()->applyCameraToScene();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after BoGameCamera::applyCameraToScene()" << endl;
 }

 // the applyCameraToScene() above is the most important call for the modelview matrix.
 // everything else will be discarded by glPushMatrix/glPopMatrix anyway (in
 // paintGL()). So we cache the matrix here, for mapCoordinates() and some other
 // stuff
 d->mModelviewMatrix.loadMatrix(GL_MODELVIEW_MATRIX);

 extractFrustum(); // modelview matrix changed
 BoGroundRenderer* renderer = BoGroundRendererManager::manager()->currentRenderer();
 if (renderer) {
	BosonMap* map = 0;
	if (canvas()) {
		map = canvas()->map();
	}
	renderer->generateCellList(map);
 }

 d->mUfoCanvasWidget->setParticlesDirty(true);

 const QValueVector<BoLight*>* lights = BoLightManager::lights();
 for (unsigned int i = 0; i < lights->size(); i++) {
	if (lights->at(i) != 0) {
		lights->at(i)->refreshPosition();
	}
 }

 // Update position of environmental effects
 QPtrListIterator<BosonEffect> it(*canvas()->effects());
 while (it.current()) {
	if (it.current()->type() == BosonEffect::ParticleEnvironmental) {
		it.current()->setPosition(camera()->cameraPos().toFixed());
	}
	++it;
 }


 boWaterManager->modelviewMatrixChanged(d->mModelviewMatrix);
 boWaterManager->setCameraPos(camera()->cameraPos());
 BoShader::setCameraPos(camera()->cameraPos());

#if 0
 QPoint cellTL; // topleft cell
 QPoint cellTR; // topright cell
 QPoint cellBL; // bottomleft cell
 QPoint cellBR; // bottomright cell
 mapCoordinatesToCell(QPoint(0, 0), &cellTL);
 mapCoordinatesToCell(QPoint(0, d->mViewport[3]), &cellBL);
 mapCoordinatesToCell(QPoint(d->mViewport[2], 0), &cellTR);
 mapCoordinatesToCell(QPoint(d->mViewport[2], d->mViewport[3]), &cellBR);
 emit signalChangeViewport(this, cellTL, cellTR, cellBL, cellBR);
#endif

 updateCursorCanvasVector(d->mCursorQtWidgetPos, cursorWidgetPos());
}

void BosonGameView::advanceCamera()
{
 autoCamera()->advance();
}

void BosonGameView::slotCenterHomeBase()
{
 BO_CHECK_NULL_RET(localPlayerIO());
 slotReCenterDisplay(localPlayerIO()->homeBase());
}

void BosonGameView::slotReCenterDisplay(const QPoint& pos)
{
 BO_CHECK_NULL_RET(camera());
 camera()->setLookAt(BoVector3Float(((float)pos.x()), -((float)pos.y()), 0));
}

bool BosonGameView::mapCoordinates(const QPoint& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth) const
{
 return Bo3dTools::mapCoordinates(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		pos, posX, posY, posZ, useRealDepth);
}

bool BosonGameView::mapDistance(int windx, int windy, GLfloat* dx, GLfloat* dy) const
{
 return Bo3dTools::mapDistance(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		windx, windy, dx, dy);
}

bool BosonGameView::mapCoordinatesToCell(const QPoint& pos, QPoint* cell)
{
 GLfloat x, y, z;
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return false;
 }
 if (!mapCoordinates(pos, &x, &y, &z)) {
	return false;
 }
 y *= -1;
 int cellX = (int)(x);
 int cellY = (int)(y);
 cellX = QMAX(0, QMIN((int)canvas()->mapWidth(), cellX));
 cellY = QMAX(0, QMIN((int)canvas()->mapHeight(), cellY));
 cell->setX(cellX);
 cell->setY(cellY);
 return true;
}

void BosonGameView::scrollBy(int dx, int dy)
{
 BO_CHECK_NULL_RET(camera());
 GLfloat x, y;
 mapDistance(dx, dy, &x, &y);
 camera()->changeLookAt(BoVector3Float(x, y, 0));
}

void BosonGameView::slotScroll(int dir)
{
 switch ((ScrollDirection)dir) {
	case ScrollUp:
		scrollBy(0, -boConfig->uintValue("ArrowKeyStep"));
		break;
	case ScrollRight:
		scrollBy(boConfig->uintValue("ArrowKeyStep"), 0);
		break;
	case ScrollDown:
		scrollBy(0, boConfig->uintValue("ArrowKeyStep"));
		break;
	case ScrollLeft:
		scrollBy(-boConfig->uintValue("ArrowKeyStep"), 0);
		break;
	default:
		return;
 }
}

void BosonGameView::rotate(float delta)
{
 camera()->changeRotation(delta);
}

void BosonGameView::zoom(float delta)
{
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(canvas()->map());

 camera()->changeZ(delta);
}

void BosonGameView::slotResetViewProperties()
{
 BO_CHECK_NULL_RET(canvas());
 d->mFovY = 60.0;
 d->mAspect = 1.0;
 setCamera(BoGameCamera(canvas()));
 slotWidgetResized();
}


void BosonGameView::initUfoGUI()
{
 glPushAttrib(GL_ALL_ATTRIB_BITS);

 // AB: note that BoUfo widgets differ from usual Qt widgets API-wise.
 // You need to create them without a parent and then add them to their parent
 // widget using parent->addWidget(child). This also adds child to the layout of
 // parent.
 // WARNING: ALL widget that are created MUST be added to another widget!
 // Otherwise the created widget won't be deleted!

 BoUfoLayeredPane* layeredPane = new BoUfoLayeredPane();
 layeredPane->setOpaque(false);
 addWidget(layeredPane);

 d->mUfoCanvasWidget = new BosonUfoCanvasWidget();
 d->mUfoCanvasWidget->setGameGLMatrices(d->mGameGLMatrices);
 d->mUfoCanvasWidget->setCamera(&d->mCamera);

 d->mUfoPlacementPreviewWidget = new BosonUfoPlacementPreviewWidget();
 d->mUfoPlacementPreviewWidget->setGameGLMatrices(d->mGameGLMatrices);

 d->mUfoLineVisualizationWidget = new BosonUfoLineVisualizationWidget();
 d->mUfoLineVisualizationWidget->setGameGLMatrices(d->mGameGLMatrices);

 d->mUfoGameGUI = new BosonUfoGameGUI(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewFrustum, d->mViewport);
 d->mUfoGameGUI->setCursorWidgetPos(&d->mCursorWidgetPos);
 d->mUfoGameGUI->setCursorCanvasVector(&d->mCursorCanvasVector);
 d->mUfoGameGUI->setSelection(selection());
 d->mUfoGameGUI->setCanvas(canvas());
 d->mUfoGameGUI->setCamera(camera());
 d->mUfoGameGUI->setGLMiniMap(d->mGLMiniMap);
 connect(this, SIGNAL(signalSelectionChanged(BoSelection*)),
		d->mUfoGameGUI, SIGNAL(signalSelectionChanged(BoSelection*)));

 d->mToolTipLabel = new BoUfoLabel();
 d->mToolTipLabel->setForegroundColor(Qt::white);
 d->mToolTips->setLabel(d->mToolTipLabel);

 d->mUfoCursorWidget = new BosonUfoCursorWidget();
 d->mUfoCursorWidget->setGameGLMatrices(d->mGameGLMatrices);
 d->mUfoCursorWidget->setCursorWidgetPos(&d->mCursorWidgetPos);
 connect(d->mUfoCursorWidget, SIGNAL(signalSetWidgetCursor(BosonCursor*)),
		this, SIGNAL(signalSetWidgetCursor(BosonCursor*)));

 d->mUfoSelectionRectWidget = new BosonUfoSelectionRectWidget();
 d->mUfoSelectionRectWidget->setGameGLMatrices(d->mGameGLMatrices);

 connect(d->mSelectionRect, SIGNAL(signalVisible(bool)),
		d->mUfoSelectionRectWidget, SLOT(slotSelectionRectVisible(bool)));
 connect(d->mSelectionRect, SIGNAL(signalChanged(const QRect&)),
		d->mUfoSelectionRectWidget, SLOT(slotSelectionRectChanged(const QRect&)));

 layeredPane->addWidget(d->mUfoCanvasWidget);
 layeredPane->addWidget(d->mUfoPlacementPreviewWidget);
 layeredPane->addWidget(d->mUfoLineVisualizationWidget);
 layeredPane->addWidget(d->mUfoGameGUI);
 layeredPane->addWidget(d->mToolTipLabel);
 layeredPane->addWidget(d->mUfoCursorWidget);
 layeredPane->addWidget(d->mUfoSelectionRectWidget);

#if 0
 // AB: these are not necessary anymore atm, as we call them in BoUfoWidget.
 d->mUfoCanvasWidget->setMouseEventsEnabled(false, false);
 d->mUfoPlacementPreviewWidget->setMouseEventsEnabled(false, false);
 d->mUfoLineVisualizationWidget->setMouseEventsEnabled(false, false);
 d->mUfoGameGUI->setMouseEventsEnabled(false, false);
 d->mToolTipLabel->setMouseEventsEnabled(false, false);
 d->mUfoCursorWidget->setMouseEventsEnabled(false, false);
 d->mUfoSelectionRectWidget->setMouseEventsEnabled(false, false);
 layeredPane->setMouseEventsEnabled(false, false);
#endif

 glPopAttrib();
}

void BosonGameView::setCanvas(BosonCanvas* canvas)
{
 BosonCanvas* previousCanvas = mCanvas;
 if (mCanvas) {
	disconnect(previousCanvas, 0, this, 0);
	disconnect(previousCanvas, 0, mSelection, 0);
 }
 mCanvas = canvas;
 if (d->mInput) {
	d->mInput->setCanvas(mCanvas);
 }
 d->mUfoGameGUI->setCanvas(mCanvas);
 d->mUfoCanvasWidget->setCanvas(mCanvas);
 d->mUfoPlacementPreviewWidget->setCanvas(mCanvas);
 if (!mCanvas) {
	return;
 }

 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		d->mSelectionGroups, SLOT(slotRemoveItem(BosonItem*)));
 connect(mCanvas, SIGNAL(signalUnitRemoved(Unit*)),
		d->mSelectionGroups, SLOT(slotRemoveUnit(Unit*)));
 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		this, SLOT(slotRemovedItemFromCanvas(BosonItem*)));
 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		mSelection, SLOT(slotRemoveItem(BosonItem*)));
 if (d->mGLMiniMap) {
	if (previousCanvas) {
		disconnect(previousCanvas, 0, d->mGLMiniMap, 0);
		disconnect(d->mGLMiniMap, 0, previousCanvas, 0);
	}
	disconnect(d->mGLMiniMap, 0, this, 0);
	disconnect(d->mGLMiniMap, 0, displayInput(), 0);
	connect(mCanvas, SIGNAL(signalUnitMoved(Unit*, bofixed, bofixed)),
		d->mGLMiniMap, SLOT(slotUnitMoved(Unit*, bofixed, bofixed)));
	connect(mCanvas, SIGNAL(signalUnitRemoved(Unit*)),
		d->mGLMiniMap, SLOT(slotUnitDestroyed(Unit*)));

	connect(d->mGLMiniMap, SIGNAL(signalReCenterView(const QPoint&)),
			this, SLOT(slotReCenterDisplay(const QPoint&)));
	connect(d->mGLMiniMap, SIGNAL(signalMoveSelection(int, int)),
			displayInput(), SLOT(slotMoveSelection(int, int)));
 }

 d->mCamera.setCanvas(mCanvas);
 slotResetViewProperties();

 BO_CHECK_NULL_RET(mCanvas->map());
 boDebug() << k_funcinfo << endl;
 BosonMap* map = mCanvas->map();
 if (d->mGLMiniMap) {
	d->mGLMiniMap->createMap(map, d->mViewport);
 } else {
	BO_NULL_ERROR(d->mGLMiniMap);
 }

 if (!boGame->gameMode()) { // AB: is this valid at this point?
	d->mUfoGameGUI->setGroundTheme(map->groundTheme());
 }

}


void BosonGameView::setLocalPlayerIO(PlayerIO* io)
{
 boDebug() << k_funcinfo << endl;
 resetGameMode();
 if (localPlayerIO() && io) {
	// note that we do this even if io == d->mLocalPlayerIO.
	// we do this to guarantee that _all_ objects are properly initialized
	// with the new player, even if they did not exist yet when the player was set
	// the first time
	boDebug() << k_funcinfo << "already a local playerIO present! unset..." << endl;
	setLocalPlayerIO(0);
 }

 PlayerIO* previousPlayerIO = localPlayerIO();
 d->mLocalPlayerIO = io;
 boDebug() << k_funcinfo << "d->mLocalPlayerIO now: " << d->mLocalPlayerIO << endl;

 d->mToolTips->setPlayerIO(localPlayerIO());
 BoGroundRendererManager::manager()->setLocalPlayerIO(localPlayerIO());

 delete d->mMouseIO;
 d->mMouseIO = 0;

 boWaterManager->setLocalPlayerIO(localPlayerIO());

 if (d->mGLMiniMap) {
	if (previousPlayerIO) {
		previousPlayerIO->disconnect(0, d->mGLMiniMap, 0);
	}
	if (localPlayerIO()) {
		PlayerIO* io = localPlayerIO();
		io->connect(SIGNAL(signalFog(int, int)),
				d->mGLMiniMap, SLOT(slotFog(int, int)));
		io->connect(SIGNAL(signalUnfog(int, int)),
				d->mGLMiniMap, SLOT(slotUnfog(int, int)));
		io->connect(SIGNAL(signalFog(int, int)),
				this, SLOT(slotFog(int, int)));
		io->connect(SIGNAL(signalUnfog(int, int)),
				this, SLOT(slotUnfog(int, int)));
		if (boGame->gameMode()) {
			io->connect(SIGNAL(signalShowMiniMap(bool)),
					d->mGLMiniMap, SLOT(slotShowMiniMap(bool)));
			d->mGLMiniMap->slotShowMiniMap(io->hasMiniMap());
		} else {
			d->mGLMiniMap->slotShowMiniMap(true);
		}
	}
	d->mGLMiniMap->setLocalPlayerIO(localPlayerIO());
 }

 if (previousPlayerIO) {
	// AB: we should probably add such a signal to the IO and use the one in
	// the IO then!
	disconnect((KPlayer*)previousPlayerIO->player(), SIGNAL(signalUnitChanged(Unit*)), this, 0);
 }

 d->mUfoGameGUI->setLocalPlayerIO(localPlayerIO());
 d->mUfoCanvasWidget->setLocalPlayerIO(localPlayerIO());

 if (d->mInput) {
	d->mInput->setLocalPlayerIO(localPlayerIO());
 }

 if (!localPlayerIO()) {
	return;
 }

 // at this point the game mode is already fixed, so calling this here should be
 // ok
 setGameMode(boGame->gameMode());

 // AB: we should probably add such a signal to the IO and use the one in
 // the IO then!
 connect((KPlayer*)localPlayerIO()->player(), SIGNAL(signalUnitChanged(Unit*)),
		this, SLOT(slotUnitChanged(Unit*)));


 BosonLocalPlayerInput* i = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 if (i) {
	if (i->eventListener()) {
		setLocalPlayerScript(i->eventListener()->script());
	}
 } else {
	boError() << k_funcinfo << "local player does not have any BosonLocalPlayerInput!" << endl;
 }

 if (canvas()) {
	slotInitMiniMapFogOfWar();
 }
}

PlayerIO* BosonGameView::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonGameView::setActionCollection(BoUfoActionCollection* c)
{
 d->mActionCollection = c;
}

BoUfoActionCollection* BosonGameView::actionCollection() const
{
 return d->mActionCollection;
}

void BosonGameView::slotUnitChanged(Unit* unit)
{
// FIXME: we might want to place this code into BoSelection directly (cleaner)
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	if (selection()->contains(unit)) {
		selection()->removeUnit(unit);
	}
 }
}

void BosonGameView::slotFog(int x, int y)
{
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (r) {
	r->cellFogChanged(x, y);
 }
}

void BosonGameView::slotUnfog(int x, int y)
{
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (r) {
	r->cellFogChanged(x, y);
 }
}

void BosonGameView::slotRemovedItemFromCanvas(BosonItem* item)
{
 // be careful with the item pointer! this is usually called from the BosonItem
 // destructor, its functions might be destroyed already!
 BO_CHECK_NULL_RET(item);
 d->mToolTips->unsetItem(item);
}

void BosonGameView::setToolTipCreator(int type)
{
 d->mToolTips->setToolTipCreator(type);
}

void BosonGameView::setToolTipUpdatePeriod(int ms)
{
 d->mToolTips->setUpdatePeriod(ms);
}

void BosonGameView::slotInitMiniMapFogOfWar()
{
 BO_CHECK_NULL_RET(d->mGLMiniMap);
 if (boGame->gameMode()) {
	d->mGLMiniMap->initFogOfWar(localPlayerIO());
 } else {
	d->mGLMiniMap->initFogOfWar(0);
 }
}

void BosonGameView::addChatMessage(const QString& message)
{
 d->mUfoGameGUI->addChatMessage(message);
}

void BosonGameView::slotEditorShowPlaceFacilities()
{
 d->mUfoGameGUI->slotShowPlaceFacilities(localPlayerIO());
}

void BosonGameView::slotEditorShowPlaceMobiles()
{
 d->mUfoGameGUI->slotShowPlaceMobiles(localPlayerIO());
}

void BosonGameView::slotEditorShowPlaceGround()
{
 d->mUfoGameGUI->slotShowPlaceGround();
}

void BosonGameView::slotEditorDeleteSelectedUnits()
{
 displayInput()->deleteSelectedUnits();
}

void BosonGameView::slotEditorEditHeight(bool on)
{
 BO_CHECK_NULL_RET(displayInput());
 if (on) {
	BoSpecificAction action;
	action.setType(ActionChangeHeight);
	displayInput()->action(action);
 } else {
	displayInput()->unlockAction();
 }
}

void BosonGameView::slotWidgetShown()
{
 if (displayInput()) {
	displayInput()->updateCursor();
 }
}

void BosonGameView::resetGameMode()
{
 d->mUfoCursorWidget->slotChangeCursor(boConfig->intValue("CursorMode"), boConfig->stringValue("CursorDir"));

 d->mUfoGameGUI->setGameMode(true);

 if (localPlayerIO()) {
	KGameIO* io = localPlayerIO()->findRttiIO(BosonMenuInput::RTTI);
	if (io) {
		localPlayerIO()->removeGameIO(io, true);
	}
 }

 BO_CHECK_NULL_RET(actionCollection());
 actionCollection()->clearActions();
}


void BosonGameView::setGameMode(bool mode)
{
 BO_CHECK_NULL_RET(actionCollection());
 resetGameMode();
 d->mUfoGameGUI->setGameMode(mode);

 if (localPlayerIO()) {
	KGameIO* oldIO = localPlayerIO()->findRttiIO(BosonMenuInput::RTTI);
	if (oldIO) {
		boError() << k_funcinfo << "still an old menuinput IO around!" << endl;
		localPlayerIO()->removeGameIO(oldIO, true);
	}

	BosonMenuInput* io = new BosonMenuInput(mode);
	io->setCamera(camera());
	io->setPlayerIO(localPlayerIO());
	io->setActionCollection(actionCollection());
	localPlayerIO()->addGameIO(io);

	connect(io, SIGNAL(signalToggleStatusbar(bool)),
			this, SIGNAL(signalToggleStatusbar(bool)));
	connect(io, SIGNAL(signalToggleChatVisible()),
			this, SIGNAL(signalToggleChatVisible()));
	connect(io, SIGNAL(signalResetViewProperties()),
			this, SLOT(slotResetViewProperties()));
	connect(io, SIGNAL(signalShowLight0Widget()),
			this, SLOT(slotShowLight0Widget()));
	connect(io, SIGNAL(signalSelectSelectionGroup(int)),
			d->mSelectionGroups, SLOT(slotSelectSelectionGroup(int)));
	connect(io, SIGNAL(signalCreateSelectionGroup(int)),
			d->mSelectionGroups, SLOT(slotCreateSelectionGroup(int)));
	connect(io, SIGNAL(signalPreferencesApply()),
			this, SLOT(slotPreferencesApply()));
	connect(io, SIGNAL(signalUpdateOpenGLSettings()),
			this, SLOT(slotUpdateOpenGLSettings()));
	connect(io, SIGNAL(signalChangeCursor(int, const QString&)),
			d->mUfoCursorWidget, SLOT(slotChangeCursor(int, const QString&)));
	connect(io, SIGNAL(signalEndGame()),
			this, SIGNAL(signalEndGame()));
	connect(io, SIGNAL(signalQuit()),
			this, SIGNAL(signalQuit()));
	connect(io, SIGNAL(signalSaveGame()),
			this, SIGNAL(signalSaveGame()));
	connect(io, SIGNAL(signalEditorChangeLocalPlayer(Player*)),
			this, SIGNAL(signalEditorChangeLocalPlayer(Player*)));
	connect(io, SIGNAL(signalEditorShowPlaceFacilities()),
			this, SLOT(slotEditorShowPlaceFacilities()));
	connect(io, SIGNAL(signalEditorShowPlaceMobiles()),
			this, SLOT(slotEditorShowPlaceMobiles()));
	connect(io, SIGNAL(signalEditorShowPlaceGround()),
			this, SLOT(slotEditorShowPlaceGround()));
	connect(io, SIGNAL(signalEditorDeleteSelectedUnits()),
			this, SLOT(slotEditorDeleteSelectedUnits()));
	connect(io, SIGNAL(signalEditorEditHeight(bool)),
			this, SLOT(slotEditorEditHeight(bool)));

 }
}

void BosonGameView::setLocalPlayerScript(BosonScript* script)
{
 // AB: there is no need to save the pointer atm.
 // we just need to do these connects.
 d->mScriptConnector->connectToScript(script);
}

BoLight* BosonGameView::light(int id) const
{
 return BoLightManager::light(id);
}

BoLight* BosonGameView::newLight()
{
 return BoLightManager::createLight();
}

void BosonGameView::removeLight(int id)
{
 BoLightManager::deleteLight(id);
}

void BosonGameView::slotAdvance(unsigned int advanceCallsCount, bool advanceFlag)
{
 // AB: note that in the big display no game logic must be done!
 // -> this slotAdvance() is here for certain optimizations on rendering, not
 //    for advancing the game itself
 d->mUfoCanvasWidget->setParticlesDirty(true);
 advanceCamera();

 d->mUfoLineVisualizationWidget->slotAdvance(advanceCallsCount, advanceFlag);

#warning FIXME: movie
 // TODO: probably emit a signalGrabMovieFrameAndSave() and implement it in the
 // GL widget
#if 0
 grabMovieFrameAndSave();
#endif
}

void BosonGameView::loadFromXML(const QDomElement& root)
{
 boDebug() << k_funcinfo << endl;

 QDomElement unitGroups = root.namedItem(QString::fromLatin1("UnitGroups")).toElement();
 if (unitGroups.isNull()) {
	boError(260) << k_funcinfo << "no UnitGroups tag" << endl;
	return;
 }
 if (!d->mSelectionGroups->loadFromXML(unitGroups)) {
	boError() << k_funcinfo << "could not load selectiong groups" << endl;
	return;
 }

 QDomElement displays = root.namedItem(QString::fromLatin1("Displays")).toElement();
 if (displays.isNull()) {
	boError(260) << k_funcinfo << "no displays" << endl;
	return;
 }
 QDomElement display = displays.namedItem(QString::fromLatin1("Display")).toElement();
 if (display.isNull()) {
	boError(260) << k_funcinfo << "no display" << endl;
	return;
 }

 // Load camera
 QDomElement cam = display.namedItem(QString::fromLatin1("Camera")).toElement();
 if (!cam.isNull()) {
	camera()->loadFromXML(cam);
 } else {
	boError(260) << k_funcinfo << "no camera" << endl;
 }
 // Load selection
 QDomElement sel = display.namedItem(QString::fromLatin1("Selection")).toElement();
 if (!sel.isNull()) {
	selection()->loadFromXML(sel, true);
 } else {
	boError(260) << k_funcinfo << "no selection" << endl;
	selection()->clear(); // just in case...
 }
}

void BosonGameView::saveAsXML(QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 QDomDocument doc = root.ownerDocument();

 // Save selection groups
 QDomElement unitGroups = doc.createElement(QString::fromLatin1("UnitGroups"));
 d->mSelectionGroups->saveAsXML(root);
 root.appendChild(unitGroups);


 // we use a Displays and a Display tag for historic reasons. we have only one
 // Display these days.
 QDomElement displays = doc.createElement(QString::fromLatin1("Displays"));
 QDomElement display = doc.createElement(QString::fromLatin1("Display"));
 displays.appendChild(display);
 root.appendChild(displays);

 // Save camera
 QDomElement cam = doc.createElement(QString::fromLatin1("Camera"));
 camera()->saveAsXML(cam);
 display.appendChild(cam);
 // Save current selection
 QDomElement sel = doc.createElement(QString::fromLatin1("Selection"));
 selection()->saveAsXML(sel);
 display.appendChild(sel);
}

void BosonGameView::updateOpenGLSettings()
{
 // AB: note there seems to be hardly a difference between flat and smooth
 // shading (in both quality and speed)
 if (boConfig->boolValue("SmoothShading", true)) {
	glShadeModel(GL_SMOOTH);
 } else {
	glShadeModel(GL_FLAT);
 }
}

void BosonGameView::slotUpdateOpenGLSettings()
{
 updateOpenGLSettings();
}

void BosonGameView::slotPreferencesApply()
{
 // apply all options from boConfig to boson, that need to be applied. all
 // options that are stored in boConfig only don't need to be touched.
 // AB: cursor is still a special case and not handled here.
 boDebug() << k_funcinfo << endl;
 setToolTipCreator(boConfig->intValue("ToolTipCreator"));
 setToolTipUpdatePeriod(boConfig->intValue("ToolTipUpdatePeriod"));
 emit signalSetUpdateInterval(boConfig->uintValue("GLUpdateInterval"));
}

void BosonGameView::setFont(const BoFontInfo& font)
{
}

void BosonGameView::slotShowLight0Widget()
{
#warning TODO use BoUfoLightCameraWidget
#if 0
 delete d->mLightWidget;
 d->mLightWidget = new BoLightCameraWidget1(this, true);
 d->mLightWidget->show();
 d->mLightWidget->setLight(light(0), context());
#endif
}

void BosonGameView::moveSelectionRect(const QPoint& widgetPos)
{
 if (d->mSelectionRect->isVisible()) {
	d->mSelectionRect->setEndWidgetPos(widgetPos);
	// would be great here but is a huge performance
	// problem (therefore in releasebutton):
	// selectArea();
 }
}

void BosonGameView::removeSelectionRect(bool replace)
{
 BO_CHECK_NULL_RET(displayInput());
 BO_CHECK_NULL_RET(localPlayerIO());

 if (d->mSelectionRect->isVisible()) {
	// here as there is a performance problem in
	// mousemove:
	BoItemList* items = d->mSelectionRect->items(localPlayerIO(), canvas(), *d->mGameGLMatrices);
	displayInput()->selectArea(items, replace);

	d->mSelectionRect->setVisible(false);
	if (!selection()->isEmpty()) {
		Unit* u = selection()->leader();
		if (localPlayerIO()->ownsUnit(u)) {
			// TODO: do not play sound here
			// instead make virtual and play in derived class
			u->playSound(SoundOrderSelect);
		}
	}
 } else {
	// a simple click on the map
	GLfloat x,y,z;
	if (!mapCoordinates(d->mSelectionRect->startPos(), &x, &y, &z)) {
		boError() << k_funcinfo << "Cannot map coordinates" << endl;
		return;
	}
	BoVector3Fixed canvasVector(x, -y, z);
	Unit* unit = 0;
	if (!canvas()->onCanvas(canvasVector)) {
		return;
	}

	unit = localPlayerIO()->findUnitAt(canvas(), canvasVector);
	if (unit) {
		boDebug() << k_funcinfo << "select unit at " << canvasVector.x() << "," << canvasVector.y() << " (canvas)" << endl;
		displayInput()->selectSingle(unit, replace);
		// cannot be placed into selection() cause we don't have localPlayer
		// there
		if (localPlayerIO()->ownsUnit(unit)) {
			unit->playSound(SoundOrderSelect);
		}
	} else {
		if (replace) {
			selection()->clear();
		}
	}
 }
}

void BosonGameView::setDisplayInput(BosonGameViewInputBase* input)
{
 if (d->mInput) {
	boWarning() << k_funcinfo << "existing input non-NULL" << endl;
	delete d->mInput;
 }
 d->mInput = input;

 if (!d->mInput) {
	return;
 }
 d->mInput->setSelection(selection());

 d->mInput->setCursor(d->mUfoCursorWidget->cursor());

 d->mInput->setCanvas(canvas());
 d->mInput->setLocalPlayerIO(localPlayerIO());
 d->mInput->setCursorCanvasVector(&d->mCursorCanvasVector);

 connect(d->mUfoGameGUI, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)),
		input, SLOT(slotPlaceGround(unsigned int, unsigned char*)));
 connect(d->mUfoGameGUI, SIGNAL(signalPlaceUnit(unsigned int, Player*)),
		input, SLOT(slotPlaceUnit(unsigned int, Player*)));

 connect(input, SIGNAL(signalSetPlacementPreviewData(const UnitProperties*, bool, bool, bool)),
		d->mUfoPlacementPreviewWidget, SLOT(slotSetPlacementPreviewData(const UnitProperties*, bool, bool, bool)));
 connect(input, SIGNAL(signalSetPlacementCellPreviewData(unsigned int, unsigned char*, bool)),
		d->mUfoPlacementPreviewWidget, SLOT(slotSetPlacementCellPreviewData(unsigned int, unsigned char*, bool)));
 connect(input, SIGNAL(signalLockAction(bool, int)),
		d->mUfoPlacementPreviewWidget, SLOT(slotLockAction(bool, int)));
}

BosonGameViewInputBase* BosonGameView::displayInput() const
{
 return d->mInput;
}

bool BosonGameView::isInputInitialized()
{
 return d->mInputInitialized;
}

void BosonGameView::setInputInitialized(bool initialized)
{
 d->mInputInitialized = initialized;
}

void BosonGameView::slotAction(const BoSpecificAction& action)
{
 if (!displayInput()) {
	return;
 }
 displayInput()->action(action);
}


// one day we might support swapping LMB and RMB so let's use defines already to
// make that easier.
#define LEFT_BUTTON LeftButton
#define RIGHT_BUTTON RightButton

void BosonGameView::slotMouseEvent(QMouseEvent* e)
{
 BO_CHECK_NULL_RET(displayInput());
 GLfloat posX = 0.0;
 GLfloat posY = 0.0;
 GLfloat posZ = 0.0;
 if (!mapCoordinates(e->pos(), &posX, &posY, &posZ)) {
	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return;
 }
 BoVector3Fixed canvasVector(posX, -posY, posZ);

 BoMouseEvent event;
 event.setCanvasVector(canvasVector);
 event.setWorldPos(posX, posY, posZ);
 if (e->type() != QEvent::Wheel) {
	event.setQtWidgetPos(e->pos());
	event.setWidgetPos(e->pos());
	event.setControlButton(e->state() & ControlButton);
	event.setShiftButton(e->state() & ShiftButton);
	event.setAltButton(e->state() & AltButton);
 } else {
	QWheelEvent* w = (QWheelEvent*)e;
	event.setQtWidgetPos(w->pos());
	event.setWidgetPos(w->pos());
	event.setControlButton(w->state() & ControlButton);
	event.setShiftButton(w->state() & ShiftButton);
	event.setAltButton(w->state() & AltButton);
 }

 // our actions are done on Button*Release*, not Press. That conflicts with
 // DblClick, so we store whether the last Press event was an actual press event
 // or a double click.
 static ButtonState isDoubleClick = NoButton;


 switch (e->type()) {
	case QEvent::Wheel:
	{
		QWheelEvent* wheel = (QWheelEvent*)e;
		float delta = -wheel->delta() / 120;//120: see QWheelEvent::delta()
		mouseEventWheel(delta, wheel->orientation(), event);
		wheel->accept();
		break;
	}
	case QEvent::MouseMove:
	{
		isDoubleClick = NoButton; // when the mouse was pressed twice but the second press is hold down and moved then it isn't a double click anymore.
		mouseEventMove(e->state(), event);
		e->accept();
		break;
	}
	case QEvent::MouseButtonDblClick:
	{
		isDoubleClick = e->button();
		// actual actions will happen on ButtonRelease!
		e->accept();
		break;
	}
	case QEvent::MouseButtonPress:
	{
		// no action should happen here!
		isDoubleClick = NoButton;
		if (e->button() == LEFT_BUTTON) {
			d->mSelectionRect->setStartWidgetPos(e->pos());
		} else if (e->button() == MidButton) {
			// nothing to be done here
		} else if (e->button() == RIGHT_BUTTON) {
			if (boConfig->boolValue("RMBMove")) {
				//AB: this might be obsolete..
				d->mMouseMoveDiff.moveEvent(e->pos()); // set position, but do not yet start!
			}
		}
		e->accept();
		break;
	}
	case QEvent::MouseButtonRelease:
	{
		if (e->button() == isDoubleClick) {
			mouseEventReleaseDouble(e->button(), event);
		} else {
			mouseEventRelease(e->button(), event);
		}
		e->accept();
		break;
	}
	default:
		boWarning() << k_funcinfo << "unexpected mouse event " << e->type() << endl;
		e->ignore();
		return;
 }
}

void BosonGameView::slotWheelEvent(QWheelEvent* e)
{
 // AB: very unclean. however it makes things easy :-)
 QEvent* ev = (QEvent*)e;
 slotMouseEvent((QMouseEvent*)ev);
}

void BosonGameView::mouseEventWheel(float delta, Orientation orientation, const BoMouseEvent& boEvent)
{
 int action;
 if (boEvent.shiftButton()) {
	action = boConfig->intValue("MouseWheelShiftAction");
 } else {
	action = boConfig->intValue("MouseWheelAction");
 }
 switch (action) {
	case CameraMove:
	{
		int scrollX, scrollY;
		if (boEvent.controlButton()) {
			scrollX = width();
			scrollY = height();
		} else {
			scrollX = 20;
			scrollY = 20;
			delta *= QApplication::wheelScrollLines();
		}
		if (orientation == Horizontal) {
			scrollX *= (int)delta;
			scrollY = 0;
		} else {
			scrollX = 0;
			scrollY *= (int)delta;
		}
		scrollBy(scrollX, scrollY);
		break;
	}
	case CameraZoom:
		if (boEvent.controlButton()) {
			delta *= 3;
		} else {
			delta *= 1; // no effect, btw
		}
		zoom(delta);
		break;
	case CameraRotate:
		if (boEvent.controlButton()) {
			delta *= 30;
		} else {
			delta *= 10;
		}
		rotate(delta);
		break;
	default:
	{
		boWarning() << k_funcinfo << "invalid wheel action: " << action << endl;
		break;
	}
 }
}

void BosonGameView::mouseEventMove(int buttonState, const BoMouseEvent& event)
{
 float posX, posY, posZ;
 event.worldPos(&posX, &posY, &posZ);
 d->mMouseMoveDiff.moveEvent(event.widgetPos());
 if (event.altButton()) {
	// The Alt button is the camera modifier in boson.
	// Better don't do important stuff (like unit movement
	// or selections) here, since a single push on Alt gives
	// the focus to the menu which might be very confusing
	// during a game.
	if (buttonState & LEFT_BUTTON) {
		d->mMouseMoveDiff.start(LEFT_BUTTON);
		camera()->changeZ(d->mMouseMoveDiff.dy());
	} else if (buttonState & RIGHT_BUTTON) {
		d->mMouseMoveDiff.start(RIGHT_BUTTON);
		camera()->changeRotation(d->mMouseMoveDiff.dx());
		camera()->changeRadius(d->mMouseMoveDiff.dy());
	}
 } else if (buttonState & LEFT_BUTTON) {
	if (!displayInput()->actionLocked()) {
		// selection rect gets drawn.
		// other modifiers are ignored
		d->mSelectionRect->setVisible(true);
		moveSelectionRect(event.widgetPos());
	} else if (!boGame->gameMode() && displayInput()->actionType() != ActionChangeHeight) {
		// In editor mode, try to place unit/ground whenever mouse moves. This
		//  enables you to edit big areas easily. But it's not done for height
		//  changing (yet).
		displayInput()->actionClicked(event);
	}
 } else if (buttonState & RIGHT_BUTTON) {
	// RMB+MouseMove does *not* depend on CTRL or Shift. the
	// map is moved in all cases (currently - we have some
	// free buttons here :))
	if (boConfig->boolValue("RMBMove")) {
		// problem is that QCursor::setPos() also causes
		// a mouse move event. we can use this hack in
		// order to check whether it is a real mouse
		// move event or we caused it here.
		// TODO: use d->mMouseMoveDiff.x()/y() in
		// paintGL() for the cursor, not QCursor::pos()
//		static bool a = false;
//		if (a) {
//			a = false;
//			break;
//		}
//		a = true;moveLookAtBy
//		QPoint pos = mapToGlobal(QPoint(d->mMouseMoveDiff.oldX(), d->mMouseMoveDiff.oldY()));
//		QCursor::setPos(pos);

		// modifiers are ignored.
		d->mMouseMoveDiff.start(RIGHT_BUTTON);
		GLfloat dx, dy;
		int moveX = d->mMouseMoveDiff.dx();
		int moveY = d->mMouseMoveDiff.dy();
		mapDistance(moveX, moveY, &dx, &dy);
		camera()->changeLookAt(BoVector3Float(dx, dy, 0));
	} else {
		d->mMouseMoveDiff.stop();
	}
 } else if (buttonState & MidButton) {
	// currently unused
 }
 updateCursorCanvasVector(event.qtWidgetPos(), event.widgetPos());
 displayInput()->setPlacementFreePlacement(event.controlButton());
 displayInput()->setPlacementDisableCollisions(event.shiftButton());
 displayInput()->updatePlacementPreviewData();

 // AB: we might want to use a timer here instead - then we would also be able
 // to change the cursor type when units move under the cursor. i don't want to
 // call updateCursor() from BosonCanvas::slotAdvance() as it would get called
 // too often then
 displayInput()->updateCursor();
}

void BosonGameView::mouseEventRelease(ButtonState button, const BoMouseEvent& event)
{
 switch (button) {
	case LEFT_BUTTON:
	{
		if (displayInput()->actionLocked()) {
			// basically the same as a normal RMB
			displayInput()->actionClicked(event);
		} else if (event.shiftButton()) {
			BoItemList* items = d->mSelectionRect->items(localPlayerIO(), canvas(), *d->mGameGLMatrices);
			displayInput()->unselectArea(items);
			d->mSelectionRect->setVisible(false);
		} else if (event.controlButton()) {
			removeSelectionRect(false);
		} else {
			// select the unit(s) below the cursor/inside the
			// selection rect
			removeSelectionRect(true);
		}
		break;
	}
	case MidButton:
	{
		// we ignore all modifiers here, currently.
		if (boConfig->boolValue("MMBMove")) {
			float posX, posY, posZ;
			event.worldPos(&posX, &posY, &posZ);
			int cellX, cellY;
			cellX = (int)(posX);
			cellY = (int)(-posY);
			slotReCenterDisplay(QPoint(cellX, cellY));
			displayInput()->updateCursor();
		}
		break;
	}
	case RIGHT_BUTTON:
	{
		if (!d->mMouseMoveDiff.isStopped()) {
			d->mMouseMoveDiff.stop();
		} else if (displayInput()->actionLocked()) {
			displayInput()->unlockAction();
			displayInput()->updateCursor();
		} else {
			displayInput()->actionClicked(event);
		}
		break;
	}
	default:
		boError() << k_funcinfo << "invalid mouse button " << button << endl;
		break;
 }
}

void BosonGameView::mouseEventReleaseDouble(ButtonState button, const BoMouseEvent& event)
{
 switch (button) {
	case LEFT_BUTTON:
	{
		// we ignore UnitAction is locked here
		// currently!
		bool replace = !event.controlButton();
		bool onScreenOnly = !event.shiftButton();
		Unit* unit = localPlayerIO()->findUnitAt(canvas(), event.canvasVector());
		if (unit) {
			if (onScreenOnly) {
				boDebug() << k_funcinfo << "TODO: select only those that are currently on the screen!" << endl;
			}
			if (!displayInput()->selectAll(unit->unitProperties(), replace)) {
				displayInput()->selectSingle(unit, replace);
			}
		}
		break;
	}
	case MidButton:
	case RIGHT_BUTTON:
	{
		// we ignore all other (RMB, MMB) for now. we
		// might use this one day.
		break;
	}
	default:
		boError() << k_funcinfo << "invalid mouse button " << button << endl;
		break;
 }
}

void BosonGameView::paint()
{
 if (!isVisible()) {
	return;
 }
 BO_CHECK_NULL_RET(camera());

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at start of " << k_funcinfo << endl;
 }

 if (boGame->delayedMessageCount() >= 10) {
	// if >= 10 messages are delayed, we'll render at most less frames per
	// second, so that we have more CPU time to deliver these messages
	// TODO: also do that if at least 2 (even 1?) advance messages got
	// delayed!

	long long time = gameFPSCounter()->counter()->timeSinceLastFrame(true);
//	boDebug() << (int)time << endl;
	if (time > 0 && time < 500000) {
//		boDebug() << "skipping" << endl;
		// now we need to let the counter know, that this frame is being skipped
		gameFPSCounter()->skipFrame();
		return;
	}
 }


 glPushAttrib(GL_ALL_ATTRIB_BITS);

 boTextureManager->invalidateCache();

 // AB: we us a viewport to make life in our 3d widgets a _lot_ easier. with
 // this viewport (0,0) is bottom-left and (width(), height) top-right of the
 // widget, as we would expect in OpenGL. the menubar is "cut off", we do not
 // have to take it into account here.
 glPushAttrib(GL_VIEWPORT_BIT);
 QRect rect = widgetViewportRect();
 // TODO: It'd be sufficient to just set d->mViewport
 setViewport(rect.x(), rect.y(), rect.width(), rect.height());
 glPopAttrib(); // normal ufo widgets dont need/like our viewport

 // apply the camera to the scene, if necessary. we do not need to keep the
 // matrix, as it is saved into d->mModelviewMatrix.
 glPushMatrix();
 if (camera()->isCameraChanged()) {
	cameraChanged();
 }


 glLoadMatrixf(d->mModelviewMatrix.data());
 if (light(0)) {
	// AB: lights may depend on the modelview matrix, therefore we must
	// update the light at least once with a valid 3d modelview matrix.
	//
	// TODO: note that probably we should do that for _all_ lights, not just
	// the first one!
	// (however usually only light(0) is used, so I leave it up to someone
	// else to fix this issue)
	light(0)->updateStates();
 }


 glPopMatrix();

 d->mUfoGameGUI->updateUfoLabels();

 // the original implementation paints the children
 BoUfoCustomWidget::paint();

 glPopAttrib();

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at end of " << k_funcinfo << endl;
 }
}

void BosonGameView::paintWidget()
{
 // nothing to do here, everything is done in child widgets.
 // all GL calls that should apply to child widgets as well should be made in
 // paint(), not here
}








BosonGameViewScriptConnector::BosonGameViewScriptConnector(BosonGameView* parent)
	: QObject(parent, "script_to_display_connector")
{
 mDisplay = parent;
 if (!mDisplay) {
	BO_NULL_ERROR(mDisplay);
 }
}

BosonGameViewScriptConnector::~BosonGameViewScriptConnector()
{
}

void BosonGameViewScriptConnector::reconnect(const QObject* sender, const char* signal, const QObject* receiver, const char* slot)
{
 // make sure noone else connects to that signal
 disconnect(sender, signal, 0, 0);
 connect(sender, signal, receiver, slot);
}

void BosonGameViewScriptConnector::connectToScript(BosonScript* script)
{
 BO_CHECK_NULL_RET(script);
 BosonScriptInterface* i = script->interface();
 BO_CHECK_NULL_RET(i);

 // AB: the slots often provide the return value for a signal, so there must be
 // at most 1 slot to a signal. reconnect() ensures that.

 reconnect(i, SIGNAL(signalAddLight(int*)), this, SLOT(slotAddLight(int*)));
 reconnect(i, SIGNAL(signalRemoveLight(int)), this, SLOT(slotRemoveLight(int)));
 reconnect(i, SIGNAL(signalGetLightPos(int, BoVector4Float*)),
		this, SLOT(slotGetLightPos(int, BoVector4Float*)));
 reconnect(i, SIGNAL(signalGetLightAmbient(int, BoVector4Float*)),
		this, SLOT(slotGetLightAmbient(int, BoVector4Float*)));
 reconnect(i, SIGNAL(signalGetLightDiffuse(int, BoVector4Float*)),
		this, SLOT(slotGetLightDiffuse(int, BoVector4Float*)));
 reconnect(i, SIGNAL(signalGetLightSpecular(int, BoVector4Float*)),
		this, SLOT(slotGetLightSpecular(int, BoVector4Float*)));
 reconnect(i, SIGNAL(signalGetLightAttenuation(int, BoVector3Float*)),
		this, SLOT(slotGetLightAttenuation(int, BoVector3Float*)));
 reconnect(i, SIGNAL(signalGetLightEnabled(int, bool*)),
		this, SLOT(slotGetLightEnabled(int, bool*)));
 reconnect(i, SIGNAL(signalSetLightPos(int, const BoVector4Float&)),
		this, SLOT(slotSetLightPos(int, const BoVector4Float&)));
 reconnect(i, SIGNAL(signalSetLightAmbient(int, const BoVector4Float&)),
		this, SLOT(slotSetLightAmbient(int, const BoVector4Float&)));
 reconnect(i, SIGNAL(signalSetLightDiffuse(int, const BoVector4Float&)),
		this, SLOT(slotSetLightDiffuse(int, const BoVector4Float&)));
 reconnect(i, SIGNAL(signalSetLightSpecular(int, const BoVector4Float&)),
		this, SLOT(slotSetLightSpecular(int, const BoVector4Float&)));
 reconnect(i, SIGNAL(signalSetLightAttenuation(int, const BoVector3Float&)),
		this, SLOT(slotSetLightAttenuation(int, const BoVector3Float&)));
 reconnect(i, SIGNAL(signalSetLightEnabled(int, bool)),
		this, SLOT(slotSetLightEnabled(int, bool)));

 reconnect(i, SIGNAL(signalGetCameraPos(BoVector3Float*)),
		this, SLOT(slotGetCameraPos(BoVector3Float*)));
 reconnect(i, SIGNAL(signalGetCameraLookAt(BoVector3Float*)),
		this, SLOT(slotGetCameraLookAt(BoVector3Float*)));
 reconnect(i, SIGNAL(signalGetCameraUp(BoVector3Float*)),
		this, SLOT(slotGetCameraUp(BoVector3Float*)));
 reconnect(i, SIGNAL(signalGetCameraRotation(float*)),
		this, SLOT(slotGetCameraRotation(float*)));
 reconnect(i, SIGNAL(signalGetCameraRadius(float*)),
		this, SLOT(slotGetCameraRadius(float*)));
 reconnect(i, SIGNAL(signalGetCameraZ(float*)),
		this, SLOT(slotGetCameraZ(float*)));
 reconnect(i, SIGNAL(signalSetUseCameraLimits(bool)),
		this, SLOT(slotSetUseCameraLimits(bool)));
 reconnect(i, SIGNAL(signalSetCameraFreeMovement(bool)),
		this, SLOT(slotSetCameraFreeMovement(bool)));
 reconnect(i, SIGNAL(signalSetCameraPos(const BoVector3Float&)),
		this, SLOT(slotSetCameraPos(const BoVector3Float&)));
 reconnect(i, SIGNAL(signalSetCameraLookAt(const BoVector3Float&)),
		this, SLOT(slotSetCameraLookAt(const BoVector3Float&)));
 reconnect(i, SIGNAL(signalSetCameraUp(const BoVector3Float&)),
		this, SLOT(slotSetCameraUp(const BoVector3Float&)));
 reconnect(i, SIGNAL(signalAddCameraPosPoint(const BoVector3Float&, float)),
		this, SLOT(slotAddCameraPosPoint(const BoVector3Float&, float)));
 reconnect(i, SIGNAL(signalAddCameraLookAtPoint(const BoVector3Float&, float)),
		this, SLOT(slotAddCameraLookAtPoint(const BoVector3Float&, float)));
 reconnect(i, SIGNAL(signalAddCameraUpPoint(const BoVector3Float&, float)),
		this, SLOT(slotAddCameraUpPoint(const BoVector3Float&, float)));
 reconnect(i, SIGNAL(signalSetCameraRotation(float)),
		this, SLOT(slotSetCameraRotation(float)));
 reconnect(i, SIGNAL(signalSetCameraRadius(float)),
		this, SLOT(slotSetCameraRadius(float)));
 reconnect(i, SIGNAL(signalSetCameraZ(float)),
		this, SLOT(slotSetCameraZ(float)));
 reconnect(i, SIGNAL(signalSetCameraMoveMode(int)),
		this, SLOT(slotSetCameraMoveMode(int)));
 reconnect(i, SIGNAL(signalSetCameraInterpolationMode(int)),
		this, SLOT(slotSetCameraInterpolationMode(int)));
 reconnect(i, SIGNAL(signalCommitCameraChanges(int)),
		this, SLOT(slotCommitCameraChanges(int)));
 reconnect(i, SIGNAL(signalSetAcceptUserInput(bool)),
		this, SLOT(slotSetAcceptUserInput(bool)));
}

void BosonGameViewScriptConnector::slotAddLight(int* id)
{
 BoLight* l = mDisplay->newLight();
 if (!l) {
	*id = -1;
 } else {
	*id = l->id();
 }
}

void BosonGameViewScriptConnector::slotRemoveLight(int id)
{
 mDisplay->removeLight(id);
}

void BosonGameViewScriptConnector::slotGetLightPos(int id, BoVector4Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector4Float();
	return;
 } else {
	*v = l->position();
 }
}

void BosonGameViewScriptConnector::slotGetLightAmbient(int id, BoVector4Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector4Float();
	return;
 } else {
	*v = l->ambient();
 }
}

void BosonGameViewScriptConnector::slotGetLightDiffuse(int id, BoVector4Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector4Float();
	return;
 } else {
	*v = l->diffuse();
 }
}

void BosonGameViewScriptConnector::slotGetLightSpecular(int id, BoVector4Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector4Float();
	return;
 } else {
	*v = l->specular();
 }
}

void BosonGameViewScriptConnector::slotGetLightAttenuation(int id, BoVector3Float* v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*v = BoVector3Float();
	return;
 } else {
	*v = l->attenuation();
 }
}

void BosonGameViewScriptConnector::slotGetLightEnabled(int id, bool* e)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	*e = false;
	return;
 } else {
	*e = l->isEnabled();
 }
}

void BosonGameViewScriptConnector::slotSetLightPos(int id, const BoVector4Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setPosition(v);
 }
}

void BosonGameViewScriptConnector::slotSetLightAmbient(int id, const BoVector4Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setAmbient(v);
 }
}

void BosonGameViewScriptConnector::slotSetLightDiffuse(int id, const BoVector4Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setDiffuse(v);
 }
}

void BosonGameViewScriptConnector::slotSetLightSpecular(int id, const BoVector4Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setSpecular(v);
 }
}

void BosonGameViewScriptConnector::slotSetLightAttenuation(int id, const BoVector3Float& v)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setAttenuation(v);
 }
}

void BosonGameViewScriptConnector::slotSetLightEnabled(int id, bool e)
{
 BoLight* l = mDisplay->light(id);
 if (!l) {
	boError() << k_funcinfo << "no light with id " << id << endl;
	return;
 } else {
	l->setEnabled(e);
 }
}

void BosonGameViewScriptConnector::slotGetCameraPos(BoVector3Float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->cameraPos();
}

void BosonGameViewScriptConnector::slotGetCameraLookAt(BoVector3Float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->lookAt();
}

void BosonGameViewScriptConnector::slotGetCameraUp(BoVector3Float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->up();
}

void BosonGameViewScriptConnector::slotGetCameraRotation(float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->rotation();
}

void BosonGameViewScriptConnector::slotGetCameraRadius(float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->radius();
}

void BosonGameViewScriptConnector::slotGetCameraZ(float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->z();
}

void BosonGameViewScriptConnector::slotSetUseCameraLimits(bool u)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 mDisplay->camera()->setUseLimits(u);
}

void BosonGameViewScriptConnector::slotSetCameraFreeMovement(bool u)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 mDisplay->camera()->setFreeMovement(u);
}

void BosonGameViewScriptConnector::slotSetCameraPos(const BoVector3Float& v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setCameraPos(v);
}

void BosonGameViewScriptConnector::slotSetCameraLookAt(const BoVector3Float& v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setLookAt(v);
}

void BosonGameViewScriptConnector::slotSetCameraUp(const BoVector3Float& v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setUp(v);
}

void BosonGameViewScriptConnector::slotAddCameraPosPoint(const BoVector3Float& v, float time)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->addCameraPosPoint(v, time);
}

void BosonGameViewScriptConnector::slotAddCameraLookAtPoint(const BoVector3Float& v, float time)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->addLookAtPoint(v, time);
}

void BosonGameViewScriptConnector::slotAddCameraUpPoint(const BoVector3Float& v, float time)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->addUpPoint(v, time);
}

void BosonGameViewScriptConnector::slotSetCameraRotation(float v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setRotation(v);
}

void BosonGameViewScriptConnector::slotSetCameraRadius(float v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setRadius(v);
}

void BosonGameViewScriptConnector::slotSetCameraZ(float v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setZ(v);
}

void BosonGameViewScriptConnector::slotSetCameraMoveMode(int v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setMoveMode((BoAutoCamera::MoveMode)v);
}

void BosonGameViewScriptConnector::slotSetCameraInterpolationMode(int v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setInterpolationMode((BoAutoCamera::InterpolationMode)v);
}

void BosonGameViewScriptConnector::slotCommitCameraChanges(int ticks)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->commitChanges(ticks);
}

void BosonGameViewScriptConnector::slotSetAcceptUserInput(bool accept)
{
 QPtrList<KGameIO>* iolist = mDisplay->localPlayerIO()->ioList();
 QPtrListIterator<KGameIO> it(*iolist);
 while (it.current()) {
	(*it)->blockSignals(!accept);
	++it;
 }
}

