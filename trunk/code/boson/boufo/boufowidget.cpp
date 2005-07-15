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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufowidget.h"
#include "boufowidget.moc"

#include "boufoimage.h"
#include "boufodrawable.h"
#include "boufomanager.h"

#include <qdom.h>
#include <qimage.h>

#include <bodebug.h>

#include <math.h>


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


class BoUfoWidgetDeleter : public ufo::UCollectable
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
}

void BoUfoWidget::invalidate()
{
// boDebug() << k_funcinfo << endl;
 widget()->invalidateTree();
}

void BoUfoWidget::setMouseEventsEnabled(bool enabled, bool moveEnabled)
{
 widget()->setEventState(ufo::UEvent::MousePressed, enabled);
 widget()->setEventState(ufo::UEvent::MouseReleased, enabled);
 widget()->setEventState(ufo::UEvent::MouseClicked, enabled);
 widget()->setEventState(ufo::UEvent::MouseWheel, enabled);
 widget()->setEventState(ufo::UEvent::MouseMoved, moveEnabled);
 widget()->setEventState(ufo::UEvent::MouseDragged, moveEnabled);

 // AB: I do not consider these to be "move" events, however libufo does
 // (enabling one of these also enables MouseMoved).
 widget()->setEventState(ufo::UEvent::MouseEntered, moveEnabled);
 widget()->setEventState(ufo::UEvent::MouseExited, moveEnabled);
}

void BoUfoWidget::setKeyEventsEnabled(bool enabled)
{
 widget()->setEventState(ufo::UEvent::KeyPressed, enabled);
 widget()->setEventState(ufo::UEvent::KeyReleased, enabled);
 widget()->setEventState(ufo::UEvent::KeyTyped, enabled);
}

void BoUfoWidget::setFocusEventsEnabled(bool enabled)
{
 widget()->setEventState(ufo::UEvent::FocusGained, enabled);
 widget()->setEventState(ufo::UEvent::FocusLost, enabled);
}

void BoUfoWidget::setWidgetEventsEnabled(bool enabled)
{
 widget()->setEventState(ufo::UEvent::WidgetMoved, enabled);
 widget()->setEventState(ufo::UEvent::WidgetResized, enabled);
 widget()->setEventState(ufo::UEvent::WidgetShown, enabled);
 widget()->setEventState(ufo::UEvent::WidgetHidden, enabled);
}

void BoUfoWidget::addWidget(BoUfoWidget* w)
{
 BO_CHECK_NULL_RET(widget());
 BO_CHECK_NULL_RET(w);
 BO_CHECK_NULL_RET(w->widget());
 widget()->add(w->widget());
}

bool BoUfoWidget::removeWidget(BoUfoWidget* w)
{
 if (!widget()) {
	BO_NULL_ERROR(widget());
	return false;
 }
 if (!widget()) {
	BO_NULL_ERROR(widget());
	return false;
 }
 if (!w) {
	BO_NULL_ERROR(w);
	return false;
 }
 if (!w->widget()) {
	BO_NULL_ERROR(w->widget());
	return false;
 }
 return widget()->remove(w->widget());
}

unsigned int BoUfoWidget::removeAllWidgets()
{
 if (!widget()) {
	BO_NULL_ERROR(widget());
	return 0;
 }
 return widget()->removeAll();
}

unsigned int BoUfoWidget::widgetCount() const
{
 if (!widget()) {
	BO_NULL_ERROR(widget());
	return 0;
 }
 return widget()->getWidgetCount();
}

void BoUfoWidget::render(BoUfoManager* ufoManager)
{
 BO_CHECK_NULL_RET(widget());
 BO_CHECK_NULL_RET(ufoManager);
 BO_CHECK_NULL_RET(ufoManager->context());
 BO_CHECK_NULL_RET(ufoManager->context()->getGraphics());
 widget()->paint(ufoManager->context()->getGraphics());
}

void BoUfoWidget::addSpacing(int spacing)
{
 BO_CHECK_NULL_RET(widget());
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
		widget()->setVerticalAlignment(ufo::AlignTop);
		break;
	case AlignBottom:
		widget()->setVerticalAlignment(ufo::AlignBottom);
		break;
	case AlignVCenter:
		widget()->setVerticalAlignment(ufo::AlignCenter);
		break;
 }
}

BoUfoWidget::VerticalAlignment BoUfoWidget::verticalAlignment() const
{
 ufo::Alignment v = widget()->getVerticalAlignment();
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
		widget()->setHorizontalAlignment(ufo::AlignLeft);
		break;
	case AlignRight:
		widget()->setHorizontalAlignment(ufo::AlignRight);
		break;
	case AlignHCenter:
		widget()->setHorizontalAlignment(ufo::AlignCenter);
		break;
 }
}

BoUfoWidget::HorizontalAlignment BoUfoWidget::horizontalAlignment() const
{
 ufo::Alignment h = widget()->getHorizontalAlignment();
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
 ufo::BorderType b = widget()->getBorder();
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
		widget()->setBorder(ufo::NoBorder);
		break;
	case ufo::BottomLineBorder:
		widget()->setBorder(ufo::BottomLineBorder);
		break;
	case ufo::LineBorder:
		widget()->setBorder(ufo::LineBorder);
		break;
	case ufo::RaisedBevelBorder:
		widget()->setBorder(ufo::RaisedBevelBorder);
		break;
	case ufo::LoweredBevelBorder:
		widget()->setBorder(ufo::LoweredBevelBorder);
		break;
	case ufo::StyleBorder:
		widget()->setBorder(ufo::StyleBorder);
		break;
	case ufo::CssBorder:
		widget()->setBorder(ufo::CssBorder);
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
		setLayout(new ufo::UBoxLayout(ufo::Horizontal));
		break;
	case UVBoxLayout:
		setLayout(new ufo::UBoxLayout(ufo::Vertical));
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
 widget()->setLayout(layout);
}

void BoUfoWidget::setEnabled(bool e)
{
 widget()->setEnabled(e);
}

bool BoUfoWidget::isEnabled() const
{
 return widget()->isEnabled();
}

void BoUfoWidget::setOpaque(bool o)
{
 widget()->setOpaque(o);
}

bool BoUfoWidget::opaque() const
{
 return widget()->isOpaque();
}

void BoUfoWidget::setSize(int w, int h)
{
 widget()->setSize(w, h);
}

void BoUfoWidget::setPos(int x, int y)
{
 widget()->setLocation(x, y);
}

int BoUfoWidget::width() const
{
 return widget()->getWidth();
}

int BoUfoWidget::height() const
{
 return widget()->getHeight();
}

int BoUfoWidget::x() const
{
 return widget()->getX();
}

int BoUfoWidget::y() const
{
 return widget()->getY();
}

void BoUfoWidget::setMinimumWidth(int w)
{
 ufo::UDimension s1 = widget()->getMinimumSize();
 ufo::UDimension s(w, s1.getHeight());
 setMinimumSize(s);
}

int BoUfoWidget::minimumWidth() const
{
 ufo::UDimension s = widget()->getMinimumSize();
 return s.getWidth();
}

void BoUfoWidget::setMinimumHeight(int h)
{
 ufo::UDimension s1 = widget()->getMinimumSize();
 ufo::UDimension s(s1.getWidth(), h);
 setMinimumSize(s);
}

int BoUfoWidget::minimumHeight() const
{
 ufo::UDimension s = widget()->getMinimumSize();
 return s.getHeight();
}

void BoUfoWidget::setPreferredWidth(int w)
{
 ufo::UDimension s1 = widget()->getPreferredSize();
 ufo::UDimension s(w, s1.getHeight());
 setPreferredSize(s);
}

int BoUfoWidget::preferredWidth() const
{
 ufo::UDimension s = widget()->getPreferredSize();
 return s.getWidth();
}

void BoUfoWidget::setPreferredHeight(int h)
{
 ufo::UDimension s1 = widget()->getPreferredSize();
 ufo::UDimension s(s1.getWidth(), h);
 setPreferredSize(s);
}

int BoUfoWidget::preferredHeight() const
{
 ufo::UDimension s = widget()->getPreferredSize();
 return s.getHeight();
}

void BoUfoWidget::setMinimumSize(const ufo::UDimension& s)
{
 widget()->setMinimumSize(s);
}

void BoUfoWidget::setPreferredSize(const ufo::UDimension& s)
{
 widget()->setPreferredSize(s);
}

void BoUfoWidget::setVisible(bool v)
{
 widget()->setVisible(v);
 invalidate(); // AB: libufo fails to do so
}

bool BoUfoWidget::isVisible() const
{
 return widget()->isVisible();
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
 widget()->setBackground(drawable);
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

void BoUfoWidget::setBackgroundImageFile(const QString& file)
{
 if (!file.isEmpty()) {
	QImage img;
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
 widget()->setForegroundColor(color);
 std::vector<ufo::UWidget*> widgets = widget()->getWidgets();
 std::vector<ufo::UWidget*>::iterator it;
 for (it = widgets.begin(); it != widgets.end(); ++it) {
	(*it)->setForegroundColor(color);
 }
}

void BoUfoWidget::setBackgroundColor(const QColor& c)
{
 ufo::UColor color(c.red(), c.green(), c.blue());
 widget()->setBackgroundColor(color);
 std::vector<ufo::UWidget*> widgets = widget()->getWidgets();
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
 widget()->put("layout", new ufo::UString(c.latin1()));
}

QString BoUfoWidget::constraints() const
{
 ufo::UObject* o = widget()->get("layout");
 if (!o) {
	return QString::null;
 }
 return o->toString().c_str();
}

void BoUfoWidget::setGridLayoutColumns(int c)
{
 QString s = QString::number(c);
 widget()->put("gridLayoutColumns", new ufo::UString(s.latin1()));
}

int BoUfoWidget::gridLayoutColumns() const
{
 ufo::UObject* o = widget()->get("gridLayoutColumns");
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
 widget()->put("gridLayoutRows", new ufo::UString(s.latin1()));
}

int BoUfoWidget::gridLayoutRows() const
{
 ufo::UObject* o = widget()->get("gridLayoutRows");
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
 widget()->put("stretch", new ufo::UString(f.latin1()));
}

int BoUfoWidget::stretch() const
{
 ufo::UObject* o = widget()->get("stretch");
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
 return widget()->hasMouseFocus();
}

QPoint BoUfoWidget::rootLocation() const
{
 ufo::UPoint p = widget()->getRootLocation();
 return QPoint(p.x, p.y);
}

QPoint BoUfoWidget::mapFromRoot(const QPoint& pos) const
{
 ufo::UPoint p = widget()->rootPointToPoint(pos.x(), pos.y());
 return QPoint(p.x, p.y);
}

QPoint BoUfoWidget::mapToRoot(const QPoint& pos) const
{
 ufo::UPoint p = widget()->pointToRootPoint(pos.x(), pos.y());
 return QPoint(p.x, p.y);
}

QRect BoUfoWidget::widgetViewportRect() const
{
 ufo::UContext* context = widget()->getContext();
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
#if 0
 int button = convertUfoMouseButtonToQt(e->getButton());
 int state = convertUfoModifierStateToQt(e->getModifiers());
 state |= button;
 QMouseEvent me(QEvent::MouseButtonXYZ, QPoint(e->getX(), e->getY()), button, state);
#endif
 emit signalMouseClicked(e);
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

void BoUfoWidget::uslotWidgetAdded(ufo::UWidgetEvent*)
{
 emit signalWidgetAdded();
}

void BoUfoWidget::uslotWidgetRemoved(ufo::UWidgetEvent*)
{
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



