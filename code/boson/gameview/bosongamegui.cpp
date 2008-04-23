/*
    This file is part of the Boson game
    Copyright (C) 2004-2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosongamegui.h"
#include "bosongamegui.moc"

#include "../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "ui_bosongameguihelper.h"
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
#include "../bosonchat.h"
#include "../bomatrixwidget.h"
#include "minimap/bosonminimap.h"
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
//#include <bodebuglog.h>
#include "bodebug.h"

#include <klocale.h>

#include <qdatetime.h>
#include <QLabel>
#include <QProgressBar>

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


class BosonGameGUIPrivate
{
public:
	BosonGameGUIPrivate(const BoMatrix& modelview, const BoMatrix& projection,
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
		mChat = 0;
		mMiniMap = 0;
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

	// pointers to widgets in the BosonGameGUIHelper
	QWidget* mResourcesBox;
	QLabel* mMineralsLabel;
	QLabel* mOilLabel;
	QLabel* mGenericAmmoLabel;
	QLabel* mPowerGeneratedLabel;
	QLabel* mPowerConsumedLabel;
	QProgressBar* mPower;
	QLabel* mFPSLabel;
	QLabel* mGroundRendererDebug;
	QWidget* mMapCoordinates;
	QLabel* mMapCoordinatesWorldLabel;
	QLabel* mMapCoordinatesCanvasLabel;
	QLabel* mMapCoordinatesWindowLabel;
	QLabel* mPathFinderDebug;
	QWidget* mMatricesDebug;
	BoMatrixWidget* mMatricesDebugProjection;
	BoMatrixWidget* mMatricesDebugModelview;
	BoMatrixWidget* mMatricesDebugProjMod;
	BoMatrixWidget* mMatricesDebugProjModInv;
	QLabel* mMatricesDebugText;
	QLabel* mItemWorkStatistics;
	QLabel* mOpenGLCamera;
	QLabel* mRenderCounts;
	QLabel* mAdvanceCalls;
	QLabel* mTextureMemory;
	QLabel* mGamePaused;
	BosonChat* mChat;
	BosonChat* mErrorDisplay;
	BosonChat* mDebugDisplay;
	BosonMiniMap* mMiniMap;
	BosonCommandFrame* mUfoCommandFrame;

	// pointers to widgets that are created in this class
	QLabel* mMemoryUsage;
	QLabel* mCPUUsage;
};

BosonGameGUI::BosonGameGUI(const BoMatrix& modelview, const BoMatrix& projection,
		const BoFrustum& viewFrustum, const GLint* viewport, QWidget* parent)
	: QWidget(parent)
{
 setObjectName("BosonGameGUI");
 d = new BosonGameGUIPrivate(modelview, projection, viewFrustum, viewport);

 initWidgets();


#if 0
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
#else
#warning FIXME: BoDebugLog is disabled
 boWarning() << k_funcinfo << "FIXME: BoDebugLog is disabled";
#endif
}

BosonGameGUI::~BosonGameGUI()
{
 boDebug() << k_funcinfo << endl;
 delete d;
}

void BosonGameGUI::bosonObjectCreated(Boson* boson)
{
 Q_UNUSED(boson);
}

void BosonGameGUI::bosonObjectAboutToBeDestroyed(Boson* boson)
{
 Q_UNUSED(boson);
}

void BosonGameGUI::initWidgets()
{
 QVBoxLayout* layout = new QVBoxLayout(this);

 QWidget* widget = new QWidget(this);
 layout->addWidget(widget);
 Ui::BosonGameGUIHelper* ui = new Ui::BosonGameGUIHelper();
 ui->setupUi(widget);

#warning TODO: the GUIHelper is a stacked widget, we want a layered widget!!
 // AB: the GUIHelper widget MUST be a QStackedWidget, because we want the
 // "pages" in qt designer.

 // just pointers (no need to add to another widget, as they already are added)
 d->mResourcesBox = ui->mResourcesBox;
 d->mMineralsLabel = ui->mMineralsLabel;
 d->mOilLabel = ui->mOilLabel;
 d->mGenericAmmoLabel = ui->mGenericAmmoLabel;
 d->mPowerGeneratedLabel = ui->mPowerGeneratedLabel;
 d->mPowerConsumedLabel = ui->mPowerConsumedLabel;
 d->mPower = ui->mPower;
 d->mFPSLabel = ui->mFPSLabel;
 d->mGroundRendererDebug = ui->mGroundRendererDebug;
 d->mMapCoordinates = ui->mMapCoordinates;
 d->mMapCoordinatesWorldLabel = ui->mMapCoordinatesWorldLabel;
 d->mMapCoordinatesCanvasLabel = ui->mMapCoordinatesCanvasLabel;
 d->mMapCoordinatesWindowLabel = ui->mMapCoordinatesWindowLabel;
 d->mPathFinderDebug = ui->mPathFinderDebug;
 d->mMatricesDebug = ui->mMatricesDebug;
 d->mMatricesDebugText = ui->mMatricesDebugText;
 d->mItemWorkStatistics = ui->mItemWorkStatistics;
 d->mOpenGLCamera = ui->mOpenGLCamera;
 d->mRenderCounts = ui->mRenderCounts;
 d->mAdvanceCalls = ui->mAdvanceCalls;
 d->mTextureMemory = ui->mTextureMemory;
 d->mGamePaused = ui->mGamePaused;


 QVBoxLayout* minimapLayout = new QVBoxLayout(ui->mMiniMapContainer);
 d->mMiniMap = new BosonMiniMap(ui->mMiniMapContainer);
 minimapLayout->addWidget(d->mMiniMap);

#warning TODO: commandframe
#if 0
 d->mUfoCommandFrame = new BosonCommandFrame();
 connect(this, SIGNAL(signalSelectionChanged(BoSelection*)),
		d->mUfoCommandFrame, SLOT(slotSelectionChanged(BoSelection*)));
 connect(d->mUfoCommandFrame, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)),
		this, SIGNAL(signalPlaceGround(unsigned int, unsigned char*)));
 connect(d->mUfoCommandFrame, SIGNAL(signalPlaceUnit(unsigned int, Player*)),
		this, SIGNAL(signalPlaceUnit(unsigned int, Player*)));
 d->mUfoCommandFrame->slotSelectionChanged(selection());
 widget->mUfoCommandFrameContainer->addWidget(d->mUfoCommandFrame);
#endif


 QVBoxLayout* matrixProjectionLayout = new QVBoxLayout(ui->mMatricesDebugProjectionContainer);
 QVBoxLayout* matrixModelviewLayout = new QVBoxLayout(ui->mMatricesDebugModelviewContainer);
 QVBoxLayout* matrixProjModLayout = new QVBoxLayout(ui->mMatricesDebugProjModContainer);
 QVBoxLayout* matrixProModInvLayout = new QVBoxLayout(ui->mMatricesDebugProjModInvContainer);
 d->mMatricesDebugProjection = new BoMatrixWidget(ui->mMatricesDebugProjectionContainer);
 d->mMatricesDebugModelview = new BoMatrixWidget(ui->mMatricesDebugModelviewContainer);
 d->mMatricesDebugProjMod = new BoMatrixWidget(ui->mMatricesDebugProjModContainer);
 d->mMatricesDebugProjModInv = new BoMatrixWidget(ui->mMatricesDebugProjModInvContainer);
 matrixProjectionLayout->addWidget(d->mMatricesDebugProjection);
 matrixModelviewLayout->addWidget(d->mMatricesDebugModelview);
 matrixProjModLayout->addWidget(d->mMatricesDebugProjMod);
 matrixProModInvLayout->addWidget(d->mMatricesDebugProjModInv);

 QVBoxLayout* chatContainerLayout = new QVBoxLayout(ui->mChatContainer);
 d->mDebugDisplay = new BosonChat(ui->mChatContainer);
 d->mDebugDisplay->setObjectName("GameViewDebugDisplay");
 d->mDebugDisplay->setMessageId(0); // no messages
 d->mDebugDisplay->setSendBoxVisible(false);
 chatContainerLayout->addWidget(d->mDebugDisplay);

 d->mErrorDisplay = new BosonChat(ui->mChatContainer);
 d->mErrorDisplay->setObjectName("GameViewErrorDisplay");
 d->mErrorDisplay->setMessageId(0); // no messages
 d->mErrorDisplay->setSendBoxVisible(false);
 chatContainerLayout->addWidget(d->mErrorDisplay);

 d->mChat = new BosonChat(ui->mChatContainer);
 d->mChat->setObjectName("GameViewChat");
 d->mChat->setMessageId(BosonMessageIds::IdChat);
 chatContainerLayout->addWidget(d->mChat);


 QVBoxLayout* northEastDebugContainerLayout = new QVBoxLayout(ui->mNorthEastDebugContainer->layout());
 d->mMemoryUsage = new QLabel(ui->mNorthEastDebugContainer);
 northEastDebugContainerLayout->addWidget(d->mMemoryUsage);
 d->mCPUUsage = new QLabel(ui->mNorthEastDebugContainer);
 northEastDebugContainerLayout->addWidget(d->mCPUUsage);
 d->mCPUTimes.update();
 d->mCPUTimes2.update();
}

BosonMiniMap* BosonGameGUI::miniMapWidget() const
{
 return d->mMiniMap;
}

void BosonGameGUI::setSelection(BoSelection* s)
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

BoSelection* BosonGameGUI::selection() const
{
 return d->mSelection;
}

void BosonGameGUI::setLocalPlayerIO(PlayerIO* io)
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
 d->mChat->setFromPlayer((KPlayer*)localPlayerIO()->player());

 BosonLocalPlayerInput* i = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 if (i) {
	connect(d->mUfoCommandFrame, SIGNAL(signalAction(const BoSpecificAction&)),
			i, SLOT(slotAction(const BoSpecificAction&)));
 }
}

PlayerIO* BosonGameGUI::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonGameGUI::setGroundTheme(BosonGroundTheme* t)
{
 d->mUfoCommandFrame->slotSetGroundTheme(t);
}

void BosonGameGUI::setGameFPSCounter(BosonGameFPSCounter* counter)
{
 d->mFPSCounter = counter;
}

void BosonGameGUI::updateLabels()
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
 quint32 powerGenerated, powerConsumed;
 localPlayerIO()->calculatePower(&powerGenerated, &powerConsumed);
 d->mPowerGeneratedLabel->setText(QString::number(powerGenerated));
 d->mPowerConsumedLabel->setText(QString::number(powerConsumed));
 d->mPower->setRange(0, powerConsumed * 1.5);
#warning TODO (BoUfo->Qt4 port)
#if 0
 d->mPower->setEndExtensionValueRange(powerConsumed * 0.5);
#endif
 d->mPower->setValue(powerGenerated);

 double fps;
 double skippedFPS;
 fps = d->mFPSCounter->cachedFps(&skippedFPS);
 d->mFPSLabel->setText(ki18n("FPS: %1\nSkipped FPS: %2").subs(fps, 0, 'f', 3).subs(skippedFPS, 0, 'f', 3).toString());
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

 updateLabelPathFinderDebug();
 updateLabelMatricesDebug();
 updateLabelItemWorkStatistics();
 updateLabelOpenGLCamera();
 updateLabelRenderCounts();
 updateLabelAdvanceCalls();
 updateLabelTextureMemory();
 updateLabelMemoryUsage();
 updateLabelCPUUsage();
 d->mGamePaused->setVisible(boGame->gamePaused());
}

void BosonGameGUI::updateLabelPathFinderDebug()
{
 if (!boConfig->boolValue("debug_pf_data")) {
	d->mPathFinderDebug->setVisible(false);
	return;
 }
 d->mPathFinderDebug->setVisible(true);

 BosonPath* pf = boGame->canvas()->pathFinder();
 d->mPathFinderDebug->setText(pf->debugText(cursorCanvasVector().x(), cursorCanvasVector().y()));
}

void BosonGameGUI::updateLabelMatricesDebug()
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
 QString text = ki18n("CursorPos = (Projection * Modelview)^(-1) * (%1 , %2 , %3 , %4)^T:").
		subs(v[0], 6, 'f', 3).
		subs(v[1], 6, 'f', 3).
		subs(v[2], 6, 'f', 3).
		subs(v[3], 6, 'f', 3).toString();
 QString resultText = ki18n("(%1 , %2 , %3 , %3)^T").
		subs(result[0], 6, 'f', 3).
		subs(result[1], 6, 'f', 3).
		subs(result[2], 6, 'f', 3).
		subs(result[3], 6, 'f', 3).toString();
 if (result[3] == 0.0f) {
	d->mMatricesDebugText->setText("ERROR");
	boError() << k_funcinfo << endl;
	return;
 }
 QString realCoords = i18n("x = %1  ;  y = %2  ;  z = %3", 
		result[0] / result[3], 
		result[1] / result[3], 
		result[2] / result[3]);



 // display the planes. they consist of the normal vector and the
 // distance from the origin
 QString planes = i18n("Right Plane: %1\n", planeDebugString(d->mViewFrustum.right()));
 planes += i18n("Left Plane: %1\n", planeDebugString(d->mViewFrustum.left()));
 planes += i18n("Bottom Plane: %1\n", planeDebugString(d->mViewFrustum.bottom()));
 planes += i18n("Top Plane: %1\n", planeDebugString(d->mViewFrustum.top()));
 planes += i18n("Far Plane: %1\n", planeDebugString(d->mViewFrustum.far()));
 planes += i18n("Near Plane: %1", planeDebugString(d->mViewFrustum.near()));

 d->mMatricesDebugText->setText(i18n("%1\n%2\n%3\n\n%4",
		 text, resultText, realCoords,
		 planes));
}

void BosonGameGUI::updateLabelItemWorkStatistics()
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
 text += i18n("Total items: %1\n", canvas()->allItemsCount());
 text += i18n("-1 (items): %1\n", workCounts[-1]),
 text += i18n("Idle:     %1\n", workCounts[(int)UnitBase::WorkIdle]);
 text += i18n("Moving or turning: %1\n", 
		workCounts[(int)UnitBase::WorkMove] +
		workCounts[(int)UnitBase::WorkTurn]);
 text += i18n("Attacking:         %1\n", 
		workCounts[(int)UnitBase::WorkAttack]);
 text += i18n("Other:             %1\n", 
		workCounts[(int)UnitBase::WorkConstructed] +
		workCounts[(int)UnitBase::WorkDestroyed] +
		workCounts[(int)UnitBase::WorkFollow] +
		workCounts[(int)UnitBase::WorkPlugin]);

 d->mItemWorkStatistics->setText(text);
}

void BosonGameGUI::updateLabelOpenGLCamera()
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
 text += i18n("LookAt: (%1; %2; %3)\n", lookAt.x(), 
		lookAt.y(), lookAt.z());
 text += i18n("CameraPos: (%1; %2; %3)\n", cameraPos.x(), 
		cameraPos.y(), cameraPos.z());
 text += i18n("Up: (%1; %2; %3)\n", up.x(), 
		up.y(), up.z());
 text += i18n("Distance: %1\n", camera()->distance());
 text += i18n("Rotation: %1\n", camera()->rotation());
 text += i18n("XRotation: %1\n", camera()->xRotation());

 d->mOpenGLCamera->setText(text);
}

void BosonGameGUI::updateLabelRenderCounts()
{
 if (!boConfig->boolValue("debug_rendercounts")) {
	d->mRenderCounts->setVisible(false);
	return;
 }
 d->mRenderCounts->setVisible(true);
 QString text;
#define HAVE_CANVAS_RENDERER 0
#if HAVE_CANVAS_RENDERER
 text += i18n("Items rendered: %1\n", d->mCanvasRenderer->renderedItems());
 text += i18n("Particles rendered: %1\n", d->mCanvasRenderer->renderedParticles());
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
 text += i18n("Texture binds: %1 (C: %2; I: %3; W: %4; P: %5)\n",
		 boTextureManager->textureBinds(), d->mCanvasRenderer->textureBindsCells(), d->mCanvasRenderer->textureBindsItems(), d->mCanvasRenderer->textureBindsWater(), d->mCanvasRenderer->textureBindsParticles());
#endif

 d->mRenderCounts->setText(text);
}

void BosonGameGUI::updateLabelAdvanceCalls()
{
 if (!boConfig->boolValue("debug_advance_calls")) {
	d->mAdvanceCalls->setVisible(false);
	return;
 }
 d->mAdvanceCalls->setVisible(true);
 QString text;
 text += i18n("Advance calls passed: %1\n", boGame->advanceCallsCount());
 text += i18n("Delayed messages: %1 (delayed advance messages: %2)\n", boGame->delayedMessageCount(), boGame->delayedAdvanceMessageCount());
 text += i18n("Advance message interval: %1 ms\n", Boson::advanceMessageInterval());
 text += i18n("Game speed (advance calls per advance message): %1\n", boGame->gameSpeed());
 d->mAdvanceCalls->setText(text);
}

void BosonGameGUI::updateLabelTextureMemory()
{
 if (!boConfig->boolValue("debug_texture_memory")) {
	d->mTextureMemory->setVisible(false);
	return;
 }
 d->mTextureMemory->setVisible(true);
 QString text;
 text += i18n("Texture memory in use (approximately): %1 kb\n", boTextureManager->usedTextureMemory() / 1024);
 text += i18n(" in %1 texures\n", boTextureManager->textureCount());
 d->mTextureMemory->setText(text);
}

void BosonGameGUI::updateLabelMemoryUsage()
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

void BosonGameGUI::updateLabelCPUUsage()
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

void BosonGameGUI::setCursorCanvasVector(const BoVector3Fixed* v)
{
 d->mCursorCanvasVector = v;
}

void BosonGameGUI::setCursorWidgetPos(const QPoint* pos)
{
 d->mCursorWidgetPos = pos;
}

void BosonGameGUI::setCursorRootPos(const QPoint* pos)
{
 d->mUfoCommandFrame->setCursorRootPos(pos);
}

const BoVector3Fixed& BosonGameGUI::cursorCanvasVector() const
{
 return *d->mCursorCanvasVector;
}

const QPoint& BosonGameGUI::cursorWidgetPos() const
{
 return *d->mCursorWidgetPos;
}

void BosonGameGUI::setCanvas(const BosonCanvas* c)
{
 d->mCanvas = c;
}

const BosonCanvas* BosonGameGUI::canvas() const
{
 return d->mCanvas;
}

void BosonGameGUI::setCamera(BoGameCamera* c)
{
 d->mCamera = c;
}

BoGameCamera* BosonGameGUI::camera() const
{
 return d->mCamera;
}

bool BosonGameGUI::isChatVisible() const
{
 return d->mChat->isVisible();
}

void BosonGameGUI::setGameMode(bool mode)
{
 d->mUfoCommandFrame->setGameMode(mode);
 d->mChat->setKGame(boGame);
}

void BosonGameGUI::slotShowPlaceFacilities(PlayerIO* io)
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeFacilities(io);
}

void BosonGameGUI::slotShowPlaceMobiles(PlayerIO* io)
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeMobiles(io);
}

void BosonGameGUI::slotShowPlaceGround()
{
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 d->mUfoCommandFrame->placeGround();
}

void BosonGameGUI::addChatMessage(const QString& message)
{
 d->mChat->addMessage(message);
}

void BosonGameGUI::slotBoDebugOutput(const BoDebugMessage& m)
{
#if 0
 QString area = m.areaName();
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 QString from = i18n("DEBUG: ");
 if (!area.isEmpty()) {
	from = i18n("DEBUG (%1): ", area);
 }
 d->mDebugDisplay->addMessage(i18n("%1: %2", from, message));
#else
#warning FIXME: BoDebugLog is disabled
 boWarning() << k_funcinfo << "FIXME: BoDebugLog is disabled";
#endif
}

void BosonGameGUI::slotBoDebugWarning(const BoDebugMessage& m)
{
#if 0
 QString area = m.areaName();
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 QString from = i18n("WARNING: ");
 if (!area.isEmpty()) {
	from = i18n("WARNING(%1): ", area);
 }
 d->mErrorDisplay->addMessage(i18n("%1: %2", from, message));
#else
#warning FIXME: BoDebugLog is disabled
 boWarning() << k_funcinfo << "FIXME: BoDebugLog is disabled";
#endif
}

void BosonGameGUI::slotBoDebugError(const BoDebugMessage& m)
{
#if 0
 QString area = m.areaName();
 QString message = m.message();
 if (!message.isEmpty() && message[message.length() - 1] == '\n') {
	message = message.left(message.length() - 1);
 }
 QString from = i18n("ERROR: ");
 if (!area.isEmpty()) {
	from = i18n("ERROR(%1): ", area);
 }
 d->mErrorDisplay->addMessage(i18n("%1: %2", from, message));
#else
#warning FIXME: BoDebugLog is disabled
 boWarning() << k_funcinfo << "FIXME: BoDebugLog is disabled";
#endif
}


