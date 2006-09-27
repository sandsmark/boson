/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonufogamegui.h"
#include "bosonufogamegui.moc"

#include "../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "bosonufogameguihelper.h"
#include "../bomath.h"
#include "../bo3dtools.h"
#include "../bosonconfig.h"
#include "../gameengine/boson.h"
#include "../gameengine/bosoncanvas.h"
#include "../gameengine/bosoncanvasstatistics.h"
#include "../bogroundrenderer.h"
#include "../bogroundrenderermanager.h"
#include "../modelrendering/bomeshrenderermanager.h"
#include "../botexture.h"
#include "../bocamera.h"
#include "../bosonufochat.h"
#include "minimap/bosonufominimap.h"
#include "commandframe/bosoncommandframe.h"
#include "../gameengine/bosonmessageids.h"
#include "boselection.h"
#include "../gameengine/playerio.h"
#include "bosonlocalplayerinput.h"
#include "../gameengine/unit.h"
#include "../gameengine/bosonpath.h"
#include "../gameengine/cell.h"
#include "../bowaterrenderer.h"
#include "../bosonfpscounter.h"
#include "../info/boinfo.h"
#include "../info/bocurrentinfo.h"
#include <bodebuglog.h>
#include "bodebug.h"

#include <klocale.h>

#include <qdatetime.h>

/**
 * @return A string that displays @p plane. The plane consists of a normal
 * vector in the first 3 numbers and the distance from the origin in the 4th
 * number.
 **/
static QString planeDebugString(const BoPlane& plane)
{
 return QString("((%1,%2,%3),%4)").arg(plane.normal().x()).arg(plane.normal().y()).arg(plane.normal().z()).arg(plane.distanceFromOrigin());
}

class CPUTimes
{
public:
	CPUTimes()
	{
		mUTime = 0;
		mSTime = 0;
		mCUTime = 0;
		mCSTime = 0;
	}
	bool update();

	CPUTimes& operator=(const CPUTimes& t)
	{
		mUpdated = t.mUpdated;
		mUTime = t.mUTime;
		mSTime = t.mSTime;
		mCUTime = t.mCUTime;
		mCSTime = t.mCSTime;
		return *this;
	}
	unsigned int msecsTo(const QTime& t) const
	{
		if (mUpdated.isNull() || t.isNull()) {
			return 0;
		}
		return mUpdated.msecsTo(t);
	}
	unsigned int msecsSince(const QTime& old) const
	{
		int ms = old.msecsTo(mUpdated);
		if (ms < 0) {
			return 0;
		}
		return ms;
	}
	unsigned int msecsSince(const CPUTimes& old) const
	{
		return msecsSince(old.mUpdated);
	}
	unsigned long int dutime(const CPUTimes& old) const
	{
		if (mUTime < old.mUTime) {
			return 0;
		}
		return mUTime - old.mUTime;
	}
	unsigned long int dstime(const CPUTimes& old) const
	{
		if (mSTime < old.mSTime) {
			return 0;
		}
		return mSTime - old.mSTime;
	}
	unsigned long int dcutime(const CPUTimes& old) const
	{
		if (mCUTime < old.mCUTime) {
			return 0;
		}
		return mCUTime - old.mCUTime;
	}
	unsigned long int dcstime(const CPUTimes& old) const
	{
		if (mCSTime < old.mCSTime) {
			return 0;
		}
		return mCSTime - old.mCSTime;
	}
	float utimePercent(const CPUTimes& old) const
	{
		int ms = msecsSince(old);
		if (ms == 0) {
			return 0.0f;
		}
		float djiffies = (float)dutime(old);
		float dms = djiffies * 10.0f;
		float percent = (dms * 100.0f) / ((float)ms);
		return percent;
	}
	float stimePercent(const CPUTimes& old) const
	{
		int ms = msecsSince(old);
		if (ms == 0) {
			return 0.0f;
		}
		float djiffies = (float)dstime(old);
		float dms = djiffies * 10.0f;
		float percent = (dms * 100.0f) / ((float)ms);
		return percent;
	}
	float cutimePercent(const CPUTimes& old) const
	{
		int ms = msecsSince(old);
		if (ms == 0) {
			return 0.0f;
		}
		float djiffies = (float)dcutime(old);
		float dms = djiffies * 10.0f;
		float percent = (dms * 100.0f) / ((float)ms);
		return percent;
	}
	float cstimePercent(const CPUTimes& old) const
	{
		int ms = msecsSince(old);
		if (ms == 0) {
			return 0.0f;
		}
		float djiffies = (float)dcstime(old);
		float dms = djiffies * 10.0f;
		float percent = (dms * 100.0f) / ((float)ms);
		return percent;
	}
	float allTimePercent(const CPUTimes& old) const
	{
		int ms = msecsSince(old);
		if (ms == 0) {
			return 0.0f;
		}
		float djiffies = (float)dutime(old)
				+ (float)dstime(old)
				+ (float)dcutime(old)
				+ (float)dcstime(old);
		float dms = djiffies * 10.0f;
		float percent = (dms * 100.0f) / ((float)ms);
		return percent;
	}

private:
	QTime mUpdated;
	unsigned long int mUTime;
	unsigned long int mSTime;
	long int mCUTime;
	long int mCSTime;
};

bool CPUTimes::update()
{
 BoCurrentInfo info;
 CPUTimes times;
 if (!info.cpuTime(&mUTime, &mSTime, &mCUTime, &mCSTime)) {
	mUpdated = QTime::currentTime();
	return false;
 }
 mUpdated = QTime::currentTime();
 return true;
}


class BosonUfoGameGUIPrivate
{
public:
	BosonUfoGameGUIPrivate(const BoMatrix& modelview, const BoMatrix& projection,
			const BoFrustum& viewFrustum, const GLint* viewport)
		: mModelviewMatrix(modelview), mProjectionMatrix(projection),
		mViewFrustum(viewFrustum), mViewport(viewport)
	{
		mCursorCanvasVector = 0;
		mCursorWidgetPos = 0;
		mSelection = 0;
		mCamera = 0;
		mCanvas = 0;
		mLocalPlayerIO = 0;
		mFPSCounter = 0;

		mResourcesBox = 0;
		mMineralsLabel = 0;
		mOilLabel = 0;
		mGenericAmmoLabel = 0;
		mPowerGeneratedLabel = 0;
		mPowerConsumedLabel = 0;
		mPower = 0;
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
		mMatricesDebugText = 0;
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

	const BoVector3Fixed* mCursorCanvasVector;
	const QPoint* mCursorWidgetPos;
	BoSelection* mSelection;
	BoGameCamera* mCamera;
	const BosonCanvas* mCanvas;
	const BoMatrix& mModelviewMatrix;
	const BoMatrix& mProjectionMatrix;
	const BoFrustum& mViewFrustum;
	const GLint* mViewport;
	PlayerIO* mLocalPlayerIO;
	BosonGameFPSCounter* mFPSCounter;
	CPUTimes mCPUTimes;
	CPUTimes mCPUTimes2;

	// pointers to widgets in the BosonUfoGameGUIHelper
	BoUfoHBox* mResourcesBox;
	BoUfoLabel* mMineralsLabel;
	BoUfoLabel* mOilLabel;
	BoUfoLabel* mGenericAmmoLabel;
	BoUfoLabel* mPowerGeneratedLabel;
	BoUfoLabel* mPowerConsumedLabel;
	BoUfoExtendedProgress* mPower;
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
	BosonUfoChat* mUfoErrorDisplay;
	BosonUfoChat* mUfoDebugDisplay;
	BosonUfoMiniMap* mUfoMiniMap;
	BosonCommandFrame* mUfoCommandFrame;

	// pointers to widgets that are created in this class
	BoUfoLabel* mMemoryUsage;
	BoUfoLabel* mCPUUsage;
};

BosonUfoGameGUI::BosonUfoGameGUI(const BoMatrix& modelview, const BoMatrix& projection,
		const BoFrustum& viewFrustum, const GLint* viewport)
	: BoUfoWidget()
{
 setName("BosonUfoGameGUI");
 d = new BosonUfoGameGUIPrivate(modelview, projection, viewFrustum, viewport);

 QColor defaultColor = BoUfoLabel::defaultForegroundColor();
 BoUfoLabel::setDefaultForegroundColor(Qt::white);
 initUfoWidgets();
 BoUfoLabel::setDefaultForegroundColor(defaultColor);


 BoDebugLog* debugLog = BoDebugLog::debugLog();
 if (debugLog) {
	connect(debugLog, SIGNAL(signalError(const BoDebugMessage&)),
			this, SLOT(slotBoDebugError(const BoDebugMessage&)));
	connect(debugLog, SIGNAL(signalWarn(const BoDebugMessage&)),
			this, SLOT(slotBoDebugWarning(const BoDebugMessage&)));
	connect(debugLog, SIGNAL(signalDebug(const BoDebugMessage&)),
			this, SLOT(slotBoDebugOutput(const BoDebugMessage&)));

	// we could provide config entries for these, that allow enabling _INFO
	// as well
	debugLog->setEmitSignal(BoDebug::KDEBUG_ERROR, true);
	debugLog->setEmitSignal(BoDebug::KDEBUG_WARN, true);
 }
}

BosonUfoGameGUI::~BosonUfoGameGUI()
{
 boDebug() << k_funcinfo << endl;
 delete d;
}

void BosonUfoGameGUI::bosonObjectCreated(Boson* boson)
{
 Q_UNUSED(boson);
}

void BosonUfoGameGUI::bosonObjectAboutToBeDestroyed(Boson* boson)
{
 Q_UNUSED(boson);
}

void BosonUfoGameGUI::initUfoWidgets()
{
 // AB: note that BoUfo widgets differ from usual Qt widgets API-wise.
 // You need to create them without a parent and then add them to their parent
 // widget using parent->addWidget(child). This also adds child to the layout of
 // parent.
 // WARNING: ALL widget that are created MUST be added to another widget!
 // Otherwise the created widget won't be deleted!

 // A "UBorderLayout" layout requires it's children to have so-call
 // "constraints". These specify where the widget is placed (north, south, ...)
 setLayoutClass(BoUfoWidget::UVBoxLayout);

 BosonUfoGameGUIHelper* widget = new BosonUfoGameGUIHelper();
 addWidget(widget);

 // just pointers (no need to add to another widget, as they already are added)
 d->mResourcesBox = widget->mResourcesBox;
 d->mMineralsLabel = widget->mMineralsLabel;
 d->mOilLabel = widget->mOilLabel;
 d->mGenericAmmoLabel = widget->mGenericAmmoLabel;
 d->mPowerGeneratedLabel = widget->mPowerGeneratedLabel;
 d->mPowerConsumedLabel = widget->mPowerConsumedLabel;
 d->mPower = widget->mPower;
 d->mFPSLabel = widget->mFPSLabel;
 d->mGroundRendererDebug = widget->mGroundRendererDebug;
 d->mMapCoordinates = widget->mMapCoordinates;
 d->mMapCoordinatesWorldLabel = widget->mMapCoordinatesWorldLabel;
 d->mMapCoordinatesCanvasLabel = widget->mMapCoordinatesCanvasLabel;
 d->mMapCoordinatesWindowLabel = widget->mMapCoordinatesWindowLabel;
 d->mPathFinderDebug = widget->mPathFinderDebug;
 d->mMatricesDebug = widget->mMatricesDebug;
 d->mMatricesDebugText = widget->mMatricesDebugText;
 d->mItemWorkStatistics = widget->mItemWorkStatistics;
 d->mOpenGLCamera = widget->mOpenGLCamera;
 d->mRenderCounts = widget->mRenderCounts;
 d->mAdvanceCalls = widget->mAdvanceCalls;
 d->mTextureMemory = widget->mTextureMemory;
 d->mGamePaused = widget->mGamePaused;


 d->mUfoMiniMap = new BosonUfoMiniMap();
 widget->mUfoMiniMapContainer->addWidget(d->mUfoMiniMap);

 d->mUfoCommandFrame = new BosonCommandFrame();
 connect(this, SIGNAL(signalSelectionChanged(BoSelection*)),
		d->mUfoCommandFrame, SLOT(slotSelectionChanged(BoSelection*)));
 connect(d->mUfoCommandFrame, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)),
		this, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)));
 connect(d->mUfoCommandFrame, SIGNAL(signalPlaceUnit(unsigned int, Player*)),
		this, SIGNAL(signalPlaceUnit(unsigned int, Player*)));
 d->mUfoCommandFrame->slotSelectionChanged(selection());
 widget->mUfoCommandFrameContainer->addWidget(d->mUfoCommandFrame);


 d->mMatricesDebugProjection = new BoUfoMatrix();
 d->mMatricesDebugModelview = new BoUfoMatrix();
 d->mMatricesDebugProjMod = new BoUfoMatrix();
 d->mMatricesDebugProjModInv = new BoUfoMatrix();
 widget->mMatricesDebugProjectionContainer->addWidget(d->mMatricesDebugProjection);
 widget->mMatricesDebugModelviewContainer->addWidget(d->mMatricesDebugModelview);
 widget->mMatricesDebugProjModContainer->addWidget(d->mMatricesDebugProjMod);
 widget->mMatricesDebugProjModInvContainer->addWidget(d->mMatricesDebugProjModInv);


 d->mUfoDebugDisplay = new BosonUfoChat();
 d->mUfoDebugDisplay->setName("GameViewDebugDisplay");
 d->mUfoDebugDisplay->setMessageId(0); // no messages
 d->mUfoDebugDisplay->setSendBoxVisible(false);
 widget->mUfoChatContainer->addWidget(d->mUfoDebugDisplay);

 d->mUfoErrorDisplay = new BosonUfoChat();
 d->mUfoErrorDisplay->setName("GameViewErrorDisplay");
 d->mUfoErrorDisplay->setMessageId(0); // no messages
 d->mUfoErrorDisplay->setSendBoxVisible(false);
 widget->mUfoChatContainer->addWidget(d->mUfoErrorDisplay);

 d->mUfoChat = new BosonUfoChat();
 d->mUfoChat->setName("GameViewUfoChat");
 d->mUfoChat->setMessageId(BosonMessageIds::IdChat);
 widget->mUfoChatContainer->addWidget(d->mUfoChat);


 d->mMemoryUsage = new BoUfoLabel();
 widget->mNorthEastDebugContainer->addWidget(d->mMemoryUsage);
 d->mCPUUsage = new BoUfoLabel();
 widget->mNorthEastDebugContainer->addWidget(d->mCPUUsage);
 d->mCPUTimes.update();
 d->mCPUTimes2.update();
}

BosonUfoMiniMap* BosonUfoGameGUI::miniMapWidget() const
{
 return d->mUfoMiniMap;
}

void BosonUfoGameGUI::setSelection(BoSelection* s)
{
 if (d->mSelection) {
	disconnect(d->mSelection, 0, d->mUfoCommandFrame, 0);
 }
 d->mSelection = s;
 if (d->mSelection) {
	connect(d->mUfoCommandFrame, SIGNAL(signalSelectUnit(Unit*)),
			selection(), SLOT(slotSelectSingleUnit(Unit*)));
 }
}

BoSelection* BosonUfoGameGUI::selection() const
{
 return d->mSelection;
}

void BosonUfoGameGUI::setLocalPlayerIO(PlayerIO* io)
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
 d->mUfoChat->setFromPlayer((KPlayer*)localPlayerIO()->player());

 BosonLocalPlayerInput* i = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 if (i) {
	connect(d->mUfoCommandFrame, SIGNAL(signalAction(const BoSpecificAction&)),
			i, SLOT(slotAction(const BoSpecificAction&)));
 }
}

PlayerIO* BosonUfoGameGUI::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonUfoGameGUI::setGroundTheme(BosonGroundTheme* t)
{
 d->mUfoCommandFrame->slotSetGroundTheme(t);
}

void BosonUfoGameGUI::setGameFPSCounter(BosonGameFPSCounter* counter)
{
 d->mFPSCounter = counter;
}

void BosonUfoGameGUI::updateUfoLabels()
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(d->mFPSCounter);
 QString minerals = QString::number(localPlayerIO()->minerals());
 QString oil = QString::number(localPlayerIO()->oil());
 QString genericAmmo = QString::number(localPlayerIO()->ammunition("Generic"));
 d->mMineralsLabel->setText(minerals);
 d->mOilLabel->setText(oil);
 d->mGenericAmmoLabel->setText(genericAmmo);
 d->mResourcesBox->setVisible(boConfig->boolValue("show_resources"));
 unsigned long int powerGenerated, powerConsumed;
 localPlayerIO()->calculatePower(&powerGenerated, &powerConsumed);
 d->mPowerGeneratedLabel->setText(QString::number(powerGenerated));
 d->mPowerConsumedLabel->setText(QString::number(powerConsumed));
 d->mPower->setRange(0, powerConsumed * 1.5);
 d->mPower->setEndExtensionValueRange(powerConsumed * 0.5);
 d->mPower->setValue(powerGenerated);

 double fps;
 double skippedFPS;
 fps = d->mFPSCounter->cachedFps(&skippedFPS);
 d->mFPSLabel->setText(i18n("FPS: %1\nSkipped FPS: %2").arg(fps, 0, 'f', 3).arg(skippedFPS, 0, 'f', 3));
 d->mFPSLabel->setVisible(boConfig->boolValue("debug_fps"));

 bool renderGroundRendererDebug = boConfig->boolValue("debug_groundrenderer_debug");
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
	QPoint widgetPos = cursorWidgetPos();
	BoVector3Fixed canvasVector = cursorCanvasVector();

	QString world = QString::fromLatin1("World:  (%1,%2,%2)").
			arg((double)canvasVector.x(), 6, 'f', 3).
			arg((double)-canvasVector.y(), 6, 'f', 3).
			arg((double)canvasVector.z(), 6, 'f', 3);
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
 updateUfoLabelMemoryUsage();
 updateUfoLabelCPUUsage();
 d->mGamePaused->setVisible(boGame->gamePaused());
}

void BosonUfoGameGUI::updateUfoLabelPathFinderDebug()
{
 if (!boConfig->boolValue("debug_pf_data")) {
	d->mPathFinderDebug->setVisible(false);
	return;
 }
 d->mPathFinderDebug->setVisible(true);

 BosonPath* pf = boGame->canvas()->pathFinder();
 d->mPathFinderDebug->setText(pf->debugText(cursorCanvasVector().x(), cursorCanvasVector().y()));
}

void BosonUfoGameGUI::updateUfoLabelMatricesDebug()
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

 QPoint widgetPos = cursorWidgetPos();
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
 QString planes = i18n("Right Plane: %1\n").arg(planeDebugString(d->mViewFrustum.right()));
 planes += i18n("Left Plane: %1\n").arg(planeDebugString(d->mViewFrustum.left()));
 planes += i18n("Bottom Plane: %1\n").arg(planeDebugString(d->mViewFrustum.bottom()));
 planes += i18n("Top Plane: %1\n").arg(planeDebugString(d->mViewFrustum.top()));
 planes += i18n("Far Plane: %1\n").arg(planeDebugString(d->mViewFrustum.far()));
 planes += i18n("Near Plane: %1").arg(planeDebugString(d->mViewFrustum.near()));

 // AB: this label can be used to measure the performance of displaying multiple
 // lines in ULabel
 d->mMatricesDebugText->setText(i18n("%1\n%2\n%3\n\n%4")
		.arg(text).arg(resultText).arg(realCoords)
		.arg(planes));
}

void BosonUfoGameGUI::updateUfoLabelItemWorkStatistics()
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

void BosonUfoGameGUI::updateUfoLabelOpenGLCamera()
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
 text += i18n("Distance: %1\n").arg(camera()->distance());
 text += i18n("Rotation: %1\n").arg(camera()->rotation());
 text += i18n("XRotation: %1\n").arg(camera()->xRotation());

 d->mOpenGLCamera->setText(text);
}

void BosonUfoGameGUI::updateUfoLabelRenderCounts()
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
 text += boWaterRenderer->currentRenderStatisticsData();
 text += i18n("\n");

#if HAVE_CANVAS_RENDERER
 text += i18n("Texture binds: %1 (C: %2; I: %3; W: %4; P: %5)\n")
		.arg(boTextureManager->textureBinds()).arg(d->mCanvasRenderer->textureBindsCells()).arg(d->mCanvasRenderer->textureBindsItems()).arg(d->mCanvasRenderer->textureBindsWater()).arg(d->mCanvasRenderer->textureBindsParticles());
#endif

 d->mRenderCounts->setText(text);
}

void BosonUfoGameGUI::updateUfoLabelAdvanceCalls()
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

void BosonUfoGameGUI::updateUfoLabelTextureMemory()
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

void BosonUfoGameGUI::updateUfoLabelMemoryUsage()
{
 if (!boConfig->boolValue("debug_memory_usage") && !boConfig->boolValue("debug_memory_vmdata_only")) {
	d->mMemoryUsage->setVisible(false);
	return;
 }
 d->mMemoryUsage->setVisible(true);
 BoCurrentInfo info;
 QString vmSize;
 QString vmLck;
 QString vmRSS;
 QString vmData;
 QString vmStk;
 QString vmExe;
 QString vmLib;
 QString vmPTE;
 if (!info.memoryInUse(&vmSize, &vmLck, &vmRSS, &vmData, &vmStk, &vmExe, &vmLib, &vmPTE)) {
	d->mMemoryUsage->setText("Memory Usage: cannot read data on your system");
	return;
 }
 QString text = QString("VmSize: %1\n"
		"VmLck: %2\n"
		"VmRSS: %3\n"
		"VmData: %4\n"
		"VmStk: %5\n"
		"VmExe: %6\n"
		"VmLib: %7\n"
		"VmPTE: %8")
	 	.arg(vmSize)
	 	.arg(vmLck)
	 	.arg(vmRSS)
	 	.arg(vmData)
	 	.arg(vmStk)
	 	.arg(vmExe)
	 	.arg(vmLib)
	 	.arg(vmPTE);
 if (boConfig->boolValue("debug_memory_vmdata_only")) {
	text = QString("VmData: %1").arg(vmData);
 }
 d->mMemoryUsage->setText(text);
}

void BosonUfoGameGUI::updateUfoLabelCPUUsage()
{
 if (!boConfig->boolValue("debug_cpu_usage")) {
	d->mCPUUsage->setVisible(false);
	return;
 }
 d->mCPUUsage->setVisible(true);

 const unsigned int updateInterval = 1500; // in ms
 if (d->mCPUTimes.msecsTo(QTime::currentTime()) < updateInterval) {
	return;
 }

 CPUTimes times;
 if (!times.update()) {
	d->mCPUUsage->setText("CPU Usage: cannot read data on your system");
	return;
 }
 if (times.msecsSince(d->mCPUTimes) == 0) {
	d->mCPUUsage->setText("CPU Usage: read invalid data");
	return;
 }

 QString text = QString("User Mode: %1\n"
		"System Mode: %2\n"
		"Children: User: %3 System: %3\n"
		"All: %4")
		.arg(times.utimePercent(d->mCPUTimes))
		.arg(times.stimePercent(d->mCPUTimes))
		.arg(times.cutimePercent(d->mCPUTimes))
		.arg(times.cstimePercent(d->mCPUTimes))
		.arg(times.allTimePercent(d->mCPUTimes));
 d->mCPUUsage->setText(text);

 d->mCPUTimes = d->mCPUTimes2;
 d->mCPUTimes2 = times;
}

void BosonUfoGameGUI::setCursorCanvasVector(const BoVector3Fixed* v)
{
 d->mCursorCanvasVector = v;
}

void BosonUfoGameGUI::setCursorWidgetPos(const QPoint* pos)
{
 d->mCursorWidgetPos = pos;
}

void BosonUfoGameGUI::setCursorRootPos(const QPoint* pos)
{
 d->mUfoCommandFrame->setCursorRootPos(pos);
}

const BoVector3Fixed& BosonUfoGameGUI::cursorCanvasVector() const
{
 return *d->mCursorCanvasVector;
}

const QPoint& BosonUfoGameGUI::cursorWidgetPos() const
{
 return *d->mCursorWidgetPos;
}

void BosonUfoGameGUI::setCanvas(const BosonCanvas* c)
{
 d->mCanvas = c;
}

const BosonCanvas* BosonUfoGameGUI::canvas() const
{
 return d->mCanvas;
}

void BosonUfoGameGUI::setCamera(BoGameCamera* c)
{
 d->mCamera = c;
}

BoGameCamera* BosonUfoGameGUI::camera() const
{
 return d->mCamera;
}

bool BosonUfoGameGUI::isChatVisible() const
{
 return d->mUfoChat->isVisible();
}

void BosonUfoGameGUI::setGameMode(bool mode)
{
 d->mUfoCommandFrame->setGameMode(mode);
 d->mUfoChat->setKGame(boGame);
}

void BosonUfoGameGUI::slotShowPlaceFacilities(PlayerIO* io)
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeFacilities(io);
}

void BosonUfoGameGUI::slotShowPlaceMobiles(PlayerIO* io)
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeMobiles(io);
}

void BosonUfoGameGUI::slotShowPlaceGround()
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeGround();
}

void BosonUfoGameGUI::addChatMessage(const QString& message)
{
 d->mUfoChat->addMessage(message);
}

void BosonUfoGameGUI::slotBoDebugOutput(const BoDebugMessage& m)
{
 QString area = m.areaName();
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 QString from = i18n("DEBUG: ");
 if (!area.isEmpty()) {
	from = i18n("DEBUG (%1): ").arg(area);
 }
 d->mUfoDebugDisplay->addMessage(i18n("%1: %2").arg(from).arg(message));
}

void BosonUfoGameGUI::slotBoDebugWarning(const BoDebugMessage& m)
{
 QString area = m.areaName();
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 QString from = i18n("WARNING: ");
 if (!area.isEmpty()) {
	from = i18n("WARNING(%1): ").arg(area);
 }
 d->mUfoErrorDisplay->addMessage(i18n("%1: %2").arg(from).arg(message));
}

void BosonUfoGameGUI::slotBoDebugError(const BoDebugMessage& m)
{
 QString area = m.areaName();
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 QString from = i18n("ERROR: ");
 if (!area.isEmpty()) {
	from = i18n("ERROR(%1): ").arg(area);
 }
 d->mUfoErrorDisplay->addMessage(i18n("%1: %2").arg(from).arg(message));
}


