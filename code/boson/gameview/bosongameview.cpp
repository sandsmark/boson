/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2006 Rivo Laks (rivolaks@hot.ee)

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

#include "bosongameview.h"
#include "bosongameview.moc"

#include "../bomemory/bodummymemory.h"
#include "../no_player.h"
#include "../defines.h"
#include "../gameengine/bosoncanvas.h"
#include "../gameengine/bosoncanvasstatistics.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/cell.h"
#include "../gameengine/boitemlist.h"
#include "../gameengine/rtti.h"
#include "../gameengine/unit.h"
#include "../gameengine/unitproperties.h"
#include "../gameengine/speciestheme.h"
#include "../speciesdata.h"
#include "../gameengine/playerio.h"
#include "boselection.h"
#include "../bosonconfig.h"
#include "../bosonprofiling.h"
#include "../gameengine/boson.h"
#include "bodebug.h"
#include "../gameengine/bosonshot.h"
#include "../gameengine/unitplugins.h"
#include "../bo3dtools.h"
#include "bosongameviewinputbase.h"
#include "bosongameviewinput.h"
#include "editorviewinput.h"
#include "../bogltooltip.h"
#include "../bosongroundthemedata.h"
#include "../bocamera.h"
#include "../boautocamera.h"
#include "../bogroundrenderer.h"
#include "../bogroundrenderermanager.h"
#include "../modelrendering/bomeshrenderermanager.h"
#include "../bolight.h"
#include "../bosonglminimap.h"
#include "../info/boinfo.h"
#include "../speciesdata.h"
#include "../bowaterrenderer.h"
#include "../botexture.h"
#include "../boufo/boufoaction.h"
#include "../bosonufominimap.h"
#include "bosonufogamegui.h"
#include "bosonlocalplayerinput.h"
#include "../gameengine/bosonplayfield.h"
#include "../bosondata.h"
#include "../bocamerawidget.h"
#include "../boaction.h"
#include "../gameengine/boeventlistener.h"
#include "bosonmenuinput.h"
#include "../boshader.h"
#include "bosonufogamewidgets.h"
#include "bosonufocanvaswidget.h"
#include "../gameengine/script/bosonscript.h"
#include "../gameengine/script/bosonscriptinterface.h"
#include "../bomousemovediff.h"
#include "../bosonfpscounter.h"
#include "../bosongameviewpluginmanager.h"
#include "../bosongameviewpluginbase.h"
#include "../bosonviewdata.h"
#include "bosongamevieweventlistener.h"
#include "bosoneffectpropertiesparticle.h"

#include <kgame/kgameio.h>
#include <kgame/kplayer.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include "../gameengine/boeventloop.h"

#include <qtimer.h>
#include <qcursor.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qdir.h>
#include <qdom.h>
#include <qvaluevector.h>
#include <qapplication.h>


#warning d->mMouseMoveDiff.start() mostly removed. probably fix displayInput()->actionLoced() actions !


// #define CAMERA_MODIFIER_CONFIGURABLE 1

static bool cameraModifier(const BoMouseEvent& event)
{
 // One button is the "camera modifier". When this button is pressed, mouse move
 // events behave differently.
 //
 // Note that in a state diagram a mouse move event with the camera modifier
 // being pressed causes a state change: not only has the mousemove event a
 // different meaning, but also the button release event will behave differently
 // (it does not cause any action)!
 // -> however if the camera modifier is released before the button is released,
 //    the original state is entered again.
 bool cameraModifier = false;
#ifdef CAMERA_MODIFIER_CONFIGURABLE
 if (boConfig->boolValue("CameraModifier") == "ALT") {
	cameraModifier = event.altButton();
 } else if (boConfig->boolValue("CameraModifier") == "Shift") {
	cameraModifier = event.shiftButton();
 } else if (boConfig->boolValue("CameraModifier") == "CTRL") {
	cameraModifier = event.ctrlButton();
 }
#else
 cameraModifier = event.altButton();
#endif
 return cameraModifier;
}


class BoMouseDoubleClickRecognizer
{
public:
	BoMouseDoubleClickRecognizer()
	{
		mButton = Qt::NoButton;
		mCount = 0;
	}

	void mouseMoveEvent(QMouseEvent*)
	{
		// when the mouse was pressed twice but the second press is
		// hold down and moved then it isn't a double click anymore.
		reset();
	}
	void mousePressEvent(QMouseEvent* e)
	{
		if (mTimeSinceLastPress.elapsed() > QApplication::doubleClickInterval()) {
			reset();
		}
		mTimeSinceLastPress.restart();
		if (mCount == 0) {
			mButton = e->button();
			mCount++;
		} else {
			if (mButton == e->button()) {
				mCount++;
			} else {
				reset();
			}
		}
	}

	/**
	 * @return TRUE when this is a double click release event.
	 **/
	bool mouseReleaseEvent(QMouseEvent*)
	{
		if (mCount >= 2) {
			return true;
		}
		return false;
	}

protected:
	void reset()
	{
		mButton = Qt::NoButton;
		mCount = 0;
	}

private:
	Qt::ButtonState mButton;
	int mCount;
	QTime mTimeSinceLastPress;
};

BoMouseButtonState::BoMouseButtonState()
{
 mButtonIsReleased = true;
 mIsMove = false;
 mIsCameraAction = false;

 mCurrentWidgetPosDiffX = 0;
 mCurrentWidgetPosDiffY = 0;
}

BoMouseButtonState::~BoMouseButtonState()
{
}

void BoMouseButtonState::pressButton(const QPoint& pos)
{
 mButtonIsReleased = false;

 mStartWidgetPos = pos;

 mCurrentWidgetPos = mStartWidgetPos;

 mCurrentWidgetPosDiffX = 0;
 mCurrentWidgetPosDiffY = 0;
}

void BoMouseButtonState::releaseButton(const BoMouseEvent& modifiers, bool doubleRelease)
{
 if (mButtonIsReleased) {
	return;
 }
 mButtonIsReleased = true;

 if (mIsMove) {
	if (!mIsCameraAction) {
		actionAfterMove(modifiers);
	}
 } else {
	if (doubleRelease) {
		actionDouble(modifiers);
	} else {
		action(modifiers);
	}
 }
 mIsMove = false;
 mIsCameraAction = false;
}

void BoMouseButtonState::mouseMoved(const BoMouseEvent& e)
{
 if (mButtonIsReleased) {
	return;
 }

 // diff = currentPos - oldPos
 mCurrentWidgetPosDiffX = e.gameViewWidgetPos().x() - currentWidgetPos().x();
 mCurrentWidgetPosDiffY = e.gameViewWidgetPos().y() - currentWidgetPos().y();

 // update currentPos
 mCurrentWidgetPos = e.gameViewWidgetPos();

 mIsMove = true;
 mIsCameraAction = cameraModifier(e);
 if (mIsCameraAction) {
	cameraAction(e);
 } else {
	moveAction(e);
 }
}

void BoMouseButtonState::actionDouble(const BoMouseEvent& modifiers)
{
 action(modifiers);
}




class BoGameViewMouseState : public BoMouseButtonState
{
public:
	BoGameViewMouseState(BosonGameView* gameView, const BoGLMatrices* matrices)
		: BoMouseButtonState()
	{
		mGameView = gameView;
		mGameGLMatrices = matrices;
	}

	BosonGameView* gameView() const { return mGameView; }
	BoSelection* selection() const { return gameView()->selection(); }
	PlayerIO* localPlayerIO() const { return gameView()->localPlayerIO(); }
	BoGameCamera* camera() const { return gameView()->camera(); }
	BosonGameViewInputBase* displayInput() const { return gameView()->displayInput(); }
	const BosonCanvas* canvas() const { return gameView()->canvas(); }
	const BoGLMatrices* gameGLMatrices() const { return mGameGLMatrices; }
	bool mapDistance(int windx, int windy, GLfloat* dx, GLfloat* dy) const
	{
		return Bo3dTools::mapDistance(gameGLMatrices()->modelviewMatrix(),
				gameGLMatrices()->projectionMatrix(),
				gameGLMatrices()->viewport(),
				windx, windy, dx, dy);
	}

private:
	BosonGameView* mGameView;
	const BoGLMatrices* mGameGLMatrices;
};

class BoLeftMouseButtonState : public BoGameViewMouseState
{
public:
	BoLeftMouseButtonState(BosonGameView* v, const BoGLMatrices* m) : BoGameViewMouseState(v, m)
	{
		mSelectionRect = 0;
		mUfoCanvasWidget = 0;
	}

	void setSelectionRect(SelectionRect* r)
	{
		mSelectionRect = r;
	}
	void setUfoCanvasWidget(const BosonUfoCanvasWidget* w)
	{
		mUfoCanvasWidget = w;
	}

protected:
	virtual void action(const BoMouseEvent&);
	virtual void actionDouble(const BoMouseEvent&);
	virtual void actionAfterMove(const BoMouseEvent&);
	virtual void cameraAction(const BoMouseEvent&);
	virtual void moveAction(const BoMouseEvent&);

private:
	SelectionRect* mSelectionRect;
	const BosonUfoCanvasWidget* mUfoCanvasWidget;
};

class BoRightMouseButtonState : public BoGameViewMouseState
{
public:
	BoRightMouseButtonState(BosonGameView* v, const BoGLMatrices* m) : BoGameViewMouseState(v, m)
	{
	}

protected:
	virtual void action(const BoMouseEvent&);
	virtual void actionDouble(const BoMouseEvent&);
	virtual void actionAfterMove(const BoMouseEvent&);
	virtual void cameraAction(const BoMouseEvent&);
	virtual void moveAction(const BoMouseEvent&);
};

class BoMiddleMouseButtonState : public BoGameViewMouseState
{
public:
	BoMiddleMouseButtonState(BosonGameView* v, const BoGLMatrices* m) : BoGameViewMouseState(v, m)
	{
	}

protected:
	virtual void action(const BoMouseEvent&);
	virtual void actionDouble(const BoMouseEvent&);
	virtual void actionAfterMove(const BoMouseEvent&);
	virtual void cameraAction(const BoMouseEvent&);
	virtual void moveAction(const BoMouseEvent&);
};

void BoLeftMouseButtonState::action(const BoMouseEvent& e)
{
 BO_CHECK_NULL_RET(displayInput());
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(boViewData);
 BO_CHECK_NULL_RET(mUfoCanvasWidget);

 if (displayInput()->actionLocked()) {
	// basically the same as a normal RMB
	displayInput()->actionClicked(e);
	return;
 }

 Unit* unit = mUfoCanvasWidget->unitAtWidgetPos(startedAtWidgetPos());
 bool playSelectSound = false;

 if (e.controlButton()) {
	if (unit) {
		if (localPlayerIO()->ownsUnit(unit)) {
			playSelectSound = true;
		}
		displayInput()->selectSingle(unit, false);
	}
 } else if (e.shiftButton()) {
	if (unit) {
		BoItemList* list = new BoItemList();
		list->append(unit);
		displayInput()->unselectArea(list);
	}
 } else {
	if (unit) {
		if (localPlayerIO()->ownsUnit(unit)) {
			playSelectSound = true;
		}
		displayInput()->selectSingle(unit, true);
	} else {
		selection()->clear();
	}
 }

 if (unit && playSelectSound) {
	boViewData->speciesData(unit->speciesTheme())->playSound(unit, SoundOrderSelect);
 }
}

void BoLeftMouseButtonState::actionDouble(const BoMouseEvent& e)
{
 BO_CHECK_NULL_RET(displayInput());

 // we ignore UnitAction is locked here currently!
 bool replace = !e.controlButton();
 bool onScreenOnly = !e.shiftButton();
 Unit* unit = e.unitAtEventPos();
 if (unit) {
	if (onScreenOnly) {
		boDebug() << k_funcinfo << "TODO: select only those that are currently on the screen!" << endl;
	}
	if (!displayInput()->selectAll(unit->unitProperties(), replace)) {
		displayInput()->selectSingle(unit, replace);
	}
 }
}

void BoLeftMouseButtonState::actionAfterMove(const BoMouseEvent& e)
{
 BO_CHECK_NULL_RET(displayInput());
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(boViewData);
 BO_CHECK_NULL_RET(mSelectionRect);
 BO_CHECK_NULL_RET(mUfoCanvasWidget);

 BoItemList* items = mSelectionRect->items(mUfoCanvasWidget);
 bool playSelectionSoundForLeader = false;
 if (e.controlButton()) {
	if (selection()->count() > 0) {
		playSelectionSoundForLeader = true;
	}
	displayInput()->selectArea(items, false);
 } else if (e.shiftButton()) {
	playSelectionSoundForLeader = false;
	displayInput()->unselectArea(items);
 } else {
	playSelectionSoundForLeader = true;
	displayInput()->selectArea(items, true);
 }
 mSelectionRect->setVisible(false);
 if (playSelectionSoundForLeader && !selection()->isEmpty()) {
	Unit* u = selection()->leader();
	if (localPlayerIO()->ownsUnit(u)) {
		boViewData->speciesData(u->speciesTheme())->playSound(u, SoundOrderSelect);
	}
 }
}

void BoLeftMouseButtonState::cameraAction(const BoMouseEvent& e)
{
 Q_UNUSED(e);
 camera()->changeDistance(currentWidgetPosDiffY());
}

void BoLeftMouseButtonState::moveAction(const BoMouseEvent& e)
{
 BO_CHECK_NULL_RET(mSelectionRect);
 if (!displayInput()->actionLocked()) {
	// TODO: use canvas coordinates for the selection rect.
	//       atm the widget size is the largest possible area for the
	//       selection rect, but that is not usually desired.
	//       also, when drawing a selection rect and using RMB moving at the
	//       same time, very unuseful things happens (the rect is scrolled
	//       along with the screen - its start position should stay at the
	//       same canvas position ideally)
	mSelectionRect->setVisible(true);
	mSelectionRect->setStartWidgetPos(startedAtWidgetPos());
	mSelectionRect->setEndWidgetPos(e.gameViewWidgetPos());
 } else {
	if (!boGame->gameMode() && displayInput()->actionType() != ActionChangeHeight) {
		// In editor mode, try to place unit/ground whenever mouse moves. This
		//  enables you to edit big areas easily. But it's not done for height
		//  changing (yet).
		displayInput()->actionClicked(e);
	}
 }
}


void BoRightMouseButtonState::action(const BoMouseEvent& e)
{
 if (displayInput()->actionLocked()) {
	displayInput()->unlockAction();
	displayInput()->updateCursor();
 } else {
	displayInput()->actionClicked(e);
 }
}

void BoRightMouseButtonState::actionDouble(const BoMouseEvent& e)
{
 action(e);
}

void BoRightMouseButtonState::actionAfterMove(const BoMouseEvent& e)
{
 Q_UNUSED(e);
 // no action should be taken here (RMB move is ended)
}

void BoRightMouseButtonState::cameraAction(const BoMouseEvent& e)
{
 Q_UNUSED(e);
 BO_CHECK_NULL_RET(camera());
 camera()->changeRotation(currentWidgetPosDiffX());
 camera()->changeXRotation(currentWidgetPosDiffY());
}

void BoRightMouseButtonState::moveAction(const BoMouseEvent& e)
{
 if (boConfig->boolValue("RMBMove")) {
	// problem is that QCursor::setPos() also causes
	// a mouse move event. we can use this hack in
	// order to check whether it is a real mouse
	// move event or we caused it here.
	// TODO: use d->mMouseMoveDiff.x()/y() in
	// paintGL() for the cursor, not QCursor::pos()
//	static bool a = false;
//	if (a) {
//		a = false;
//		break;
//	}
//	a = true;moveLookAtBy
//	QPoint pos = mapToGlobal(QPoint(d->mMouseMoveDiff.oldX(), d->mMouseMoveDiff.oldY()));
//	QCursor::setPos(pos);

	// modifiers are ignored.
	GLfloat dx, dy;
	int moveX = currentWidgetPosDiffX();
	int moveY = currentWidgetPosDiffY();
	mapDistance(moveX, moveY, &dx, &dy);
	camera()->changeLookAt(BoVector3Float(dx, dy, 0));
 }
}


void BoMiddleMouseButtonState::action(const BoMouseEvent& e)
{
 // we ignore all modifiers here, currently.
 if (boConfig->boolValue("MMBMove")) {
	float posX, posY, posZ;
	e.groundWorldPos(&posX, &posY, &posZ);
	int cellX, cellY;
	cellX = (int)(posX);
	cellY = (int)(-posY);
	gameView()->slotReCenterDisplay(QPoint(cellX, cellY));
	displayInput()->updateCursor();
 }
}

void BoMiddleMouseButtonState::actionDouble(const BoMouseEvent& e)
{
 action(e);
}

void BoMiddleMouseButtonState::actionAfterMove(const BoMouseEvent& e)
{
 action(e);
}

void BoMiddleMouseButtonState::cameraAction(const BoMouseEvent& e)
{
 Q_UNUSED(e);
}

void BoMiddleMouseButtonState::moveAction(const BoMouseEvent& e)
{
 Q_UNUSED(e);
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


BoItemList* SelectionRect::items(const BosonUfoCanvasWidget* ufoCanvasWidget) const
{
 if (!ufoCanvasWidget) {
	BO_NULL_ERROR(ufoCanvasWidget);
	return new BoItemList();
 }
 QRect widgetRect_;
 widgetRect(&widgetRect_);

 QValueList<BosonItem*> items = ufoCanvasWidget->itemsAtWidgetRect(widgetRect_);

 BoItemList* list = new BoItemList();
 for (QValueList<BosonItem*>::iterator it = items.begin(); it != items.end(); ++it) {
	list->append(*it);
 }

 return list;
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

/**
 * This class is a simple container class for the @ref BoUfoGameGUI class. It
 * exists only so that we can easily profile the time that is spent in @ref
 * paint.
 **/
class BoUfoGameGUIContainer : public BoUfoCustomWidget
{
public:
	virtual void paint()
	{
		PROFILE_METHOD
		BoUfoCustomWidget::paint();
	}
};





class BosonGameViewPrivate
{
public:
	BosonGameViewPrivate()
	{
		mViewData = 0;
		mLayeredPane = 0;
		mUfoCanvasWidget = 0;
		mUfoGameGUI = 0;
		mToolTipLabel = 0;
		mUfoCursorWidget = 0;
		mUfoSelectionRectWidget = 0;
		mUfoFPSGraphWidget = 0;
		mUfoProfilingGraphWidget = 0;

		mFPSCounter = 0;
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

		mGameViewPlugin = 0;
		mGameViewPluginWidget = 0;
		mGameViewPluginWidgetContainer = 0;

		mEventListener = 0;

		mLeftButtonState = 0;
		mMiddleButtonState = 0;
		mRightButtonState = 0;
	}

	BosonViewData* mViewData;
	BoUfoLayeredPane* mLayeredPane;
	BosonUfoCanvasWidget* mUfoCanvasWidget;
	BosonUfoPlacementPreviewWidget* mUfoPlacementPreviewWidget;
	BosonUfoLineVisualizationWidget* mUfoLineVisualizationWidget;
	BosonUfoGameGUI* mUfoGameGUI;
	BoUfoLabel* mToolTipLabel;
	BosonUfoCursorWidget* mUfoCursorWidget;
	BosonUfoSelectionRectWidget* mUfoSelectionRectWidget;
	BosonUfoFPSGraphWidget* mUfoFPSGraphWidget;
	BosonUfoProfilingGraphWidget* mUfoProfilingGraphWidget;

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

	BoCursorPos mCursorPos;
	BoGameCamera mCamera;
	BoSelectionGroup* mSelectionGroups;

	GLint mViewport[4];
	BoMatrix mProjectionMatrix;
	BoMatrix mModelviewMatrix;
	BoFrustum mViewFrustum;
	BoGLMatrices* mGameGLMatrices;
	GLfloat mFovY; // see gluPerspective
	GLfloat mAspect; // see gluPerspective

	BoLightCameraWidget1* mLightWidget;

	BosonGameViewPluginBase* mGameViewPlugin;
	BoUfoWidget* mGameViewPluginWidget;
	BoUfoWidget* mGameViewPluginWidgetContainer;

	BosonGameViewEventListener* mEventListener;

	BoMouseDoubleClickRecognizer mDoubleClickRecognizer;
	BoLeftMouseButtonState* mLeftButtonState;
	BoMiddleMouseButtonState* mMiddleButtonState;
	BoRightMouseButtonState* mRightButtonState;

	bool mGameMode;
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
 delete d->mEventListener;
 boDebug() << k_funcinfo << "quitGame() done" << endl;
 if (!removeWidget(d->mLayeredPane)) { // AB: do NOT delete it directly!
	// AB: probably d->mLayeredPane is not a direct child of this widget
	// anymore?
	boError() << k_funcinfo << "could not remove layered pane. maybe widget design is changed?" << endl;
 }
 boDebug() << k_funcinfo << "layered pane removed" << endl;
 delete d->mActionCollection;
 delete d->mSelectionRect;
 delete d->mScriptConnector;
 delete d->mSelectionGroups;
 delete mSelection;
 delete d->mGameGLMatrices;
 delete d->mGLMiniMap;
 delete d->mToolTips;
 d->mGameViewPlugin = 0;
 BoMeshRendererManager::manager()->unsetCurrentRenderer();
 BoGroundRendererManager::manager()->unsetCurrentRenderer();
 BosonGameViewPluginManager::manager()->unsetCurrentPlugin();
 SpeciesData::clearSpeciesData();
 BoGroundRendererManager::deleteStatic();
 BoMeshRendererManager::deleteStatic();
 BosonGameViewPluginManager::deleteStatic();
 BoWaterRenderer::deleteStatic();
 BoLightManager::deleteStatic();
 BosonViewData::setGlobalViewData(0);
 delete d->mViewData;
 delete d->mLeftButtonState;
 delete d->mMiddleButtonState;
 delete d->mRightButtonState;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonGameView::init()
{
 PROFILE_METHOD
 d = new BosonGameViewPrivate;
 mCanvas = 0;
 d->mGameMode = true;
 d->mInputInitialized = false;
 d->mFPSCounter = 0;

 setName("BosonGameView");

 d->mViewData = new BosonViewData(this);
 BosonViewData::setGlobalViewData(d->mViewData);

 d->mFovY = 60.0f;

 for (int i = 0; i < 4; i++) {
	d->mViewport[i] = 0;
 }

 boProfiling->push("initStatic");
 BoLightManager::initStatic();
 BoWaterRenderer::initStatic();
 BoGroundRendererManager::initStatic();
 BoMeshRendererManager::initStatic();
 BosonGameViewPluginManager::initStatic();
 boProfiling->pop();
 BoMeshRendererManager::manager()->makeRendererCurrent(QString::null);
 BoGroundRendererManager::manager()->makeRendererCurrent(QString::null);
 BosonGameViewPluginManager::manager()->makePluginCurrent(QString::null);
 resetGameViewPlugin();

 boWaterRenderer->setViewFrustum(&d->mViewFrustum);


 d->mGameGLMatrices = new BoGLMatrices(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewFrustum, d->mViewport, d->mFovY, d->mAspect);
 BoGroundRendererManager::manager()->setMatrices(&d->mModelviewMatrix, &d->mProjectionMatrix, d->mViewport);
 BoGroundRendererManager::manager()->setViewFrustum(&d->mViewFrustum);
 resetGameViewPlugin(); // set GL matrices

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

 d->mLeftButtonState = new BoLeftMouseButtonState(this, d->mGameGLMatrices);
 d->mLeftButtonState->setSelectionRect(d->mSelectionRect);
 d->mMiddleButtonState = new BoMiddleMouseButtonState(this, d->mGameGLMatrices);
 d->mRightButtonState = new BoRightMouseButtonState(this, d->mGameGLMatrices);




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
 boWaterRenderer->setSun(l);
 BoShader::setSun(l);


 boWaterRenderer->initOpenGL();
 boConfig->setBoolValue("TextureFOW", boTextureManager->textureUnits() > 1);
 if (!BosonGroundThemeData::shadersSupported()) {
	boConfig->setBoolValue("UseGroundShaders", false);
 }


 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << endl;
 }




 initUfoGUI();

 connect(this, SIGNAL(signalWidgetResized()),
		this, SLOT(slotWidgetResized()));
 connect(this, SIGNAL(signalWidgetShown(ufo::UWidgetEvent*)),
		this, SLOT(slotWidgetShown()));
 connect(this, SIGNAL(signalWidgetHidden(ufo::UWidgetEvent*)),
		this, SLOT(slotWidgetHidden()));
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
 setFocusEventsEnabled(true);
}

void BosonGameView::quitGame()
{
 boDebug() << k_funcinfo << endl;
 resetGameMode();

 d->mGameViewPlugin->quitGame();
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

bool BosonGameView::initializeItems()
{
 if (!d->mUfoCanvasWidget->initializeItems()) {
	boError() << k_funcinfo << "ufo canvas widget could not initialize items" << endl;
	return false;
 }
 return true;
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

 d->mProjectionMatrix = createMatrixFromOpenGL(GL_PROJECTION_MATRIX);
 d->mViewFrustum.loadViewFrustum(d->mModelviewMatrix, d->mProjectionMatrix);

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

const BoVector3Fixed& BosonGameView::cursorCanvasVector() const
{
 return d->mCursorPos.canvasVector();
}

const QPoint& BosonGameView::cursorWidgetPos() const
{
 return d->mCursorPos.gameViewPos();
}

void BosonGameView::updateCursorCanvasVector(const QPoint& cursorGameViewPos)
{
 GLfloat x = 0.0, y = 0.0, z = 0.0;
 mapCoordinatesToGround(cursorGameViewPos, &x, &y, &z);
 BoVector3Fixed canvas(x, -y, z); // AB: are these already real z coordinates?
 QPoint rootPanePos = mapToRoot(cursorGameViewPos);

 d->mCursorPos.set(cursorGameViewPos, rootPanePos, canvas);

 emit signalCursorCanvasVectorChanged(d->mCursorPos.canvasVector());
}

void BosonGameView::setGameFPSCounter(BosonGameFPSCounter* counter)
{
 d->mFPSCounter = counter;
 d->mUfoGameGUI->setGameFPSCounter(gameFPSCounter());
 d->mUfoFPSGraphWidget->setGameFPSCounter(gameFPSCounter());
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

 d->mModelviewMatrix = createMatrixFromOpenGL(GL_MODELVIEW_MATRIX);
 d->mViewFrustum.loadViewFrustum(d->mModelviewMatrix, d->mProjectionMatrix);
 BoGroundRenderer* renderer = BoGroundRendererManager::manager()->currentRenderer();
 if (renderer) {
	BosonMap* map = 0;
	if (canvas()) {
		map = canvas()->map();
	}
	renderer->generateCellList(map);
 }

 BoLightManager::manager()->cameraChanged();

 d->mUfoCanvasWidget->cameraChanged();

 boWaterRenderer->modelviewMatrixChanged(d->mModelviewMatrix);
 boWaterRenderer->setCameraPos(camera()->cameraPos());
 BoShader::setCameraPos(camera()->cameraPos());

 updateCursorCanvasVector(cursorWidgetPos());
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

bool BosonGameView::mapCoordinatesToGround(const QPoint& widgetPos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth) const
{
 BoVector3Float p;
 bool ret = d->mUfoCanvasWidget->emulatePickGroundPos(widgetPos, &p);
 if (ret) {
	*posX = p.x();
	*posY = p.y();
	*posZ = p.z();
 }
 return ret;
}

bool BosonGameView::mapDistance(int windx, int windy, GLfloat* dx, GLfloat* dy) const
{
 return Bo3dTools::mapDistance(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewport,
		windx, windy, dx, dy);
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

void BosonGameView::slotCenterOnSelectionGroup(int n)
{
 BO_CHECK_NULL_RET(selection());
 d->mSelectionGroups->slotSelectSelectionGroup(n);

 // AB: you know games where ALT+num centers the center of the selection group?
 //     you know situations when the group is spread over the whole map and when
 //     centering on the center of that group not even a single unit of that
 //     group is visible?
 //     you too think that sucks?
 //     I do!
 //     -> we center on the leader only.
 //
 //     if we ever decide to center on the center of the map (i.e. take
 //     positions of all units of the group into account), then we should do
 //     1. check if all unis are on the screen
 //     2. if not, center on the unit that is closest to the center of the group
 Unit* leader = selection()->leader();
 if (!leader) {
	return;
 }
 slotReCenterDisplay(QPoint((int)leader->x(), (int)leader->y()));
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

 camera()->changeDistance(delta);
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
 PROFILE_METHOD
 glPushAttrib(GL_ALL_ATTRIB_BITS);

 // AB: note that BoUfo widgets differ from usual Qt widgets API-wise.
 // You need to create them without a parent and then add them to their parent
 // widget using parent->addWidget(child). This also adds child to the layout of
 // parent.
 // WARNING: ALL widget that are created MUST be added to another widget!
 // Otherwise the created widget won't be deleted!

 d->mLayeredPane = new BoUfoLayeredPane();
 d->mLayeredPane->setName("GameViewLayeredPane");
 d->mLayeredPane->setOpaque(false);
 addWidget(d->mLayeredPane);

 d->mUfoCanvasWidget = new BosonUfoCanvasWidget();
 d->mUfoCanvasWidget->setGameGLMatrices(d->mGameGLMatrices);
 d->mUfoCanvasWidget->setCamera(&d->mCamera);
 d->mUfoCanvasWidget->setCanvas(canvas());
 d->mLeftButtonState->setUfoCanvasWidget(d->mUfoCanvasWidget);

 d->mUfoPlacementPreviewWidget = new BosonUfoPlacementPreviewWidget();
 d->mUfoPlacementPreviewWidget->setGameGLMatrices(d->mGameGLMatrices);
 d->mUfoPlacementPreviewWidget->setCursorCanvasVectorPointer(d->mCursorPos.canvasVectorPointer());

 d->mUfoLineVisualizationWidget = new BosonUfoLineVisualizationWidget();
 d->mUfoLineVisualizationWidget->setGameGLMatrices(d->mGameGLMatrices);
 d->mUfoLineVisualizationWidget->setCanvas(canvas());

 BoUfoGameGUIContainer* ufoGameGUIContainer = new BoUfoGameGUIContainer();
 ufoGameGUIContainer->setName("ufoGameGUIContainer");
 d->mUfoGameGUI = new BosonUfoGameGUI(d->mModelviewMatrix, d->mProjectionMatrix, d->mViewFrustum, d->mViewport);
 d->mUfoGameGUI->setCursorWidgetPos(d->mCursorPos.gameViewPosPointer());
 d->mUfoGameGUI->setCursorCanvasVector(d->mCursorPos.canvasVectorPointer());
 d->mUfoGameGUI->setCursorRootPos(d->mCursorPos.rootPanePosPointer());
 d->mUfoGameGUI->setSelection(selection());
 d->mUfoGameGUI->setCanvas(canvas());
 d->mUfoGameGUI->setCamera(camera());
 d->mUfoGameGUI->setGLMiniMap(d->mGLMiniMap);
 connect(this, SIGNAL(signalSelectionChanged(BoSelection*)),
		d->mUfoGameGUI, SIGNAL(signalSelectionChanged(BoSelection*)));
 ufoGameGUIContainer->addWidget(d->mUfoGameGUI);

 d->mToolTipLabel = new BoUfoLabel();
 d->mToolTipLabel->setName("ToolTipLabel");
 d->mToolTipLabel->setForegroundColor(Qt::white);
 d->mToolTips->setLabel(d->mToolTipLabel);

 d->mUfoCursorWidget = new BosonUfoCursorWidget();
 d->mUfoCursorWidget->setGameGLMatrices(d->mGameGLMatrices);
 d->mUfoCursorWidget->setCursorWidgetPos(d->mCursorPos.gameViewPosPointer());
 connect(d->mUfoCursorWidget, SIGNAL(signalSetWidgetCursor(BosonCursor*)),
		this, SIGNAL(signalSetWidgetCursor(BosonCursor*)));

 d->mUfoSelectionRectWidget = new BosonUfoSelectionRectWidget();
 d->mUfoSelectionRectWidget->setGameGLMatrices(d->mGameGLMatrices);

 d->mUfoFPSGraphWidget = new BosonUfoFPSGraphWidget();
 d->mUfoFPSGraphWidget->setGameGLMatrices(d->mGameGLMatrices);
 d->mUfoFPSGraphWidget->setGameFPSCounter(gameFPSCounter());

 d->mUfoProfilingGraphWidget = new BosonUfoProfilingGraphWidget();
 d->mUfoProfilingGraphWidget->setGameGLMatrices(d->mGameGLMatrices);

 connect(d->mSelectionRect, SIGNAL(signalVisible(bool)),
		d->mUfoSelectionRectWidget, SLOT(slotSelectionRectVisible(bool)));
 connect(d->mSelectionRect, SIGNAL(signalChanged(const QRect&)),
		d->mUfoSelectionRectWidget, SLOT(slotSelectionRectChanged(const QRect&)));

 d->mGameViewPluginWidgetContainer = new BoUfoWidget();
 d->mGameViewPluginWidgetContainer->setName("GameViewPluginWidgetContainer");

 d->mLayeredPane->addWidget(d->mUfoCanvasWidget);
 d->mLayeredPane->addWidget(d->mUfoPlacementPreviewWidget);
 d->mLayeredPane->addWidget(d->mUfoLineVisualizationWidget);
 d->mLayeredPane->addWidget(ufoGameGUIContainer);
 d->mLayeredPane->addWidget(d->mToolTipLabel);
 d->mLayeredPane->addWidget(d->mUfoCursorWidget);
 d->mLayeredPane->addWidget(d->mUfoSelectionRectWidget);
 d->mLayeredPane->addWidget(d->mUfoFPSGraphWidget);
 d->mLayeredPane->addWidget(d->mUfoProfilingGraphWidget);
 d->mLayeredPane->addWidget(d->mGameViewPluginWidgetContainer);

#if 0
 d->mLayeredPane->setMouseEventsEnabled(true, true);
 d->mLayeredPane->setKeyEventsEnabled(true);
 d->mLayeredPane->setFocusEventsEnabled(true);
 d->mUfoGameGUI->setMouseEventsEnabled(true, true);
 d->mUfoGameGUI->setKeyEventsEnabled(true);
 d->mUfoGameGUI->setFocusEventsEnabled(true);
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
 delete d->mEventListener;
 d->mEventListener = 0;
 if (d->mInput) {
	d->mInput->setCanvas(mCanvas);
 }
 d->mUfoLineVisualizationWidget->setCanvas(mCanvas);
 d->mUfoGameGUI->setCanvas(mCanvas);
 d->mUfoCanvasWidget->setCanvas(mCanvas);
 d->mUfoPlacementPreviewWidget->setCanvas(mCanvas);
 resetGameViewPlugin(); // setCanvas()
 if (!mCanvas) {
	return;
 }
 d->mEventListener = new BosonGameViewEventListener(boGame->eventManager(), this);
 d->mEventListener->setCanvas(mCanvas);

 connect(d->mEventListener, SIGNAL(signalFacilityConstructed(Unit*)),
		d->mUfoCanvasWidget, SLOT(slotFacilityConstructed(Unit*)));

 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		d->mSelectionGroups, SLOT(slotRemoveItem(BosonItem*)));
 connect(mCanvas, SIGNAL(signalUnitRemoved(Unit*)),
		d->mSelectionGroups, SLOT(slotRemoveUnit(Unit*)));
 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		this, SLOT(slotRemovedItemFromCanvas(BosonItem*)));
 connect(mCanvas, SIGNAL(signalRemovedItem(BosonItem*)),
		mSelection, SLOT(slotRemoveItem(BosonItem*)));

 { // minimap
	if (previousCanvas) {
		disconnect(previousCanvas, 0, d->mGLMiniMap, 0);
		disconnect(d->mGLMiniMap, 0, previousCanvas, 0);
	}
	disconnect(d->mGLMiniMap, 0, this, 0);
	disconnect(d->mGLMiniMap, 0, displayInput(), 0);
	connect(mCanvas, SIGNAL(signalUnitMoved(Unit*, bofixed, bofixed)),
		d->mGLMiniMap, SLOT(slotUnitMoved(Unit*, bofixed, bofixed)));
	connect(mCanvas, SIGNAL(signalUnitRemoved(Unit*)),
		d->mGLMiniMap, SLOT(slotUnitRemoved(Unit*)));
	connect(mCanvas, SIGNAL(signalItemAdded(BosonItem*)),
		d->mGLMiniMap, SLOT(slotItemAdded(BosonItem*)));
	connect(boGame, SIGNAL(signalAdvance(unsigned int, bool)),
			d->mGLMiniMap, SLOT(slotAdvance(unsigned int)));
 connect(d->mEventListener, SIGNAL(signalFacilityConstructed(Unit*)),
		d->mGLMiniMap, SLOT(slotFacilityConstructed(Unit*)));

	connect(d->mGLMiniMap, SIGNAL(signalReCenterView(const QPoint&)),
			this, SLOT(slotReCenterDisplay(const QPoint&)));
	if (displayInput()) {
		connect(d->mGLMiniMap, SIGNAL(signalMoveSelection(int, int)),
				displayInput(), SLOT(slotMoveSelection(int, int)));
	}
 }

 d->mCamera.setCanvas(mCanvas);
 slotResetViewProperties();

 boDebug() << k_funcinfo << endl;

 d->mGLMiniMap->createMap(mCanvas, d->mGameGLMatrices);

 if (!boGame->gameMode()) { // AB: is this valid at this point?
	d->mUfoGameGUI->setGroundTheme(mCanvas->map()->groundTheme());
 }

}

void BosonGameView::bosonObjectCreated(Boson* boson)
{
 connect(boson, SIGNAL(signalAdvance(unsigned int, bool)),
		this, SLOT(slotAdvance(unsigned int, bool)));
 connect(boson, SIGNAL(signalChangeTexMap(int, int, unsigned int, unsigned int*, unsigned char*)),
		this, SLOT(slotChangeTexMap(int, int)));
 connect(boson, SIGNAL(signalChangeHeight(int, int, float)),
		this, SLOT(slotChangeHeight(int, int)));
 connect(boson, SIGNAL(signalGameOver()),
		this, SLOT(slotGameOver()));

 d->mUfoGameGUI->bosonObjectCreated(boson);
}

void BosonGameView::bosonObjectAboutToBeDestroyed(Boson* boson)
{
 d->mUfoGameGUI->bosonObjectAboutToBeDestroyed(boson);
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

 boWaterRenderer->setLocalPlayerIO(localPlayerIO());

 { // minimap
	if (previousPlayerIO) {
		previousPlayerIO->disconnect(0, d->mGLMiniMap, 0);
	}
	if (localPlayerIO()) {
		PlayerIO* io = localPlayerIO();
		io->connect(SIGNAL(signalExplored(int, int)),
				d->mGLMiniMap, SLOT(slotExplored(int, int)));
		io->connect(SIGNAL(signalUnexplored(int, int)),
				d->mGLMiniMap, SLOT(slotUnexplored(int, int)));

		io->connect(SIGNAL(signalExplored(int, int)),
				this, SLOT(slotExplored(int, int)));
		io->connect(SIGNAL(signalUnexplored(int, int)),
				this, SLOT(slotUnexplored(int, int)));
		io->connect(SIGNAL(signalFog(int, int)),
				this, SLOT(slotFog(int, int)));
		io->connect(SIGNAL(signalUnfog(int, int)),
				this, SLOT(slotUnfog(int, int)));
		if (boGame->gameMode()) {
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

#warning might be required for editor
#if 0
	KGameIO* oldIO = previousPlayerIO->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
	if (oldIO) {
		previousPlayerIO->removeGameIO(oldIO);
	}
#endif
 }

 d->mUfoGameGUI->setLocalPlayerIO(localPlayerIO());
 d->mUfoCanvasWidget->setLocalPlayerIO(localPlayerIO());
 d->mUfoPlacementPreviewWidget->setLocalPlayerIO(localPlayerIO());
 resetGameViewPlugin(); // setLocalPlayerIO()

 if (d->mInput) {
	d->mInput->setLocalPlayerIO(localPlayerIO());
 }

 if (!localPlayerIO()) {
	return;
 }

 if (localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI)) {
	slotPlugLocalPlayerInput();
 }

 // at this point the game mode is already fixed, so calling this here should be
 // ok
 setGameMode(boGame->gameMode());

 // AB: we should probably add such a signal to the IO and use the one in
 // the IO then!
 connect((KPlayer*)localPlayerIO()->player(), SIGNAL(signalUnitChanged(Unit*)),
		this, SLOT(slotUnitChanged(Unit*)));


 if (canvas()) {
	slotInitMiniMapFogOfWar();
 }
}

PlayerIO* BosonGameView::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonGameView::createActionCollection(BoUfoActionCollection* parent)
{
 BO_CHECK_NULL_RET(parent);
 delete d->mActionCollection;
 d->mActionCollection = new BoUfoActionCollection(parent, this, "gameview_actioncollection");
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

void BosonGameView::slotExplored(int x, int y)
{
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (r) {
	 r->cellExploredChanged(x, y, x, y);
 }
 boWaterRenderer->cellExploredChanged(x, y, x, y);
}

void BosonGameView::slotUnexplored(int x, int y)
{
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (r) {
	r->cellExploredChanged(x, y, x, y);
 }
 boWaterRenderer->cellExploredChanged(x, y, x, y);
}

void BosonGameView::slotFog(int x, int y)
{
	BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
	if (r) {
		r->cellFogChanged(x, y, x, y);
	}
}

void BosonGameView::slotUnfog(int x, int y)
{
	BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
	if (r) {
		r->cellFogChanged(x, y, x, y);
	}
}

void BosonGameView::slotChangeTexMap(int x, int y)
{
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (r) {
	r->cellTextureChanged(x, y, x, y);
 }
 if (d->mGLMiniMap) {
	d->mGLMiniMap->slotUpdateTerrainAtCorner(x, y);
 }
}

void BosonGameView::slotChangeHeight(int x, int y)
{
 BoGroundRenderer* r = BoGroundRendererManager::manager()->currentRenderer();
 if (r) {
	r->cellHeightChanged(x, y, x, y);
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

void BosonGameView::slotReloadGameViewPlugin()
{
 if (d->mGameViewPluginWidget) {
	d->mGameViewPluginWidgetContainer->removeWidget(d->mGameViewPluginWidget);
	d->mGameViewPluginWidget = 0;
 }
 bool unusable = false;
 bool r = BosonGameViewPluginManager::manager()->reloadPlugin(&unusable);
 if (!r || unusable) {
	KMessageBox::sorry(0, i18n("Reloading gameview plugin failed. quitting."));
	exit(1);
	return;
 }
 boDebug() << k_funcinfo << "gameviewplugin reloading succeeded" << endl;

 if (BosonGameViewPluginManager::manager()->currentPlugin()) {
	BosonGameViewPluginBase* p = (BosonGameViewPluginBase*)BosonGameViewPluginManager::manager()->currentPlugin();
	if (p) {
		p->init();
	} else {
		BO_NULL_ERROR(p);
		return;
	}
 }
 resetGameViewPlugin();
}

void BosonGameView::resetGameViewPlugin()
{
 resetGameViewPlugin(boGame ? boGame->gameMode() : true);
}

void BosonGameView::resetGameViewPlugin(bool gameMode)
{
 d->mGameViewPlugin = (BosonGameViewPluginBase*)BosonGameViewPluginManager::manager()->currentPlugin();
 BO_CHECK_NULL_RET(d->mGameViewPlugin);

 d->mGameViewPlugin->init();

 if (!d->mGameViewPluginWidget && d->mGameViewPluginWidgetContainer) {
	d->mGameViewPluginWidget = d->mGameViewPlugin->ufoWidget();
	d->mGameViewPluginWidgetContainer->addWidget(d->mGameViewPluginWidget);
	connect(this, SIGNAL(signalSelectionChanged(BoSelection*)),
			d->mGameViewPlugin, SLOT(slotSelectionChanged(BoSelection*)));
 }

 d->mGameViewPlugin->setGameGLMatrices(d->mGameGLMatrices);
 d->mGameViewPlugin->setCanvas(mCanvas);
 d->mGameViewPlugin->setLocalPlayerIO(localPlayerIO());
 d->mGameViewPlugin->setGameMode(gameMode);
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
 BO_CHECK_NULL_RET(displayInput());
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

void BosonGameView::slotEditorUndo()
{
 displayInput()->undo();
}

void BosonGameView::slotEditorRedo()
{
 displayInput()->redo();
}

void BosonGameView::slotWidgetShown()
{
 if (displayInput()) {
	displayInput()->updateCursor();
 }
}

void BosonGameView::slotWidgetHidden()
{
 if (displayInput()) {
	displayInput()->makeCursorInvalid();
 }
}

void BosonGameView::slotChangeCursor(int mode, const QString& dir)
{
 if (boGame && boGame->gameMode()) {
	d->mUfoCursorWidget->slotChangeCursor(mode, dir);
 } else {
	d->mUfoCursorWidget->slotChangeCursor(CursorKDE, boConfig->stringValue("CursorDir"));
 }
}

void BosonGameView::resetGameMode()
{
 d->mGameMode = true;
 slotChangeCursor(boConfig->intValue("CursorMode"), boConfig->stringValue("CursorDir"));

 delete d->mInput;
 d->mInput = 0;

 d->mUfoGameGUI->setGameMode(true);
 resetGameViewPlugin(true);

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
 d->mGameMode = mode;
 if (d->mInput) {
	boError() << k_funcinfo << "already an input present! should have been deleted.." << endl;
	delete d->mInput;
	d->mInput = 0;
 }
 if (d->mGameMode) {
	setDisplayInput(new BosonGameViewInput());
 } else {
	setDisplayInput(new EditorViewInput());
 }
 setInputInitialized(true);
 d->mUfoGameGUI->setGameMode(mode);
 resetGameViewPlugin(mode);

 slotAddMenuInput();
}

void BosonGameView::slotAddMenuInput()
{
 if (localPlayerIO()) {
	KGameIO* oldIO = localPlayerIO()->findRttiIO(BosonMenuInput::RTTI);
	if (oldIO) {
		boError() << k_funcinfo << "still an old menuinput IO around!" << endl;
		localPlayerIO()->removeGameIO(oldIO, true);
	}

	BosonMenuInput* io = new BosonMenuInput(d->mGameMode);

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
	connect(io, SIGNAL(signalScroll(int)),
			this, SLOT(slotScroll(int)));
	connect(io, SIGNAL(signalCreateSelectionGroup(int)),
			d->mSelectionGroups, SLOT(slotCreateSelectionGroup(int)));
	connect(io, SIGNAL(signalShowSelectionGroup(int)),
			this, SLOT(slotCenterOnSelectionGroup(int)));
	connect(io, SIGNAL(signalEndGame()),
			this, SIGNAL(signalEndGame()));
	connect(io, SIGNAL(signalQuit()),
			this, SIGNAL(signalQuit()));
	connect(io, SIGNAL(signalSaveGame()),
			this, SIGNAL(signalSaveGame()));
	connect(io, SIGNAL(signalLoadGame()),
			this, SIGNAL(signalLoadGame()));
	connect(io, SIGNAL(signalQuicksaveGame()),
			this, SIGNAL(signalQuicksaveGame()));
	connect(io, SIGNAL(signalQuickloadGame()),
			this, SIGNAL(signalQuickloadGame()));
	connect(io, SIGNAL(signalReloadGameViewPlugin()),
			this, SLOT(slotReloadGameViewPlugin()));
	connect(io, SIGNAL(signalDebugAddedLocalPlayerInput()),
			this, SLOT(slotPlugLocalPlayerInput()));
	connect(io, SIGNAL(signalDebugAddMenuInput()),
			this, SLOT(slotAddMenuInput()));
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
	connect(io, SIGNAL(signalEditorUndo()),
			this, SLOT(slotEditorUndo()));
	connect(io, SIGNAL(signalEditorRedo()),
			this, SLOT(slotEditorRedo()));
	connect(this, SIGNAL(signalEditorHasUndo(const QString&)),
			io, SLOT(slotEditorHasUndo(const QString&)));
	connect(this, SIGNAL(signalEditorHasRedo(const QString&)),
			io, SLOT(slotEditorHasRedo(const QString&)));

	io->setCamera(camera());
	io->setPlayerIO(localPlayerIO());
	io->setActionCollection(actionCollection());
	localPlayerIO()->addGameIO(io);
 }
}

void BosonGameView::slotPlugLocalPlayerInput()
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BosonLocalPlayerInput* input = (BosonLocalPlayerInput*)localPlayerIO()->findRttiIO(BosonLocalPlayerInput::LocalPlayerInputRTTI);
 BO_CHECK_NULL_RET(input);
 disconnect(input, 0, this, 0);

 connect(input, SIGNAL(signalAction(const BoSpecificAction&)),
		this, SLOT(slotAction(const BoSpecificAction&)));
 connect(input, SIGNAL(signalShowMiniMap(bool)),
		this, SLOT(slotShowMiniMap(bool)));

 setLocalPlayerScript(0);
 if (input->eventListener()) {
	setLocalPlayerScript(input->eventListener()->script());
 }
}

void BosonGameView::setLocalPlayerScript(BosonScript* script)
{
 // AB: there is no need to save the pointer atm.
 // we just need to do these connects.
 if (script) {
	d->mScriptConnector->connectToScript(script);
 }
}

BoLight* BosonGameView::light(int id) const
{
 return BoLightManager::manager()->light(id);
}

BoLight* BosonGameView::newLight()
{
 return BoLightManager::manager()->createLight();
}

void BosonGameView::removeLight(int id)
{
 BoLightManager::manager()->deleteLight(id);
}

void BosonGameView::slotAdvance(unsigned int advanceCallsCount, bool advanceFlag)
{
 // AB: note that in the big display no game logic must be done!
 // -> this slotAdvance() is here for certain optimizations on rendering, not
 //    for advancing the game itself
 advanceCamera();

 d->mUfoCanvasWidget->slotAdvance(advanceCallsCount, advanceFlag);
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

 // AB: note that a failure in this methods is an error, but it does not abort
 // loading.

 QDomElement unitGroups = root.namedItem(QString::fromLatin1("UnitGroups")).toElement();
 if (unitGroups.isNull()) {
	boError(260) << k_funcinfo << "no UnitGroups tag" << endl;
	// do not return
 } else if (!d->mSelectionGroups->loadFromXML(unitGroups)) {
	boError(260) << k_funcinfo << "could not load selection groups" << endl;
	// do not return
 }

 QDomElement effects = root.namedItem("Effects").toElement();
 if (effects.isNull()) {
	boError(260) << k_funcinfo << "no Effects tag" << endl;
	// do not return
 } else if (!d->mUfoCanvasWidget->loadEffectsFromXML(effects)) {
	boError(260) << k_funcinfo << "could not load effects" << endl;
	// do not return
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
 }
 // Load selection
 QDomElement sel = display.namedItem(QString::fromLatin1("Selection")).toElement();
 selection()->clear();
 if (!sel.isNull()) {
	selection()->loadFromXML(sel, true);
 }
}

void BosonGameView::saveAsXML(QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 QDomDocument doc = root.ownerDocument();

 // Save selection groups
 QDomElement unitGroups = doc.createElement(QString::fromLatin1("UnitGroups"));
 d->mSelectionGroups->saveAsXML(unitGroups);
 root.appendChild(unitGroups);

 QDomElement effects = doc.createElement(QString::fromLatin1("Effects"));
 d->mUfoCanvasWidget->saveEffectsAsXML(effects);
 root.appendChild(effects);


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

 d->mInput->slotSetCursor(d->mUfoCursorWidget->cursor());

 d->mInput->setCanvas(canvas());
 d->mInput->setLocalPlayerIO(localPlayerIO());
 d->mInput->setCursorCanvasVector(d->mCursorPos.canvasVectorPointer());

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

 connect(d->mUfoCursorWidget, SIGNAL(signalSetCursor(BosonCursor*)),
		input, SLOT(slotSetCursor(BosonCursor*)));
 if (d->mGLMiniMap) {
	connect(d->mGLMiniMap, SIGNAL(signalMoveSelection(int, int)),
			input, SLOT(slotMoveSelection(int, int)));
 }

 connect(input, SIGNAL(signalEditorHasUndo(const QString&)),
		this, SIGNAL(signalEditorHasUndo(const QString&)));
 connect(input, SIGNAL(signalEditorHasRedo(const QString&)),
		this, SIGNAL(signalEditorHasRedo(const QString&)));

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

void BosonGameView::slotShowMiniMap(bool show)
{
 d->mGLMiniMap->slotShowMiniMap(show);
}


void BosonGameView::slotMouseEvent(QMouseEvent* e)
{
 BO_CHECK_NULL_RET(displayInput());
 GLfloat posX = 0.0;
 GLfloat posY = 0.0;
 GLfloat posZ = 0.0;
 if (!mapCoordinatesToGround(e->pos(), &posX, &posY, &posZ)) {
//	boError() << k_funcinfo << "Cannot map coordinates" << endl;
	return;
 }
 BoVector3Fixed canvasVector(posX, -posY, posZ);

 BoMouseEvent event;
 event.setGroundCanvasVector(canvasVector);
 event.setGroundWorldPos(posX, posY, posZ);
 if (e->type() != QEvent::Wheel) {
	event.setGameViewWidgetPos(e->pos());
	event.setControlButton(e->state() & ControlButton);
	event.setShiftButton(e->state() & ShiftButton);
	event.setAltButton(e->state() & AltButton);
 } else {
	QWheelEvent* w = (QWheelEvent*)e;
	event.setGameViewWidgetPos(w->pos());
	event.setControlButton(w->state() & ControlButton);
	event.setShiftButton(w->state() & ShiftButton);
	event.setAltButton(w->state() & AltButton);
 }
 event.setUnitAtEventPos(d->mUfoCanvasWidget->unitAtWidgetPos(event.gameViewWidgetPos()));

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
		d->mDoubleClickRecognizer.mouseMoveEvent(e);
		mouseEventMove(e->state(), event);
		e->accept();
		break;
	}
	case QEvent::MouseButtonDblClick:
	{
		// AB: won't happen, since BoUfo won't emit this kind of
		// events!
		e->accept();
		break;
	}
	case QEvent::MouseButtonPress:
	{
		// no action should happen here!

		d->mDoubleClickRecognizer.mousePressEvent(e);
		switch (e->button()) {
			case LeftButton:
				d->mLeftButtonState->pressButton(event.gameViewWidgetPos());
				break;
			case MidButton:
				d->mMiddleButtonState->pressButton(event.gameViewWidgetPos());
				break;
			case RightButton:
				d->mRightButtonState->pressButton(event.gameViewWidgetPos());
				break;
			default:
				break;
		}
		e->accept();
		break;
	}
	case QEvent::MouseButtonRelease:
	{
		bool isDoubleClick = d->mDoubleClickRecognizer.mouseReleaseEvent(e);
		if (isDoubleClick) {
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

		//bricofoy's scrolling stuff

		if (boConfig->boolValue("WheelMoveZoom")) {
			int threshold = 3; // hardcoded, TODO: use boConfig instead
			static int lastX = 0, lastY = 0;

			if (delta < 0) { //we scroll only when zooming in, not when zooming out
				int curX, curY;
				float posX, posY, posZ;
				boEvent.groundWorldPos(&posX, &posY, &posZ);
				int cellX, cellY;
				cellX = (int)(posX);
				cellY = (int)(-posY);

				QWidget* w = qApp->focusWidget();
				BO_CHECK_NULL_RET(w);
				QPoint pos = w->mapToGlobal(QPoint(w->width()/2, w->height()/2));
				QPoint pos2 = QCursor::pos();

				curX = pos2.x();
				curY = pos2.y();

				int dx = lastX - curX;
				int dy = lastY - curY;
				int dx2 = dx * dx;
				int dy2 = dy * dy;
				int t2 = threshold * threshold;
				if (dx2 > t2 || dy2 > t2) {
					slotReCenterDisplay(QPoint(cellX, cellY));
					displayInput()->updateCursor();
					QCursor::setPos(pos);
					curX = pos.x();
					curY = pos.y();
				}
				lastX = curX;
				lastY = curY;
			}
		}
		//end bricofoy's scrolling stuff
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
 if (buttonState & LeftButton) {
	d->mLeftButtonState->mouseMoved(event);
 }
 if (buttonState & MidButton) {
	d->mMiddleButtonState->mouseMoved(event);
 }
 if (buttonState & RightButton) {
	d->mRightButtonState->mouseMoved(event);
 }

 updateCursorCanvasVector(event.gameViewWidgetPos());
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
	case LeftButton:
	{
		d->mLeftButtonState->releaseButton(event);
		break;
	}
	case MidButton:
	{
		d->mMiddleButtonState->releaseButton(event);
		break;
	}
	case RightButton:
	{
		d->mRightButtonState->releaseButton(event);
		break;
	}
	default:
		boError() << k_funcinfo << "invalid mouse button " << button << endl;
		break;
 }
 displayInput()->updateCursor();
}

void BosonGameView::mouseEventReleaseDouble(ButtonState button, const BoMouseEvent& event)
{
 switch (button) {
	case LeftButton:
	{
		d->mLeftButtonState->releaseButton(event, true);
		break;
	}
	case MidButton:
	{
		d->mMiddleButtonState->releaseButton(event, true);
		break;
	}
	case RightButton:
	{
		d->mRightButtonState->releaseButton(event, true);
		break;
	}
	default:
		boError() << k_funcinfo << "invalid mouse button " << button << endl;
		break;
 }
 displayInput()->updateCursor();
}

void BosonGameView::paint()
{
 if (!isVisible()) {
	return;
 }
 BO_CHECK_NULL_RET(camera());
 BO_CHECK_NULL_RET(boTextureManager);
 BO_CHECK_NULL_RET(boGame);
 PROFILE_METHOD

 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "OpenGL error at start of " << k_funcinfo << endl;
 }

 if (boGame->delayedAdvanceMessageCount() >= 2) {
	// if there are delayed advance messages, we only ensure that we render
	// a frame once in a while, but most CPU power goes to advance
	// procedures.

	long long time = gameFPSCounter()->counter()->timeSinceLastFrame(true);
	if (time > 0 && time < 500000) {
		// now we need to let the counter know, that this frame is being skipped
		gameFPSCounter()->skipFrame();
		return;
	}
 }

 glPushAttrib(GL_ALL_ATTRIB_BITS);

 // AB: note there seems to be hardly a difference between flat and smooth
 // shading (in both quality and speed)
 if (boConfig->boolValue("SmoothShading", true)) {
	glShadeModel(GL_SMOOTH);
 } else {
	glShadeModel(GL_FLAT);
 }

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
 // AB: lights may depend on the modelview matrix, therefore we must
 // update the lights at least once with a valid 3d modelview matrix.
 BoLightManager::manager()->updateAllStates();


 glPopMatrix();

 d->mUfoGameGUI->updateUfoLabels();
 if (d->mGameViewPlugin) {
	d->mGameViewPlugin->updateBeforePaint();
 }

 // the original implementation paints the children
 boProfiling->push("BoUfoCustomWidget::paint()");
 BoUfoCustomWidget::paint();
 boProfiling->pop(); // "BoUfoCustomWidget::paint()"

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

void BosonGameView::slotGameOver()
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(boGame);
 boDebug() << k_funcinfo << endl;

 QString winner;
 QString loser;
 QString rest;
 bool localWon = false;
 bool localLost = false;
 for (unsigned int i = 0; i < boGame->gamePlayerCount(); i++) {
	PlayerIO* io = boGame->playerIOAtGameIndex(i);
	if (io->hasLost()) {
		if (io == localPlayerIO()) {
			localLost = true;
		}
		if (loser.isEmpty()) {
			loser = i18n("%1").arg(io->name());
		} else {
			loser = i18n("%1, %1").arg(loser).arg(io->name());
		}
	} else if (io->hasWon()) {
		if (io == localPlayerIO()) {
			localWon = true;
		}
		if (winner.isEmpty()) {
			winner = i18n("%1").arg(io->name());
		} else {
			winner = i18n("%1, %1").arg(winner).arg(io->name());
		}
	} else {
		if (rest.isEmpty()) {
			rest = i18n("%1").arg(io->name());
		} else {
			rest = i18n("%1, %1").arg(rest).arg(io->name());
		}
	}
 }
 if (winner.isEmpty()) {
	winner = i18n("No winner in this game");
 }
 if (loser.isEmpty()) {
	loser = i18n("No loser in this game");
 }

 QString local;
 if (localWon) {
	local = i18n("You won! Damn you made it! You really rock!!!");
 } else if (localLost) {
	local = i18n("You are a loser. You suck.");
 } else {
	local = i18n("You can't decide whether to win or to lose hm? Booring!");
 }
 if (!rest.isEmpty()) {
	rest = i18n("Neither winner nor loser: %1").arg(rest);
 }
 KMessageBox::information(0,
		i18n("The game is over.\n"
		"Winner: %1\nLoser: %2\n"
		"%3\n"
		"\n%4\n"
		"This messagebox means that the developers have been too lazy to implement a nice gameover dialog. Sorry about this, but you didn't help, so it's your fault!")
		.arg(winner).arg(loser).arg(rest).arg(local),
		i18n("Game is over"));

 QTimer::singleShot(0, this, SIGNAL(signalEndGame()));
}

void BosonGameView::addEffect(unsigned int id, const BoVector3Fixed& pos, bofixed zrot)
{
 d->mUfoCanvasWidget->createEffect(id, pos, zrot);
}

void BosonGameView::addEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot)
{
 d->mUfoCanvasWidget->createAttachedEffect(unitid, effectid, offset, zrot);
}

void BosonGameView::advanceEffects(int ticks)
{
 d->mUfoCanvasWidget->advanceEffects(ticks * 0.05);
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
 reconnect(i, SIGNAL(signalGetCameraXRotation(float*)),
		this, SLOT(slotGetCameraXRotation(float*)));
 reconnect(i, SIGNAL(signalGetCameraDistance(float*)),
		this, SLOT(slotGetCameraDistance(float*)));
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
 reconnect(i, SIGNAL(signalSetCameraXRotation(float)),
		this, SLOT(slotSetCameraXRotation(float)));
 reconnect(i, SIGNAL(signalSetCameraDistance(float)),
		this, SLOT(slotSetCameraDistance(float)));
 reconnect(i, SIGNAL(signalSetCameraMoveMode(int)),
		this, SLOT(slotSetCameraMoveMode(int)));
 reconnect(i, SIGNAL(signalSetCameraInterpolationMode(int)),
		this, SLOT(slotSetCameraInterpolationMode(int)));
 reconnect(i, SIGNAL(signalCommitCameraChanges(int)),
		this, SLOT(slotCommitCameraChanges(int)));
 reconnect(i, SIGNAL(signalSetAcceptUserInput(bool)),
		this, SLOT(slotSetAcceptUserInput(bool)));

 reconnect(i, SIGNAL(signalAddEffect(unsigned int, const BoVector3Fixed&, bofixed)),
		this, SLOT(slotAddEffect(unsigned int, const BoVector3Fixed&, bofixed)));
 reconnect(i, SIGNAL(signalAddEffectToUnit(int, unsigned int, BoVector3Fixed, bofixed)),
		this, SLOT(slotAddEffectToUnit(int, unsigned int, BoVector3Fixed, bofixed)));
 reconnect(i, SIGNAL(signalAdvanceEffects(int)),
		this, SLOT(slotAdvanceEffects(int)));
 reconnect(i, SIGNAL(signalSetWind(const BoVector3Float&)),
		this, SLOT(slotSetWind(const BoVector3Float&)));
 reconnect(i, SIGNAL(signalGetWind(BoVector3Float*)),
		this, SLOT(slotGetWind(BoVector3Float*)));
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

void BosonGameViewScriptConnector::slotGetCameraXRotation(float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->xRotation();
}

void BosonGameViewScriptConnector::slotGetCameraDistance(float* v)
{
 BO_CHECK_NULL_RET(mDisplay->camera());
 *v = mDisplay->camera()->distance();
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

void BosonGameViewScriptConnector::slotSetCameraXRotation(float v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setXRotation(v);
}

void BosonGameViewScriptConnector::slotSetCameraDistance(float v)
{
 BO_CHECK_NULL_RET(mDisplay->autoCamera());
 mDisplay->autoCamera()->setDistance(v);
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

void BosonGameViewScriptConnector::slotAddEffect(unsigned int id, const BoVector3Fixed& pos, bofixed zrot)
{
 mDisplay->addEffect(id, pos, zrot);
}

void BosonGameViewScriptConnector::slotAddEffectToUnit(int unitid, unsigned int effectid, BoVector3Fixed offset, bofixed zrot)
{
 mDisplay->addEffectToUnit(unitid, effectid, offset, zrot);
}

void BosonGameViewScriptConnector::slotAdvanceEffects(int ticks)
{
 mDisplay->advanceEffects(ticks);
}

void BosonGameViewScriptConnector::slotSetWind(const BoVector3Float& wind)
{
 BosonEffectPropertiesParticle::setWind(wind);
}

void BosonGameViewScriptConnector::slotGetWind(BoVector3Float* wind)
{
 *wind = BosonEffectPropertiesParticle::wind();
}

