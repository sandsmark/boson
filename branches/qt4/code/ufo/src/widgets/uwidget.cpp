/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uwidget.cpp
    begin             : Sun May 13 2001
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/widgets/uwidget.hpp"

// headers in top_dir
//#include "ufo/ufo_gl.hpp"

//#include "ufo/utexture.hpp"
#include "ufo/ukeystroke.hpp"
#include "ufo/uinputmap.hpp"

#include "ufo/udisplay.hpp"
#include "ufo/utoolkit.hpp"
#include "ufo/ucontext.hpp"

#include "ufo/udrawable.hpp"

#include "ufo/ufocusmanager.hpp"
#include "ufo/urepaintmanager.hpp"

//#include "ufo/ui/uuimanager.hpp"
//#include "ufo/ui/uwidgetui.hpp"
#include "ufo/ui/ustylemanager.hpp"
#include "ufo/ui/ustyle.hpp"
#include "ufo/ui/ustylehints.hpp"

#include "ufo/util/uinteger.hpp"
#include "ufo/util/ustring.hpp"

#include "ufo/widgets/urootpane.hpp"
#include "ufo/widgets/upopupmenu.hpp"

// headers in events
#include "ufo/events/uevents.hpp"

#include "ufo/layouts/ulayoutmanager.hpp"
#include "ufo/layouts/uboxlayout.hpp"

//#include "ufo/borders/uborder.hpp"

#include "ufo/font/ufont.hpp"

#include "ufo/ugraphics.hpp"

#include "ufo/umodel.hpp"

#include <algorithm>

using namespace ufo;

class PaintNotifier
{
public:
	PaintNotifier(UBoUfoWidgetDeleter* d)
		: m_object(d)
	{
		if (m_object) {
			m_object->startPaint();
		}
	}
	~PaintNotifier()
	{
		if (m_object) {
			m_object->endPaint();
		}
	}
private:
	UBoUfoWidgetDeleter* m_object;
};

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UWidget, UObject)

UWidget * UWidget::sm_inputFocusWidget = NULL;
UWidget * UWidget::sm_mouseFocusWidget = NULL;
UWidget * UWidget::sm_dragWidget = NULL;

static UBoxLayout ufo_defaultLayout;

UWidget::UWidget()
	: m_context(NULL)
	, m_model(new UWidgetModel())
	, m_isVisible(false)
	, m_hasClipping(true)
	, m_isEnabled(true)
	, m_isFocusable(true)
	, m_isInValidHierarchy(false)
	, m_hasInvalidLayout(false)
	, m_styleHintsDetached(false)
	, m_eventState(0xffffffff)//MouseEvents | MouseMotionEvents) // FIXME
	, m_parent(NULL)
	, m_children()
	, m_layout(&ufo_defaultLayout)
	, m_popupMenu(NULL)
	, m_cssType("widget")
	, m_cssClass()
	, m_style(NULL)
	, m_styleHints(NULL)
	, m_bounds()
	, m_clipBounds(URectangle::invalid)
	, m_cachedRootLocation(UPoint::invalid)
	, m_cachedPreferredSize(UDimension::invalid)
	, m_properties()
	, m_inputMap(new UInputMap())
	, m_ancestorInputMap(new UInputMap())
	, m_boUfoWidgetDeleter(NULL)
{
	// register at mem manager
	trackPointer(m_inputMap);
	trackPointer(m_ancestorInputMap);
	// FIXME: is this really necessarry?
	if (UToolkit * tk = UToolkit::getToolkit()) {
		m_context = tk->getCurrentContext();
	}
	m_model->widgetState = 0;
}

UWidget::UWidget(ULayoutManager * layout)
	: m_context(NULL)
	, m_model(new UWidgetModel())
	, m_isVisible(false)
	, m_hasClipping(true)
	, m_isEnabled(true)
	, m_isFocusable(true)
	, m_isInValidHierarchy(false)
	, m_hasInvalidLayout(false)
	, m_styleHintsDetached(false)
	, m_eventState(0xffffffff)//MouseEvents | MouseMotionEvents) // FIXME
	, m_parent(NULL)
	, m_children()
	, m_layout(layout)
	, m_popupMenu(NULL)
	, m_cssType("widget")
	, m_cssClass()
	, m_style(NULL)
	, m_styleHints(NULL)
	, m_bounds()
	, m_clipBounds(URectangle::invalid)
	, m_cachedRootLocation(UPoint::invalid)
	, m_cachedPreferredSize(UDimension::invalid)
	, m_properties()
	, m_inputMap(new UInputMap())
	, m_ancestorInputMap(new UInputMap())
	, m_boUfoWidgetDeleter(NULL)
{
	// register at mem manager
	trackPointer(m_layout);
	trackPointer(m_inputMap);
	trackPointer(m_ancestorInputMap);
	if (UToolkit * tk = UToolkit::getToolkit()) {
		m_context = tk->getCurrentContext();
	}
	m_model->widgetState = 0;
}

UWidget::~UWidget() {
	if (isFocused()) {
		releaseFocus();
	}
	// BoUfo extension: delete the widget deleter _first_!
	delete m_boUfoWidgetDeleter;

	releaseAllShortcuts();
	removeAll();
	// clear properties
	for (UPropertiesMap::iterator iter = m_properties.begin();
			iter != m_properties.end();
			++iter) {
		if ((*iter).second != NULL) {
			(*iter).second->unreference();
		}
	}
	m_properties.clear();

	if (m_styleHintsDetached && m_styleHints) {
		delete (m_styleHints);
	}

	delete (m_model);
}

void
UWidget::setBoUfoWidgetDeleter(UBoUfoWidgetDeleter* deleter) {
	delete m_boUfoWidgetDeleter;
	m_boUfoWidgetDeleter = deleter;
}

void
UWidget::setVisible(bool v) {
	// This explicitely shows/hides the widgets
	if (!v) {
		setState(WidgetForceInvisible);
	} else {
		setState(WidgetForceInvisible, false);
	}
	if (v == testState(WidgetVisible)) {
		return;
	}

	if (v && getParent() && !getParent()->isVisible()) {
		return;
	}
	setChildrenVisible(v);
}

void
UWidget::setChildrenVisible(bool b) {
	if (b && testState(WidgetForceInvisible)) {
		return;
	}
	// this implicitely shows/hides the window
	if (b != testState(WidgetVisible)) {
		if (b) {
			// show self
			setState(WidgetVisible);
			UWidgetEvent * e = new UWidgetEvent(this, UEvent::WidgetShown);
			processWidgetEvent(e);
		} else if (!b) {
			// hide self
			setState(WidgetVisible, false);
			UWidgetEvent * e = new UWidgetEvent(this, UEvent::WidgetHidden);
			processWidgetEvent(e);
		}
	}
	for (std::vector<UWidget*>::iterator
			iter = m_children.begin();
			iter != m_children.end(); ++iter ) {
		(*iter)->setChildrenVisible(b);
	}
	invalidate();
	repaint();
}

bool
UWidget::isVisible() const {
	return testState(WidgetVisible);//m_isVisible;
}

void
UWidget::setClipping(bool enable) {
	m_hasClipping = enable;
	repaint();
}

bool
UWidget::hasClipping() const {
	return m_hasClipping;
}

bool
UWidget::isEnabled() const {
	return !testState(WidgetDisabled);//m_isEnabled;
}

void
UWidget::setEnabled(bool b) {
	/*if (b != m_isEnabled) {
		m_isEnabled = b;
		// FIXME: invalidate?
		repaint();
	}*/
	if (!b) {
		setState(WidgetForceDisabled);
	} else {
		setState(WidgetForceDisabled, false);
	}
	if (b == isEnabled()) {
		return;
	}
	if (b && getParent() && !getParent()->isEnabled()) {
		return;
	}
	setChildrenEnabled(b);
}
void
UWidget::setChildrenEnabled(bool b) {
	if (b && testState(WidgetForceDisabled)) {
		return;
	}
	if (b) {
		// enable
		setState(WidgetDisabled, false);
	} else {
		// disable
		setState(WidgetDisabled);
	}
	for (std::vector<UWidget*>::iterator
			iter = m_children.begin();
			iter != m_children.end(); ++iter ) {
		(*iter)->setChildrenEnabled(b);
	}
	invalidate();
	repaint();
}


void
UWidget::setOpaque(bool o) {
	if (o) {
		setOpacity(1.0f);
	} else {
		setOpacity(0.0f);
	}
}
bool
UWidget::isOpaque() const {
	return (getStyleHints()->opacity == 1.0f);//m_isOpaque;
}

void
UWidget::setOpacity(float opacity) {
	if (getStyleHints()->opacity != opacity) {
		detachStyleHints();
		m_styleHints->opacity = opacity;
		processStyleHintChange(UStyleHints::OpacityHint);
	}
}

float
UWidget::getOpacity() const {
	return getStyleHints()->opacity;
}

bool
UWidget::isActive() const {
	return false;
}

bool
UWidget::isInValidHierarchy() const {
	return m_isInValidHierarchy;
}

bool
UWidget::isValid() const {
	return !m_hasInvalidLayout;
}
void
UWidget::validate() {
	validateSelf();
	for (std::vector<UWidget*>::iterator iter = m_children.begin();
			iter != m_children.end(); ++iter) {
		(*iter)->validate();
	}
}

void
UWidget::validateSelf() {
	if (m_hasInvalidLayout) {
		m_hasInvalidLayout = false;
		doLayout();
		m_hasInvalidLayout = false;
	}
}

void
UWidget::invalidateSelf() {
	m_hasInvalidLayout = true;
	m_cachedRootLocation = UPoint::invalid;
	m_cachedPreferredSize = UDimension::invalid;
	m_clipBounds = URectangle::invalid;
	//if (invalidA & ValidationLayout) {
	/*	m_cachedRootLocation.x = m_cachedRootLocation.y = 0;
		m_cachedPreferredSize.w = m_cachedPreferredSize.h = 0;
		m_clipBounds.w = m_clipBounds.h = 0;*/
		//m_cachedMinimumSize.w = m_cachedMinimumSize.h = 0;
	//}
}
void
UWidget::invalidate() {
	invalidateSelf();//invalidA);

	// && m_parent->m_needsValidation) { // != VALIDATION_NONE) {
	// evil hack see invalidateTree
	if (m_parent) { //m_parent->m_needsValidation != -1) {
		m_parent->invalidate();//invalidA);
	}
}

void
UWidget::invalidateTree() {
	// FIXME this is a hack but it avoids recursive invocation
	static UWidget * topmost = NULL;
	if (!topmost) {
		topmost = this;
	}

	if (m_children.size()) {
		for (std::vector<UWidget*>::const_iterator iter = m_children.begin();
				iter != m_children.end(); ++iter) {
			(*iter)->invalidateTree();//invalidA);
		}
	}

	if (this == topmost) {
		invalidate();//invalidA);
		topmost = NULL;
	} else {
		invalidateSelf();//invalidA);
	}
}

void
UWidget::setBounds(int x, int y, int w, int h) {
	// check size
	if (w != m_bounds.w || h != m_bounds.h) {
		m_bounds.w = w;
		m_bounds.h = h;

		// check for minimum size
		m_bounds.expand(getStyleHints()->minimumSize);
		m_bounds.clamp(getStyleHints()->maximumSize);
		UWidgetEvent * e = new UWidgetEvent(this, UWidgetEvent::WidgetResized);
		processWidgetEvent(e);

		// size changed, invalidate childs (and ancestors)
		invalidateTree();
		repaint();
	}
	// check location
	if (x != m_bounds.x || y != m_bounds.y) {
		m_bounds.x = x;
		m_bounds.y = y;
		UWidgetEvent * e = new UWidgetEvent(this, UWidgetEvent::WidgetMoved);
		processWidgetEvent(e);

		// position changed, invalidate ancestors
		//invalidate();
		// FIXME: need an invalidate position vs. invalidate size
		invalidateTree();
		repaint();
	}
}

URectangle
UWidget::getBounds() const {
	return m_bounds;
}

URectangle
UWidget::getClipBounds() const {
	if (m_clipBounds.isInvalid()) {
		m_clipBounds = URectangle(UPoint(), UDimension::maxDimension);
		// compute recursive (expensive, but only called when invalid)
		if (getParent()) {
			URectangle pRect(getParent()->getClipBounds());
			pRect -= getParent()->getInsets();
			m_clipBounds.intersect(pRect);
		}
		if (hasClipping()) {
			m_clipBounds.intersect(getRootBounds());
		}
	}
	return m_clipBounds;
}

UPoint
UWidget::getRootLocation() const {
	if (m_cachedRootLocation.isInvalid()) {
		m_cachedRootLocation = getLocation();

		for (UWidget * w = getParent(); w != NULL; w = w->getParent()) {
			m_cachedRootLocation += w->getLocation();
		}
	}

	return m_cachedRootLocation;
}


const UPalette &
UWidget::getPalette() const {
	return getStyleHints()->palette;
}

void
UWidget::setPalette(const UPalette & palette) {
	if (getStyleHints()->palette != palette) {
		detachStyleHints();
		m_styleHints->palette = palette;
		processStyleHintChange(UStyleHints::PaletteHint);
	}
}

void
UWidget::setBackgroundColor(const UColor & background) {
	if (getStyleHints()->palette.background() != background) {
		detachStyleHints();
		m_styleHints->palette.setColor(UPalette::Background, background);
		processStyleHintChange(UStyleHints::PaletteHint);
	}
}

const UColor &
UWidget::getBackgroundColor() const {
	return getStyleHints()->palette.getColor(UPalette::Background);
}

void
UWidget::setForegroundColor(const UColor & foreground) {
	if (getStyleHints()->palette.foreground() != foreground) {
		detachStyleHints();
		m_styleHints->palette.setColor(UPalette::Foreground, foreground);
		processStyleHintChange(UStyleHints::PaletteHint);
	}
}
const UColor &
UWidget::getForegroundColor() const {
	return getStyleHints()->palette.getColor(UPalette::Foreground);
}


bool
UWidget::hasBackground() const {
	return (getStyleHints()->background != NULL);
	//return (m_bgDrawable != NULL);
}
void
UWidget::setBackground(UDrawable * background) {
	if (getStyleHints()->background != background) {
		detachStyleHints();
		m_styleHints->background = background;
		processStyleHintChange(UStyleHints::BackgroundHint);
	}
}

UDrawable *
UWidget::getBackground() const {
	return getStyleHints()->background;
	//return m_bgDrawable;
}

/** @return current look and feel */
//const ULookAndFeel * UWidget::getLookAndFeel()  {
//	return m_context->getUIManager()->getLookAndFeel();//m_lookAndFeel;
//}

#define DO_UFO_PROFILING 1
void
UWidget::paint(UGraphics * g) {
	if (isVisible()) {
#ifdef DO_UFO_PROFILING
		PaintNotifier notifier(m_boUfoWidgetDeleter);
#endif
		if (!isValid()) {
			validate();
		}

		// set graphics attributes
		URectangle old_clipping;
		if (hasClipping()) {
			// FIXME: This just silently returns. Is this correct?
			if (getWidth() <= 0 || getHeight() <= 0) {
				return;
			}
			old_clipping = g->getClipRect();
			g->setClipRect(getClipBounds());
		}

		g->translate(getX(), getY());
		g->setFont(getFont());

		paintWidget(g);
		paintChildren(g);
		paintBorder(g);

		// restore graphics attributes
		g->translate(-getX(), -getY());

		if (hasClipping()) {
			g->setClipRect(old_clipping);
		}
	}
}


void
UWidget::repaint() {
	if (m_context) {
		m_context->getRepaintManager()->addDirtyWidget(this);
	}
}

void
UWidget::paintWidget(UGraphics * g) {
	//if (m_ui) {
	//	m_ui->paint(g, this);
	/*
	if (m_style) {
		m_style->paint(g, this);
	} else {
		uError() << this << " has no ui delegate\n";
	}
	*/
	getStyle()->paintComponent(g, getStyleType(), getSize(), getStyleHints(), getModel(), this);
}

void
UWidget::paintBorder(UGraphics * g) {
	/*if (m_border) {
		//UInsets * borderIn = m_border->getBorderInsets(this);
		//const UInsets & borderIn = getInsets();
		m_border->paintBorder(g, this, 0, 0, //-borderIn.left, -borderIn.top,
		                     getWidth(), getHeight());
	}*/
	getStyle()->paintBorder(g, getStyleHints()->border->borderType, getSize(), getStyleHints(), getModel()->widgetState);
	/*if (m_ui) {
		m_ui->paintBorder(g, this);
	}*/
}

void
UWidget::paintChildren(UGraphics * g) {
	// paint the last added widget first
	for (std::vector<UWidget*>::reverse_iterator
			iter = m_children.rbegin();
			iter != m_children.rend(); ++iter ) {
		(*iter)->paint(g);
	}
}


const UWidgetModel *
UWidget::getModel() const {
	return m_model;
}

UStyleManager *
UWidget::getStyleManager() const {
	/*if (URootPane * pane = getRootPane(true)) {
		return pane->getStyleManager();
	}*/
	return UToolkit::getToolkit()->getStyleManager();
}

UStyle *
UWidget::getStyle() const {
	if (m_style) {
		return m_style;
	}
	return getStyleManager()->getStyle();
}

void
UWidget::setStyle(UStyle * style) {
	m_style = style;
}

const UStyleHints *
UWidget::getStyleHints() const {
	if (!m_styleHints) {
		m_styleHints = getStyleManager()->
			getStyleHints(getCssType(), getCssClass(), getName());
		(const_cast<UWidget*>(this))->processStyleHintChange(UStyleHints::AllHints);
	}
	return m_styleHints;
}

void
UWidget::setStyleHints(UStyleHints * hints) {
	if (m_styleHintsDetached && m_styleHints) {
		delete (m_styleHints);
	}
	m_styleHints = hints;
	if (hints == NULL) {
		m_styleHintsDetached = false;
	} else {
		m_styleHintsDetached = true;
	}
	processStyleHintChange(UStyleHints::AllHints);
}

void
UWidget::detachStyleHints() {
	// ensure style hint
	getStyleHints();
	if (!m_styleHintsDetached) {
		m_styleHints = m_styleHints->clone();
		m_styleHintsDetached = true;
	}
}

void
UWidget::setCssType(const std::string & type) {
	m_cssType = type;
}

std::string
UWidget::getCssType() const {
	return m_cssType;
}

void
UWidget::setCssClass(const std::string & cssClass) {
	m_cssClass = cssClass;
}

std::string
UWidget::getCssClass() const {
	return m_cssClass;
}


UContext *
UWidget::getContext() const {
	return m_context;
}

void
UWidget::setContext(UContext * context) {
	if (context) {
		m_context = context;
	}
}

UUIManager *
UWidget::getUIManager() const {
	if (m_context) {
		return m_context->getUIManager();
	} else {
		return NULL;
	}
}

UGraphics *
UWidget::getGraphics() const {
	if (m_context) {
		return m_context->getGraphics();
	} else {
		return NULL;
	}
}



void
UWidget::setPopupMenu(UPopupMenu * popupMenu) {
	swapPointers(m_popupMenu, popupMenu);
	m_popupMenu = popupMenu;
	if (m_popupMenu) {
		m_popupMenu->setInvoker(this);
	}
}

UPopupMenu *
UWidget::getPopupMenu() const {
	return m_popupMenu;
}


bool
UWidget::removeImpl(int index) {
	if (m_children.size() > index ) {
		UWidget * ret = m_children[index];

		ret->setChildrenVisible(false);
		ret->removedFromHierarchy();
		ret->m_parent = NULL;

		// remove from mem manager
		releasePointer(ret);

		m_children.erase(m_children.begin() + index);

		invalidateTree();
		return true;
	}
	return false;
}

bool
UWidget::remove(UWidget * w) {
	std::vector<UWidget*>::iterator iter = std::find(m_children.begin(),
		m_children.end(), w);

	return removeImpl(iter - m_children.begin());
}

bool
UWidget::remove(unsigned int n) {
	return removeImpl(n);
}

UWidget *
UWidget::removeAndReturn(unsigned int n) {
	if (m_children.size() > n ) {
		UWidget * ret = m_children[n];
		if (ret) {
			ret->reference();
		}
		removeImpl(n);
		return ret;
	}
	return NULL;
}

unsigned int
UWidget::removeAll() {
	unsigned int count = 0;
	while (removeImpl(0)) {
		count++;
	}
	// count should be exactly the former widget count ...

	invalidate();
	return count;
}


void
UWidget::add(UWidget * w, const UObject * constraints, int index) {
	UObject * newConstraints;
	/*
	try {
		newConstraints = constraints->clone();
	} catch (...) {
		std::cerr << "cloning exception at\n!"
		<< "void UWidget::add(UWidget *, const UObject *, int)\n"
		<< "constraints must be clonable" << std::endl;

		newConstraints = NULL;
	}
	*/
	newConstraints = constraints->clone();
	addImpl(w, newConstraints, index);
}

void
UWidget::addImpl(UWidget * w, UObject * constraints, int index) {
	if (! w) {
		// throw exception?
		uError() << "tried to add a NULL pointer to an widget!\n";
		return ;
	}
	// we are already parent of this widget. Just move to new position
	if (w->m_parent == this) {
		setIndexOf(w, index);
		if (constraints) {
			w->put("layout", constraints);
		}
		return;
	}

	// add to mem manager
	trackPointer(w);

	for (UWidget * wp = this; wp; wp = wp->m_parent) {
		if (wp == w) {
			// oops, you wanted to add a parent to its own child
			// throw exception ?
			uError() << "tried to add a parent to its own child!\n";
			releasePointer(w);
			return;
		}
	}
	if (w->m_parent != this) {
		if (w->m_parent) {
			w->m_parent->remove(w);
		}
		w->m_parent = this;

		if (index == -1 || (unsigned int) index == m_children.size()) {
			m_children.push_back(w);
		} else {
			m_children.insert(m_children.begin() + index, w);
		}
	}

	if (constraints) {
		w->put("layout", constraints);
	}

	w->invalidateTree();
	/*
	// FIXME: Should we also invalidate the UI classes
	if (m_context != w->m_context) {
		w->invalidateTree(ValidationAll);
	} else {
		w->invalidateTree();
	}*/

	if (isInValidHierarchy()) {
		w->addedToHierarchy();
	}

	//w->m_isVisible = true;
	if (isVisible()) {
		w->setChildrenVisible(true);
	}
	if (!isEnabled()) {
		w->setChildrenEnabled(false);
	}

	repaint();
}

void
UWidget::addedToHierarchy() {
	// FIXME this is a hack but it avoids recursive invocation
	static UWidget * topmost = NULL;
	if (!topmost) {
		topmost = this;
	}
	// FIXME: top-level root panes do not have parents, but all others should
	if (getParent()) {
		m_context = getParent()->m_context;
		//updateUI();
	}
	// FIXME: should we check whether the parent is valid?
	// FIXME: is this the right position to enable?
	m_isInValidHierarchy = true;
	// we need to call this signal before validating as some widgets
	// (e.g. items) need install methods to get attributes like font objects
	processWidgetEvent(new UWidgetEvent(this, UEvent::WidgetAdded));

	for (std::vector<UWidget*>::iterator iter = m_children.begin();
			iter != m_children.end(); ++iter) {
		(*iter)->addedToHierarchy();
	}

	if (this == topmost) {
		validate();
		topmost = NULL;
	}
}

void
UWidget::removedFromHierarchy() {
	m_isInValidHierarchy = false;
	resetFocus();
	for (std::vector<UWidget*>::iterator iter = m_children.begin();
			iter != m_children.end();
			++iter) {
		(*iter)->removedFromHierarchy();
	}
	processWidgetEvent(new UWidgetEvent(this, UEvent::WidgetRemoved));
}


std::ostream &
UWidget::paramString(std::ostream & os) const {
	// widget bounds
	UObject::paramString(os);
	os << m_bounds.x << "," << m_bounds.y << ";"
	<< m_bounds.w << "x" << m_bounds.h;

	if (m_hasInvalidLayout) {
		os << ",invalid layout";
	}
	//if (m_needsValidation & ValidationUI) {
	//	os << ",invalid ui";
	//}
	if (!isVisible()) {
		os << ",hidden";
	}
	if (!isEnabled()) {
		os << ",disabled";
	}
	return os;
}


void
UWidget::setFont(const UFont & font) {
	detachStyleHints();
	m_styleHints->font = font;
	processStyleHintChange(UStyleHints::FontHint);
	//m_font = font;
	//unmarkUIAttribute(AttribFont);
	invalidate();
	repaint();
}

const UFont &
UWidget::getFont() const {
	return getStyleHints()->font;
	// FIXME
	//return (m_font) ? m_font : NULL;
	       //m_context->getUIManager()->getTheme()->getControlTextFont();
}

void
UWidget::setBorder(BorderType borderType) {//const UBorder * border) {
	//swapPointers(m_border, border);
	/*m_border = border;
	removeUIAttribute(AttribBorder);
	repaint();*/

	detachStyleHints();
	m_styleHints->border->borderType = borderType;
	processStyleHintChange(UStyleHints::BorderHint);
	//m_border = borderType;
	//unmarkUIAttribute(AttribBorder);
	invalidate();
	repaint();
}

BorderType
UWidget::getBorder() const {
	return BorderType(getStyleHints()->border->borderType);
	//return m_border;
}


void
UWidget::setLayout(ULayoutManager * layout) {
	swapPointers(m_layout, layout);
	m_layout = layout;
	invalidate();
	repaint();
}

ULayoutManager *
UWidget::getLayout() const {
	return m_layout;
}
void
UWidget::doLayout() {
	if (m_layout) {
		m_layout->layoutContainer(this);
	}
}



bool
UWidget::isFocused() const {
	return (sm_inputFocusWidget == this);
}

bool
UWidget::isChildFocused() const {
	// going upwards is much faster
	// than going downwards from this widget
	for (UWidget * container = sm_inputFocusWidget; container;
			container = container->m_parent) {
		if (container == this) {
			return true;
		}
	}
	return false;
}

bool
UWidget::isAncestorFocused() const {
	for (const UWidget * container = this; container;
			container = container->m_parent) {
		if (container == sm_inputFocusWidget) {
			return true;
		}
	}
	return false;
}

UWidget *
UWidget::requestFocus() {
	UWidget * ret = sm_inputFocusWidget;
	if (!isFocusable()) {
		return ret;
	}
	if (sm_inputFocusWidget) {
		if (sm_inputFocusWidget->releaseFocus()) {
			sm_inputFocusWidget = this;
			processFocusEvent(new UFocusEvent(this, UEvent::FocusGained));//UFocusEvent::FOCUS_GAINED));
		}
	} else {
		sm_inputFocusWidget = this;
		processFocusEvent(new UFocusEvent(this, UEvent::FocusGained));//UFocusEvent::FOCUS_GAINED));
	}
	return ret;
}

bool
UWidget::releaseFocus() {
	processFocusEvent(new UFocusEvent(this, UEvent::FocusLost));//UFocusEvent::FOCUS_LOST));
	sm_inputFocusWidget = NULL;
	return true;
}

UWidget *
UWidget::getFocusedWidget() {
	return sm_inputFocusWidget;
}

bool
UWidget::hasMouseFocus() const {
	return (sm_mouseFocusWidget == this);
}

UWidget *
UWidget::getMouseFocusWidget() {
	return sm_mouseFocusWidget;
}

bool
UWidget::isFocusable() const {
	return m_isFocusable;
}

void
UWidget::setFocusable(bool focusable) {
	m_isFocusable = focusable;
}

URootPane *
UWidget::getRootPane(bool topmost) const {
	if (isInValidHierarchy() && topmost) {
		return m_context->getRootPane();
	}

	// inquire all parents

	UWidget * w = this->getParent();
	URootPane * ret = NULL;
	for ( ; w ; w = w->getParent() ) {
		ret = dynamic_cast<URootPane*>(w);

		if (ret) {
			if (topmost && ret->getParent()) {
				// we are not yet at the top most root pane
				ret = NULL;
			} else {
				break;
			}
		}
	}

	return ret;
}

UWidget *
UWidget::getParent() const {
	return m_parent;
}

void
UWidget::setMargin(const UInsets & margin) {
	detachStyleHints();
	m_styleHints->margin = margin;
	processStyleHintChange(UStyleHints::MarginHint);
	//m_margin = margin;
}
void
UWidget::setMargin(int top, int left, int bottom, int right) {
	setMargin(UInsets(top, left, bottom, right));
}

const UInsets &
UWidget::getMargin() const {
	return getStyleHints()->margin;
}

UInsets
UWidget::getInsets() const {
	/*if (m_border) {
		return m_border->getBorderInsets(this);
	} else {
		return UInsets();
	}*/
	/*if (m_ui) {
		return getMargin() + m_ui->getBorderInsets(const_cast<UWidget*>(this));
	}*/
	//return getMargin() + getStyle()->getBorderInsets(getStyleType(), getStyleHints());
	return getStyle()->getInsets(getStyleType(), getStyleHints(), getModel());
}

void
UWidget::dispatchEvent(UEvent * e) {
	// anything to do?
	e->reference();
	bool propagate = true;
	if (isEnabled() && isEventEnabled(e->getType())) {
		processEvent(e);
		propagate = !e->isConsumed();
	}
#if 0 // libufo event code (not ULayeredWidget compatible)
	if (propagate && m_parent && dynamic_cast<UInputEvent*>(e) &&
			e->getType() != UEvent::MouseEntered &&
			e->getType() != UEvent::MouseExited) {
		// reparent event
		e->setSource(m_parent);
		if (UMouseEvent * me = dynamic_cast<UMouseEvent*>(e)) {
			me->translate(getLocation());
		}
		m_parent->dispatchEvent(e);
	}
#endif
	e->unreference();
}

void
UWidget::processEvent(UEvent * e) {
	if (UMouseEvent * pe = dynamic_cast<UMouseEvent*>(e)) {
		processMouseEvent(pe);
		UFocusManager::getFocusManager()->processEvent(e);
	} else if (UMouseWheelEvent * pe = dynamic_cast<UMouseWheelEvent*>(e)) {
		processMouseWheelEvent(pe);
	} else if (UKeyEvent * pe = dynamic_cast<UKeyEvent*>(e)) {
		processKeyEvent(pe);
		UFocusManager::getFocusManager()->processEvent(e);
	} else if (UFocusEvent * pe = dynamic_cast<UFocusEvent*>(e)) {
		processFocusEvent(pe);
	} else if (dynamic_cast<UShortcutEvent*>(e) && isVisible()) {
		processShortcutEvent(dynamic_cast<UShortcutEvent*>(e));
	} else if (UWidgetEvent * pe = dynamic_cast<UWidgetEvent*>(e)) {
		processWidgetEvent(pe);
	}
}


void
UWidget::processMouseEvent(UMouseEvent * e) {
	// hmm, should this widget process the event in any manner?
	switch (e->getType()) {
		case UEvent::MouseEntered:
			if (!hasMouseFocus()) {
				if (sm_mouseFocusWidget) {
					sm_mouseFocusWidget->setState(WidgetHasMouseFocus, false);
				}
				sm_mouseFocusWidget = this;
				setState(WidgetHasMouseFocus);
				repaint();
			}
		break;
		case UEvent::MouseExited:
			if (hasMouseFocus()) {
				sm_mouseFocusWidget = NULL;
				setState(WidgetHasMouseFocus, false);
				repaint();
			}
		break;
		default:
		break;
	}
	fireMouseEvent(e);
}

void
UWidget::processMouseWheelEvent(UMouseWheelEvent * e) {
	m_sigMouseWheel(e);
}

void
UWidget::processKeyEvent(UKeyEvent * e) {
	fireKeyEvent(e);

	if (e->isConsumed() == false) {
		if (processKeyBindings(e)) {
			e->consume();
		}
	}
}

void
UWidget::processShortcutEvent(UShortcutEvent * e) {
}

void
UWidget::processFocusEvent(UFocusEvent * e) {
	e->reference();
	if (e->getType() == UEvent::FocusGained) {
		setState(WidgetHasFocus);
	} else {
		setState(WidgetHasFocus, false);
	}
	fireFocusEvent(e);
	e->unreference();
}

void
UWidget::processWidgetEvent(UWidgetEvent * e) {
	e->reference();
	fireWidgetEvent(e);
	e->unreference();
}

void
UWidget::processStateChangeEvent(uint32_t state) {
}

void
UWidget::processStyleHintChange(uint32_t styleHint) {
	invalidate();
	repaint();
}

bool
UWidget::fireFocusEvent(UFocusEvent * e) {
	switch (e->getType()) {
		case UEvent::FocusGained:
			m_sigFocusGained(e);
			break;
		case UEvent::FocusLost:
			m_sigFocusLost(e);
			break;

		default:
			break;
	}
	return e->isConsumed();
}

bool
UWidget::firePropertyChangeEvent(const std::string & prop,
		UObject * oldValue, UObject * newValue) {
	// FIXME
	return false;
}

bool
UWidget::fireMouseEvent(UMouseEvent * e) {
	switch (e->getType()) {
		case UMouseEvent::MousePressed:
			m_sigMousePressed(e);
			break;
		case UMouseEvent::MouseReleased:
			m_sigMouseReleased(e);
			break;
		case UMouseEvent::MouseClicked:
			m_sigMouseClicked(e);
			break;
		case UMouseEvent::MouseMoved:
			m_sigMouseMoved(e);
			break;
		case UMouseEvent::MouseDragged:
			m_sigMouseDragged(e);
			break;
		case UMouseEvent::MouseEntered:
			m_sigMouseEntered(e);
			break;
		case UMouseEvent::MouseExited:
			m_sigMouseExited(e);
			break;
		default:
			break;
	}

	return e->isConsumed();
}

bool
UWidget::fireKeyEvent(UKeyEvent * e) {
	switch (e->getType()) {
		case UKeyEvent::KeyPressed:
			m_sigKeyPressed(e);
			break;
		case UKeyEvent::KeyReleased:
			m_sigKeyReleased(e);
			break;
		case UKeyEvent::KeyTyped:
			m_sigKeyTyped(e);
			break;

		default:
			break;
	}
	return e->isConsumed();
}

bool
UWidget::fireWidgetEvent(UWidgetEvent * e) {
	switch (e->getType()) {
		case UEvent::WidgetMoved:
			m_sigWidgetMoved(e);
			break;
		case UEvent::WidgetResized:
			m_sigWidgetResized(e);
			break;
		case UEvent::WidgetShown:
			m_sigWidgetShown(e);
			break;
		case UEvent::WidgetHidden:
			m_sigWidgetHidden(e);
			break;
		case UEvent::WidgetAdded:
			m_sigWidgetAdded(e);
			break;
		case UEvent::WidgetRemoved:
			m_sigWidgetRemoved(e);
			break;

		default:
			break;
	}
	return e->isConsumed();
}


///
/// widget functions
///

int
UWidget::getIndexOf(const UWidget * w) const {
	std::vector<UWidget*>::const_iterator pos =
		std::find(m_children.begin(), m_children.end(), w);

	if (pos != m_children.end()) {
		return pos - m_children.begin();
	} else {
		return -1;
	}
}

void
UWidget::setIndexOf(UWidget * w, int index) {
	std::vector<UWidget*>::iterator pos =
		std::find(m_children.begin(), m_children.end(), w);

	if (pos != m_children.end()) {
		int old_pos = pos - m_children.begin();
		m_children.erase(pos);
		// range checking: if invalid position, push back
		if (index < 0 || (unsigned int)(index) >= m_children.size()) {
			m_children.push_back(w);
		} else {
			m_children.insert(m_children.begin() + index, w);
		}
		unsigned int min_pos = std::min(index, old_pos);
		for (unsigned int i = min_pos; i < m_children.size(); ++i) {
			getWidget(i)->processWidgetEvent(
				new UWidgetEvent(getWidget(i), UEvent::WidgetZOrderChanged)
			);
		}
		// The child widget might be partially hidden or revealed now
		repaint();
	}
}

UWidget *
UWidget::getWidget( unsigned int n ) const {
	//return (UWidget *) m_children.get(n);
	//	return m_children.at(n);
	if ( n >= getWidgetCount() ) {
		return NULL;
	} else {
		return m_children[n];
	}
}

UWidget *
UWidget::getWidgetAt(int x, int y) const {
	if ( !contains(x, y) ) {
		return NULL;
	}
	UWidget * ret = NULL;

	// search backwards /*const_*/
	for (std::vector<UWidget*>::const_iterator iter = m_children.begin();
			iter != m_children.end(); ++iter ) {

		ret = (*iter)->getWidgetAt
			(x - (*iter)->getX(), y - (*iter)->getY() );

		if (ret ) {
			return ret;
		}
	}
	// if no sub widget contains the point, return this widget
	// FIXME oops, better provide two methods
	return const_cast<UWidget*>(this);
}

UWidget *
UWidget::getWidgetAt(const UPoint & p) const {
	return getWidgetAt(p.x, p.y);
}

UWidget *
UWidget::getVisibleWidgetAt(int x, int y) const {
	if ( !contains(x, y) || !isVisible()) {
		return NULL;
	}
	UWidget * ret = NULL;

	// search backwards /*const_*/
	for (std::vector<UWidget*>::const_iterator iter = m_children.begin();
	        iter != m_children.end(); ++iter ) {

		ret = (*iter)->getVisibleWidgetAt
		      (x - (*iter)->getX(), y - (*iter)->getY() );

		if (ret ) {
			return ret;
		}
	}
	// if no sub widget contains the point, return this widget
	// FIXME oops, better provide two methods
	return const_cast<UWidget*>(this);
}

UWidget *
UWidget::getVisibleWidgetAt(const UPoint & p) const {
	return getVisibleWidgetAt(p.x, p.y);
}


unsigned int
UWidget::getWidgetCount() const {
	return m_children.size();
}

const std::vector<UWidget*> &
UWidget::getWidgets() const {
	return m_children;
}
std::vector<UWidget*> &
UWidget::getWidgets() {
	return m_children;
}

///
/// layout functions
///

void
UWidget::setMinimumSize(const UDimension & minimumSize) {
	if (getStyleHints()->minimumSize != minimumSize) {
		detachStyleHints();
		m_styleHints->minimumSize = minimumSize;
		processStyleHintChange(UStyleHints::MinimumSizeHint);
	}
}


UDimension
UWidget::getMinimumSize() const {
	return getStyleHints()->minimumSize;
}


void
UWidget::setPreferredSize(const UDimension & preferredSize) {
	if (getStyleHints()->preferredSize != preferredSize) {
		detachStyleHints();
		m_styleHints->preferredSize = preferredSize;
		processStyleHintChange(UStyleHints::PreferredSizeHint);
	}
}


UDimension
UWidget::getPreferredSize() const {
	return getPreferredSize(UDimension::maxDimension);
	/*// check whether there is a explicetly requested preferred size
	if (m_styleHints->preferredSize.isInvalid() == false) {
		return m_styleHints->preferredSize;
	}
	// check whether this value is already cached
	if (m_cachedPreferredSize.isInvalid() == false && isValid()) {
		return m_cachedPreferredSize;
	} else if (m_style) {
		// query the UI object for a preferred size
		m_cachedPreferredSize = m_style->getPreferredSize(this, UDimension::invalid);
	}
	if (m_cachedPreferredSize.isInvalid() && m_layout) {
		// try to query the layout manager
		m_cachedPreferredSize = m_layout->getPreferredLayoutSize(this);
	}
	if (m_cachedPreferredSize.isInvalid() && m_styleHints->background) {
		// try to query the background drawable
		// FIXME: Should this value be cached?
		m_cachedPreferredSize = UDimension(
			m_styleHints->background->getDrawableWidth(),
			m_styleHints->background->getDrawableHeight()
		);
	}

	if (m_cachedPreferredSize.isInvalid() == false) {
		return m_cachedPreferredSize;
	}
	return UDimension();*/
	/*
	// FIXME: last resort
	if (getSize().isEmpty()) {
		return m_minimumSize;
	} else {
		return getSize();
	}*/
}

UDimension
UWidget::getPreferredSize(const UDimension & maxSize) const {
	if (maxSize.isEmpty()) {
		return UDimension(0, 0);
	}
	// make sure that we are valid
	(const_cast<UWidget*>(this))->validate();
	// check whether there is a explicetly requested preferred size
	UDimension ret(getStyleHints()->preferredSize);
	// check the contents size (layout or custom)
	if (!ret.isValid()) {
		ret.clamp(maxSize);
		ret = getStyle()->getSizeFromContents(
			getStyleType(),
			getContentsSize(ret - getInsets()),
			getStyleHints(),
			getModel());
		//ret = getContentsSize(ret - getInsets());
	}
	// check background drawable
	if (!ret.isValid() && getStyleHints()->background) {
		ret = m_styleHints->background->getDrawableSize();
	}

	// clamp
	if (ret.isValid()) {
		// add margin and border insets
		//ret += getInsets();
		// update valid preferred size value (width or height)
		ret.transcribe(getStyleHints()->preferredSize);
		ret.clamp(maxSize);
		return ret;
	}
	return UDimension();
}


UDimension
UWidget::getContentsSize(const UDimension & maxSize) const {
	if (m_layout) {
		return m_layout->getPreferredLayoutSize(this, maxSize);
	}
	return UDimension::invalid;
}

void
UWidget::setMaximumSize(const UDimension & maximumSize) {
	if (getStyleHints()->maximumSize != maximumSize) {
		detachStyleHints();
		m_styleHints->maximumSize = maximumSize;
		processStyleHintChange(UStyleHints::MaximumSizeHint);
	}
}


UDimension
UWidget::getMaximumSize() const {
	return getStyleHints()->maximumSize;
	//return m_maximumSize;
}


void
UWidget::put(const std::string & key, UObject * value) {
	// mem manager code
	if (value) {
		value->reference();
	}
	if (m_properties[key]) {
		m_properties[key]->unreference();
	}
	m_properties[key] = value;
}
/*void
UWidget::put(const std::string & key, int value) {
	put(key, new UInteger(value));
}*/
void
UWidget::put(const std::string & key, const std::string & value) {
	put(key, new UString(value));
}

UObject *
UWidget::get(const std::string & key) const {
	UPropertiesMap::const_iterator iter;
	iter = m_properties.find(key);

	if (iter != m_properties.end()) {
		return (*iter).second;
	}

	return NULL;
}

std::string
UWidget::getString(const std::string & key) const {
	UPropertiesMap::const_iterator iter;
	iter = m_properties.find(key);

	if (iter != m_properties.end()) {
		if (UString * str = dynamic_cast<UString*>((*iter).second)) {
			return *str;
		}
	}

	return "";
}


void
UWidget::setHorizontalAlignment(Alignment hAlignment) {
	if (getStyleHints()->hAlignment != hAlignment) {
		detachStyleHints();
		m_styleHints->hAlignment = hAlignment;
		processStyleHintChange(UStyleHints::HAlignmentHint);
	}
}

Alignment
UWidget::getHorizontalAlignment() const {
	return getStyleHints()->hAlignment;
	//return m_horizontalAlignment;
}


void
UWidget::setVerticalAlignment(Alignment vAlignment) {
	if (getStyleHints()->vAlignment != vAlignment) {
		detachStyleHints();
		m_styleHints->vAlignment = vAlignment;
		processStyleHintChange(UStyleHints::VAlignmentHint);
	}
}

Alignment
UWidget::getVerticalAlignment() const {
	return getStyleHints()->vAlignment;
	//return m_verticalAlignment;
}

void
UWidget::setDirection(Direction direction) {
	if (getStyleHints()->direction != direction) {
		detachStyleHints();
		m_styleHints->direction = direction;
		processStyleHintChange(UStyleHints::DirectionHint);
	}
}

Direction
UWidget::getDirection() const {
	return getStyleHints()->direction;
}

void
UWidget::setOrientation(Orientation orientation) {
	if (getStyleHints()->orientation != orientation) {
		detachStyleHints();
		m_styleHints->orientation = orientation;
		processStyleHintChange(UStyleHints::OrientationHint);
	}
}

Orientation
UWidget::getOrientation() const {
	return getStyleHints()->orientation;
}

UInputMap *
UWidget::getInputMap(InputCondition conditionA) {
	// enable key events
	setEventState(UEvent::KeyPressed, true);
	if (conditionA == WhenFocused) {
		return m_inputMap;
	} else { //(conditionA == WhenAncestorFocused)
		return m_ancestorInputMap;
	}
}

void
UWidget::setInputMap(UInputMap * newInputMapA, InputCondition conditionA) {
	if (conditionA == WhenFocused) {
		swapPointers(m_inputMap, newInputMapA);
		m_inputMap = newInputMapA;
	} else {
		swapPointers(m_ancestorInputMap, newInputMapA);
		m_ancestorInputMap = newInputMapA;
	}
}

bool
UWidget::notifyKeyBindingAction(const UKeyStroke & ks, UKeyEvent * e, InputCondition condition) {
	UActionSlot * action = getInputMap(condition)->get(ks);

	if (action) {
		UActionEvent * ae = new UActionEvent(this, UEvent::Action, e->getModifiers(), ks.toString());
		ae->reference();
		(*action)(ae);
		ae->unreference();
		return true;
	}
	return false;
}


static std::list<std::pair<UKeyStroke, UWidget*> > ufo_s_shortcuts;
static unsigned int ufo_s_oldIndex = 0;
static UKeyStroke ufo_s_oldStroke;

void
UWidget::grabShortcut(const UKeyStroke & stroke) {
	ufo_s_shortcuts.push_back(std::make_pair(stroke, this));
}

void
UWidget::releaseShortcut(const UKeyStroke & stroke) {
	for (std::list<std::pair<UKeyStroke, UWidget*> >::iterator iter = ufo_s_shortcuts.begin();
			iter != ufo_s_shortcuts.end();
			++iter) {
		if (((*iter).first == stroke) &&
				((*iter).second == this)) {
			ufo_s_shortcuts.erase(iter);
			// we have invalidated our iterators
			break;
		}
	}
}
void
UWidget::releaseAllShortcuts() {
	for (std::list<std::pair<UKeyStroke, UWidget*> >::iterator iter = ufo_s_shortcuts.begin();
			iter != ufo_s_shortcuts.end();
			++iter) {
		if ((*iter).second == this) {
			ufo_s_shortcuts.erase(iter);
			// we have invalidated our iterators
			break;
		}
	}
}

bool
UWidget::processKeyBindings(UKeyEvent * e) {
	UKeyStroke stroke(e);

	// vectors of lists tend to be ugly
	std::vector<std::list<std::pair<UKeyStroke, UWidget*> >::iterator> receivers;
	for (std::list<std::pair<UKeyStroke, UWidget*> >::iterator iter = ufo_s_shortcuts.begin();
			iter != ufo_s_shortcuts.end();
			++iter) {
		if ((*iter).first == stroke) {
			receivers.push_back(iter);/*
			//todo
			UShortcutEvent * e = new UShortcutEvent((*iter).second, UEvent::Shortcut,
				stroke, false);
			e->reference();
			(*iter).second->processEvent(e);
			e->unreference();*/
		}
	}
	if (!receivers.size()) {
		return false;
	}

	bool ambig = receivers.size() > 1;
	if (ufo_s_oldStroke == stroke && ambig) {
		ufo_s_oldIndex++;
	}
	// clamp
	if (ufo_s_oldIndex >= receivers.size()) {
		ufo_s_oldIndex = 0;
	}
	ufo_s_oldStroke = stroke;
	std::vector<std::list<std::pair<UKeyStroke, UWidget*> >::iterator>::iterator iter2 = receivers.begin();
	if (ufo_s_oldIndex) {
		iter2 += ufo_s_oldIndex;
	}

	bool ret = false;
	UShortcutEvent * se = new UShortcutEvent((*(*iter2)).second, UEvent::Shortcut,
			stroke, ambig);
	se->reference();
	(*(*iter2)).second->processEvent(se);
	ret = se->isConsumed();
	se->unreference();
	return ret;

	/*
	for (std::vector<std::vector<std::pair<UKeyStroke, UWidget*> >::iterator>::iterator iter = receivers.begin();
			iter != receivers.end();
			++iter) {
		UShortcutEvent * e = new UShortcutEvent((*(*iter)).second, UEvent::Shortcut,
			stroke, ambig);
		e->reference();
		(*(*iter)).second->processEvent(e);
		e->unreference();
	}*/
	/*
	//UKeyStroke * stroke = UKeyStroke::getKeyStroke(e);
	UKeyStroke stroke(e);
	if (notifyKeyBindingAction(stroke, e, WhenFocused)) {
		return true;
	}

	for (UWidget * container = this; container; container = container->m_parent) {
		if (container->notifyKeyBindingAction(stroke, e, WhenAncestorFocused)) {
			return true;
		}
	}
	return false;
	*/
}

void
UWidget::resetFocus() {
	URootPane * root = getRootPane(true);
	if (isFocused() || isChildFocused()) {
		getFocusedWidget()->releaseFocus();
		// focus root pane if possible
		if (root == this) {
			sm_inputFocusWidget = NULL;
		} else if (root) {
			root->requestFocus();
		}
	}
	if (hasMouseFocus() && root) {
		setState(WidgetHasMouseFocus, false);
		UPoint mouse_pos;
		UDisplay::getDefault()->getMouseState(&mouse_pos.x, &mouse_pos.y);
		// getVisibleWidget does already necessary NULL return
		// if no widget is visible
		sm_mouseFocusWidget = root->getVisibleWidgetAt(mouse_pos);
		if (sm_mouseFocusWidget)
			sm_mouseFocusWidget->setState(WidgetHasMouseFocus);
	}
}

void
UWidget::setEventState(UEvent::Type type, bool b) {
	if (b) {
		switch (type) {
			case UEvent::MousePressed:
			case UEvent::MouseReleased:
			case UEvent::MouseClicked:
				m_eventState |= MouseEvents;
			break;
			case UEvent::MouseMoved:
			case UEvent::MouseDragged:
			case UEvent::MouseEntered:
			case UEvent::MouseExited:
				m_eventState |= MouseMotionEvents;
			break;
			case UEvent::MouseWheel:
				m_eventState |= MouseWheelEvents;
			break;

			case UEvent::KeyPressed:
			case UEvent::KeyReleased:
			case UEvent::KeyTyped:
				m_eventState |= KeyEvents;
			break;

			case UEvent::FocusGained:
			case UEvent::FocusLost:
				m_eventState |= FocusEvents;
			break;

			case UEvent::WidgetMoved:
			case UEvent::WidgetResized:
			case UEvent::WidgetShown:
			case UEvent::WidgetHidden:
				m_eventState |= WidgetEvents;
			break;

			default:
			break;
		}
	} else {
		switch (type) {
			case UEvent::MousePressed:
			case UEvent::MouseReleased:
			case UEvent::MouseClicked:
				m_eventState &= ~MouseEvents;
			break;
			case UEvent::MouseMoved:
			case UEvent::MouseDragged:
			case UEvent::MouseEntered:
			case UEvent::MouseExited:
				m_eventState &= ~MouseMotionEvents;
			break;
			case UEvent::MouseWheel:
				m_eventState &= ~MouseWheelEvents;
			break;

			case UEvent::KeyPressed:
			case UEvent::KeyReleased:
			case UEvent::KeyTyped:
				m_eventState &= ~KeyEvents;
			break;

			case UEvent::FocusGained:
			case UEvent::FocusLost:
				m_eventState &= ~FocusEvents;
			break;

			case UEvent::WidgetMoved:
			case UEvent::WidgetResized:
			case UEvent::WidgetShown:
			case UEvent::WidgetHidden:
				m_eventState &= ~WidgetEvents;
			break;

			default:
			break;
		}
	}
}

bool
UWidget::isEventEnabled(UEvent::Type type) const {
	bool ret = false;
	switch (type) {
		case UEvent::MousePressed:
		case UEvent::MouseReleased:
		case UEvent::MouseClicked:
			ret = (m_eventState & MouseEvents);
		break;
		case UEvent::MouseMoved:
		case UEvent::MouseDragged:
		case UEvent::MouseEntered:
		case UEvent::MouseExited:
			ret = (m_eventState & MouseMotionEvents);
		break;
		case UEvent::MouseWheel:
			ret = (m_eventState & MouseWheelEvents);
		break;

		case UEvent::KeyPressed:
		case UEvent::KeyReleased:
		case UEvent::KeyTyped:
			ret = (m_eventState & KeyEvents);
		break;

		case UEvent::FocusGained:
		case UEvent::FocusLost:
			ret = (m_eventState & FocusEvents);
		break;

		case UEvent::WidgetMoved:
		case UEvent::WidgetResized:
		case UEvent::WidgetShown:
		case UEvent::WidgetHidden:
			ret = (m_eventState & WidgetEvents);
		break;

		default:
		break;
	}
	return ret;
}

//*
//* protected functions
//*


bool
UWidget::testState(uint32_t state) const {
	return (m_model->widgetState & state);
}

void
UWidget::setState(uint32_t state, bool b) {
	if (testState(state) != b) {
		if (b) {
			m_model->widgetState |= state;
		} else {
			m_model->widgetState &= ~state;
		}
		processStateChangeEvent(state);
		repaint();
	}
}

void
UWidget::setStates(uint32_t states) {
	m_model->widgetState = states;
}

uint32_t
UWidget::getStates() const {
	return m_model->widgetState;
}
