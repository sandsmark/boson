/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>
#include <ufo/ux/ux.hpp>
#include <ufo/events/umousewheelevent.hpp>
#include <ufo/events/ukeysym.hpp>
#include <ufo/ui/uuidefs.hpp>
#include <ufo/gl/ugl_graphics.hpp>
#include "ufoext/ubosonstyle.h"
#include "ufoext/boufofontrenderer.h"

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufomanager.h"
#include "boufomanager.moc"

#include "boufowidget.h"
#include "boufointernalframe.h"
#include "boufolayeredpane.h"
#include "boufofontinfo.h"
#include "boufoaction.h"
#include "boufoprofiling.h"

#include <kglobal.h>
#include <kstandarddirs.h>

#include <qobject.h>
#include <qdom.h>
#include <qdir.h>

#include <bodebug.h>

#include <math.h>

static BoUfoManager* g_currentManager = 0;
static QMap<ufo::UContext*, BoUfoManager*> g_context2Manager;


/**
 * Macro to simplify ufo to Qt signal connection.
 *
 * @param className the name of the class you are working in, i.e. where the
 * (Qt) slot is defined in.
 * @param widget Where the signal comes from (i.e. the libufo widget)
 * @param signal The name of the signal. That's the part after the "sig" prefix
 * in libufo. There must be a corresponding "slot" starting with "uslot"
 **/
#define CONNECT_UFO_TO_QT(className, widget, signal) \
		widget->sig##signal().connect(slot(*this, &className::uslot##signal));



static ufo::UMod_t convertQtMouseButtonToUfo(int button)
{
 ufo::UMod_t ubutton = ufo::UMod::NoModifier;
 switch (button) {
	case Qt::LeftButton:
		ubutton = ufo::UMod::LeftButton;
		break;
	case Qt::RightButton:
		ubutton = ufo::UMod::RightButton;
		break;
	case Qt::MidButton:
		ubutton = ufo::UMod::MiddleButton;
		break;
	default:
		ubutton = ufo::UMod::NoModifier;
		break;
 }
 return ubutton;
}

static ufo::UMod_t convertQtModifierStateToUfo(int modifiers, ufo::UMod_t origUfoModifiers = ufo::UMod::NoModifier)
{
 int s = ufo::UMod::NoModifier;
 if (modifiers & Qt::LeftButton) {
	s |= ufo::UMod::LeftButton;
 }
 if (modifiers & Qt::RightButton) {
	s |= ufo::UMod::RightButton;
 }
 if (modifiers & Qt::MidButton) {
	s |= ufo::UMod::MiddleButton;
 }

 if (modifiers & Qt::ShiftButton) {
	s |= ufo::UMod::Shift;
 }
 if (modifiers & Qt::ControlButton) {
	s |= ufo::UMod::Ctrl;
 }
 if (modifiers & Qt::AltButton) {
	s |= ufo::UMod::Alt;
 }
 if (modifiers & Qt::MetaButton) {
	s |= ufo::UMod::Meta;
 }

 // AB: libufo keeps track of several states that Qt does not:
 // * numlock
 // * capslock
 // * left/right shift (2 different buttons!)
 // * left/right ctrl (2 different buttons!)
 // * left/right alt (2 different buttons!)
 // * left/right meta (2 different buttons!)
 // * lsuper (left "windows" key)
 // * rsuper (right "windows" key)
 // * alt_graph
 //
 // we try to retrieve them from origUfoModifiers.
 // Note that we ignore these items that Qt provides 1 key, but libufo provides 2 for.
 modifiers |= (origUfoModifiers & ufo::UMod::Num);
 modifiers |= (origUfoModifiers & ufo::UMod::Caps);
 modifiers |= (origUfoModifiers & ufo::UMod::AltGraph);
 return (ufo::UMod_t)s;
}

static ufo::UKeyCode_t convertQtKeyToUfo(int key)
{
 ufo::UKeyCode_t uk = ufo::UKey::UK_UNKOWN;
 if ((int)Qt::Key_0 != (int)ufo::UKey::UK_0) {
	boError() << k_funcinfo << "key constants changed!" << endl;
 }
 if (key >= Qt::Key_Space && key <= Qt::Key_AsciiTilde) {
	uk = ufo::UKeyCode_t(key);
	return uk;
 } else if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
	if (key > Qt::Key_F24) {
		uk = ufo::UKey::UK_UNKOWN;
	} else {
		uk = ufo::UKeyCode_t(key - Qt::Key_F1);
	}
	return uk;
 }

#warning TODO: UK_KP_*
 // TODO: UK_KP_* (i.e. keys of numpad when numlock is off)
 // TODO: UK_KP0..9
 // TODO: UK_MODE aka UK_ALT_GRAPH ("alt gr" key)
 // TODO: UK_COMPOSE
 // TODO: UK_BREAK
 // TODO: UK_POWER
 // TODO: UK_EURO
 // TODO: UK_WORLD_* ?
 // TODO: Key_Backtab

 uk = ufo::UKey::UK_UNKOWN; // AB: typo in libufo - UK_UNK_N_OWN
 switch (key) {
	case Qt::Key_Escape:
		uk = ufo::UKey::UK_ESCAPE;
		break;
	case Qt::Key_Tab:
		uk = ufo::UKey::UK_TAB;
		break;
	case Qt::Key_Backtab: // TODO
		uk = ufo::UKey::UK_UNKOWN;
		break;
	case Qt::Key_Backspace:
		uk = ufo::UKey::UK_BACKSPACE;
		break;
	case Qt::Key_Return:
		uk = ufo::UKey::UK_RETURN;
		break;
	case Qt::Key_Enter: // FIXME: is this correct? no UK_ENTER
		uk = ufo::UKey::UK_KP_ENTER;
		break;
	case Qt::Key_Insert:
		uk = ufo::UKey::UK_INSERT;
		break;
	case Qt::Key_Delete:
		uk = ufo::UKey::UK_DELETE;
		break;
	case Qt::Key_Pause:
		uk = ufo::UKey::UK_PAUSE;
		break;
	case Qt::Key_Print:
		uk = ufo::UKey::UK_PRINT;
		break;
	case Qt::Key_SysReq:
		uk = ufo::UKey::UK_SYSREQ;
		break;
	case Qt::Key_Clear:
		uk = ufo::UKey::UK_CLEAR;
		break;
	case Qt::Key_Home:
		uk = ufo::UKey::UK_HOME;
		break;
	case Qt::Key_End:
		uk = ufo::UKey::UK_END;
		break;
	case Qt::Key_Left:
		uk = ufo::UKey::UK_LEFT;
		break;
	case Qt::Key_Up:
		uk = ufo::UKey::UK_UP;
		break;
	case Qt::Key_Right:
		uk = ufo::UKey::UK_RIGHT;
		break;
	case Qt::Key_Down:
		uk = ufo::UKey::UK_DOWN;
		break;
	case Qt::Key_Prior: // PageUp
		uk = ufo::UKey::UK_PAGEUP;
		break;
	case Qt::Key_Next: // PageDown
		uk = ufo::UKey::UK_PAGEDOWN;
		break;
	case Qt::Key_Shift:
#warning FIXME
		// FIXME: does qt tell us whether "shift" is left or right
		// shift??
		uk = ufo::UKey::UK_RSHIFT;
		uk = ufo::UKey::UK_LSHIFT;
		break;
	case Qt::Key_Control:
#warning FIXME
		// FIXME: see Key_Shift
		uk = ufo::UKey::UK_RCTRL;
		uk = ufo::UKey::UK_LCTRL;
		break;
	case Qt::Key_Meta:
#warning FIXME
		// FIXME: see Key_Shift
		uk = ufo::UKey::UK_RMETA;
		uk = ufo::UKey::UK_LMETA;
		break;
	case Qt::Key_Alt:
#warning FIXME
		// FIXME: see Key_Shift
		uk = ufo::UKey::UK_RALT;
		uk = ufo::UKey::UK_LALT;
		break;
	case Qt::Key_CapsLock:
		uk = ufo::UKey::UK_CAPSLOCK;
		break;
	case Qt::Key_NumLock:
		uk = ufo::UKey::UK_NUMLOCK;
		break;
	case Qt::Key_ScrollLock:
		uk = ufo::UKey::UK_SCROLLOCK;
		break;
	case Qt::Key_Super_L:
		uk = ufo::UKey::UK_LSUPER;
		break;
	case Qt::Key_Super_R:
		uk = ufo::UKey::UK_RSUPER;
		break;
	case Qt::Key_Menu:
		uk = ufo::UKey::UK_MENU;
		break;
	case Qt::Key_Hyper_L:
		break;
	case Qt::Key_Hyper_R:
		break;
	case Qt::Key_Help:
		uk = ufo::UKey::UK_HELP;
		break;
	case Qt::Key_Direction_L:
		break;
	case Qt::Key_Direction_R:
		break;
	case Qt::Key_unknown:
	default:
		uk = ufo::UKey::UK_UNKOWN;
		break;
 }
 return uk;
}


BoUfoManager::BoUfoManager(int w, int h, bool opaque)
	: QObject(0, "ufomanager")
{
 mGlobalFont = new BoUfoFontInfo();
 if (!ufo::UToolkit::getToolkit()) {
	ufo::UXToolkit* tk = new ufo::UXToolkit();
	tk->getStyleManager()->setStyle(new ufo::UBosonStyle);
	QString data_dir;
	if (KGlobal::_instance) { // NULL in boufodesigner
		data_dir = KGlobal::dirs()->findResourceDir("data", "boson/pics/boson-startup-logo.png");
	}
	if (data_dir.isEmpty()) {
		boWarning() << k_funcinfo << "cannot determine data_dir" << endl;
//		tk->putProperty("data_dir", "/usr/local/share/ufo");
//		tk->putProperty("font_dir", "/usr/local/share/ufo/font");
	} else {
		data_dir += "boson";

		setDataDir(data_dir);
	}

#if 1
	ufo::UPluginInfo bosonGLFontPlugin;
	bosonGLFontPlugin.lib = NULL;
	bosonGLFontPlugin.category = "font";
	bosonGLFontPlugin.feature = "boson_font";
	bosonGLFontPlugin.create = &BoUfoFontRenderer::createPlugin;
	bosonGLFontPlugin.destroy = &BoUfoFontRenderer::destroyPlugin;
	// load the plugin
	tk->loadPlugin(bosonGLFontPlugin);

	// ensure that we use this font
	// AB: currently disabled. use export UFO_FONT="boson_font" to enable
//	tk->putProperty("font", "boson_font");
#endif


 } else {
	// TODO: make sure that it is a UXToolkit
	boDebug() << k_funcinfo << "have already a toolkit" << endl;
 }

 if (!BoUfoProfiling::profiling()) {
	BoUfoProfiling::setProfiling(new BoUfoProfiling());
 }
 BoUfoProfiling::reference();

 mDisplay = new ufo::UXDisplay("dummy");

 ufo::URectangle deviceRect(0, 0, w, h);
 ufo::URectangle contextRect(0, 0, w, h);
 mContext = new ufo::UXContext(deviceRect, contextRect);
 g_context2Manager.insert(mContext, this);

 mRootPane = mContext->getRootPane();
 mRootPaneWidget = 0,
 mLayeredPaneWidget = 0;
 mContentPane = 0;
 mContentWidget = 0;
 mMenuBarData = 0;
 mToolBarData = 0;
 mActionCollection = 0;

 makeContextCurrent();
 if (!mRootPane) {
	BO_NULL_ERROR(mRootPane);
 } else {
	mRootPaneWidget = new BoUfoWidget(mRootPane);
	if (mRootPane->getLayeredPane()) {
		mLayeredPaneWidget = new BoUfoLayeredPane(mRootPane->getLayeredPane(), false);
	} else {
		BO_NULL_ERROR(mRootPane->getLayeredPane());
	}
	mContentPane = mRootPane->getContentPane();
	if (mContentPane) {
		mContentWidget = new BoUfoWidget(mContentPane);
		mContentWidget->setLayoutClass(BoUfoWidget::UVBoxLayout);
	} else {
		BO_NULL_ERROR(mContentPane);
	}
 }

 if (rootPane()) {
	// required for shortcuts and accelerators.
	rootPaneWidget()->setKeyEventsEnabled(true);
 }
 if (rootPane() && contentWidget()) {
	rootPane()->setOpaque(opaque);
	contentWidget()->setOpaque(opaque);
 }

 mToolBarDockWidget = 0;
 mToolBarContentWidget = 0;
 if (mRootPane) {
	mToolBarDockWidget = new ufo::UDockWidget();
	mToolBarContentWidget = new BoUfoWidget();
	mToolBarContentWidget->setLayoutClass(BoUfoWidget::UHBoxLayout);
	mToolBarDockWidget->add(mToolBarContentWidget->ufoWidget());
	mToolBarContentWidget->setName("boufotoolbarcontent");
	mRootPane->addDockWidget(mToolBarDockWidget, ufo::TopDockWidgetArea);
 }
}

BoUfoManager::~BoUfoManager()
{
 boDebug() << k_funcinfo << endl;
 if (g_currentManager == this) {
	g_currentManager = 0;
 }
 {
	QValueList<ufo::UContext*> remove;
	QMap<ufo::UContext*, BoUfoManager*>::iterator it;
	for (it = g_context2Manager.begin(); it != g_context2Manager.end(); ++it) {
		if (it.data() == this) {
			remove.append(it.key());
		}
	}
	while (!remove.isEmpty()) {
		g_context2Manager.remove(remove.first());
		remove.pop_front();
	}
 }
 delete mActionCollection;
 setMenuBarData(0);
 if (contentWidget()) {
	boDebug() << k_funcinfo << "removing all widgets" << endl;
	contentWidget()->removeAllWidgets();
	boDebug() << k_funcinfo << "removed all widgets" << endl;
 }
 delete mGlobalFont;
 delete mContext;
 delete mDisplay;

 BoUfoProfiling::unreference();

 // TODO: do we have to delete the toolkit ?
 // -> there is only a single static instance of it, so that might be tricky.
 // does libufo take care of that?

 boDebug() << k_funcinfo << "done" << endl;
}

void BoUfoManager::setDataDir(const QString& data_dir)
{
 if (data_dir.isEmpty()) {
	boWarning() << k_funcinfo << "no data_dir given" << endl;
	return;
 }
 QDir dir(data_dir);
 if (!dir.exists()) {
	boWarning() << k_funcinfo << "given data_dir " << data_dir << " does not exist" << endl;
	return;
 }
 QString font_dir = data_dir + "/font";
 setUfoToolkitProperty("data_dir", data_dir);
 setUfoToolkitProperty("font_dir", font_dir);
}

QString BoUfoManager::dataDir() const
{
 return ufoToolkitProperty("data_dir");
}

BoUfoManager* BoUfoManager::currentUfoManager()
{
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 if (!tk) {
	return 0;
 }
 ufo::UContext* c = tk->getCurrentContext();
 if (!c) {
	return 0;
 }
 if (g_currentManager) {
	if ((ufo::UContext*)g_currentManager->context() == c) {
		return g_currentManager;
	}
	g_currentManager = 0;
 }
 g_currentManager = g_context2Manager[c];
 return g_currentManager;
}

BoUfoManager* BoUfoManager::ufoManagerForContext(ufo::UContext* context)
{
 BO_CHECK_NULL_RET0(context);
 if (!g_context2Manager.contains(context)) {
	return 0;
 }
 return g_context2Manager[context];
}

ufo::UXToolkit* BoUfoManager::toolkit() const
{
 return (ufo::UXToolkit*)ufo::UToolkit::getToolkit();
}

void BoUfoManager::addFrame(BoUfoInternalFrame* frame)
{
 rootPane()->addFrame(frame->frame());
}

void BoUfoManager::removeFrame(BoUfoInternalFrame* frame)
{
 rootPane()->removeFrame(frame->frame());
}

void BoUfoManager::setMenuBarData(BoUfoMenuBar* m)
{
 // warning: BoUfoMenuBar is NOT a BoUfoWidget! therefore it is NOT
 // deleted when the UMenuBar is deleted!
 BoUfoMenuBar* del = mMenuBarData;
 mMenuBarData = 0; // ~BoUfoMenuBar calls setMenuBar() which deletes mMenuBarData
 delete del;
 if (mRootPane && mRootPane->getMenuBar()) {
	mRootPane->setMenuBar(0); // deletes the old menubar
 }
 mMenuBarData = m;
}

void BoUfoManager::setToolBarData(BoUfoToolBar* m)
{
 // warning: BoUfoToolBar is NOT a BoUfoWidget! therefore it is NOT
 // deleted when the UMenuBar is deleted!
 BoUfoToolBar* del = mToolBarData;
 mToolBarData = 0; // ~BoUfoToolBar calls setToolBar() which deletes mToolBarData
 delete del;
 if (mToolBarContentWidget) {
	mToolBarContentWidget->removeAllWidgets();
	mToolBarContentWidget->setVisible(false);
 }
 mToolBarData = m;
}

void BoUfoManager::makeContextCurrent()
{
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 tk->makeContextCurrent(context());
}

void BoUfoManager::dispatchEvents()
{
 if (display()) {
	display()->dispatchEvents();
 }
}

void BoUfoManager::render(bool pushAttributesMatrices)
{
 BO_CHECK_NULL_RET(context());
 context()->getRepaintManager()->clearDirtyRegions();

 if (pushAttributesMatrices) {
	context()->pushAttributes();
 }

 context()->getGraphics()->resetDeviceAttributes();
 context()->getGraphics()->resetDeviceViewMatrix();
 context()->getRootPane()->paint(context()->getGraphics());

 if (pushAttributesMatrices) {
	context()->popAttributes();
 }
}


bool BoUfoManager::sendEvent(QEvent* e)
{
 // AB: under certain circumstances the keyboard mod states of libufo may be
 // obsolete (e.g. if alt+tab was used to switch to another window and back to
 // the libufo window - the window manager eats these events then)
 // so before dispatching an event, we update the keyboard state, if possible
 bool updateKeys = false;

 // AB: sometime even the state of currently pressed buttons may become invalid:
 //     if you press+hold a button and then open a model dialog (e.g. through a
 //     timer) and then release the button, then the event won't ever get
 //     delivered to the original widget.
 bool updateButtons = false;
 Qt::ButtonState state = Qt::NoButton;
 switch (e->type()) {
	case QEvent::Wheel:
		state = ((QWheelEvent*)e)->state();
		updateKeys = true;
		break;
	case QEvent::MouseMove:
		updateButtons = true;
		// fall through intended
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
		state = ((QMouseEvent*)e)->state();
		updateKeys = true;
		break;
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		state = ((QKeyEvent*)e)->state();
		updateKeys = true;
		break;
	default:
		break;
 }
 if (updateKeys) {
	int x, y;
	ufo::UMod_t mouseState = display()->getMouseState(&x, &y);
	ufo::UMod_t keyState = convertQtModifierStateToUfo(state & ~Qt::MouseButtonMask, display()->getModState());

	ufo::UMod_t modState = (ufo::UMod_t)((int)mouseState | (int)keyState);
	display()->setModState(modState);
 }
 if (updateButtons) {
	int x, y;
	ufo::UMod_t mouseState = display()->getMouseState(&x, &y);
	if (mouseState & ufo::UMod::LeftButton && !(state & Qt::LeftButton)) {
		display()->dispatchMouseButtonUp(context(), x, y, ufo::UMod::LeftButton);
	}
	if (mouseState & ufo::UMod::MiddleButton && !(state & Qt::MidButton)) {
		display()->dispatchMouseButtonUp(context(), x, y, ufo::UMod::MiddleButton);
	}
	if (mouseState & ufo::UMod::RightButton && !(state & Qt::RightButton)) {
		display()->dispatchMouseButtonUp(context(), x, y, ufo::UMod::RightButton);
	}
 }

 switch (e->type()) {
	case QEvent::Wheel:
		return sendWheelEvent((QWheelEvent*)e);
	case QEvent::MouseMove:
		return sendMouseMoveEvent((QMouseEvent*)e);
	case QEvent::MouseButtonDblClick:
		// AB: Qt divides press events into simple press events and
		// dblclick events, libufo does not
		// -> so we treat both events equally
	case QEvent::MouseButtonPress:
		return sendMousePressEvent((QMouseEvent*)e);
	case QEvent::MouseButtonRelease:
		return sendMouseReleaseEvent((QMouseEvent*)e);
	case QEvent::KeyPress:
		return sendKeyPressEvent((QKeyEvent*)e);
	case QEvent::KeyRelease:
		return sendKeyReleaseEvent((QKeyEvent*)e);
	case QEvent::Resize:
	{
		QResizeEvent* r = (QResizeEvent*)e;
		return sendResizeEvent(r->size().width(), r->size().height());
	}
	default:
		break;
 }
 boDebug() << k_funcinfo << "unhandled event " << e->type() << endl;
 return false;
}

bool BoUfoManager::sendResizeEvent(int w, int h)
{
 if (!context()) {
	BO_NULL_ERROR(conext());
	return false;
 }
 ufo::URectangle deviceRect(0, 0, w, h);
 ufo::URectangle contextRect(0, 0, w, h);
 context()->setDeviceBounds(deviceRect);
 context()->setContextBounds(contextRect);
 return true;
}

bool BoUfoManager::sendMousePressEvent(QMouseEvent* e)
{
 if (!e) {
	BO_NULL_ERROR(e);
	return false;
 }
 if (!context()) {
	BO_NULL_ERROR(context());
	return false;
 }
 if (!display()) {
	BO_NULL_ERROR(display());
	return false;
 }
 ufo::UMod_t button = convertQtMouseButtonToUfo(e->button());

 bool ret = display()->dispatchMouseButtonDown(context(), e->x(), e->y(), button);
 if (ret) {
	e->accept();
 } else {
	e->ignore();
 }
 return ret;
}

bool BoUfoManager::sendMouseReleaseEvent(QMouseEvent* e)
{
 if (!e) {
	BO_NULL_ERROR(e);
	return false;
 }
 if (!context()) {
	BO_NULL_ERROR(context());
	return false;
 }
 if (!display()) {
	BO_NULL_ERROR(display());
	return false;
 }
 ufo::UMod_t button = convertQtMouseButtonToUfo(e->button());

 bool ret = display()->dispatchMouseButtonUp(context(), e->x(), e->y(), button);
 if (ret) {
	e->accept();
 } else {
	e->ignore();
 }
 return ret;
}

bool BoUfoManager::sendMouseMoveEvent(QMouseEvent* e)
{
 if (!e) {
	BO_NULL_ERROR(e);
	return false;
 }
 if (!context()) {
	BO_NULL_ERROR(context());
	return false;
 }
 if (!display()) {
	BO_NULL_ERROR(display());
	return false;
 }

 bool ret = display()->dispatchMouseMove(context(), e->x(), e->y());
 if (ret) {
	e->accept();
 } else {
	e->ignore();
 }
 return ret;
}

bool BoUfoManager::sendWheelEvent(QWheelEvent* e)
{
 if (!e) {
	BO_NULL_ERROR(e);
	return false;
 }
 if (!context()) {
	BO_NULL_ERROR(context());
	return false;
 }
 if (!display()) {
	BO_NULL_ERROR(display());
	return false;
 }
 int wheelNum = 0;
 if (e->orientation() == Qt::Vertical) {
	wheelNum = 0;
 } else {
	wheelNum = 1;
 }
 ufo::UMouseWheelEvent* event = new ufo::UMouseWheelEvent(
		rootPane(),
		ufo::UEvent::MouseWheel,
		display()->getModState(),
		ufo::UPoint(e->x(), e->y()),
		e->delta(),
		wheelNum);
 event->reference();
 display()->dispatchEvent(event);
 bool ret = event->isConsumed();
 event->unreference();
 if (ret) {
	e->accept();
 } else {
	e->ignore();
 }
 return ret;
}

bool BoUfoManager::sendKeyPressEvent(QKeyEvent* e)
{
 if (!e) {
	BO_NULL_ERROR(e);
	return false;
 }
 if (!context()) {
	BO_NULL_ERROR(context());
	return false;
 }
 if (!display()) {
	BO_NULL_ERROR(display());
	return false;
 }
 ufo::UKeyCode_t key = convertQtKeyToUfo(e->key());
 wchar_t keyChar = e->ascii(); // AB: I have no idea what I am doing here

 bool ret = display()->dispatchKeyDown(context(), key, keyChar);
 if (ret) {
	e->accept();
 } else {
	e->ignore();
 }
 return ret;
}

bool BoUfoManager::sendKeyReleaseEvent(QKeyEvent* e)
{
 if (!e) {
	BO_NULL_ERROR(e);
	return false;
 }
 if (!context()) {
	BO_NULL_ERROR(context());
	return false;
 }
 if (!display()) {
	BO_NULL_ERROR(display());
	return false;
 }
 ufo::UKeyCode_t key = convertQtKeyToUfo(e->key());
 wchar_t keyChar = e->ascii(); // AB: I have no idea what I am doing here

 bool ret = display()->dispatchKeyUp(context(), key, keyChar);
 if (ret) {
	e->accept();
 } else {
	e->ignore();
 }
 return ret;
}

void BoUfoManager::setUfoToolkitProperty(const QString& key, const QString& value)
{
 ufo::UXToolkit* tk = toolkit();
 if (!tk) {
	return;
 }
 tk->putProperty(key.latin1(), value.latin1());
}

QString BoUfoManager::ufoToolkitProperty(const QString& key) const
{
 const ufo::UXToolkit* tk = toolkit();
 if (!tk) {
	return QString::null;
 }
 return QString::fromLatin1(tk->getProperty(key.latin1()).c_str());
}

QMap<QString, QString> BoUfoManager::toolkitProperties() const
{
 QMap<QString, QString> p;
 ufo::UXToolkit* tk = toolkit();
 if (!tk) {
	return p;
 }
#define UFO_PROPERTY(x) p.insert(QString(x), QString(tk->getProperty(x).c_str()))
 UFO_PROPERTY("user_name");
 UFO_PROPERTY("real_name");
 UFO_PROPERTY("home_dir");
 UFO_PROPERTY("tmp_dir");
 UFO_PROPERTY("prg_name");
 UFO_PROPERTY("look_and_feel");
 UFO_PROPERTY("theme_config");
 UFO_PROPERTY("video_driver");
 UFO_PROPERTY("data_dir");
 UFO_PROPERTY("font");
 UFO_PROPERTY("font_dir");
#undef UFO_PROPERTY
 return p;
}

QValueList<BoUfoFontInfo> BoUfoManager::listFonts(const BoUfoFontInfo& fontInfo)
{
 QString originalFontPlugin = ufoToolkitProperty("font");
 QString fontPlugin = fontInfo.fontPlugin();
 if (fontPlugin.isEmpty()) {
	fontPlugin = originalFontPlugin;
 }
 setUfoToolkitProperty("font", fontInfo.fontPlugin());

 QValueList<BoUfoFontInfo> ret;
 std::vector<ufo::UFontInfo> fonts = toolkit()->listFonts(fontInfo.ufoFontInfo());
 for (unsigned int i = 0; i < fonts.size(); i++) {
	ret.append(BoUfoFontInfo(fontPlugin, fonts[i]));
 }
 setUfoToolkitProperty("font", originalFontPlugin);
 return ret;
}

QValueList<BoUfoFontInfo> BoUfoManager::listFonts()
{
 QString originalFontPlugin = ufoToolkitProperty("font");
 if (!toolkit()) {
	BO_NULL_ERROR(toolkit());
	return QValueList<BoUfoFontInfo>();
 }

 QStringList fontPlugins;
 std::vector<ufo::UPluginInfo> pluginInfos = toolkit()->getPluginInfos();
 for (unsigned int i = 0; i < pluginInfos.size(); i++) {
	if (pluginInfos[i].category == "font") {
		fontPlugins.append(pluginInfos[i].feature.c_str());
	}
 }

 QValueList<BoUfoFontInfo> ret;
 for (QStringList::iterator it = fontPlugins.begin(); it != fontPlugins.end(); ++it) {
	setUfoToolkitProperty("font", *it);
	std::vector<ufo::UFontInfo> fonts = toolkit()->listFonts();
	for (unsigned int i = 0; i < fonts.size(); i++) {
		ret.append(BoUfoFontInfo(*it, fonts[i]));
	}
 }
 setUfoToolkitProperty("font", originalFontPlugin);
 return ret;
}

bool BoUfoManager::focusedWidgetTakesKeyEvents() const
{
 ufo::UWidget* focused = ufo::UWidget::getFocusedWidget();
 if (!focused) {
	return false;
 }
 if (focused->isEventEnabled(ufo::UEvent::KeyPressed) ||
		focused->isEventEnabled(ufo::UEvent::KeyReleased) ||
		focused->isEventEnabled(ufo::UEvent::KeyTyped)) {
	return true;
 }
 return false;
}

void BoUfoManager::setGlobalFont(const BoUfoFontInfo& font)
{
 BO_CHECK_NULL_RET(mGlobalFont);
 *mGlobalFont = font;
 if (rootPaneWidget()) {
	if (rootPaneWidget()->providesOwnFont()) {
		return;
	}
	rootPaneWidget()->unsetFont();
 }
}

const BoUfoFontInfo& BoUfoManager::globalFont() const
{
 if (!mGlobalFont) {
	BO_NULL_ERROR(mGlobalFont);
	// will crash
 }
 return *mGlobalFont;
}

BoUfoWidget* BoUfoManager::toolBarContentWidget()
{
 if (mToolBarContentWidget) {
	mToolBarContentWidget->setVisible(true);
 }
 return mToolBarContentWidget;
}


