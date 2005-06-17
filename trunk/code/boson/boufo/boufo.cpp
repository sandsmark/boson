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
#include "ubolabelui.h"
#include "uboprogress.h"
#include "uboprogressui.h"
#include "ubogridlayout.h"
// AB: even though this starts with "boufo" it is actually a ufo clas
#include "boufofontrenderer.h"

#include "boufoaction.h"

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufo.h"
#include "boufo.moc"

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qobject.h>
#include <qdom.h>
#include <qintdict.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qgl.h>

#include <bodebug.h>

QColor BoUfoLabel::mDefaultForegroundColor = Qt::black;



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


BoUfoFontInfo::BoUfoFontInfo()
{
 mPointSize = 12.0f;
 mStyle = 0;
 mFixedSize = false;
}

BoUfoFontInfo::BoUfoFontInfo(const BoUfoFontInfo& font)
{
 *this = font;
}

BoUfoFontInfo::BoUfoFontInfo(const QString& fontPlugin, const ufo::UFontInfo& font)
{
 mFontPlugin = fontPlugin;
 mPointSize = font.pointSize;
 mFixedSize = false;
 mStyle = 0;

 if (fontPlugin == "builtin_font") {
	// AB: the builtin fonts don't use font.face
	switch (font.family) {
		default:
		case ufo::UFontInfo::DefaultFamily:
		case ufo::UFontInfo::SansSerif:
		case ufo::UFontInfo::Decorative:
			mFamily = "SansSerif";
			break;
		case ufo::UFontInfo::Serif:
			mFamily = "Serif";
			break;
		case ufo::UFontInfo::MonoSpaced:
			mFamily = "MonoSpaced";
			mFixedSize = true;
			break;
	}
 } else {
	mFamily = QString::fromLatin1(font.face.c_str());
	if (mFamily.isEmpty()) {
		mFamily = QString("Unknown");
	}
 }

 // non-builtin plugins currently don't use the family property, but for
 // completelyness we check anyway
 if (font.family == ufo::UFontInfo::MonoSpaced) {
	mFixedSize = true;
 }

 switch (font.weight) {
	default:
		break;
	case ufo::UFontInfo::Bold:
	case ufo::UFontInfo::Black:
		mStyle |= StyleBold;
		break;
 }
 if (font.style & ufo::UFontInfo::Italic) {
	mStyle |= StyleItalic;
 }
 if (font.style & ufo::UFontInfo::Underline) {
	mStyle |= StyleUnderline;
 }
 if (font.style & ufo::UFontInfo::StrikeOut) {
	mStyle |= StyleStrikeOut;
 }
}

BoUfoFontInfo& BoUfoFontInfo::operator=(const BoUfoFontInfo& font)
{
 mFontPlugin = font.fontPlugin();
 mFamily = font.family();
 mStyle = font.style();
 mPointSize = font.pointSize();
 mFixedSize = font.fixedSize();
 return *this;
}

ufo::UFont* BoUfoFontInfo::ufoFont(BoUfoManager* manager) const
{
 QString originalFontPlugin = manager->ufoToolkitProperty("font");
 if (!fontPlugin().isEmpty()) {
	manager->setUfoToolkitProperty("font", fontPlugin());
 }

 ufo::UFont* font = new ufo::UFont(ufoFontInfo());

 manager->setUfoToolkitProperty("font", originalFontPlugin);
 return font;
}

ufo::UFontInfo BoUfoFontInfo::ufoFontInfo() const
{
 ufo::UFontInfo::Family family = ufo::UFontInfo::DefaultFamily;
 std::string face = "";
 bool fixed = fixedSize();

 if (mFontPlugin == "builtin_font") {
	if (mFamily == "Decorative") {
		family = ufo::UFontInfo::SansSerif;
	} else if (mFamily == "SansSerif") {
		family = ufo::UFontInfo::SansSerif;
	} else if (mFamily == "Serif") {
		family = ufo::UFontInfo::Serif;
	} else if (mFamily == "MonoSpaced") {
		fixed = true;
		family = ufo::UFontInfo::MonoSpaced;
	}
	face = ""; // unused by this plugin
 } else {
	family = ufo::UFontInfo::DefaultFamily; // unused by all other plugins
	face = std::string(mFamily.latin1());
 }

 int weight = ufo::UFontInfo::Normal;
 int style = ufo::UFontInfo::AnyStyle;
 if (italic()) {
	style |= ufo::UFontInfo::Italic;
 }
 if (underline()) {
	style |= ufo::UFontInfo::Underline;
 }
 if (strikeOut()) {
	style |= ufo::UFontInfo::StrikeOut;
 }
 if (style == ufo::UFontInfo::AnyStyle) {
	style = ufo::UFontInfo::Plain;
 }

 if (fixed) {
	family = ufo::UFontInfo::MonoSpaced;
	face = "";
 }

 ufo::UFontInfo fontInfo;
 fontInfo.family = family;
 fontInfo.face = face;
 fontInfo.pointSize = pointSize();
 fontInfo.weight = weight;
 fontInfo.style = style;
 fontInfo.encoding = ufo::UFontInfo::Encoding_Default;
 return fontInfo;
}


BoUfoImageIO::BoUfoImageIO()
{
 init();
}

BoUfoImageIO::BoUfoImageIO(const QPixmap& p)
{
 init();
 setPixmap(p);
}

BoUfoImageIO::BoUfoImageIO(const QImage& img)
{
 init();
 setImage(img);
}

void BoUfoImageIO::init()
{
 mImageIO = 0;
}

BoUfoImageIO::~BoUfoImageIO()
{
 if (mImageIO) {
	mImageIO->unreference();
 }
}

void BoUfoImageIO::setPixmap(const QPixmap& p)
{
 if (mImageIO) {
	boError() << k_funcinfo << "data is already set" << endl;
	return;
 }
 QImage img = p.convertToImage();
 setImage(img);
}

void BoUfoImageIO::setImage(const QImage& _img)
{
 if (mImageIO) {
	boError() << k_funcinfo << "data is already set" << endl;
	return;
 }
 // AB: atm UImage uses a format with y-coordinates flipped
 QImage img = QGLWidget::convertToGLFormat(_img.mirror(false, true));
 mImageIO = new ufo::UImageIO(img.bits(), img.width(), img.height(), 4);
 mImageIO->reference();
}


BoUfoImage::BoUfoImage()
{
 init();
}

BoUfoImage::BoUfoImage(const QPixmap& p)
{
 init();
 load(p);
}

BoUfoImage::BoUfoImage(const QImage& img)
{
 init();
 load(img);
}

BoUfoImage::BoUfoImage(const BoUfoImage& img)
{
 init();
 load(img);
}

BoUfoImage::~BoUfoImage()
{
 if (mImage) {
	mImage->unreference();
 }
}

void BoUfoImage::load(const QPixmap& p)
{
 if (p.isNull()) {
	return;
 }
 BoUfoImageIO io(p);
 set(&io);
}

void BoUfoImage::load(const QImage& img)
{
 if (img.isNull()) {
	return;
 }
 BoUfoImageIO io(img);
 set(&io);
}

void BoUfoImage::load(const BoUfoImage& img)
{
 if (!img.image()) {
	return;
 }
 load(img);
 set(img.image());
}

void BoUfoImage::init()
{
 mImage = 0;
}

void BoUfoImage::set(BoUfoImageIO* io)
{
 BO_CHECK_NULL_RET(io);
 BO_CHECK_NULL_RET(io->imageIO());
 if (mImage) {
	mImage->unreference();
	mImage = 0;
 }
 ufo::UImage* image = new ufo::UGL_Image(io->imageIO());
 set(image);
}

void BoUfoImage::set(ufo::UImage* img)
{
 BO_CHECK_NULL_RET(img);
 if (mImage) {
	boError() << k_funcinfo << "image not NULL" << endl;
	return;
 }
 mImage = img;
 mImage->reference();
}

class UCustomWidgetRenderer : public ufo::UWidget
{
	UFO_DECLARE_DYNAMIC_CLASS(UCustomWidgetRenderer)
public:
	UCustomWidgetRenderer(BoUfoWidget* widget)
		: ufo::UWidget()
	{
		mWidget = widget;
	}

	virtual void paint(ufo::UGraphics* g)
	{
		Q_UNUSED(g);
		mWidget->paint();
	}
	virtual void paintWidget(ufo::UGraphics* g)
	{
		Q_UNUSED(g);
		mWidget->paintWidget();
	}
	virtual void paintBorder(ufo::UGraphics* g)
	{
		Q_UNUSED(g);
		mWidget->paintBorder();
	}
#if 0
	virtual ufo::UDimension getPreferredSize()
	{
		QSize s = mWidget->preferredSize();
		return ufo::UDimension(s.width(), s.height());
	}
	virtual ufo::UDimension getPreferredSize(const ufo::UDimension& maxSize)
	{
		QSize s = mRenderer->preferredSize(QSize(maxSize.w, maxSize.h));
		return ufo::UDimension(s.width(), s.height());
	}
	virtual ufo::UDimension getMinimumSize()
	{
		QSize s = mRenderer->minimumSize();
		return ufo::UDimension(s.width(), s.height());
	}
	virtual ufo::UDimension getMaximumSize()
	{
		QSize s = mRenderer->maximumSize();
		return ufo::UDimension(s.width(), s.height());
	}
#endif


	void ufoPaint(ufo::UGraphics* g)
	{
		ufo::UWidget::paint(g);
	}
	void ufoPaintWidget(ufo::UGraphics* g)
	{
		ufo::UWidget::paintWidget(g);
	}
	void ufoPaintBorder(ufo::UGraphics* g)
	{
		ufo::UWidget::paintBorder(g);
	}
	ufo::UDimension getUfoPreferredSize()
	{
		return ufo::UWidget::getPreferredSize();
	}
	ufo::UDimension getUfoPreferredSize(const ufo::UDimension& maxSize)
	{
		return ufo::UWidget::getPreferredSize(maxSize);
	}
	ufo::UDimension getUfoMinimumSize()
	{
		return ufo::UWidget::getMinimumSize();
	}
	ufo::UDimension getUfoMaximumSize()
	{
		return ufo::UWidget::getMaximumSize();
	}

private:
	BoUfoWidget* mWidget;
};

UFO_IMPLEMENT_DYNAMIC_CLASS(UCustomWidgetRenderer, UWidget)

BoUfoCustomWidget::BoUfoCustomWidget()
	: BoUfoWidget(new UCustomWidgetRenderer(this))
{
 setLayoutClass(UVBoxLayout);
}

BoUfoCustomWidget::~BoUfoCustomWidget()
{
}

void BoUfoCustomWidget::paint()
{
 BO_CHECK_NULL_RET(widget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)widget())->ufoPaint(g);
}

void BoUfoCustomWidget::paintWidget()
{
 BO_CHECK_NULL_RET(widget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)widget())->ufoPaintWidget(g);
}

void BoUfoCustomWidget::paintBorder()
{
 BO_CHECK_NULL_RET(widget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)widget())->ufoPaintBorder(g);
}


BoUfoManager::BoUfoManager(int w, int h, bool opaque)
	: QObject(0, "ufomanager")
{
 boDebug() << k_funcinfo << w << "x" << h << endl;
 if (!ufo::UToolkit::getToolkit()) {
	ufo::UXToolkit* tk = new ufo::UXToolkit();
	QString data_dir;
	if (KGlobal::_instance) { // NULL in boufodesigner
		data_dir = KGlobal::dirs()->findResourceDir("data", "boson/pics/boson-startup-logo.png");
	}
	if (data_dir.isEmpty()) {
		boWarning() << k_funcinfo << "cannot determine data_dir" << endl;
//		tk->putProperty("data_dir", "/usr/local/share/ufo");
//		tk->putProperty("font_dir", "/usr/local/share/ufo/font");
	} else {
		data_dir += "boson/ufo";
		tk->putProperty("data_dir", data_dir.latin1());

		QString font_dir = data_dir + "/font";
		tk->putProperty("font_dir", font_dir.latin1());
	}

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


 } else {
	// TODO: make sure that it is a UXToolkit
	boDebug() << k_funcinfo << "have already a toolkit" << endl;
 }
 mDisplay = new ufo::UXDisplay("dummy");

 ufo::URectangle deviceRect(0, 0, w, h);
 ufo::URectangle contextRect(0, 0, w, h);
 mContext = new ufo::UXContext(deviceRect, contextRect);
 if (!mContext->getUIManager()) {
	BO_NULL_ERROR(mContext->getUIManager());
 } else {
	mContext->getUIManager()->setUI("ULabelUI", (ufo::UI_HANDLER)&ufo::UBoLabelUI::createUI);
	mContext->getUIManager()->setUI("UBoProgressUI", (ufo::UI_HANDLER)&ufo::UBoProgressUI::createUI);
 }

 mRootPane = mContext->getRootPane();
 mRootPaneWidget = 0,
 mLayeredPaneWidget = 0;
 mContentPane = 0;
 mContentWidget = 0;
 mMenuBar = 0;
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

 if (rootPane() && contentWidget()) {
	rootPane()->setOpaque(opaque);
	contentWidget()->setOpaque(opaque);
 }
}

BoUfoManager::~BoUfoManager()
{
 boDebug() << k_funcinfo << endl;
 delete mActionCollection;
 setMenuBar(0);
 if (contentWidget()) {
	boDebug() << k_funcinfo << "removing all widgets" << endl;
	contentWidget()->removeAllWidgets();
	boDebug() << k_funcinfo << "removed all widgets" << endl;
 }
 boDebug() << k_funcinfo << "deleting context" << endl;
 delete mContext;
 boDebug() << k_funcinfo << "deleting display" << endl;
 delete mDisplay;

 // TODO: do we have to delete the toolkit ?
 // -> there is only a single static instance of it, so that might be tricky.
 // does libufo take care of that?

 boDebug() << k_funcinfo << "done" << endl;
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

void BoUfoManager::setMenuBar(BoUfoMenuBar* m)
{
 // warning: BoUfoMenuBar is NOT a BoUfoWidget! therefore it is NOT
 // deleted when the UMenuBar is deleted!
 BoUfoMenuBar* del = mMenuBar;
 mMenuBar = 0; // ~BoUfoMenuBar calls setMenuBar() which deletes mMenuBar
 delete del;
 if (mRootPane && mRootPane->getMenuBar()) {
	mRootPane->setMenuBar(0); // deletes the old menubar
 }
 mMenuBar = m;
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

void BoUfoManager::render()
{
 BO_CHECK_NULL_RET(context());
 context()->pushAttributes();
#if 0
 context()->repaint();
#else
 context()->getGraphics()->resetDeviceAttributes();
 context()->getGraphics()->resetDeviceViewMatrix();

 // AB: ufo currently (0.7.4) enables blending by default. however we don't use
 // it currently!
 // (and additionally we never should when we are in software rendering)
 glDisable(GL_BLEND);

 context()->getRepaintManager()->clearDirtyRegions();
 context()->getRootPane()->paint(context()->getGraphics());

 // AB: used by UXContext::repaint(), but we don't need it anyway
 // (just does a glFlush() call)
// context()->getGraphics()->flush();
#endif
 context()->popAttributes();

// TODO glViewport(0, 0, w, h);
// -> libufo sets a different viewport, but doesn't reset to the original one
}

bool BoUfoManager::sendEvent(QEvent* e)
{
 switch (e->type()) {
	case QEvent::Wheel:
		return sendWheelEvent((QWheelEvent*)e);
	case QEvent::MouseMove:
		return sendMouseMoveEvent((QMouseEvent*)e);
	case QEvent::MouseButtonPress:
		return sendMousePressEvent((QMouseEvent*)e);
	case QEvent::MouseButtonRelease:
		return sendMouseReleaseEvent((QMouseEvent*)e);
	case QEvent::KeyPress:
		return sendKeyPressEvent((QKeyEvent*)e);
	case QEvent::KeyRelease:
		return sendKeyReleaseEvent((QKeyEvent*)e);
	default:
		break;
 }
 return false;
}

bool BoUfoManager::sendResizeEvent(int w, int h)
{
 if (!context()) {
	BO_NULL_ERROR(conext());
	return false;
 }
 boDebug() << k_funcinfo << w << "x" << h << endl;
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


class UMyDrawable : public ufo::UDrawable
{
public:
	UMyDrawable(BoUfoDrawable* drawable)
	{
		mDrawable = drawable;
	}

	virtual void paintDrawable(ufo::UGraphics*, int x, int y, int w, int h)
	{
		mDrawable->render(x, y, w, h);
	}
	virtual int getDrawableWidth() const
	{
		return mDrawable->drawableWidth();
	}
	virtual int getDrawableHeight() const
	{
		return mDrawable->drawableHeight();
	}

private:
	BoUfoDrawable* mDrawable;
};

BoUfoDrawable::BoUfoDrawable()
{
 mDrawable = new UMyDrawable(this);
}

BoUfoDrawable::~BoUfoDrawable()
{
 delete mDrawable;
}


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
 w->setClipping(false);

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
	case ufo::LineBorder:
		t = LineBorder;
		break;
	case ufo::RaisedBevelBorder:
		t = RaisedBevelBorder;
		break;
	case ufo::LoweredBevelBorder:
		t = LoweredBevelBorder;
		break;
	case ufo::TitledBorder:
		t = TitledBorder;
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
	case ufo::LineBorder:
		widget()->setBorder(ufo::LineBorder);
		break;
	case ufo::RaisedBevelBorder:
		widget()->setBorder(ufo::RaisedBevelBorder);
		break;
	case ufo::LoweredBevelBorder:
		widget()->setBorder(ufo::LoweredBevelBorder);
		break;
	case ufo::TitledBorder:
		widget()->setBorder(ufo::TitledBorder);
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
		setLayout(new ufo::UBoxLayout(ufo::UBoxLayout::XAxis));
		break;
	case UVBoxLayout:
		setLayout(new ufo::UBoxLayout(ufo::UBoxLayout::YAxis));
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
	setSize(drawable->getDrawableWidth(), drawable->getDrawableHeight());
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

void BoUfoWidget::uslotWidgetAdded(ufo::UWidget*)
{
 emit signalWidgetAdded();
}

void BoUfoWidget::uslotWidgetRemoved(ufo::UWidget*)
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


BoUfoSlider::BoUfoSlider(Qt::Orientation o) : BoUfoWidget()
{
 init(o);
}

void BoUfoSlider::init(Qt::Orientation o)
{
 mMin = 0.0f;
 mMax = 100.0f;
 mStep = 1.0f;
 setLayoutClass(UHBoxLayout);

 if (o == Horizontal) {
	mSlider = new ufo::USlider(ufo::Horizontal);
 } else {
	mSlider = new ufo::USlider(ufo::Vertical);
 }
 widget()->add(mSlider);

 CONNECT_UFO_TO_QT(BoUfoSlider, mSlider, ValueChanged);

 setRange(0, 100);
 setValue(50);
}

void BoUfoSlider::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mSlider->setOpaque(o);
}

void BoUfoSlider::uslotValueChanged(ufo::USlider*, int)
{
 emit signalValueChanged(value());
 emit signalFloatValueChanged(floatValue());
}

void BoUfoSlider::setValue(int v)
{
 mSlider->setValue(v);
}

void BoUfoSlider::setFloatValue(float v)
{
 mSlider->setValue((int)(v / mStep));
}

void BoUfoSlider::setRange(int min, int max)
{
 setFloatRange(min, max, 1.0f);
}

void BoUfoSlider::setFloatRange(float min, float max, float step)
{
 if (max < min) {
	boError() << k_funcinfo << "max < min" << endl;
	max = min;
 }
 mMin = min;
 mMax = max;
 mStep = step;

 mSlider->setMinimum((int)(mMin/mStep));
 mSlider->setMaximum((int)(mMax/mStep));
}

int BoUfoSlider::value() const
{
 return mSlider->getValue();
}

float BoUfoSlider::floatValue() const
{
 return mStep * ((float)value());
}

BoUfoProgress::BoUfoProgress(Qt::Orientation o) : BoUfoWidget()
{
 init(o);
}

void BoUfoProgress::init(Qt::Orientation o)
{
 setLayoutClass(UHBoxLayout);

 mProgress = new ufo::UBoProgress();
 mProgress->updateUI();
 setOrientation(o);
 widget()->add(mProgress);
}

void BoUfoProgress::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mProgress->setOpaque(o);
}

void BoUfoProgress::setOrientation(Orientation o)
{
 if (o == Horizontal) {
	mProgress->setOrientation(ufo::Horizontal);
 } else {
	mProgress->setOrientation(ufo::Vertical);
 }
}

double BoUfoProgress::value() const
{
 return mProgress->getValue();
}

double BoUfoProgress::minimumValue() const
{
 return mProgress->getMinimumValue();
}

double BoUfoProgress::maximumValue() const
{
 return mProgress->getMaximumValue();
}

void BoUfoProgress::setValue(double v)
{
 mProgress->setValue(v);
}

void BoUfoProgress::setRange(double min, double max)
{
 mProgress->setMinimumValue(min);
 mProgress->setMaximumValue(max);
}

void BoUfoProgress::setMinimumValue(double min)
{
 double max = QMAX(min, maximumValue());
 setRange(min, max);
}

void BoUfoProgress::setMaximumValue(double max)
{
 double min = QMIN(max, minimumValue());
 setRange(min, max);
}

void BoUfoProgress::setStartColor(const QColor& c)
{
 mProgress->setStartColor(ufo::UColor(c.red(), c.green(), c.blue()));
}

QColor BoUfoProgress::startColor() const
{
 ufo::UColor c = mProgress->startColor();
 return QColor((int)(c.getRed() * 255), (int)(c.getGreen() * 255), (int)(c.getBlue() * 255));
}

void BoUfoProgress::setEndColor(const QColor& c)
{
 mProgress->setEndColor(ufo::UColor(c.red(), c.green(), c.blue()));
}

QColor BoUfoProgress::endColor() const
{
 ufo::UColor c = mProgress->endColor();
 return QColor((int)(c.getRed() * 255), (int)(c.getGreen() * 255), (int)(c.getBlue() * 255));
}

void BoUfoProgress::setColor(const QColor& c)
{
 mProgress->setColor(ufo::UColor(c.red(), c.green(), c.blue()));
}

BoUfoNumInput::BoUfoNumInput() : BoUfoWidget()
{
 init();
}

void BoUfoNumInput::init()
{
 setLayoutClass(UHBoxLayout);

 mLabel = new BoUfoLabel();

 mSlider = new BoUfoSlider();
 connect(mSlider, SIGNAL(signalFloatValueChanged(float)),
		this, SLOT(slotSliderChanged(float)));

 // TODO: allow only float inputs. there is a
 // UDocumentFactory::createDigitDocument(), but that one allows digits only,
 // i.e. only integers
 mLineEdit = new BoUfoLineEdit();
 // TODO: do not only accept input when return was pressed, but also after a
 // short timeout!
 connect(mLineEdit, SIGNAL(signalActivated(const QString&)),
		this, SLOT(slotTextEntered(const QString&)));

 // TODO: a spinbox would be nice (i.e. up/down arrows next to the lineedit)

 addWidget(mLabel);
 addWidget(mSlider);
 addWidget(mLineEdit);

 setRange(0.0f, 100.0f);
 setValue(50.0f);
}

void BoUfoNumInput::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mLabel->setOpaque(o);
 mSlider->setOpaque(o);
 mLineEdit->setOpaque(o);
}

void BoUfoNumInput::slotSliderChanged(float v)
{
 static bool recursive = false;
 if (recursive) {
	return;
 }
 recursive = true;
 setValue(v);
 recursive = false;

 emit signalValueChanged(v);
}

void BoUfoNumInput::slotTextEntered(const QString& text)
{
 bool ok;
 float v = text.toFloat(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "invalid text entered (not a float)" << endl;
	v = value();
 }
 setValue(v);

 emit signalValueChanged(v);
}

float BoUfoNumInput::value() const
{
 return mSlider->floatValue();
}

float BoUfoNumInput::minimumValue() const
{
 return mSlider->minimumValue();
}

float BoUfoNumInput::maximumValue() const
{
 return mSlider->maximumValue();
}

float BoUfoNumInput::stepSize() const
{
 return mSlider->stepSize();
}

void BoUfoNumInput::setLabel(const QString& label, int a)
{
 mLabel->setText(label);

#warning TODO: a
 // alignment should be pretty easy using a BorderLayout
}

QString BoUfoNumInput::label() const
{
 return mLabel->text();
}

void BoUfoNumInput::setValue(float v)
{
 mSlider->setFloatValue(v);
 mLineEdit->setText(QString::number(value()));
}

void BoUfoNumInput::slotSetMaxValue(float max)
{
 double min = QMIN(max, minimumValue());
 setRange(min, max);
}

void BoUfoNumInput::slotSetMinValue(float min)
{
 double max = QMAX(min, maximumValue());
 setRange(min, max);
}

void BoUfoNumInput::setStepSize(float s)
{
 mSlider->setFloatRange(mSlider->minimumValue(), mSlider->maximumValue(), s);
}

void BoUfoNumInput::setRange(float min, float max)
{
 if (min > max) {
	boError() << k_funcinfo << "min > max" << endl;
	min = max;
 }
 mSlider->setFloatRange(min, max, mSlider->stepSize());

 if (value() < minimumValue()) {
	setValue(minimumValue());
 } else if (value() > maximumValue()) {
	setValue(maximumValue());
 }
}


BoUfoPushButton::BoUfoPushButton()
	: BoUfoWidget(new ufo::UButton())
{
 init();
}

BoUfoPushButton::BoUfoPushButton(const QString& text)
	: BoUfoWidget(new ufo::UButton())
{
 init();
 setText(text);
}

void BoUfoPushButton::init()
{
 mButton = (ufo::UButton*)widget();

 CONNECT_UFO_TO_QT(BoUfoPushButton, mButton, Activated);
 CONNECT_UFO_TO_QT(BoUfoPushButton, mButton, Highlighted);
}

void BoUfoPushButton::setVerticalAlignment(VerticalAlignment a)
{
 BoUfoWidget::setVerticalAlignment(a);
 mButton->setVerticalAlignment(widget()->getVerticalAlignment());
}

void BoUfoPushButton::setHorizontalAlignment(HorizontalAlignment a)
{
 BoUfoWidget::setHorizontalAlignment(a);
 mButton->setHorizontalAlignment(widget()->getHorizontalAlignment());
}

void BoUfoPushButton::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mButton->setOpaque(o);
}

void BoUfoPushButton::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalClicked();
}

void BoUfoPushButton::uslotHighlighted(ufo::UActionEvent*)
{
 emit signalHighlighted();
}

void BoUfoPushButton::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mButton->setMinimumSize(s);
}

void BoUfoPushButton::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mButton->setPreferredSize(s);
}

void BoUfoPushButton::setText(const QString& text)
{
 if (text.isNull()) {
	mButton->setText("");
 } else {
	mButton->setText(text.latin1());
 }
}

QString BoUfoPushButton::text() const
{
 QString text = mButton->getText().c_str();
 return text;
}

void BoUfoPushButton::setIcon(const BoUfoImage& img)
{
 if (!img.image()) {
	mButton->setIcon(0);
 } else {
	mButton->setIcon(new ufo::UImageIcon(img.image()));
 }
}

void BoUfoPushButton::setIconFile(const QString& file_)
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
	}
	if (!img.load(file)) {
		boError() << k_funcinfo << file << " could not be loaded" << endl;
		return;
	}
	setIcon(img);
 } else {
	setIcon(BoUfoImage());
 }
 mIconFile = file;
}

QString BoUfoPushButton::iconFile() const
{
 return mIconFile;
}

void BoUfoPushButton::setToggleButton(bool t)
{
 mButton->setToggable(t);
}

bool BoUfoPushButton::isToggleButton() const
{
 return mButton->isToggable();
}

void BoUfoPushButton::setOn(bool on)
{
 mButton->setSelected(on);
}

bool BoUfoPushButton::isOn() const
{
 return mButton->isSelected();
}


BoUfoLineEdit::BoUfoLineEdit() : BoUfoWidget()
{
 init();
}

void BoUfoLineEdit::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mLineEdit->setMinimumSize(s);
}

void BoUfoLineEdit::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mLineEdit->setPreferredSize(s);
}

void BoUfoLineEdit::init()
{
 setLayoutClass(UHBoxLayout);
 mLineEdit = new ufo::ULineEdit();
 widget()->add(mLineEdit);

 CONNECT_UFO_TO_QT(BoUfoLineEdit, mLineEdit, Activated);
}

void BoUfoLineEdit::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mLineEdit->setOpaque(o);
}

void BoUfoLineEdit::setEditable(bool e)
{
 mLineEdit->setEditable(e);
}

bool BoUfoLineEdit::isEditable() const
{
 return mLineEdit->isEditable();
}

void BoUfoLineEdit::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalActivated(text());
}

void BoUfoLineEdit::setText(const QString& text)
{
 if (text.isNull()) {
	mLineEdit->setText("");
 } else {
	mLineEdit->setText(text.latin1());
 }
}

QString BoUfoLineEdit::text() const
{
 QString text = mLineEdit->getText().c_str();
 return text;
}

BoUfoTextEdit::BoUfoTextEdit() : BoUfoWidget()
{
 init();
}

void BoUfoTextEdit::init()
{
 setLayoutClass(UHBoxLayout);
 mTextEdit = new ufo::UTextEdit();
 widget()->add(mTextEdit);
}

void BoUfoTextEdit::setEditable(bool e)
{
 mTextEdit->setEditable(e);
}

bool BoUfoTextEdit::isEditable() const
{
 return mTextEdit->isEditable();
}

void BoUfoTextEdit::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mTextEdit->setOpaque(o);
}

void BoUfoTextEdit::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mTextEdit->setMinimumSize(s);
}

void BoUfoTextEdit::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mTextEdit->setPreferredSize(s);
}

void BoUfoTextEdit::setText(const QString& text)
{
 if (text.isNull()) {
	mTextEdit->setText("");
 } else {
	mTextEdit->setText(text.latin1());
 }
}

QString BoUfoTextEdit::text() const
{
 QString text = mTextEdit->getText().c_str();
 return text;
}

BoUfoComboBox::BoUfoComboBox() : BoUfoWidget()
{
 init();
}

void BoUfoComboBox::init()
{
 setLayoutClass(UHBoxLayout);
 mComboBox = new ufo::UComboBox();
 widget()->add(mComboBox);

 CONNECT_UFO_TO_QT(BoUfoComboBox, mComboBox, Activated);
 CONNECT_UFO_TO_QT(BoUfoComboBox, mComboBox, Highlighted);
 CONNECT_UFO_TO_QT(BoUfoComboBox, mComboBox, SelectionChanged);
}

void BoUfoComboBox::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mComboBox->setOpaque(o);
}

void BoUfoComboBox::uslotActivated(ufo::UComboBox*, int i)
{
 emit signalActivated(i);
}

void BoUfoComboBox::uslotHighlighted(ufo::UComboBox*, int i)
{
 emit signalHighlighted(i);
}

void BoUfoComboBox::uslotSelectionChanged(ufo::UComboBox*, int i1, int i2)
{
 emit signalSelectionChanged(i1, i2);
}

void BoUfoComboBox::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mComboBox->setMinimumSize(s);
}

void BoUfoComboBox::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mComboBox->setPreferredSize(s);
}

void BoUfoComboBox::clear()
{
 mComboBox->removeAllItems();
}

int BoUfoComboBox::currentItem() const
{
 return mComboBox->getCurrentItem();
}

void BoUfoComboBox::setCurrentItem(int i)
{
 if (i < 0) {
	return;
 }
 if ((unsigned int)i >= count()) {
	return;
 }
 mComboBox->setCurrentItem(i);
}

QString BoUfoComboBox::currentText() const
{
 return QString::fromLatin1(mComboBox->getCurrentText().c_str());
}

QStringList BoUfoComboBox::items() const
{
 QStringList list;
 std::vector<ufo::UItem*> items = mComboBox->getItems();
 for (unsigned int i = 0; i < items.size(); i++) {
	list.append(items[i]->itemToString().c_str());
 }
 return list;
}

void BoUfoComboBox::setItems(const QStringList& items)
{
 clear();
 for (QStringList::const_iterator it = items.begin(); it != items.end(); ++it) {
	mComboBox->addItem((*it).latin1());
 }
}

unsigned int BoUfoComboBox::count() const
{
 return mComboBox->getItemCount();
}

QString BoUfoComboBox::itemText(int i) const
{
 // TODO: implement properly. this is only a trivail and slow implementation.
 if (i < 0 || (unsigned int)i >= count()) {
	return QString::null;
 }
 return items()[i];
}

void BoUfoComboBox::setItemText(int i, const QString& text)
{
 // TODO: implement properly. this is only a trivail and slow implementation.
 if (i < 0 || (unsigned int)i >= count()) {
	return;
 }
 QStringList list = items();
 list[i] = text;
 setItems(list);
}

void BoUfoComboBox::insertItem(const QString& text, int index)
{
 // TODO: implement properly. this is only a trivail and slow implementation.
 QStringList list = items();
 if (index < 0) {
	index = list.count();
 }
 list.insert(list.at(index), text);
 setItems(list);
}

void BoUfoComboBox::removeItem(int index)
{
 // TODO: implement properly. this is only a trivail and slow implementation.
 QStringList list = items();
 list.remove(list.at(index));
 setItems(list);
}


BoUfoListBox::BoUfoListBox() : BoUfoWidget()
{
 init();
}

void BoUfoListBox::init()
{
 setLayoutClass(UHBoxLayout);
 mListBox = new ufo::UListBox();
 widget()->add(mListBox);

 CONNECT_UFO_TO_QT(BoUfoListBox, mListBox, SelectionChanged);
}

void BoUfoListBox::setSelectionMode(BoUfoListBox::SelectionMode mode)
{
 ufo::UListBox::SelectionMode ufoMode;
 switch (mode) {
	default:
	case NoSelection:
		ufoMode = ufo::UListBox::NoSelection;
		break;
	case SingleSelection:
		ufoMode = ufo::UListBox::SingleSelection;
		break;
	case MultiSelection:
		ufoMode = ufo::UListBox::MultipleSelection;
		break;
 }
 mListBox->setSelectionMode(ufoMode);
}

BoUfoListBox::SelectionMode BoUfoListBox::selectionMode() const
{
 switch (mListBox->getSelectionMode()) {
	case ufo::UListBox::NoSelection:
		 return NoSelection;
	case ufo::UListBox::SingleSelection:
		 return SingleSelection;
	case ufo::UListBox::MultipleSelection:
		return MultiSelection;
 }
 return NoSelection;
}

void BoUfoListBox::clear()
{
 mListBox->removeAllItems();
}

int BoUfoListBox::selectedItem() const
{
 return mListBox->getSelectedIndex();
}

void BoUfoListBox::setSelectedItem(int i)
{
 mListBox->setSelectedIndex(i);
}

bool BoUfoListBox::isSelected(int index) const
{
 return mListBox->isSelectedIndex(index);
}

bool BoUfoListBox::isTextSelected(const QString& text) const
{
 QStringList items = selectedItemsText();
 if (items.contains(text)) {
	return true;
 }
 return false;
}

void BoUfoListBox::unselectAll()
{
 mListBox->setSelectedIndices(std::vector<unsigned int>());
}

void BoUfoListBox::setItemSelected(int index, bool select)
{
 switch (selectionMode()) {
	default:
	case NoSelection:
		break;
	case SingleSelection:
		if (select) {
			if (selectedItem() == index) {
				return;
			}
			unselectAll();
			mListBox->setSelectedIndex(index);
			return;
		} else {
			// FIXME: use mListBox->setSelectedIndex(index, false);
			// once such a thing exists!
			unselectAll();
		}
		break;
	case MultiSelection:
		// TODO: implement. ufo doesn't support multi selections yet.
		if (select) {
			mListBox->setSelectedIndex(index);
		} else {
			unselectAll();
		}
		break;
 }
}

QString BoUfoListBox::selectedText() const
{
 int i = selectedItem();
 if (i < 0) {
	return QString::null;
 }
 return items()[i];
}

QValueList<unsigned int> BoUfoListBox::selectedItems() const
{
 QValueList<unsigned int> ret;
 std::vector<unsigned int> sel = mListBox->getSelectedIndices();
 for (unsigned int i = 0; i < sel.size(); i++) {
	ret.append(sel[i]);
 }
 return ret;
}

QStringList BoUfoListBox::selectedItemsText() const
{
 // note: this may be pretty slow for large lists, it is just a trivial
 // implementation!
 // this doesnt matter since we don't have large lists in boson currently
 QStringList ret;
 QStringList allItems = items();
 QValueList<unsigned int> sel = selectedItems();
 for (QValueList<unsigned int>::iterator it = sel.begin(); it != sel.end(); ++it) {
	ret.append(allItems[*it]);
 }
 return ret;
}

QStringList BoUfoListBox::items() const
{
 QStringList list;
 std::vector<ufo::UItem*> items = mListBox->getItems();
 for (unsigned int i = 0; i < items.size(); i++) {
	list.append(items[i]->itemToString().c_str());
 }
 return list;
}

void BoUfoListBox::setItems(const QStringList& items)
{
 clear();
 for (QStringList::const_iterator it = items.begin(); it != items.end(); ++it) {
	mListBox->addItem((*it).latin1());
 }
}

unsigned int BoUfoListBox::count() const
{
 return mListBox->getItemCount();
}

QString BoUfoListBox::itemText(int i) const
{
 // TODO: implement properly. this is only a trivial and slow implementation.
 if (i < 0 || (unsigned int)i >= count()) {
	return QString::null;
 }
 return items()[i];
}

void BoUfoListBox::setItemText(int i, const QString& text)
{
 // TODO: implement properly. this is only a trivial and slow implementation.
 if (i < 0 || (unsigned int)i >= count()) {
	return;
 }
 QStringList list = items();
 list[i] = text;
 setItems(list);
}

void BoUfoListBox::insertItem(const QString& text, int index)
{
 // TODO: implement properly. this is only a trivial and slow implementation.
 QStringList list = items();
 if (index < 0) {
	index = list.count();
 }
 list.insert(list.at(index), text);
 setItems(list);
}

void BoUfoListBox::removeItem(int index)
{
 if (index < 0) {
	boError() << k_funcinfo << "index must be >= 0" << endl;
	return;
 }
 // TODO: implement properly. this is only a trivial and slow implementation.
 QStringList list = items();
 if (index >= (int)list.count()) {
	return;
 }
 list.remove(list.at(index));
 setItems(list);
}

void BoUfoListBox::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mListBox->setOpaque(o);
}

void BoUfoListBox::uslotSelectionChanged(ufo::UListBox*, int first, int last)
{
 emit signalSelectionChanged(first, last);
}

void BoUfoListBox::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mListBox->setMinimumSize(s);
}

void BoUfoListBox::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mListBox->setPreferredSize(s);
}




BoUfoLabel::BoUfoLabel() : BoUfoWidget()
{
 init();
}

BoUfoLabel::BoUfoLabel(const QString& text) : BoUfoWidget()
{
 init();
 setText(text);
}

void BoUfoLabel::init()
{
 setLayoutClass(UHBoxLayout);
 mLabel = new ufo::ULabel();
 widget()->add(mLabel);
 mLabel->setOpaque(false);
 setForegroundColor(defaultForegroundColor());
}

void BoUfoLabel::setVerticalAlignment(VerticalAlignment a)
{
 BoUfoWidget::setVerticalAlignment(a);
 mLabel->setVerticalAlignment(widget()->getVerticalAlignment());
}

void BoUfoLabel::setHorizontalAlignment(HorizontalAlignment a)
{
 BoUfoWidget::setHorizontalAlignment(a);
 mLabel->setHorizontalAlignment(widget()->getHorizontalAlignment());
}

void BoUfoLabel::setDefaultForegroundColor(const QColor& c)
{
 mDefaultForegroundColor = c;
}

const QColor& BoUfoLabel::defaultForegroundColor()
{
 return mDefaultForegroundColor;
}

void BoUfoLabel::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mLabel->setOpaque(o);
}

void BoUfoLabel::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mLabel->setMinimumSize(s);
}

void BoUfoLabel::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mLabel->setPreferredSize(s);
}

void BoUfoLabel::setText(const QString& text)
{
 if (text.isNull()) {
	mLabel->setText("");
 } else {
	mLabel->setText(text.latin1());
 }
}

QString BoUfoLabel::text() const
{
 QString text = mLabel->getText().c_str();
 return text;
}

void BoUfoLabel::setIcon(const BoUfoImage& img)
{
 if (!img.image()) {
	mLabel->setIcon(0);
 } else {
	mLabel->setIcon(new ufo::UImageIcon(img.image()));
 }
}

void BoUfoLabel::setIconFile(const QString& file_)
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
	}
	if (!img.load(file)) {
		boError() << k_funcinfo << file << " could not be loaded" << endl;
		return;
	}
	setIcon(img);
 } else {
	setIcon(BoUfoImage());
 }
 mIconFile = file;
}

QString BoUfoLabel::iconFile() const
{
 return mIconFile;
}

void BoUfoLabel::setFont(BoUfoManager* m, const BoUfoFontInfo& info)
{
 ufo::UFont* font = info.ufoFont(m);

#warning FIXME: memory leak
 // this should be done by ufo::UWidget::setFont() and that one should also
 // unreference it.
 // it is necessary because otherwise the application crashes when rendering due
 // to ufo::UGraphics::setFont().
 font->reference();

 mLabel->setFont(font);
}

BoUfoCheckBox::BoUfoCheckBox() : BoUfoWidget()
{
 init();
}

BoUfoCheckBox::BoUfoCheckBox(const QString& text, bool checked) : BoUfoWidget()
{
 init();
 setText(text);
 setChecked(checked);
}

void BoUfoCheckBox::init()
{
 setLayoutClass(UHBoxLayout);
 mCheckBox = new ufo::UCheckBox();
 widget()->add(mCheckBox);
 // AB: at least the background of the label must be transparent. unfortunately,
 // libufo uses UButton for the checkbox, so the actual checkbox and its label
 // are the same widget
 mCheckBox->setOpaque(false);

 CONNECT_UFO_TO_QT(BoUfoCheckBox, mCheckBox, Activated);
 CONNECT_UFO_TO_QT(BoUfoCheckBox, mCheckBox, Highlighted);
}

void BoUfoCheckBox::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mCheckBox->setOpaque(o);
}

void BoUfoCheckBox::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalToggled(checked());
}

void BoUfoCheckBox::uslotHighlighted(ufo::UActionEvent*)
{
 emit signalHighlighted();
}

void BoUfoCheckBox::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mCheckBox->setMinimumSize(s);
}

void BoUfoCheckBox::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mCheckBox->setPreferredSize(s);
}

void BoUfoCheckBox::setText(const QString& text)
{
 if (text.isNull()) {
	mCheckBox->setText("");
 } else {
	mCheckBox->setText(text.latin1());
 }
}

QString BoUfoCheckBox::text() const
{
 QString text = mCheckBox->getText().c_str();
 return text;
}

void BoUfoCheckBox::setChecked(bool c)
{
 mCheckBox->setSelected(c);
}

bool BoUfoCheckBox::checked() const
{
 return mCheckBox->isSelected();
}


BoUfoButtonGroup::BoUfoButtonGroup(QObject* parent)
	: QObject(parent)
{
 mButtonGroup = new ufo::UButtonGroup();
 mButtonGroup->reference();

 mButtons = new QMap<ufo::URadioButton*, BoUfoRadioButton*>();
}

BoUfoButtonGroup::~BoUfoButtonGroup()
{
 mButtonGroup->unreference();
 delete mButtons;
}

void BoUfoButtonGroup::addButton(BoUfoRadioButton* button)
{
 BO_CHECK_NULL_RET(button);
 connect(button, SIGNAL(signalActivated()),
		this, SLOT(slotButtonActivated()));
 ufo::UButton* b = (ufo::UButton*)button->radioButton();
 mButtonGroup->addButton(b);

 mButtons->insert(button->radioButton(), button);
}

void BoUfoButtonGroup::removeButton(BoUfoRadioButton* button)
{
 BO_CHECK_NULL_RET(button);
 ufo::UButton* b = (ufo::UButton*)button->radioButton();
 mButtonGroup->removeButton(b);

 mButtons->remove(button->radioButton());
}

void BoUfoButtonGroup::slotButtonActivated()
{
 emit signalButtonActivated(selectedButton());
}

BoUfoRadioButton* BoUfoButtonGroup::selectedButton() const
{
 ufo::URadioButton* b = dynamic_cast<ufo::URadioButton*>(mButtonGroup->getSelectedButton());
 BoUfoRadioButton* button = 0;
 if (b) {
	button = (*mButtons)[b];
 }
 return button;
}

BoUfoButtonGroupWidget::BoUfoButtonGroupWidget()
	: BoUfoWidget()
{
 mButtonGroup = new BoUfoButtonGroup(this);
 connect(mButtonGroup, SIGNAL(signalButtonActivated(BoUfoRadioButton*)),
		this, SIGNAL(signalButtonActivated(BoUfoRadioButton*)));
}

BoUfoButtonGroupWidget::~BoUfoButtonGroupWidget()
{
}

void BoUfoButtonGroupWidget::addWidget(BoUfoWidget* widget)
{
 BoUfoWidget::addWidget(widget);
 if (widget && widget->inherits("BoUfoRadioButton")) {
	mButtonGroup->addButton((BoUfoRadioButton*)widget);
 }
}

void BoUfoButtonGroupWidget::removeWidget(BoUfoWidget* widget)
{
 BoUfoWidget::removeWidget(widget);
 if (widget && widget->inherits("BoUfoRadioButton")) {
	mButtonGroup->removeButton((BoUfoRadioButton*)widget);
 }
}

BoUfoRadioButton* BoUfoButtonGroupWidget::selectedButton() const
{
 return mButtonGroup->selectedButton();
}

BoUfoRadioButton::BoUfoRadioButton()
	: BoUfoWidget()
{
 init();
}

BoUfoRadioButton::BoUfoRadioButton(const QString& text, bool selected)
	: BoUfoWidget()
{
 init();
 setText(text);
 setSelected(selected);
}

void BoUfoRadioButton::init()
{
 setLayoutClass(UHBoxLayout);
 mRadioButton = new ufo::URadioButton();
 widget()->add(mRadioButton);
 // AB: at least the background of the label must be transparent
 mRadioButton->setOpaque(false);

 CONNECT_UFO_TO_QT(BoUfoRadioButton, mRadioButton, Activated);
 CONNECT_UFO_TO_QT(BoUfoRadioButton, mRadioButton, Highlighted);
}

void BoUfoRadioButton::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mRadioButton->setOpaque(o);
}

void BoUfoRadioButton::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalToggled(selected());
}

void BoUfoRadioButton::uslotHighlighted(ufo::UActionEvent*)
{
 emit signalHighlighted();
}

void BoUfoRadioButton::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mRadioButton->setMinimumSize(s);
}

void BoUfoRadioButton::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mRadioButton->setPreferredSize(s);
}

void BoUfoRadioButton::setText(const QString& text)
{
 if (text.isNull()) {
	mRadioButton->setText("");
 } else {
	mRadioButton->setText(text.latin1());
 }
}

QString BoUfoRadioButton::text() const
{
 QString text = mRadioButton->getText().c_str();
 return text;
}

void BoUfoRadioButton::setSelected(bool s)
{
 mRadioButton->setSelected(s);
}

bool BoUfoRadioButton::selected() const
{
 return mRadioButton->isSelected();
}


BoUfoMatrix::BoUfoMatrix() : BoUfoWidget()
{
 init();
}

BoUfoMatrix::~BoUfoMatrix()
{
 // AB: remember NOT to delete the mMatrix[i] elements. libufo does so. we just
 // need to delete the array containing the pointers.
 delete[] mMatrix;
}

void BoUfoMatrix::init()
{
 setLayoutClass(UVBoxLayout);

 BoUfoHBox* rows[4];
 for (int i = 0; i < 4; i++) {
	rows[i] = new BoUfoHBox();
	addWidget(rows[i]);
 }

 mMatrix = new BoUfoLabel*[16];
 for (int i = 0; i < 16; i++) {
	mMatrix[i] = new BoUfoLabel();
	rows[i % 4]->addWidget(mMatrix[i]);
 }
}

void BoUfoMatrix::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 for (int i = 0; i < 16; i++) {
	mMatrix[i]->setOpaque(o);
 }
}

void BoUfoMatrix::setMatrix(const float* m)
{
 for (int i = 0; i < 16; i++) {
	mMatrix[i]->setText(QString::number(m[i]));
 }
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

class BoUfoTabWidgetPrivate
{
public:
	BoUfoTabWidgetPrivate()
	{
		mButtonsWidget = 0;
		mTabLayoutWidget = 0;
	}
	BoUfoHBox* mButtonsWidget;
	BoUfoVBox* mTabLayoutWidget;

	QIntDict<BoUfoPushButton> mButtons;
	QIntDict<BoUfoWidget> mTabs; // ownership is NOT taken
	int mCurrentTab;
};


BoUfoTabWidget::BoUfoTabWidget() : BoUfoWidget()
{
 init();
}

BoUfoTabWidget::~BoUfoTabWidget()
{
 delete d;
}

void BoUfoTabWidget::init()
{
 d = new BoUfoTabWidgetPrivate;
 BoUfoVBox* topLayoutWidget = new BoUfoVBox();
 addWidget(topLayoutWidget);

 d->mButtonsWidget = new BoUfoHBox();
 d->mTabLayoutWidget = new BoUfoVBox();

 topLayoutWidget->addWidget(d->mButtonsWidget);
 topLayoutWidget->addWidget(d->mTabLayoutWidget);
 d->mCurrentTab = -1;
}

void BoUfoTabWidget::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 for (QIntDictIterator<BoUfoPushButton> it(d->mButtons); it.current(); ++it) {
	it.current()->setOpaque(o);
 }
 for (QIntDictIterator<BoUfoWidget> it(d->mTabs); it.current(); ++it) {
	it.current()->setOpaque(o);
 }
}

int BoUfoTabWidget::addTab(BoUfoWidget* widget, const QString& label)
{
 if (!widget) {
	BO_NULL_ERROR(widget);
	return -1;
 }
 int id = findId();
 if (id < 0) { // will never happen
	boError() << k_funcinfo << "invalid id" << endl;
	return -1;
 }

 BoUfoPushButton* button = new BoUfoPushButton(label);
 d->mButtons.insert(id, button);
 d->mTabs.insert(id, widget);
 connect(button, SIGNAL(signalClicked()), this, SLOT(slotButtonClicked()));

 d->mButtonsWidget->addWidget(button);
 d->mTabLayoutWidget->addWidget(widget);

 widget->hide();
 if (!currentTab()) {
	setCurrentTab(id);
 }
 return id;
}

void BoUfoTabWidget::removeTab(BoUfoWidget* widget)
{
 QIntDictIterator<BoUfoWidget> it(d->mTabs);
 int id = -1;
 while (it.current() && it.current() != widget) {
	++it;
 }
 if (it.current()) {
	id = it.currentKey();
 } else {
	boWarning() << k_funcinfo << "did not find tab" << endl;
	return;
 }

 BoUfoPushButton* b = d->mButtons[id];
 BoUfoWidget* tab = d->mTabs[id];
 d->mTabs.remove(id);
 d->mButtons.remove(id);

#warning TODO
 boWarning() << k_funcinfo << "removing tabs does not work correctly yet" << endl;
 // remove from mButtonsWidget and mTabLayoutWidget
}

void BoUfoTabWidget::slotButtonClicked()
{
 BO_CHECK_NULL_RET(sender());
 if (!sender()->inherits("BoUfoPushButton")) {
	boError() << k_funcinfo << "sender() is not a BoUfoPushButton" << endl;
	return;
 }
 BoUfoPushButton* senderButton = (BoUfoPushButton*)sender();
 QIntDictIterator<BoUfoPushButton> it(d->mButtons);
 while (it.current()) {
	if (it.current() == senderButton) {
		setCurrentTab(it.currentKey());
		return;
	}
	++it;
 }
}

void BoUfoTabWidget::setCurrentTab(int id)
{
 BoUfoWidget* w = currentTab();
 if (w) {
	w->hide();
 }
 d->mCurrentTab = id;
 w = currentTab();
 if (!w) {
	if (d->mTabs.count() > 0) {
#warning FIXME: we use ID, not index!
		setCurrentTab(0);
	}
	return;
 }
 w->show();
 invalidate();
}

BoUfoWidget* BoUfoTabWidget::currentTab() const
{
 return d->mTabs[d->mCurrentTab];
}

int BoUfoTabWidget::findId() const
{
 int i = 0;
 while (d->mTabs[i]) {
	i++;
 }
 return i;
}



BoUfoWidget* BoUfoFactory::createWidget(const QString& className)
{
 if (!widgets().contains(className)) {
	boError() << k_funcinfo << "don't know class " << className << endl;
	return 0;
 }
#define CLASSNAME(name) if (className == #name) { return new name(); }
 CLASSNAME(BoUfoWidget)
 CLASSNAME(BoUfoHBox)
 CLASSNAME(BoUfoVBox)
 CLASSNAME(BoUfoPushButton)
 CLASSNAME(BoUfoCheckBox)
 CLASSNAME(BoUfoRadioButton)
 CLASSNAME(BoUfoButtonGroupWidget)
 CLASSNAME(BoUfoSlider)
 CLASSNAME(BoUfoProgress)
 CLASSNAME(BoUfoNumInput)
 CLASSNAME(BoUfoLabel)
 CLASSNAME(BoUfoLineEdit)
 CLASSNAME(BoUfoTextEdit)
 CLASSNAME(BoUfoComboBox)
 CLASSNAME(BoUfoListBox)
 CLASSNAME(BoUfoMatrix)
 CLASSNAME(BoUfoTabWidget)
 CLASSNAME(BoUfoWidgetStack)
 CLASSNAME(BoUfoLayeredPane)
#undef CLASSNAME
 return 0;
}

QStringList BoUfoFactory::widgets()
{
 QStringList list;
 list.append("BoUfoWidget");
 list.append("BoUfoHBox");
 list.append("BoUfoVBox");
 list.append("BoUfoPushButton");
 list.append("BoUfoCheckBox");
 list.append("BoUfoRadioButton");
 list.append("BoUfoButtonGroupWidget");
 list.append("BoUfoSlider");
 list.append("BoUfoProgress");
 list.append("BoUfoNumInput");
 list.append("BoUfoLabel");
 list.append("BoUfoLineEdit");
 list.append("BoUfoTextEdit");
 list.append("BoUfoComboBox");
 list.append("BoUfoListBox");
 list.append("BoUfoMatrix");
 list.append("BoUfoTabWidget");
 list.append("BoUfoWidgetStack");
 list.append("BoUfoLayeredPane");
 return list;
}



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

BoUfoLayeredPane::BoUfoLayeredPane()
	: BoUfoWidget(new ufo::ULayeredPane())
{
 setLayoutClass(BoUfoWidget::UFullLayout);
}

BoUfoLayeredPane::BoUfoLayeredPane(ufo::ULayeredPane* p, bool provideLayout)
	: BoUfoWidget(p)
{
 if (provideLayout) {
	setLayoutClass(BoUfoWidget::UFullLayout);
 }
}

BoUfoLayeredPane::~BoUfoLayeredPane()
{
}

void BoUfoLayeredPane::addLayer(BoUfoWidget* w, int layer, int pos)
{
 BO_CHECK_NULL_RET(w);
 BO_CHECK_NULL_RET(widget());
 addWidget(w);
 setLayer(w, layer, pos);
}

void BoUfoLayeredPane::setLayer(BoUfoWidget* w, int layer, int pos)
{
 BO_CHECK_NULL_RET(w);
 BO_CHECK_NULL_RET(widget());
 if (!dynamic_cast<ufo::ULayeredPane*>(widget())) {
	boError() << k_funcinfo << "oops - not a ULayeredPane" << endl;
	return;
 }
 dynamic_cast<ufo::ULayeredPane*>(widget())->setLayer(w->widget(), layer, pos);
}


BoUfoInternalFrame::BoUfoInternalFrame(BoUfoManager* manager, const QString& title)
	: BoUfoWidget(new ufo::UInternalFrame())
{
 init();
 setTitle(title);

 manager->addFrame(this);
}

BoUfoInternalFrame::~BoUfoInternalFrame()
{
}

void BoUfoInternalFrame::init()
{
// TODO: provide a method setCentered() or so, that relocates the frame to the
// center of the screen. that's important for dialogs.
 setBounds(0, 0, 100, 100);

 mRootPane = frame()->getRootPane();
 mContentPane = frame()->getContentPane();
 mContentWidget = new BoUfoWidget(mContentPane);
 mContentWidget->setLayoutClass(BoUfoWidget::UVBoxLayout);
 if (rootPane() && contentWidget()) {
//	rootPane()->setOpaque(opaque);
//	contentWidget()->setOpaque(opaque);
 }
}

void BoUfoInternalFrame::setBounds(int x, int y, int w, int h)
{
 frame()->setBounds(x, y, w, h);
}

void BoUfoInternalFrame::setTitle(const QString& t)
{
 frame()->setTitle(t.latin1());
}

QString BoUfoInternalFrame::title() const
{
 return QString(frame()->getTitle().c_str());
}

void BoUfoInternalFrame::setResizable(bool r)
{
 frame()->setResizable(r);
}

bool BoUfoInternalFrame::resizable() const
{
 return frame()->isResizable();
}

void BoUfoInternalFrame::setMaximizable(bool m)
{
 frame()->setMaximizable(m);
}

bool BoUfoInternalFrame::maximizable() const
{
 return frame()->isMaximizable();
}

void BoUfoInternalFrame::setMinimizable(bool m)
{
 frame()->setMinimizable(m);
}

bool BoUfoInternalFrame::minimizable() const
{
 return frame()->isMinimizable();
}

void BoUfoInternalFrame::adjustSize()
{
 frame()->pack();
}


#warning FIXME
// FIXME: libufo adds a menu with a "close" entry and a "close" button. however
// when that is used, the widget is only hidden. so if one of these is used, we
// have a memory leak, as it can't be made visible anymore
BoUfoInputDialog::BoUfoInputDialog(BoUfoManager* manager, const QString& label, const QString& title)
	: BoUfoInternalFrame(manager, title)
{
 init();
 mUfoManager = manager;
 setLabel(label);

 // TODO: position the dialog in the center of the root pane
}

BoUfoInputDialog::~BoUfoInputDialog()
{
 boDebug() << k_funcinfo << endl;
}

void BoUfoInputDialog::init()
{
 mUfoManager = 0;
 setLayoutClass(UVBoxLayout);
 mLabel = new BoUfoLabel();
 mContents = new BoUfoWidget();
 BoUfoHBox* buttons = new BoUfoHBox();
 mOk = new BoUfoPushButton(i18n("Ok"));
 mCancel = new BoUfoPushButton(i18n("Cancel"));
 mNumInput = 0;
 mLineEdit = 0;

 connect(mOk, SIGNAL(signalClicked()), this, SLOT(slotOkClicked()));
 connect(mCancel, SIGNAL(signalClicked()), this, SLOT(slotCancelled()));

 addWidget(mLabel);
 addWidget(mContents);
 buttons->addWidget(mOk);
 buttons->addWidget(mCancel);
 addWidget(buttons);
}

void BoUfoInputDialog::setLabel(const QString& label)
{
 mLabel->setText(label);
}

void BoUfoInputDialog::slotOkClicked()
{
 if (mNumInput) {
	emit signalValueEntered((int)mNumInput->value());
	emit signalValueEntered((float)mNumInput->value());
 }
 if (mLineEdit) {
	emit signalValueEntered(mLineEdit->text());
 }
 close();
}

void BoUfoInputDialog::slotCancelled()
{
 emit signalCancelled();
 close();
}

void BoUfoInputDialog::close()
{
 emit signalClosed();
 mUfoManager->removeFrame(this);
}

BoUfoNumInput* BoUfoInputDialog::numInput()
{
 if (!mNumInput) {
	mNumInput = new BoUfoNumInput();
	mContents->addWidget(mNumInput);
 }
 return mNumInput;
}

BoUfoLineEdit* BoUfoInputDialog::lineEdit()
{
 if (!mLineEdit) {
	mLineEdit = new BoUfoLineEdit();
	mContents->addWidget(mLineEdit);
 }
 return mLineEdit;
}

BoUfoInputDialog* BoUfoInputDialog::getIntegerWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title, int value, int min, int max, int step)
{
 BoUfoInputDialog* dialog = createNumDialog(manager, label, title, (float)value, (float)min, (float)max, (float)step);

 connect(dialog, SIGNAL(signalValueEntered(int)), receiver, slot);

 return dialog;
}

BoUfoInputDialog* BoUfoInputDialog::getFloatWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title, float value, float min, float max, float step)
{
 BoUfoInputDialog* dialog = createNumDialog(manager, label, title, value, min, max, step);

 connect(dialog, SIGNAL(signalValueEntered(float)), receiver, slot);

 return dialog;
}

BoUfoInputDialog* BoUfoInputDialog::getStringWidget(BoUfoManager* manager, const QObject* receiver, const char* slot, const QString& label, const QString& title, const QString& value)
{
 BoUfoInputDialog* dialog = new BoUfoInputDialog(manager, label, title);
// dialog->setResizable(true); // AB: not yet implemented in libufo

 BoUfoLineEdit* input = dialog->lineEdit();
 input->setText(value);

 dialog->adjustSize();

 connect(dialog, SIGNAL(signalValueEntered(const QString&)), receiver, slot);
 return dialog;
}


BoUfoInputDialog* BoUfoInputDialog::createNumDialog(BoUfoManager* manager, const QString& label, const QString& title, float value, float min, float max, float step)
{
 BoUfoInputDialog* dialog = new BoUfoInputDialog(manager, label, title);
// dialog->setResizable(true); // AB: not yet implemented in libufo

 BoUfoNumInput* input = dialog->numInput();
 input->setStepSize(step);
 input->setRange(min, max);
 input->setValue(value);

 dialog->adjustSize();

 return dialog;
}

BoUfoWidgetStack::BoUfoWidgetStack() : BoUfoWidget()
{
 init();
}

BoUfoWidgetStack::~BoUfoWidgetStack()
{
 delete mId2Widget;
}

void BoUfoWidgetStack::init()
{
 mId2Widget = new QMap<int, BoUfoWidget*>();
 mVisibleWidget = 0;

 // we just need some layout. there is only a single widget visible at any time,
 // so it doesnt matter so much which layout we use
 setLayoutClass(BoUfoWidget::UVBoxLayout);
}

int BoUfoWidgetStack::insertStackWidget(BoUfoWidget* widget, int id)
{
 if (id < 0) {
	id = mId2Widget->count();
	while (mId2Widget->contains(id)) {
		id++;
	}
 }
 mId2Widget->insert(id, widget);
 widget->hide();
 if (!mVisibleWidget) {
	raiseStackWidget(id);
 }
 return id;
}

void BoUfoWidgetStack::raiseStackWidget(BoUfoWidget* widget)
{
 raiseStackWidget(id(widget));
}

void BoUfoWidgetStack::raiseStackWidget(int id)
{
 if (mVisibleWidget) {
	mVisibleWidget->hide();
 }
 if (mId2Widget->contains(id)) {
	mVisibleWidget = *mId2Widget->find(id);
 } else {
	mVisibleWidget = 0;
 }
 if (mVisibleWidget) {
	mVisibleWidget->show();
 }
}

void BoUfoWidgetStack::removeStackWidget(BoUfoWidget* w)
{
 removeStackWidget(id(w));
}

void BoUfoWidgetStack::removeStackWidget(int id)
{
 if (id < 0) {
	return;
 }
 BoUfoWidget* w = widget(id);
 mId2Widget->remove(id);
 if (mVisibleWidget == w) {
	if (mId2Widget->count() > 0) {
		raiseStackWidget(mId2Widget->begin().key());
	} else {
		raiseStackWidget(-1);
	}
 }
}

BoUfoWidget* BoUfoWidgetStack::widget(int id) const
{
 if (id < 0) {
	return 0;
 }
 if (!mId2Widget->contains(id)) {
	return 0;
 }
 return *mId2Widget->find(id);
}

int BoUfoWidgetStack::id(BoUfoWidget* widget) const
{
 QMap<int, BoUfoWidget*>::iterator it;
 for (it = mId2Widget->begin(); it != mId2Widget->end(); ++it) {
	if ((*it) == widget) {
		return it.key();
	}
 }
 return -1;
}

