/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <ufo/ufo.hpp>
#include <ufo/ux/ux.hpp>
#include <ufo/events/umousewheelevent.hpp>
#include <ufo/events/ukeysym.hpp>
#include <ufo/widgets/uslider.hpp>

#include "boufo.h"
#include "boufo.moc"

#include <qevent.h>
#include <qobject.h>

#include <bodebug.h>

#include <GL/gl.h>

#if 0
// this is a UObject class that deletes the QObject that it gets as parent.
// the intention of it is as follows: add an object of this class to every Boson
// class (that is derived of QObject) that uses a Ufo object. Add this object to
// the Ufo object.
//
// When the Ufo object gets deleted, the parent (boson-) object (which is a
// QObject) is being deleted as well.
class BoUfoParentDeleter : public ufo::UObject
{
public:
	BoUfoParentDeleter(QObject* parent)
		: ufo::UObject()
	{
		mParent = parent;
	}
	~BoUfoParentDeleter()
	{
		delete mParent;
		setDeleted();
	}
	void setDeleted()
	{
		mParent = 0;
	}
private:
	QObject* mParent;
};

#endif


static ufo::UMod_t convertQtMouseButtonToUfo(int button)
{
 ufo::UMod_t ubutton = ufo::UMod::NoModifier;
 switch (button) {
	case QMouseEvent::LeftButton:
		ubutton = ufo::UMod::LeftButton;
		break;
	case QMouseEvent::RightButton:
		ubutton = ufo::UMod::RightButton;
		break;
	case QMouseEvent::MidButton:
		ubutton = ufo::UMod::MiddleButton;
		break;
	default:
		ubutton = ufo::UMod::NoModifier;
		break;
 }
 return ubutton;
}

// convert keyboard modifiers
static ufo::UMod_t convertQtKeyStateToUfo(int state)
{
#warning TODO
 return ufo::UMod::NoModifier;
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

BoUfoManager::BoUfoManager(int w, int h)
{
 boDebug() << k_funcinfo << w << "x" << h << endl;
 mToolkit = new ufo::UXToolkit();
 mDisplay = new ufo::UXDisplay();

 ufo::URectangle deviceRect(0, 0, w, h);
 ufo::URectangle contextRect(0, 0, w, h);
 mContext = new ufo::UXContext(deviceRect, contextRect);

 mRootPane = mContext->getRootPane();
 mContentWidget = 0;

 if (!mRootPane) {
	BO_NULL_ERROR(mRootPane);
 } else {
	mContentWidget = mRootPane->getContentPane();
 }

 if (rootPane() && contentWidget()) {
	initUfoWidgets();
 }
}

BoUfoManager::~BoUfoManager()
{
 boDebug() << k_funcinfo << endl;
 delete mContext;
 delete mDisplay;
 delete mToolkit;
}

void BoUfoManager::initUfoWidgets()
{
 BO_CHECK_NULL_RET(rootPane());
 BO_CHECK_NULL_RET(contentWidget());
 rootPane()->setOpaque(false);
 contentWidget()->setOpaque(false);
}

void BoUfoManager::render()
{
 BO_CHECK_NULL_RET(context());
// BO_CHECK_NULL_RET(display());

 glDisable(GL_TEXTURE_2D);
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 context()->forceRepaint();
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 
// TODO glViewport(0, 0, w, h);
// -> libufo sets a different viewport, but doesn't reset to the original one
}

void BoUfoManager::postResizeEvent(int w, int h)
{
 BO_CHECK_NULL_RET(context());
 boDebug() << k_funcinfo << w << "x" << h << endl;
 ufo::URectangle deviceRect(0, 0, w, h);
 ufo::URectangle contextRect(0, 0, w, h);
 context()->setDeviceBounds(deviceRect);
 context()->setContextBounds(contextRect);
}

void BoUfoManager::postMousePressEvent(QMouseEvent* e)
{
 BO_CHECK_NULL_RET(e);
 BO_CHECK_NULL_RET(context());
 BO_CHECK_NULL_RET(display());
 ufo::UMod_t button = convertQtMouseButtonToUfo(e->button());
 ufo::UMod_t modifiers = convertQtKeyStateToUfo(e->state());
 display()->pushMouseButtonDown(context(), modifiers, e->x(), e->y(), button);
}

void BoUfoManager::postMouseReleaseEvent(QMouseEvent* e)
{
 BO_CHECK_NULL_RET(e);
 BO_CHECK_NULL_RET(context());
 BO_CHECK_NULL_RET(display());
 ufo::UMod_t button = convertQtMouseButtonToUfo(e->button());
 ufo::UMod_t modifiers = convertQtKeyStateToUfo(e->state());
 display()->pushMouseButtonUp(context(), modifiers, e->x(), e->y(), button);
}

void BoUfoManager::postMouseMoveEvent(QMouseEvent* e)
{
 BO_CHECK_NULL_RET(e);
 BO_CHECK_NULL_RET(context());
 BO_CHECK_NULL_RET(display());
 ufo::UMod_t modifiers = convertQtKeyStateToUfo(e->state());
 display()->pushMouseMove(context(), modifiers, e->x(), e->y());
}

void BoUfoManager::postWheelEvent(QWheelEvent* e)
{
 BO_CHECK_NULL_RET(e);
 BO_CHECK_NULL_RET(context());
 BO_CHECK_NULL_RET(display());
 int wheelNum = 0;
 if (e->orientation() == Qt::Vertical) {
	wheelNum = 0;
 } else {
	wheelNum = 1;
 }
 ufo::UMod_t modifiers = convertQtKeyStateToUfo(e->state());
 ufo::UMouseWheelEvent* ue = new ufo::UMouseWheelEvent(
		rootPane(),
		ufo::UEvent::MouseWheel,
		modifiers,
		ufo::UPoint(e->x(), e->y()),
		e->delta(),
		wheelNum);
 display()->pushEvent(ue);
}

void BoUfoManager::postKeyPressEvent(QKeyEvent* e)
{
 BO_CHECK_NULL_RET(e);
 BO_CHECK_NULL_RET(display());
 BO_CHECK_NULL_RET(context());
 BO_CHECK_NULL_RET(e);
 BO_CHECK_NULL_RET(display());
 BO_CHECK_NULL_RET(context());
 ufo::UMod_t modifiers = convertQtKeyStateToUfo(e->state());
 ufo::UKeyCode_t key = convertQtKeyToUfo(e->key());
 wchar_t keyChar = e->ascii(); // AB: I have no idea what I am doing here
 display()->pushKeyDown(context(), modifiers, key, keyChar);
}

void BoUfoManager::postKeyReleaseEvent(QKeyEvent* e)
{
 BO_CHECK_NULL_RET(e);
 BO_CHECK_NULL_RET(display());
 BO_CHECK_NULL_RET(context());
 ufo::UMod_t modifiers = convertQtKeyStateToUfo(e->state());
 ufo::UKeyCode_t key = convertQtKeyToUfo(e->key());
 wchar_t keyChar = e->ascii(); // AB: I have no idea what I am doing here
 display()->pushKeyUp(context(), modifiers, key, keyChar);
}

BoUfoWidget::BoUfoWidget(QObject* parent, const char* name)
	: QObject(parent, name)
{
 mWidget = new ufo::UWidget();
 mWidget->setOpaque(false);
 mWidget->setLayout(new ufo::UFlowLayout());

#if 0
 // AB: this cannot easily be done, because add() may require an additional
 // argument, such as "North"
 if (parent && parent->inherits("BoUfoWidget")) {
	if (!parent->isA("BoUfoWidget")) {
		boWarning() << k_funcinfo << "parent is derived from BoUfoWidget, but is not a BoUfoWidget. This may have undesired effects." << endl;
		parent->contentWidget()->add(contentWidget());
	}
 }
#endif
}

BoUfoWidget::~BoUfoWidget()
{
 delete mWidget;
}

void BoUfoWidget::addWidget(BoUfoWidget* w)
{
 BO_CHECK_NULL_RET(w);
 BO_CHECK_NULL_RET(w->contentWidget());
 BO_CHECK_NULL_RET(contentWidget());
 contentWidget()->add(w->contentWidget());
}

void BoUfoWidget::addSpacing(int)
{
 boWarning() << k_funcinfo << "TODO" << endl;
}

void BoUfoWidget::setOpaque(bool o)
{
 mWidget->setOpaque(o);
 setChildrenOpaque(o);
}

void BoUfoWidget::setLayout(ufo::ULayoutManager* l)
{
 if (!isA("BoUfoWidget")) {
	boWarning() << k_funcinfo << "setLayout() has is undefined for derived widgets!" << endl;
 }
 mWidget->setLayout(l);
}


BoUfoHBox::BoUfoHBox(QObject* parent, const char* name)
	: BoUfoWidget(parent, name)
{
 // TODO
 // this widget is supposed to line the widget horizontally up
// contentWidget()->setLayout(new ufo::UFlowLayout);
 contentWidget()->setLayout(new ufo::UBoxLayout(ufo::UBoxLayout::XAxis));
}

BoUfoVBox::BoUfoVBox(QObject* parent, const char* name)
	: BoUfoWidget(parent, name)
{
 // TODO
 // this widget is supposed to line the widget vertically up
 contentWidget()->setLayout(new ufo::UBoxLayout(ufo::UBoxLayout::YAxis));
}

BoUfoPushButton::BoUfoPushButton(QObject* parent, const char* name)
	: BoUfoWidget(parent, name)
{
 init(QString::null);
}

BoUfoPushButton::BoUfoPushButton(const QString& text, QObject* parent, const char* name)
	: BoUfoWidget(parent, name)
{
 init(text);
}

void BoUfoPushButton::init(const QString& text)
{
 contentWidget()->setLayout(new ufo::UFlowLayout());
 mButton = new ufo::UButton(text.latin1());
 contentWidget()->add(mButton);
 mButton->setOpaque(false);
}

BoUfoPushButton::~BoUfoPushButton()
{
 delete mButton;
}


BoUfoCheckBox::BoUfoCheckBox(QObject* parent, const char* name)
	: BoUfoWidget(parent, name)
{
 init(QString::null);
}

BoUfoCheckBox::BoUfoCheckBox(const QString& text, QObject* parent, const char* name)
	: BoUfoWidget(parent, name)
{
 init(text);
}

BoUfoCheckBox::BoUfoCheckBox(const QString& text, bool checked, QObject* parent, const char* name)
	: BoUfoWidget(parent, name)
{
 init(text);
 setChecked(checked);
}

void BoUfoCheckBox::init(const QString& text)
{
 contentWidget()->setLayout(new ufo::UFlowLayout());
 mCheckBox = new ufo::UCheckBox(text);
 contentWidget()->add(mCheckBox);

 // AB: at least the background of the label must be transparent. unfortunately,
 // libufo uses UButton for the checkbox, so the actual checkbox and its label
 // are the same widget
 mCheckBox->setOpaque(false);
}

BoUfoCheckBox::~BoUfoCheckBox()
{
 delete mCheckBox;
}

void BoUfoCheckBox::setChecked(bool)
{
 boWarning() << k_funcinfo << "TODO" << endl;
}


BoUfoNumInput::BoUfoNumInput(QObject* parent, const char* name)
	: BoUfoWidget(parent, name)
{
 init();
}

BoUfoNumInput::~BoUfoNumInput()
{
 delete mTextEdit;
 delete mSlider;
}

void BoUfoNumInput::init()
{
 contentWidget()->setLayout(new ufo::UFlowLayout());

 //TODO USlider uses integers. provide a widget that uses floats.
 mSlider = new ufo::USlider();

 // TODO: allow only float inputs. there is a
 // UDocumentFactory::createDigitDocument(), but that one allows digits only,
 // i.e. only integers
 mTextEdit = new ufo::UTextEdit();
 contentWidget()->add(mSlider);
 contentWidget()->add(mTextEdit);
}

void BoUfoNumInput::setLabel(const QString&)
{
 boWarning() << k_funcinfo << "TODO" << endl;
}

void BoUfoNumInput::setValue(float)
{
 boWarning() << k_funcinfo << "TODO" << endl;
}

void BoUfoNumInput::slotSetMaxValue(float)
{
 boWarning() << k_funcinfo << "TODO" << endl;
}

void BoUfoNumInput::setStepSize(float)
{
 boWarning() << k_funcinfo << "TODO" << endl;
}

void BoUfoNumInput::setRange(float, float)
{
 boWarning() << k_funcinfo << "TODO" << endl;
}
