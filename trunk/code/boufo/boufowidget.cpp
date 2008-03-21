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
#include <ufo/gl/ugl_image.hpp>
#include <ufo/ux/ux.hpp>
#include <ufo/events/umousewheelevent.hpp>
#include <ufo/events/ukeysym.hpp>
#include <ufo/widgets/uslider.hpp>
#include <ufo/ui/uuidefs.hpp>
#include "ufoext/ubogridlayout.h"
#include "ufoext/uboboxlayout.h"

// AB: UBoxLayout is deprecated for us. Use UBoBoxLayout instead.
#define UBoxLayout UBoxLayoutDontUse

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufowidget.h"
#include "boufowidget.moc"

#include "boufoimage.h"
#include "boufodrawable.h"
#include "boufomanager.h"
#include "boufofontinfo.h"
#include "boufoprofiling.h"
#include "boufomanager.h"

#include <kglobal.h>
#include <kstandarddirs.h>

#include <qdom.h>
#include <qimage.h>

#include <bodebug.h>

#include <math.h>

static bool g_boufoDebugDestruction = false;

static QMap<ufo::UWidget*, BoUfoWidget*> g_ufoWidget2BoUfoWidget;

static void applyFontToWidgetAndChildren(const ufo::UFont& font, ufo::UWidget* w)
{
 BO_CHECK_NULL_RET(w);
 w->setFont(font);

 std::vector<ufo::UWidget*> widgets = w->getWidgets();
 std::vector<ufo::UWidget*>::iterator it;
 for (it = widgets.begin(); it != widgets.end(); ++it) {
	if (g_ufoWidget2BoUfoWidget.contains(*it)) {
		BoUfoWidget* w = g_ufoWidget2BoUfoWidget[*it];
		if (w->providesOwnFont()) {
			// dont apply to widgets that have a custom font
			continue;
		}
	}
	applyFontToWidgetAndChildren(font, *it);
 }
}


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

static int convertUfoMouseButtonToQt(ufo::UMod_t button)
{
 int qbutton = Qt::NoButton;
 switch (button) {
	case ufo::UMod::LeftButton:
		qbutton = Qt::LeftButton;
		break;
	case ufo::UMod::RightButton:
		qbutton = Qt::RightButton;
		break;
	case ufo::UMod::MiddleButton:
		qbutton = Qt::MidButton;
		break;
	default:
		qbutton = Qt::NoButton;
		break;
 }
 return qbutton;
}

static int convertUfoModifierStateToQt(ufo::UMod_t modifiers)
{
 int s = Qt::NoButton;
 if (modifiers & ufo::UMod::LeftButton) {
	s |= Qt::LeftButton;
 }
 if (modifiers & ufo::UMod::RightButton) {
	s |= Qt::RightButton;
 }
 if (modifiers & ufo::UMod::MiddleButton) {
	s |= Qt::MidButton;
 }
 if (modifiers & ufo::UMod::Shift) {
	s |= Qt::ShiftButton;
 }
 if (modifiers & ufo::UMod::Ctrl) {
	s |= Qt::ControlButton;
 }
 if (modifiers & ufo::UMod::Alt) {
	s |= Qt::AltButton;
 }
 if (modifiers & ufo::UMod::Meta) {
	s |= Qt::MetaButton;
 }
 return s;
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


/**
 * Layout class that is primarily meant for layered panes, that is for @ref
 * BoUfoLayeredPane. This layout simply gives all available size to all its
 * children.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUFullLayout : public ufo::ULayoutManager
{
	UFO_DECLARE_DYNAMIC_CLASS(BoUFullLayout)
public:
	BoUFullLayout()
	{
	}
	~BoUFullLayout()
	{
	}

	virtual void layoutContainer(const ufo::UWidget* parent);
	virtual ufo::UDimension getPreferredLayoutSize(const ufo::UWidget* parent, const ufo::UDimension& maxSize) const;
};
UFO_IMPLEMENT_DYNAMIC_CLASS(BoUFullLayout, ufo::ULayoutManager)

void BoUFullLayout::layoutContainer(const ufo::UWidget* container)
{
 for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
	ufo::UWidget* w = container->getWidget(i);
	if (w->isVisible()) {
		w->setBounds(0, 0, container->getWidth(), container->getHeight());
	}
 }
}

ufo::UDimension BoUFullLayout::getPreferredLayoutSize(const ufo::UWidget* container, const ufo::UDimension& maxSize) const
{
 ufo::UDimension dim(0, 0);
 for (unsigned int i = 0 ; i < container->getWidgetCount() ; i++) {
	const ufo::UWidget* w = container->getWidget(i);
	const ufo::UDimension& s = w->getPreferredSize(maxSize);
	dim.w = std::max(dim.w, s.w);
	dim.h = std::max(dim.h, s.h);
 }
 return dim;
}



//#define DO_BOUFO_PAINT_PROFILING 1
class BoUfoWidgetDeleter : public ufo::UBoUfoWidgetDeleter
{
public:
	BoUfoWidgetDeleter(BoUfoWidget* w)
	{
		mWidget = w;
	}
	~BoUfoWidgetDeleter()
	{
		delete mWidget;
	}

#ifdef DO_BOUFO_PAINT_PROFILING
	virtual void startPaint()
	{
		BoUfoProfiling::profiling()->push(QString("BoUfo: %1 (%2)")
				.arg(mWidget->className())
				.arg(mWidget->name()));
	}
	virtual void endPaint()
	{
		BoUfoProfiling::profiling()->pop();
	}
#endif

private:
	BoUfoWidget* mWidget;
};



BoUfoWidget::BoUfoWidget() : QObject(0, 0)
{
 init(new ufo::UWidget());
 setLayoutClass(UVBoxLayout);
}

BoUfoWidget::BoUfoWidget(ufo::UWidget* w) : QObject(0, 0)
{
 init(w);
}

void BoUfoWidget::init(ufo::UWidget* w)
{
 mWidget = w;
 w->setBoUfoWidgetDeleter(new BoUfoWidgetDeleter(this));
// w->setClipping(false);

 g_ufoWidget2BoUfoWidget.insert(ufoWidget(), this);
 mUsesOwnFont = false;

 mWidget->setOpaque(false);
 mBackgroundImageDrawable = 0;

 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, MouseEntered);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, MouseExited);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, MouseMoved);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, MouseDragged);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, MousePressed);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, MouseReleased);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, MouseClicked);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, MouseWheel);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, KeyPressed);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, KeyReleased);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, KeyTyped);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, WidgetAdded);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, WidgetRemoved);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, WidgetMoved);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, WidgetResized);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, WidgetShown);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, WidgetHidden);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, FocusGained);
 CONNECT_UFO_TO_QT(BoUfoWidget, mWidget, FocusLost);

 // AB: note that disabling these do not influence the child widgets!
 setMouseEventsEnabled(false, false);
 setKeyEventsEnabled(false);

 setFocusEventsEnabled(true);
 setWidgetEventsEnabled(true);

 unsetFont();
}

void BoUfoWidget::setFont(const BoUfoFontInfo& info)
{
 BO_CHECK_NULL_RET(ufoWidget());
 mUsesOwnFont = true;

 BoUfoManager* manager = BoUfoManager::ufoManagerForContext(ufoWidget()->getContext());
 BO_CHECK_NULL_RET(manager);

 applyFontToWidgetAndChildren(info.ufoFont(manager), ufoWidget());
}

void BoUfoWidget::unsetFont()
{
 BO_CHECK_NULL_RET(ufoWidget());
 mUsesOwnFont = false;

 ufo::UFont resetFont;
 if (ufoWidget()->getParent()) {
	resetFont = ufoWidget()->getParent()->getFont();
 } else {
	BoUfoManager* m = BoUfoManager::ufoManagerForContext(ufoWidget()->getContext());
	if (!m) {
		BO_NULL_ERROR(m);
	} else {
		resetFont = m->globalFont().ufoFont(m);
	}
 }

 applyFontToWidgetAndChildren(resetFont, ufoWidget());
}

void BoUfoWidget::setName(const char* name)
{
 QObject::setName(name);
 if (ufoWidget()) {
	ufoWidget()->setName(name);
 }
}

void BoUfoWidget::invalidate()
{
// boDebug() << k_funcinfo << endl;
 ufoWidget()->invalidateTree();
}

void BoUfoWidget::setMouseEventsEnabled(bool enabled, bool moveEnabled)
{
 ufoWidget()->setEventState(ufo::UEvent::MousePressed, enabled);
 ufoWidget()->setEventState(ufo::UEvent::MouseReleased, enabled);
 ufoWidget()->setEventState(ufo::UEvent::MouseClicked, enabled);
 ufoWidget()->setEventState(ufo::UEvent::MouseWheel, enabled);
 ufoWidget()->setEventState(ufo::UEvent::MouseMoved, moveEnabled);
 ufoWidget()->setEventState(ufo::UEvent::MouseDragged, moveEnabled);

 // AB: I do not consider these to be "move" events, however libufo does
 // (enabling one of these also enables MouseMoved).
 ufoWidget()->setEventState(ufo::UEvent::MouseEntered, moveEnabled);
 ufoWidget()->setEventState(ufo::UEvent::MouseExited, moveEnabled);
}

void BoUfoWidget::setKeyEventsEnabled(bool enabled)
{
 ufoWidget()->setEventState(ufo::UEvent::KeyPressed, enabled);
 ufoWidget()->setEventState(ufo::UEvent::KeyReleased, enabled);
 ufoWidget()->setEventState(ufo::UEvent::KeyTyped, enabled);
}

void BoUfoWidget::setFocusEventsEnabled(bool enabled)
{
 ufoWidget()->setEventState(ufo::UEvent::FocusGained, enabled);
 ufoWidget()->setEventState(ufo::UEvent::FocusLost, enabled);
}

void BoUfoWidget::setWidgetEventsEnabled(bool enabled)
{
 ufoWidget()->setEventState(ufo::UEvent::WidgetMoved, enabled);
 ufoWidget()->setEventState(ufo::UEvent::WidgetResized, enabled);
 ufoWidget()->setEventState(ufo::UEvent::WidgetShown, enabled);
 ufoWidget()->setEventState(ufo::UEvent::WidgetHidden, enabled);
}

bool BoUfoWidget::isMouseEventsEnabled() const
{
 bool e = true;
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::MousePressed));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::MouseReleased));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::MouseClicked));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::MouseWheel));
 return e;
}

bool BoUfoWidget::isMouseMoveEventsEnabled() const
{
 bool e = true;
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::MouseMoved));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::MouseDragged));
 return e;
}

bool BoUfoWidget::isKeyEventsEnabled() const
{
 bool e = true;
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::KeyPressed));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::KeyReleased));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::KeyTyped));
 return e;
}

bool BoUfoWidget::isFocusEventsEnabled() const
{
 bool e = true;
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::FocusGained));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::FocusLost));
 return e;
}

bool BoUfoWidget::isWidgetEventsEnabled() const
{
 bool e = true;
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::WidgetMoved));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::WidgetResized));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::WidgetShown));
 e = (e && ufoWidget()->isEventEnabled(ufo::UEvent::WidgetHidden));
 return e;
}

void BoUfoWidget::addWidget(BoUfoWidget* w)
{
 BO_CHECK_NULL_RET(ufoWidget());
 BO_CHECK_NULL_RET(w);
 BO_CHECK_NULL_RET(w->ufoWidget());
 ufoWidget()->add(w->ufoWidget());
}

bool BoUfoWidget::removeWidget(BoUfoWidget* w)
{
 if (!ufoWidget()) {
	BO_NULL_ERROR(ufoWidget());
	return false;
 }
 if (!ufoWidget()) {
	BO_NULL_ERROR(ufoWidget());
	return false;
 }
 if (!w) {
	BO_NULL_ERROR(w);
	return false;
 }
 if (!w->ufoWidget()) {
	BO_NULL_ERROR(w->ufoWidget());
	return false;
 }
 return ufoWidget()->remove(w->ufoWidget());
}

unsigned int BoUfoWidget::removeAllWidgets()
{
 if (!ufoWidget()) {
	BO_NULL_ERROR(ufoWidget());
	return 0;
 }
 return ufoWidget()->removeAll();
}

unsigned int BoUfoWidget::widgetCount() const
{
 if (!ufoWidget()) {
	BO_NULL_ERROR(ufoWidget());
	return 0;
 }
 return ufoWidget()->getWidgetCount();
}

void BoUfoWidget::render(BoUfoManager* ufoManager)
{
 BO_CHECK_NULL_RET(ufoWidget());
 BO_CHECK_NULL_RET(ufoManager);
 BO_CHECK_NULL_RET(ufoManager->context());
 BO_CHECK_NULL_RET(ufoManager->context()->getGraphics());
 ufoWidget()->paint(ufoManager->context()->getGraphics());
}

void BoUfoWidget::addSpacing(int spacing)
{
 BO_CHECK_NULL_RET(ufoWidget());
 BoUfoWidget* w = new BoUfoWidget();
 w->setSize(spacing, spacing);
 w->setMinimumWidth(spacing);
 w->setMinimumHeight(spacing);
 w->setPreferredWidth(spacing);
 w->setPreferredHeight(spacing);
 w->setOpaque(false);
 addWidget(w);
}

void BoUfoWidget::setVerticalAlignment(VerticalAlignment a)
{
 switch (a) {
	default:
	case AlignTop:
		ufoWidget()->setVerticalAlignment(ufo::AlignTop);
		break;
	case AlignBottom:
		ufoWidget()->setVerticalAlignment(ufo::AlignBottom);
		break;
	case AlignVCenter:
		ufoWidget()->setVerticalAlignment(ufo::AlignCenter);
		break;
 }
}

BoUfoWidget::VerticalAlignment BoUfoWidget::verticalAlignment() const
{
 ufo::Alignment v = ufoWidget()->getVerticalAlignment();
 VerticalAlignment a;
 switch (v) {
	case ufo::AlignTop:
		a = AlignTop;
		break;
	case ufo::AlignBottom:
		a = AlignBottom;
		break;
	default:
	case ufo::AlignCenter:
		a = AlignVCenter;
		break;
 }
 return a;
}

void BoUfoWidget::setHorizontalAlignment(HorizontalAlignment a)
{
 switch (a) {
	default:
	case AlignLeft:
		ufoWidget()->setHorizontalAlignment(ufo::AlignLeft);
		break;
	case AlignRight:
		ufoWidget()->setHorizontalAlignment(ufo::AlignRight);
		break;
	case AlignHCenter:
		ufoWidget()->setHorizontalAlignment(ufo::AlignCenter);
		break;
 }
}

BoUfoWidget::HorizontalAlignment BoUfoWidget::horizontalAlignment() const
{
 ufo::Alignment h = ufoWidget()->getHorizontalAlignment();
 HorizontalAlignment a;
 switch (h) {
	case ufo::AlignLeft:
		a = AlignLeft;
		break;
	case ufo::AlignRight:
		a = AlignRight;
		break;
	default:
	case ufo::AlignCenter:
		a = AlignHCenter;
		break;
 }
 return a;
}

BoUfoWidget::BorderType BoUfoWidget::borderType() const
{
 ufo::BorderType b = ufoWidget()->getBorder();
 BorderType t;
 switch (b) {
	default:
	case ufo::NoBorder:
		t = NoBorder;
		break;
	case ufo::BottomLineBorder:
		t = BottomLineBorder;
		break;
	case ufo::LineBorder:
		t = LineBorder;
		break;
	case ufo::RaisedBevelBorder:
		t = RaisedBevelBorder;
		break;
	case ufo::LoweredBevelBorder:
		t = LoweredBevelBorder;
		break;
	case ufo::StyleBorder:
		t = StyleBorder;
		break;
	case ufo::CssBorder:
		t = CssBorder;
		break;
 }
 return t;
}

void BoUfoWidget::setBorderType(BoUfoWidget::BorderType t)
{
 switch (t) {
	default:
	case ufo::NoBorder:
		ufoWidget()->setBorder(ufo::NoBorder);
		break;
	case ufo::BottomLineBorder:
		ufoWidget()->setBorder(ufo::BottomLineBorder);
		break;
	case ufo::LineBorder:
		ufoWidget()->setBorder(ufo::LineBorder);
		break;
	case ufo::RaisedBevelBorder:
		ufoWidget()->setBorder(ufo::RaisedBevelBorder);
		break;
	case ufo::LoweredBevelBorder:
		ufoWidget()->setBorder(ufo::LoweredBevelBorder);
		break;
	case ufo::StyleBorder:
		ufoWidget()->setBorder(ufo::StyleBorder);
		break;
	case ufo::CssBorder:
		ufoWidget()->setBorder(ufo::CssBorder);
		break;
 }
}

void BoUfoWidget::setLayoutClass(LayoutClass layout)
{
 switch (layout) {
	default:
	case NoLayout:
		setLayout(0);
		break;
	case UHBoxLayout:
		setLayout(new ufo::UBoBoxLayout(ufo::Horizontal));
		break;
	case UVBoxLayout:
		setLayout(new ufo::UBoBoxLayout(ufo::Vertical));
		break;
	case UBorderLayout:
		setLayout(new ufo::UBorderLayout());
		break;
	case UGridLayout:
		setLayout(new ufo::UBoGridLayout());
		break;
	case UFullLayout:
		setLayout(new BoUFullLayout());
		break;
 }
}

void BoUfoWidget::setLayout(ufo::ULayoutManager* layout)
{
 ufoWidget()->setLayout(layout);
}

void BoUfoWidget::setEnabled(bool e)
{
 ufoWidget()->setEnabled(e);
}

bool BoUfoWidget::isEnabled() const
{
 return ufoWidget()->isEnabled();
}

void BoUfoWidget::setOpaque(bool o)
{
 ufoWidget()->setOpaque(o);
}

bool BoUfoWidget::opaque() const
{
 return ufoWidget()->isOpaque();
}

void BoUfoWidget::setSize(int w, int h)
{
 ufoWidget()->setSize(w, h);
}

void BoUfoWidget::setPos(int x, int y)
{
 ufoWidget()->setLocation(x, y);
}

int BoUfoWidget::width() const
{
 return ufoWidget()->getWidth();
}

int BoUfoWidget::height() const
{
 return ufoWidget()->getHeight();
}

int BoUfoWidget::x() const
{
 return ufoWidget()->getX();
}

int BoUfoWidget::y() const
{
 return ufoWidget()->getY();
}

void BoUfoWidget::setMinimumWidth(int w)
{
 ufo::UDimension s1 = ufoWidget()->getMinimumSize();
 ufo::UDimension s(w, s1.getHeight());
 setMinimumSize(s);
}

int BoUfoWidget::minimumWidth() const
{
 ufo::UDimension s = ufoWidget()->getMinimumSize();
 return s.getWidth();
}

void BoUfoWidget::setMinimumHeight(int h)
{
 ufo::UDimension s1 = ufoWidget()->getMinimumSize();
 ufo::UDimension s(s1.getWidth(), h);
 setMinimumSize(s);
}

int BoUfoWidget::minimumHeight() const
{
 ufo::UDimension s = ufoWidget()->getMinimumSize();
 return s.getHeight();
}

void BoUfoWidget::setPreferredWidth(int w)
{
 ufo::UDimension s1 = ufoWidget()->getPreferredSize();
 ufo::UDimension s(w, s1.getHeight());
 setPreferredSize(s);
}

int BoUfoWidget::preferredWidth() const
{
 ufo::UDimension s = ufoWidget()->getPreferredSize();
 return s.getWidth();
}

void BoUfoWidget::setPreferredHeight(int h)
{
 ufo::UDimension s1 = ufoWidget()->getPreferredSize();
 ufo::UDimension s(s1.getWidth(), h);
 setPreferredSize(s);
}

int BoUfoWidget::preferredHeight() const
{
 ufo::UDimension s = ufoWidget()->getPreferredSize();
 return s.getHeight();
}

void BoUfoWidget::setMinimumSize(const ufo::UDimension& s)
{
 ufoWidget()->setMinimumSize(s);
}

void BoUfoWidget::setPreferredSize(const ufo::UDimension& s)
{
 ufoWidget()->setPreferredSize(s);
}

void BoUfoWidget::setVisible(bool v)
{
 ufoWidget()->setVisible(v);
 invalidate(); // AB: libufo fails to do so
}

bool BoUfoWidget::isVisible() const
{
 return ufoWidget()->isVisible();
}

void BoUfoWidget::setTakesKeyboardFocus(bool f)
{
 ufoWidget()->setFocusable(f);
}

bool BoUfoWidget::takesKeyboardFocus() const
{
 return ufoWidget()->isFocusable();
}

void BoUfoWidget::setFocus()
{
 ufoWidget()->requestFocus();
}

void BoUfoWidget::clearFocus()
{
 ufoWidget()->releaseFocus();
}

bool BoUfoWidget::hasFocus() const
{
 return ufoWidget()->isFocused();
}

void BoUfoWidget::loadPropertiesFromXML(const QDomElement& root)
{
 if (root.isNull()) {
	return;
 }
 QMap<QString, QString> properties;

 for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	properties.insert(e.tagName(), e.text());
 }
 loadProperties(properties);
}

void BoUfoWidget::setBackground(BoUfoDrawable* drawable)
{
 if (drawable) {
	setBackground(drawable->drawable());
 } else {
	setBackground((ufo::UDrawable*)0);
 }
}

void BoUfoWidget::setBackground(ufo::UDrawable* drawable)
{
 ufoWidget()->setBackground(drawable);
 if (drawable) {
	// AB: libufo should do this, I believe!
	const ufo::UDimension& s = drawable->getDrawableSize();
	setSize(s.w, s.h);
 }
}

void BoUfoWidget::setBackgroundImage(const BoUfoImage& img)
{
 BO_CHECK_NULL_RET(img.image());
 if (mBackgroundImageDrawable) {
	mBackgroundImageDrawable->unreference();
	mBackgroundImageDrawable = 0;
 }
 mBackgroundImageDrawable = img.image();
 mBackgroundImageDrawable->reference();
 setBackground(img.image()); // ufo::UImage is a ufo::UDrawable
}

void BoUfoWidget::setBackgroundImageFile(const QString& file_)
{
 QString file = file_;
 if (!file_.isEmpty()) {
	QImage img;
	if (KGlobal::_instance) { // NULL in boufodesigner
		file = locate("data", "boson/" + file_);
		if (file.isEmpty()) {
			boDebug() << k_funcinfo << "file " << file_ << " not found" << endl;
			file = file_;
		}
	} else if (BoUfoManager::currentUfoManager()) {
		QString dataDir = BoUfoManager::currentUfoManager()->dataDir();
		if (!dataDir.isEmpty()) {
			file = dataDir + "/" + file_;
		}
	}
	if (!img.load(file)) {
		boError() << k_funcinfo << file << " could not be loaded" << endl;
		return;
	}
	setBackgroundImage(img);
 } else {
	setBackgroundImage(BoUfoImage());
 }
 mBackgroundImageFile = file;
}

QString BoUfoWidget::backgroundImageFile() const
{
 return mBackgroundImageFile;
}

void BoUfoWidget::setForegroundColor(const QColor& c)
{
 ufo::UColor color(c.red(), c.green(), c.blue());
 ufoWidget()->setForegroundColor(color);
 std::vector<ufo::UWidget*> widgets = ufoWidget()->getWidgets();
 std::vector<ufo::UWidget*>::iterator it;
 for (it = widgets.begin(); it != widgets.end(); ++it) {
	(*it)->setForegroundColor(color);
 }
}

void BoUfoWidget::setBackgroundColor(const QColor& c)
{
 ufo::UColor color(c.red(), c.green(), c.blue());
 ufoWidget()->setBackgroundColor(color);
 std::vector<ufo::UWidget*> widgets = ufoWidget()->getWidgets();
 std::vector<ufo::UWidget*>::iterator it;
 for (it = widgets.begin(); it != widgets.end(); ++it) {
	(*it)->setBackgroundColor(color);
 }
}

void BoUfoWidget::loadProperties(const QMap<QString, QString>& properties)
{
 QStrList propertyNames = metaObject()->propertyNames(true);
 QStrListIterator it(propertyNames);
 while (it.current()) {
	QString name = it.current();
	++it;
	if (!properties.contains(name)) {
		continue;
	}
	if (properties[name].isEmpty()) {
		continue;
	}
	QVariant value = properties[name];
	if (!setProperty(name, value)) {
		boError() << k_funcinfo << "could not set property " << name << " to " << properties[name] << endl;
	}
 }
}

void BoUfoWidget::setConstraints(const QString& c)
{
 ufoWidget()->put("layout", new ufo::UString(c.latin1()));
}

QString BoUfoWidget::constraints() const
{
 ufo::UObject* o = ufoWidget()->get("layout");
 if (!o) {
	return QString::null;
 }
 return o->toString().c_str();
}

void BoUfoWidget::setGridLayoutColumns(int c)
{
 QString s = QString::number(c);
 ufoWidget()->put("gridLayoutColumns", new ufo::UString(s.latin1()));
}

int BoUfoWidget::gridLayoutColumns() const
{
 ufo::UObject* o = ufoWidget()->get("gridLayoutColumns");
 if (!o) {
	return -1;
 }
 bool ok;
 QString s = o->toString().c_str();
 int c = s.toInt(&ok);
 if (!ok) {
	return -1;
 }
 return c;
}

void BoUfoWidget::setGridLayoutRows(int r)
{
 QString s = QString::number(r);
 ufoWidget()->put("gridLayoutRows", new ufo::UString(s.latin1()));
}

int BoUfoWidget::gridLayoutRows() const
{
 ufo::UObject* o = ufoWidget()->get("gridLayoutRows");
 if (!o) {
	return -1;
 }
 bool ok;
 QString s = o->toString().c_str();
 int r = s.toInt(&ok);
 if (!ok) {
	return -1;
 }
 return r;
}

void BoUfoWidget::setStretch(int factor)
{
 QString f = QString::number(factor);
 ufoWidget()->put("stretch", new ufo::UString(f.latin1()));
}

int BoUfoWidget::stretch() const
{
 ufo::UObject* o = ufoWidget()->get("stretch");
 if (!o) {
	return 0;
 }
 bool ok;
 QString f = o->toString().c_str();
 int factor = f.toInt(&ok);
 if (!ok || factor < 0) {
	return 0;
 }
 return factor;
}

bool BoUfoWidget::hasMouse() const
{
 return ufoWidget()->hasMouseFocus();
}

QPoint BoUfoWidget::rootLocation() const
{
 ufo::UPoint p = ufoWidget()->getRootLocation();
 return QPoint(p.x, p.y);
}

QPoint BoUfoWidget::mapFromRoot(const QPoint& pos) const
{
 ufo::UPoint p = ufoWidget()->rootPointToPoint(pos.x(), pos.y());
 return QPoint(p.x, p.y);
}

QPoint BoUfoWidget::mapToRoot(const QPoint& pos) const
{
 ufo::UPoint p = ufoWidget()->pointToRootPoint(pos.x(), pos.y());
 return QPoint(p.x, p.y);
}

QRect BoUfoWidget::widgetViewportRect() const
{
 ufo::UContext* context = ufoWidget()->getContext();
 if (!context) {
	BO_NULL_ERROR(context);
	return QRect(0, 0, width(), height());
 }
 const ufo::URectangle& deviceBounds = context->getDeviceBounds();
 const ufo::URectangle& contextBounds = context->getContextBounds();

 QPoint pos = rootLocation();
 int w = width();
 if (pos.x() + w > contextBounds.w) {
	w = contextBounds.w - pos.x();
 }
 int h = height();
 if (pos.y() + h > contextBounds.h) {
	h = contextBounds.h - pos.y();
 }
 return QRect(contextBounds.x + pos.x(),
		(deviceBounds.h - contextBounds.y - contextBounds.h) + (contextBounds.h - pos.y() - h),
		w,
		h);
}

BoUfoWidget::~BoUfoWidget()
{
 // AB: do NOT delete the mWidget!
 // we are child of mWidget, not the other way round.

 if (mBackgroundImageDrawable) {
	mBackgroundImageDrawable->unreference();
 }
 g_ufoWidget2BoUfoWidget.remove(ufoWidget());

 if (g_boufoDebugDestruction ) { boDebug() << k_funcinfo << name() << endl; }
}

void BoUfoWidget::uslotMouseEntered(ufo::UMouseEvent* e)
{
 emit signalMouseEntered(e);
}

void BoUfoWidget::uslotMouseExited(ufo::UMouseEvent* e)
{
 emit signalMouseExited(e);
}

void BoUfoWidget::uslotMouseMoved(ufo::UMouseEvent* e)
{
 int button = convertUfoMouseButtonToQt(e->getButton());
 int state = convertUfoModifierStateToQt(e->getModifiers());
 QMouseEvent me(QEvent::MouseMove, QPoint(e->getX(), e->getY()), button, state);
 me.ignore();
 emit signalMouseMoved(&me);
 if (me.isAccepted()) {
	e->consume();
 }
}

void BoUfoWidget::uslotMouseDragged(ufo::UMouseEvent* e)
{
 int button = convertUfoMouseButtonToQt(e->getButton());
 int state = convertUfoModifierStateToQt(e->getModifiers());
 QMouseEvent me(QEvent::MouseMove, QPoint(e->getX(), e->getY()), button, state);
 me.ignore();
 emit signalMouseDragged(&me);
 if (me.isAccepted()) {
	e->consume();
 }
}

void BoUfoWidget::uslotMousePressed(ufo::UMouseEvent* e)
{
 int button = convertUfoMouseButtonToQt(e->getButton());
 int state = convertUfoModifierStateToQt(e->getModifiers());
 state &= ~button;
 QMouseEvent me(QEvent::MouseButtonPress, QPoint(e->getX(), e->getY()), button, state);
 me.ignore();
 emit signalMousePressed(&me);
 if (me.isAccepted()) {
	e->consume();
 }
}

void BoUfoWidget::uslotMouseReleased(ufo::UMouseEvent* e)
{
 int button = convertUfoMouseButtonToQt(e->getButton());
 int state = convertUfoModifierStateToQt(e->getModifiers());
 state |= button;
 QMouseEvent me(QEvent::MouseButtonRelease, QPoint(e->getX(), e->getY()), button, state);
 me.ignore();
 emit signalMouseReleased(&me);
 if (me.isAccepted()) {
	e->consume();
 }
}

void BoUfoWidget::uslotMouseClicked(ufo::UMouseEvent* e)
{
 int button = convertUfoMouseButtonToQt(e->getButton());
 int state = convertUfoModifierStateToQt(e->getModifiers());
 state |= button;
 BoUfoMouseEventClick me((QEvent::Type)BoUfoMouseEventClick::EventClick,
		QPoint(e->getX(), e->getY()),
		button,
		state,
		e->getClickCount());
 emit signalMouseClicked(&me);
 if (me.isAccepted()) {
	e->consume();
 }
}

// TODO: uslotMouseClicked, QEvent::MouseButtonDblClick
void BoUfoWidget::uslotMouseWheel(ufo::UMouseWheelEvent* e)
{
 Orientation orientation;
 if (e->getWheel() == 0) {
	orientation = Qt::Vertical;
 } else {
	orientation = Qt::Horizontal;
 }
 int state = convertUfoModifierStateToQt(e->getModifiers());
 QWheelEvent we(QPoint(e->getX(), e->getY()), e->getDelta(), state, orientation);
 we.ignore();
 emit signalMouseWheel(&we);
 if (we.isAccepted()) {
	e->consume();
 }
}

void BoUfoWidget::uslotKeyPressed(ufo::UKeyEvent* e)
{
 emit signalKeyPressed(e);
}

void BoUfoWidget::uslotKeyReleased(ufo::UKeyEvent* e)
{
 emit signalKeyReleased(e);
}

void BoUfoWidget::uslotKeyTyped(ufo::UKeyEvent* e)
{
 emit signalKeyTyped(e);
}

// called when _this_ widget is added to another widget
void BoUfoWidget::uslotWidgetAdded(ufo::UWidgetEvent*)
{
 if (!providesOwnFont()) {
	unsetFont();
 }
 emit signalWidgetAdded();
}

// called when _this_ widget is removed from another widget
void BoUfoWidget::uslotWidgetRemoved(ufo::UWidgetEvent*)
{
 if (!providesOwnFont()) {
	unsetFont();
 }
 emit signalWidgetRemoved();
}

void BoUfoWidget::uslotWidgetMoved(ufo::UWidgetEvent* e)
{
 emit signalWidgetMoved(e);
}

void BoUfoWidget::uslotWidgetResized(ufo::UWidgetEvent* e)
{
 emit signalWidgetResized();
}

void BoUfoWidget::uslotWidgetShown(ufo::UWidgetEvent* e)
{
 emit signalWidgetShown(e);
}

void BoUfoWidget::uslotWidgetHidden(ufo::UWidgetEvent* e)
{
 emit signalWidgetHidden(e);
}

void BoUfoWidget::uslotFocusGained(ufo::UFocusEvent* e)
{
 emit signalFocusGained(e);
}

void BoUfoWidget::uslotFocusLost(ufo::UFocusEvent* e)
{
 emit signalFocusLost(e);
}




BoUfoVBox::BoUfoVBox() : BoUfoWidget()
{
 init();
}

void BoUfoVBox::init()
{
 setLayoutClass(UVBoxLayout);
 setOpaque(false);
}

BoUfoHBox::BoUfoHBox() : BoUfoWidget()
{
 init();
}

void BoUfoHBox::init()
{
 setLayoutClass(UHBoxLayout);
 setOpaque(false);
}



