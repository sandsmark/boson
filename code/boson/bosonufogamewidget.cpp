/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonufogamewidget.h"
#include "bosonufogamewidget.moc"

#include "no_player.h"
#include "bomath.h"
#include "bo3dtools.h"
#include "bosonconfig.h"
#include "boson.h"
#include "bosoncanvas.h"
#include "bosoncanvasstatistics.h"
#include "bogroundrenderer.h"
#include "bogroundrenderermanager.h"
#include "bomeshrenderermanager.h"
#include "botexture.h"
#include "bocamera.h"
#include "bosonufochat.h"
#include "bosonufominimap.h"
#include "commandframe/bosoncommandframe.h"
#include "bodebug.h"
#include "bosonmessage.h"
#include "boselection.h"
#include "playerio.h"
#include "bosonbigdisplaybase.h"
#include "bosonlocalplayerinput.h"
#include "unit.h"
#include "bosonpath.h"
#include "cell.h"
#include "bowater.h"

#include <klocale.h>

#include <qcursor.h>

/**
 * @return A string that displays @p plane. The plane consists of a normal
 * vector in the first 3 numbers and the distance from the origin in the 4th
 * number.
 **/
static QString planeDebugString(const float* plane)
{
 return QString("((%1,%2,%3),%4)").arg(plane[0]).arg(plane[1]).arg(plane[2]).arg(plane[3]);
}


class BosonUfoGameWidgetPrivate
{
public:
	BosonUfoGameWidgetPrivate(const BoMatrix& modelview, const BoMatrix& projection,
			const GLfloat* viewFrustum, const GLint* viewport)
		: mModelviewMatrix(modelview), mProjectionMatrix(projection),
		mViewFrustum(viewFrustum), mViewport(viewport)
	{
		mDisplay = 0;
		mLocalPlayerIO = 0;

		mResourcesBox = 0;
		mMineralsLabel = 0;
		mOilLabel = 0;
		mFPSLabel = 0;
		mGroundRendererDebug = 0;
		mMapCoordinates = 0;
		mMapCoordinatesWorldLabel = 0;
		mMapCoordinatesCanvasLabel = 0;
		mMapCoordinatesWindowLabel = 0;
		mPathFinderDebug = 0;
		mMatricesDebug = 0;
		mMatricesDebugProjection = 0;
		mMatricesDebugModelview = 0;
		mMatricesDebugProjMod = 0;
		mItemWorkStatistics = 0;
		mOpenGLCamera = 0;
		mRenderCounts = 0;
		mAdvanceCalls = 0;
		mTextureMemory = 0;
		mGamePaused = 0;
		mUfoChat = 0;
		mUfoMiniMap = 0;
		mUfoCommandFrame = 0;
	}

	BosonBigDisplayBase* mDisplay;
	const BoMatrix& mModelviewMatrix;
	const BoMatrix& mProjectionMatrix;
	const GLfloat* mViewFrustum;
	const GLint* mViewport;
	PlayerIO* mLocalPlayerIO;

	BoUfoHBox* mResourcesBox;
	BoUfoLabel* mMineralsLabel;
	BoUfoLabel* mOilLabel;
	BoUfoLabel* mFPSLabel;
	BoUfoLabel* mGroundRendererDebug;
	BoUfoVBox* mMapCoordinates;
	BoUfoLabel* mMapCoordinatesWorldLabel;
	BoUfoLabel* mMapCoordinatesCanvasLabel;
	BoUfoLabel* mMapCoordinatesWindowLabel;
	BoUfoLabel* mPathFinderDebug;
	BoUfoVBox* mMatricesDebug;
	BoUfoMatrix* mMatricesDebugProjection;
	BoUfoMatrix* mMatricesDebugModelview;
	BoUfoMatrix* mMatricesDebugProjMod;
	BoUfoMatrix* mMatricesDebugProjModInv;
	BoUfoLabel* mMatricesDebugText;
	BoUfoLabel* mItemWorkStatistics;
	BoUfoLabel* mOpenGLCamera;
	BoUfoLabel* mRenderCounts;
	BoUfoLabel* mAdvanceCalls;
	BoUfoLabel* mTextureMemory;
	BoUfoLabel* mGamePaused;
	BosonUfoChat* mUfoChat;
	BosonUfoMiniMap* mUfoMiniMap;
	BosonCommandFrame* mUfoCommandFrame;
};

BosonUfoGameWidget::BosonUfoGameWidget(const BoMatrix& modelview, const BoMatrix& projection,
		const GLfloat* viewFrustum, const GLint* viewport, BosonBigDisplayBase* display)
	: BoUfoWidget()
{
 d = new BosonUfoGameWidgetPrivate(modelview, projection, viewFrustum, viewport);
 d->mDisplay = display;
 initUfoWidgets();
}

BosonUfoGameWidget::~BosonUfoGameWidget()
{
 delete d;
}


void BosonUfoGameWidget::initUfoWidgets()
{
 // AB: note that BoUfo widgets differ from usual Qt widgets API-wise.
 // You need to create them without a parent and then add them to their parent
 // widget using parent->addWidget(child). This also adds child to the layout of
 // parent.
 // WARNING: ALL widget that are created MUST be added to another widget!
 // Otherwise the created widget won't be deleted!

 // A "UBorderLayout" layout requires it's children to have so-call
 // "constraints". These specify where the widget is placed (north, south, ...)
 setLayoutClass(BoUfoWidget::UBorderLayout);

 BoUfoWidget* north = new BoUfoWidget();
 north->setConstraints("north");
 addWidget(north);
 north->setLayoutClass(BoUfoWidget::UBorderLayout);

 BoUfoVBox* south = new BoUfoVBox();
 south->setConstraints("south");
 addWidget(south);

 BoUfoVBox* center = new BoUfoVBox();
 center->setConstraints("center");
 addWidget(center);

 BoUfoVBox* northEast = new BoUfoVBox();
 northEast->setConstraints("east");
 north->addWidget(northEast);

 BoUfoVBox* northWest = new BoUfoVBox();
 northWest->setConstraints("west");
 north->addWidget(northWest);

 BoUfoVBox* northCenter = new BoUfoVBox();
 northCenter->setConstraints("center");
 north->addWidget(northCenter);

 // add your widgets to northEast, northWest or northCenter, not to north
 north = 0;

 d->mResourcesBox = new BoUfoHBox();
 BoUfoVBox* resourcesLabelsBox = new BoUfoVBox();
 BoUfoVBox* resourcesValuesBox = new BoUfoVBox();
 BoUfoLabel* mineralsLabel = new BoUfoLabel(i18n("Minerals:"));
 BoUfoLabel* oilLabel = new BoUfoLabel(i18n("Oil:"));
 d->mMineralsLabel = new BoUfoLabel();
 d->mOilLabel = new BoUfoLabel();
 resourcesLabelsBox->addWidget(mineralsLabel);
 resourcesLabelsBox->addWidget(oilLabel);
 resourcesValuesBox->addWidget(d->mMineralsLabel);
 resourcesValuesBox->addWidget(d->mOilLabel);
 d->mResourcesBox->addWidget(resourcesLabelsBox);
 d->mResourcesBox->addWidget(resourcesValuesBox);
 northEast->addWidget(d->mResourcesBox);
 BoUfoWidget* dummy = new BoUfoWidget();
 northEast->addWidget(dummy); // a "stretch" widget that takes additional space if available

 d->mFPSLabel = new BoUfoLabel();
 northEast->addWidget(d->mFPSLabel);

 d->mGroundRendererDebug = new BoUfoLabel();
 northEast->addWidget(d->mGroundRendererDebug);

 d->mMapCoordinates = new BoUfoVBox();
 d->mMapCoordinatesWorldLabel = new BoUfoLabel();
 d->mMapCoordinatesCanvasLabel = new BoUfoLabel();
 d->mMapCoordinatesWindowLabel = new BoUfoLabel();
 d->mMapCoordinates->addWidget(d->mMapCoordinatesWorldLabel);
 d->mMapCoordinates->addWidget(d->mMapCoordinatesCanvasLabel);
 d->mMapCoordinates->addWidget(d->mMapCoordinatesWindowLabel);
 northEast->addWidget(d->mMapCoordinates);

 d->mPathFinderDebug = new BoUfoLabel();
 northEast->addWidget(d->mPathFinderDebug);

 d->mUfoMiniMap = new BosonUfoMiniMap();
// d->mUfoMiniMap->setMiniMap(d->mGLMiniMap);
 northWest->addWidget(d->mUfoMiniMap);

 d->mUfoCommandFrame = new BosonCommandFrame();
 connect(display(), SIGNAL(signalSelectionChanged(BoSelection*)),
		d->mUfoCommandFrame, SLOT(slotSelectionChanged(BoSelection*)));
 connect(d->mUfoCommandFrame, SIGNAL(signalSelectUnit(Unit*)),
		selection(), SLOT(slotSelectSingleUnit(Unit*)));
 connect(d->mUfoCommandFrame, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)),
		this, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)));
 connect(d->mUfoCommandFrame, SIGNAL(signalPlaceUnit(unsigned int, Player*)),
		this, SIGNAL(signalPlaceUnit(unsigned int, Player*)));
 d->mUfoCommandFrame->slotSelectionChanged(selection());

 // TODO: move to somewhere more useful, not to northWest
 northWest->addWidget(d->mUfoCommandFrame);

 d->mMatricesDebug = new BoUfoVBox();
 d->mMatricesDebugProjection = new BoUfoMatrix();
 d->mMatricesDebugModelview = new BoUfoMatrix();
 d->mMatricesDebugProjMod = new BoUfoMatrix();
 d->mMatricesDebugProjModInv = new BoUfoMatrix();
 d->mMatricesDebugText = new BoUfoLabel();
 d->mMatricesDebug->addWidget(new BoUfoLabel(i18n("Projection matrix")));
 d->mMatricesDebug->addWidget(d->mMatricesDebugProjection);
 d->mMatricesDebug->addWidget(new BoUfoLabel(i18n("Modelview matrix")));
 d->mMatricesDebug->addWidget(d->mMatricesDebugModelview);
 d->mMatricesDebug->addWidget(new BoUfoLabel(i18n("Projection * Modelview")));
 d->mMatricesDebug->addWidget(d->mMatricesDebugProjMod);
 d->mMatricesDebug->addWidget(new BoUfoLabel(i18n("(Projection * Modelview)^(-1)")));
 d->mMatricesDebug->addWidget(d->mMatricesDebugProjModInv);
 d->mMatricesDebug->addWidget(d->mMatricesDebugText);
 northWest->addWidget(d->mMatricesDebug);

 d->mItemWorkStatistics = new BoUfoLabel();
 northWest->addWidget(d->mItemWorkStatistics);

 d->mOpenGLCamera = new BoUfoLabel();
 northWest->addWidget(d->mOpenGLCamera);

 d->mRenderCounts = new BoUfoLabel();
 northWest->addWidget(d->mRenderCounts);

 d->mAdvanceCalls = new BoUfoLabel();
 northWest->addWidget(d->mAdvanceCalls);

 d->mTextureMemory = new BoUfoLabel();
 northWest->addWidget(d->mTextureMemory);

 // FIXME: this is supposed to be in the center of the screen, but the "center"
 // constraint on a UBorderLayout is not sufficient for that.
 d->mGamePaused = new BoUfoLabel(i18n("The game is paused"));
 center->addWidget(d->mGamePaused);

 d->mUfoChat = new BosonUfoChat();
 south->addWidget(d->mUfoChat);
 d->mUfoChat->setMessageId(BosonMessage::IdChat);


 // TODO: tooltips ?

}

void BosonUfoGameWidget::setGLMiniMap(BosonGLMiniMap* m)
{
 d->mUfoMiniMap->setMiniMap(m);
}

BoSelection* BosonUfoGameWidget::selection() const
{
 return display()->selection();
}

void BosonUfoGameWidget::setLocalPlayerIO(PlayerIO* io)
{
 PlayerIO* previousPlayerIO = localPlayerIO();
 d->mLocalPlayerIO = io;
 if (previousPlayerIO) {
	BosonLocalPlayerInput* i = (BosonLocalPlayerInput*)previousPlayerIO->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
	if (i) {
		disconnect(i, 0, d->mUfoCommandFrame, 0);
		disconnect(d->mUfoCommandFrame, 0, i, 0);
	}
 }
 d->mUfoCommandFrame->setLocalPlayerIO(localPlayerIO());
 if (!localPlayerIO()) {
	return;
 }

 BosonLocalPlayerInput* i = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 if (i) {
	connect(d->mUfoCommandFrame, SIGNAL(signalAction(const BoSpecificAction&)),
			i, SLOT(slotAction(const BoSpecificAction&)));
 }
}

PlayerIO* BosonUfoGameWidget::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonUfoGameWidget::setGroundTheme(BosonGroundTheme* t)
{
 d->mUfoCommandFrame->slotSetGroundTheme(t);
}

BosonBigDisplayBase* BosonUfoGameWidget::display() const
{
 return d->mDisplay;
}

void BosonUfoGameWidget::updateUfoLabels()
{
 QString minerals = QString::number(localPlayerIO()->minerals());
 QString oil = QString::number(localPlayerIO()->oil());
 d->mMineralsLabel->setText(minerals);
 d->mOilLabel->setText(oil);
 d->mResourcesBox->setVisible(boConfig->boolValue("show_resources"));

 d->mFPSLabel->setText(i18n("FPS: %1").arg(display()->fps()));
 d->mFPSLabel->setVisible(boConfig->boolValue("debug_fps"));

 bool renderGroundRendererDebug = false;
 if (renderGroundRendererDebug) {
	BoVector3Fixed cursor = BoVector3Fixed(cursorCanvasVector().x(), cursorCanvasVector().y(), boGame->canvas()->heightAtPoint(cursorCanvasVector().x(), cursorCanvasVector().y()));
	cursor.canvasToWorld();
	BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
	if (r) {
		QString s = r->debugStringForPoint(cursor);
		d->mGroundRendererDebug->setText(s);
	} else {
		BO_NULL_ERROR(s);
	}
	d->mGroundRendererDebug->setVisible(true);
 } else {
	d->mGroundRendererDebug->setVisible(false);
 }


 if (boConfig->boolValue("debug_map_coordinates")) {
	QPoint widgetPos;
	GLfloat x = 0.0f, y = 0.0f, z = 0.0f;
	display()->mapFromGlobal(QCursor::pos());
	display()->mapCoordinates(widgetPos, &x, &y, &z);
	BoVector3Fixed canvasVector(x, -y, z);

	QString world = QString::fromLatin1("World:  (%1,%2,%2)").
			arg((double)x, 6, 'f', 3).
			arg((double)y, 6, 'f', 3).
			arg((double)z, 6, 'f', 3);
	QString canvas = QString::fromLatin1("Canvas: (%1,%2)").
			arg((double)canvasVector.x(), 6, 'f', 3).
			arg((double)canvasVector.y(), 6, 'f', 3);
	QString window = QString::fromLatin1("Window: %1,%2").
			arg(widgetPos.x(), 4, 10).
			arg(widgetPos.y(), 4, 10);
	d->mMapCoordinatesWorldLabel->setText(world);
	d->mMapCoordinatesCanvasLabel->setText(canvas);
	d->mMapCoordinatesWindowLabel->setText(window);
 }
 d->mMapCoordinates->setVisible(boConfig->boolValue("debug_map_coordinates"));

 updateUfoLabelPathFinderDebug();
 updateUfoLabelMatricesDebug();
 updateUfoLabelItemWorkStatistics();
 updateUfoLabelOpenGLCamera();
 updateUfoLabelRenderCounts();
 updateUfoLabelAdvanceCalls();
 updateUfoLabelTextureMemory();
 d->mGamePaused->setVisible(boGame->gamePaused());
}

void BosonUfoGameWidget::updateUfoLabelPathFinderDebug()
{
 if (!boConfig->boolValue("debug_pf_data")) {
	d->mPathFinderDebug->setVisible(false);
	return;
 }
 d->mPathFinderDebug->setVisible(true);

 Cell* cellUnderCursor = boGame->canvas()->cellAt(cursorCanvasVector().x(), cursorCanvasVector().y());
 BosonPathRegion* r = 0;
 if (cellUnderCursor) {
	r = cellUnderCursor->region();
 }

 QString cell = QString::fromLatin1("Cell pos: (%1; %2)")
		.arg((cellUnderCursor == 0) ? -1 : cellUnderCursor->x()).arg((cellUnderCursor == 0) ? -1 : cellUnderCursor->y());
 QString cellpassable = QString::fromLatin1("  passable: %1").arg((cellUnderCursor == 0) ? "n/a" : (cellUnderCursor->passable() ? "true" : "false"));
 QString celloccupied = QString::fromLatin1("  occupied: %1").arg((cellUnderCursor == 0) ? "n/a" : (cellUnderCursor->isLandOccupied() ? "true" : "false"));
 QString regid = QString::fromLatin1("Region  : %1").arg((r == 0) ? -1 : r->id);
 QString regcost = QString::fromLatin1("    cost: %1").arg((r == 0) ? bofixed(-1) : r->cost, 5, 'g', 3);
 QString regcenter = QString::fromLatin1("  center: (%1; %2)").arg((r == 0) ? bofixed(-1) : r->centerx).arg((r == 0) ? bofixed(-1) : r->centery);
 QString regcells = QString::fromLatin1("   cells: %1").arg((r == 0) ? -1 : r->cellsCount);
 QString reggroup = QString::fromLatin1("   group: 0x%1").arg((r == 0) ? 0 : (int)r->group);
 QString regneighs = QString::fromLatin1("  neighs: %1").arg((r == 0) ? -1 : (int)r->neighbors.count());
 QString neighbors;
 if (r && r->neighbors.count() > 0) {
	for (unsigned int i = 0; i < r->neighbors.count(); i++) {
		neighbors += QString::fromLatin1("\n  id: %1; border: %2; cost: %3").arg(r->neighbors[i].region->id).arg(r->neighbors[i].bordercells).arg(r->neighbors[i].cost, 5, 'g', 3);
	}
 }
 // We create temporary cellinfo and reginfo strings, because QString support
 //  only 9 markers in arg() (%1, %2 ... %9)
 QString cellinfo = QString::fromLatin1("%1\n%2\n%3").arg(cell).arg(cellpassable).arg(celloccupied);
 QString reginfo = QString::fromLatin1("%1\n%2\n%3\n%4\n%5\n%6%7").arg(regid).arg(regcost).arg(regcenter).arg(regcells).arg(reggroup).arg(regneighs).arg(neighbors);
 QString text = QString::fromLatin1("%1\n%2").arg(cellinfo).arg(reginfo);

 d->mPathFinderDebug->setText(text);
}

void BosonUfoGameWidget::updateUfoLabelMatricesDebug()
{
 if (!boConfig->boolValue("debug_matrices")) {
	d->mMatricesDebug->setVisible(false);
	return;
 }
 d->mMatricesDebug->setVisible(true);

 BoMatrix model(d->mModelviewMatrix);
 BoMatrix proj(d->mProjectionMatrix);
 BoMatrix projMod(proj);
 projMod.multiply(model.data());
 BoMatrix projModInv;
 projMod.invert(&projModInv); // invert (proj*model)

 d->mMatricesDebugProjection->setMatrix(proj.data());
 d->mMatricesDebugModelview->setMatrix(model.data());
 d->mMatricesDebugProjMod->setMatrix(projMod.data());
 d->mMatricesDebugProjModInv->setMatrix(projModInv.data());

 QPoint widgetPos = display()->mapFromGlobal(QCursor::pos());
 GLint realy = d->mViewport[3] - (GLint)widgetPos.y() - 1;
 GLfloat depth = 0.0f;
 glReadPixels(widgetPos.x(), realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

 BoVector4Float v;
 v.setX( (GLfloat)((widgetPos.x() - d->mViewport[0]) * 2) / d->mViewport[2] - 1.0f );
 v.setY( (GLfloat)((realy - d->mViewport[1]) * 2) / d->mViewport[3] - 1.0f );
 v.setZ(2 * depth - 1.0f);
 v.setW(1.0f);
 BoVector4Float result;
 projModInv.transform(&result, &v);

 // it is a column vector, but we display as a row (so ^T)
 QString text = i18n("CursorPos = (Projection * Modelview)^(-1) * (%1 , %2 , %3 , %4)^T:").
		arg(v[0], 6, 'f', 3).
		arg(v[1], 6, 'f', 3).
		arg(v[2], 6, 'f', 3).
		arg(v[3], 6, 'f', 3);
 QString resultText = i18n("(%1 , %2 , %3 , %3)^T").
		arg(result[0], 6, 'f', 3).
		arg(result[1], 6, 'f', 3).
		arg(result[2], 6, 'f', 3).
		arg(result[3], 6, 'f', 3);
 if (result[3] == 0.0f) {
	d->mMatricesDebugText->setText("ERROR");
	boError() << k_funcinfo << endl;
	return;
 }
 QString realCoords = i18n("x = %1  ;  y = %2  ;  z = %3").
		arg(result[0] / result[3]).
		arg(result[1] / result[3]).
		arg(result[2] / result[3]);



 // display the planes. they consist of the normal vector and the
 // distance from the origin
 QString planes = i18n("Right Plane: %1\n").arg(planeDebugString(&d->mViewFrustum[0 * 4]));
 planes += i18n("Left Plane: %1\n").arg(planeDebugString(&d->mViewFrustum[1 * 4]));
 planes += i18n("Bottom Plane: %1\n").arg(planeDebugString(&d->mViewFrustum[2 * 4]));
 planes += i18n("Top Plane: %1\n").arg(planeDebugString(&d->mViewFrustum[3 * 4]));
 planes += i18n("Far Plane: %1\n").arg(planeDebugString(&d->mViewFrustum[4 * 4]));
 planes += i18n("Near Plane: %1").arg(planeDebugString(&d->mViewFrustum[5 * 4]));

 // AB: this label can be used to measure the performance of displaying multiple
 // lines in ULabel
 d->mMatricesDebugText->setText(i18n("%1\n%2\n%3\n\n%4")
		.arg(text).arg(resultText).arg(realCoords)
		.arg(planes));
}

void BosonUfoGameWidget::updateUfoLabelItemWorkStatistics()
{
 if (!boConfig->boolValue("debug_works")) {
	d->mItemWorkStatistics->setVisible(false);
	return;
 }
 d->mItemWorkStatistics->setVisible(true);

 BosonCanvasStatistics* statistics = canvas()->canvasStatistics();
 QMap<int, int> workCounts = *statistics->workCounts();
 QString text;
 text += i18n("Item work statistics:\n");
 text += i18n("Total items: %1\n").arg(canvas()->allItemsCount());
 text += i18n("-1 (items): %1\n").arg(workCounts[-1]),
 text += i18n("Idle:     %1\n").arg(workCounts[(int)UnitBase::WorkIdle]);
 text += i18n("Moving or turning: %1\n").
		arg(workCounts[(int)UnitBase::WorkMove] +
		workCounts[(int)UnitBase::WorkTurn]);
 text += i18n("Attacking:         %1\n").
		arg(workCounts[(int)UnitBase::WorkAttack]);
 text += i18n("Other:             %1\n").
		arg(workCounts[(int)UnitBase::WorkConstructed] +
		workCounts[(int)UnitBase::WorkDestroyed] +
		workCounts[(int)UnitBase::WorkFollow] +
		workCounts[(int)UnitBase::WorkPlugin]);

 d->mItemWorkStatistics->setText(text);
}

void BosonUfoGameWidget::updateUfoLabelOpenGLCamera()
{
 if (!boConfig->boolValue("debug_camera")) {
	d->mOpenGLCamera->setVisible(false);
	return;
 }
 d->mOpenGLCamera->setVisible(true);

 const BoVector3Float lookAt = camera()->lookAt();
 const BoVector3Float cameraPos = camera()->cameraPos();
 const BoVector3Float up = camera()->up();
 QString text;
 text += i18n("Camera:\n");
 text += i18n("LookAt: (%1; %2; %3)\n").arg(lookAt.x()).
		arg(lookAt.y()).arg(lookAt.z());
 text += i18n("CameraPos: (%1; %2; %3)\n").arg(cameraPos.x()).
		arg(cameraPos.y()).arg(cameraPos.z());
 text += i18n("Up: (%1; %2; %3)\n").arg(up.x()).
		arg(up.y()).arg(up.z());
 text += i18n("Radius: %1\n").arg(camera()->radius());
 text += i18n("Height: %1\n").arg(camera()->z());
 text += i18n("Rotation: %1\n").arg(camera()->rotation());

 d->mOpenGLCamera->setText(text);
}

void BosonUfoGameWidget::updateUfoLabelRenderCounts()
{
 if (!boConfig->boolValue("debug_rendercounts")) {
	d->mRenderCounts->setVisible(false);
	return;
 }
 d->mRenderCounts->setVisible(true);
 QString text;
#define HAVE_CANVAS_RENDERER 0
#if HAVE_CANVAS_RENDERER
 text += i18n("Items rendered: %1\n").arg(d->mCanvasRenderer->renderedItems());
 text += i18n("Particles rendered: %1\n").arg(d->mCanvasRenderer->renderedParticles());
#endif

 text += i18n("Ground renderer statistics:\n");
 text += BoGroundRendererManager::manager()->currentStatisticsData();
 text += i18n("\n");


 text += i18n("Mesh renderer statistics:\n");
 text += BoMeshRendererManager::manager()->currentStatisticsData();
 text += i18n("\n");

 text += i18n("Water renderer statistics:\n");
 text += boWaterManager->currentRenderStatisticsData();
 text += i18n("\n");

#if HAVE_CANVAS_RENDERER
 text += i18n("Texture binds: %1 (C: %2; I: %3; W: %4; P: %5)\n")
		.arg(boTextureManager->textureBinds()).arg(d->mCanvasRenderer->textureBindsCells()).arg(d->mCanvasRenderer->textureBindsItems()).arg(d->mCanvasRenderer->textureBindsWater()).arg(d->mCanvasRenderer->textureBindsParticles());
#endif

 d->mRenderCounts->setText(text);
}

void BosonUfoGameWidget::updateUfoLabelAdvanceCalls()
{
 if (!boConfig->boolValue("debug_advance_calls")) {
	d->mAdvanceCalls->setVisible(false);
	return;
 }
 d->mAdvanceCalls->setVisible(true);
 QString text;
 text += i18n("Advance calls passed: %1\n").arg(boGame->advanceCallsCount());
 text += i18n("Delayed messages: %1 (delayed advance messages: %2)\n").arg(boGame->delayedMessageCount()).arg(boGame->delayedAdvanceMessageCount());
 text += i18n("Advance message interval: %1 ms\n").arg(Boson::advanceMessageInterval());
 text += i18n("Game speed (advance calls per advance message): %1\n").arg(boGame->gameSpeed());
 d->mAdvanceCalls->setText(text);
}

void BosonUfoGameWidget::updateUfoLabelTextureMemory()
{
 if (!boConfig->boolValue("debug_texture_memory")) {
	d->mTextureMemory->setVisible(false);
	return;
 }
 d->mTextureMemory->setVisible(true);
 QString text;
 text += i18n("Texture memory in use (approximately): %1 kb\n").arg(boTextureManager->usedTextureMemory() / 1024);
 text += i18n(" in %1 texures\n").arg(boTextureManager->textureCount());
 d->mTextureMemory->setText(text);
}

const BoVector3Fixed& BosonUfoGameWidget::cursorCanvasVector() const
{
 return display()->cursorCanvasVector();
}

const BosonCanvas* BosonUfoGameWidget::canvas() const
{
 return display()->canvas();
}

BoGameCamera* BosonUfoGameWidget::camera() const
{
 return display()->camera();
}

bool BosonUfoGameWidget::isChatVisible() const
{
 return d->mUfoChat->isVisible();
}

void BosonUfoGameWidget::setGameMode(bool mode)
{
 d->mUfoCommandFrame->setGameMode(mode);
 d->mUfoChat->setKGame(boGame);
}

void BosonUfoGameWidget::slotShowPlaceFacilities(PlayerIO* io)
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeFacilities(io);
}

void BosonUfoGameWidget::slotShowPlaceMobiles(PlayerIO* io)
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeMobiles(io);
}

void BosonUfoGameWidget::slotShowPlaceGround()
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeGround();
}

void BosonUfoGameWidget::addChatMessage(const QString& message)
{
 d->mUfoChat->addMessage(message);
}

