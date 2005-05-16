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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
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

#include "ufo/ui/uuimanager.hpp"
#include "ufo/ui/uwidgetui.hpp"

#include "ufo/util/uinteger.hpp"
#include "ufo/util/ustring.hpp"

#include "ufo/widgets/urootpane.hpp"
#include "ufo/widgets/upopupmenu.hpp"

// headers in events
#include "ufo/events/uevents.hpp"

#include "ufo/layouts/ulayoutmanager.hpp"

//#include "ufo/borders/uborder.hpp"

#include "ufo/font/ufont.hpp"

#include "ufo/ugraphics.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UWidget, UObject)

UWidget * UWidget::sm_inputFocusWidget = NULL;
UWidget * UWidget::sm_mouseFocusWidget = NULL;
UWidget * UWidget::sm_dragWidget = NULL;

UWidget::UWidget()
	: m_context(NULL)
	, m_ui(NULL)
	, m_isVisible(false)
	, m_hasClipping(true)
	, m_isEnabled(true)
	, m_isOpaque(true)
	, m_isFocusable(true)
	, m_isInValidHierarchy(false)
	, m_needsValidation(ValidationAll)
	, m_eventState(0)//MouseEvents | MouseMotionEvents) // FIXME
	, m_uiAttributes(0xffffffff)
	, m_parent(NULL)
	, m_children()
	, m_layout(NULL)
	, m_popupMenu(NULL)
	, m_bounds()
	, m_clipBounds(URectangle::invalid)
	, m_cachedRootLocation(UPoint::invalid)
	, m_minimumSize()
	, m_maximumSize(UDimension::maxDimension)
	, m_preferredSize(UDimension::invalid)
	, m_cachedPreferredSize(UDimension::invalid)
	, m_margin()
	, m_horizontalAlignment(AlignLeft)
	, m_verticalAlignment(AlignCenter)
	, m_bgDrawable(NULL)
	, m_font(NULL)
	, m_border(NoBorder)
	, m_palette(UPalette::nullPalette)
	, m_opacity(1.0f)
	//, m_bgColor(NULL)
	//, m_fgColor(NULL)
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
}

UWidget::UWidget(ULayoutManager * layout)
	: m_context(NULL)
	, m_ui(NULL)
	, m_isVisible(false)
	, m_hasClipping(true)
	, m_isEnabled(true)
	, m_isOpaque(true)
	, m_isFocusable(true)
	, m_needsValidation(ValidationAll)
	, m_eventState(0)//MouseEvents | MouseMotionEvents) // FIXME
	, m_uiAttributes(0xffffffff)
	, m_parent(NULL)
	, m_children()
	, m_layout(layout)
	, m_popupMenu(NULL)
	, m_bounds()
	, m_clipBounds(URectangle::invalid)
	, m_cachedRootLocation(UPoint::invalid)
	, m_minimumSize()
	, m_maximumSize(UDimension::maxDimension)
	, m_preferredSize(UDimension::invalid)
	, m_cachedPreferredSize(UDimension::invalid)
	, m_margin()
	, m_horizontalAlignment(AlignLeft)
	, m_verticalAlignment(AlignCenter)
	, m_bgDrawable(NULL)
	, m_font(NULL)
	, m_border(NoBorder)
	, m_palette(UPalette::nullPalette)
	, m_opacity(1.0f)
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
}

UWidget::~UWidget() {
	if (isFocused()) {
		releaseFocus();
	}
	// BoUfo extension: delete the widget deleter _first_!
	delete m_boUfoWidgetDeleter;

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
}

void
UWidget::setBoUfoWidgetDeleter(UCollectable* deleter) {
	delete m_boUfoWidgetDeleter;
	m_boUfoWidgetDeleter = deleter;
}

void
UWidget::setVisible(bool v) {
	if (v != m_isVisible) {
		m_isVisible = v;
		UEvent::Type type;
		// reset focus if this widget is focused
		if (!m_isVisible) {
			resetFocus();
			type = UEvent::WidgetHidden;
		} else {
			type = UEvent::WidgetShown;
		}
		UWidgetEvent * e = new UWidgetEvent(this, type);
		processWidgetEvent(e);

		invalidate();
		repaint();
	}
}

bool
UWidget::isVisible() const {
	return m_isVisible;
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
	return m_isEnabled;
}

void
UWidget::setEnabled(bool b) {
	if (b != m_isEnabled) {
		m_isEnabled = b;
		// FIXME: invalidate?
		repaint();
	}
}

void
UWidget::setOpaque(bool o) {
	/*if (o != m_isOpaque) {
		m_isOpaque = o;
		// FIXME: invalidate?
		repaint();
	}*/
	if (o && (m_opacity != 1.0f)) {
		m_opacity = 1.0f;
		repaint();
	} else if (!o && (m_opacity != 0.0f)) {
		m_opacity = 0.0f;
		repaint();
	}
}
bool
UWidget::isOpaque() const {
	return (m_opacity == 1.0f);//m_isOpaque;
}

void
UWidget::setOpacity(float f) {
	m_opacity = f;
}
float
UWidget::getOpacity() const {
	return m_opacity;
}

bool
UWidget::isActive() const {
	return false;//hasMouseFocus();//m_isActive;
}

bool
UWidget::isInValidHierarchy() const {
	return m_isInValidHierarchy;
}

bool
UWidget::isValid() const {
	return !(m_needsValidation & ValidationLayout);//== ValidationNone;//m_isValid;
}
void
UWidget::validate() {
/*
	// assure that this widget has a valid UContext variable
	if (m_parent) {
		m_context = m_parent->m_context;
	}
	Validation validA = ValidationAll;

#ifdef VALIDATION_DEBUG
	uDebug() << "validating " << this << " with "
		<< validA << "; invalid is " << m_needsValidation << "\n";
#endif


	if ((validA & ValidationUI) && (m_needsValidation & ValidationUI)) {
		updateUI();
		if (!m_ui) {
			uError() << "couldn't set UI class for widget " << this << "\n";
			// FIXME: should we reenable it?
			//m_needsValidation |= VALIDATION_UI;
		}
		m_needsValidation &= ~ValidationUI;
		m_needsValidation &= ~ValidationUIAttributes;
	}

	if ((validA & ValidationUIAttributes) &&
			(m_needsValidation & ValidationUIAttributes)) {
		if (!m_ui) {
			updateUI();
		} else {
			// FIXME:
			// uninstall shouldn't be necessary.
			m_ui->uninstallUI(this);
			m_ui->installUI(this);
		}
		m_needsValidation &= ~ValidationUIAttributes;
	}

	if ((validA & ValidationLayout) &&
			(m_needsValidation & ValidationLayout)) {
		doLayout();
		m_needsValidation &= ~ValidationLayout;
	}
*/
	validateSelf();
	for (std::vector<UWidget*>::iterator iter = m_children.begin();
			iter != m_children.end(); ++iter) {
		(*iter)->validate();
	}
}

void
UWidget::validateSelf() {
	if (//(validA & ValidationLayout) &&
			(m_needsValidation & ValidationLayout)) {
		m_needsValidation &= ~ValidationLayout;
		doLayout();
		m_needsValidation &= ~ValidationLayout;
	}
}

void
UWidget::invalidateSelf() {
	m_needsValidation |= ValidationLayout;
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
		m_bounds.expand(m_minimumSize);
		m_bounds.clamp(m_maximumSize);
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


const UColorGroup &
UWidget::getColorGroup() const {
	return m_palette.getColorGroup(getColorGroupType());
}

UPalette::ColorGroupType
UWidget::getColorGroupType() const {
	if (!isEnabled()) {
		return UPalette::Disabled;
	} else if (isActive()) {
		return UPalette::Active;
	} else {
		return UPalette::Inactive;
	}
}

void
UWidget::setPalette(const UPalette & palette) {
	m_palette = palette;
	unmarkUIAttribute(AttribPalette);
}

const UPalette &
UWidget::getPalette() const {
	return m_palette;
}

void
UWidget::setBackgroundColor(const UColor & col) {
	if (col != m_palette.getColor(getColorGroupType(), UColorGroup::Background)) {
		m_palette.setColor(getColorGroupType(), UColorGroup::Background, col);
		unmarkUIAttribute(AttribPalette);
		repaint();
	}
}
const UColor &
UWidget::getBackgroundColor() const {
	return getColorGroup().getColor(UColorGroup::Background);
}

void
UWidget::setForegroundColor(const UColor & col) {
	if (col != m_palette.getColor(getColorGroupType(), UColorGroup::Foreground)) {
		m_palette.setColor(getColorGroupType(), UColorGroup::Foreground, col);
		unmarkUIAttribute(AttribPalette);
		repaint();
	}
}
const UColor &
UWidget::getForegroundColor() const {
	return getColorGroup().getColor(UColorGroup::Foreground);
}


bool
UWidget::hasBackground() const {
	return (m_bgDrawable != NULL);
}
void
UWidget::setBackground(UDrawable * texA) {
	m_bgDrawable = texA;
	invalidate();
	repaint();
}

UDrawable *
UWidget::getBackground() const {
	return m_bgDrawable;
}

/** @return current look and feel */
//const ULookAndFeel * UWidget::getLookAndFeel()  {
//	return m_context->getUIManager()->getLookAndFeel();//m_lookAndFeel;
//}

void
UWidget::paint(UGraphics * g) {
	if (isVisible()) {
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
	if (m_ui) {
		m_ui->paint(g, this);
	} else {
		uError() << this << " has no ui delegate\n";
	}
}

void
UWidget::paintBorder(UGraphics * g) {
	/*if (m_border) {
		//UInsets * borderIn = m_border->getBorderInsets(this);
		//const UInsets & borderIn = getInsets();
		m_border->paintBorder(g, this, 0, 0, //-borderIn.left, -borderIn.top,
		                     getWidth(), getHeight());
	}*/
	if (m_ui) {
		m_ui->paintBorder(g, this);
	}
}

void
UWidget::paintChildren(UGraphics * g) {
	if ( m_children.size() ) {
		// paint the last added widget first
		for (std::vector<UWidget*>::reverse_iterator
				iter = m_children.rbegin();
				iter != m_children.rend(); ++iter ) {
			(*iter)->paint(g);
		}
	}
}

void
UWidget::updateUI() {
	setUI(getUIManager()->getUI(this));
}

void
UWidget::setUI(UWidgetUI * newUI) {
	if (m_ui) {
		m_ui->uninstallUI(this);
	}

	UWidgetUI * oldUI = m_ui;
	m_ui = newUI;
	if (m_ui) {
		m_ui->installUI(this);
	}
	firePropertyChangeEvent("UI", oldUI, newUI);
	invalidate();
	repaint();
}

UWidgetUI *
UWidget::getUI() const {
	return m_ui;
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
UWidget::remove(UWidget * w) {
	std::vector<UWidget*>::iterator iter = std::find(m_children.begin(),
		m_children.end(), w);

	if (iter != m_children.end()) {
		w->setVisible(false);
		w->removedFromHierarchy();
		m_children.erase(iter);

		w->m_parent = NULL;

		invalidateTree();

		// remove from mem manager
		releasePointer(w);
		return true;
	} else {
		return false;
	}
}

bool
UWidget::remove(unsigned int n) {
	if (m_children.size() > n ) {
		UWidget * ret = m_children[n];
		if (ret /* && ret->m_parent == this*/) {
			ret->setVisible(false);
			ret->removedFromHierarchy();

			ret->m_parent = NULL;

			m_children.erase( m_children.begin() + n );
			invalidateTree();

			// remove from mem manager
			releasePointer(ret);

			return true;
		}
	}
	return false;

	//	return (UWidget *) m_children.remove(n);
}

UWidget *
UWidget::removeAndReturn(unsigned int n) {
	if (m_children.size() > n ) {
		UWidget * ret = m_children[n];
		if (ret /* && ret->m_parent == this*/) {
			ret->setVisible(false);
			ret->removedFromHierarchy();

			ret->m_parent = NULL;

			m_children.erase(m_children.begin() + n);
			invalidateTree();

			// ensure that ref count doesn't drop below 1
			ret->reference();

			// remove from mem manager
			releasePointer(ret);

			return ret;
		}
	}
	return NULL;
}

unsigned int
UWidget::removeAll() {
	unsigned int size = m_children.size();

	for (std::vector<UWidget*>::iterator iter = m_children.begin();
			iter != m_children.end();
			++iter ) {
		(*iter)->setVisible(false);
		(*iter)->removedFromHierarchy();
		(*iter)->m_parent = NULL;

		// remove from mem manager
		releasePointer((*iter));
	}
	m_children.clear();

	invalidate();
	return size;
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

	w->m_isVisible = true;

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
		updateUI();
	}
	// FIXME: should we check whether the parent is valid?
	// FIXME: is this the right position to enable?
	m_isInValidHierarchy = true;
	// we need to call this signal before validating as some widgets
	// (e.g. items) need install methods to get attributes like font objects
	m_sigWidgetAdded(this);

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
	for (std::vector<UWidget*>::iterator iter = m_children.begin();
			iter != m_children.end();
			++iter) {
		(*iter)->removedFromHierarchy();
	}
	m_sigWidgetRemoved(this);
}


std::ostream &
UWidget::paramString(std::ostream & os) const {
	// widget bounds
	UObject::paramString(os);
	os << m_bounds.x << "," << m_bounds.y << ";"
	<< m_bounds.w << "x" << m_bounds.h;

	if (m_needsValidation & ValidationLayout) {
		os << ",invalid layout";
	}
	//if (m_needsValidation & ValidationUI) {
	//	os << ",invalid ui";
	//}
	if (!m_isVisible) {
		os << ",hidden";
	}
	if (!m_isEnabled) {
		os << ",disabled";
	}
	return os;
}


void
UWidget::setFont(const UFont * font) {
	m_font = font;
	unmarkUIAttribute(AttribFont);
	invalidate();
	repaint();
}

const UFont *
UWidget::getFont() const {
	// FIXME
	return (m_font) ? m_font : NULL;
	       //m_context->getUIManager()->getTheme()->getControlTextFont();
}

void
UWidget::setBorder(BorderType borderType) {//const UBorder * border) {
	//swapPointers(m_border, border);
	/*m_border = border;
	removeUIAttribute(AttribBorder);
	repaint();*/
	m_border = borderType;
	unmarkUIAttribute(AttribBorder);
	invalidate();
	repaint();
}

BorderType
UWidget::getBorder() const {
	return m_border;
}


void
UWidget::setLayout(ULayoutManager * layout) {
	swapPointers(m_layout, layout);
	m_layout = layout;
	unmarkUIAttribute(AttribLayout);
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
	for (UWidget * container = sm_inputFocusWidget;
			container;
			container = container->m_parent) {
		if (container == this) {
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
	m_margin = margin;
}
void
UWidget::setMargin(int top, int left, int bottom, int right) {
	m_margin.top = top;
	m_margin.left = left;
	m_margin.bottom = bottom;
	m_margin.right = right;
}

const UInsets &
UWidget::getMargin() const {
	return m_margin;
}

UInsets
UWidget::getInsets() const {
	/*if (m_border) {
		return m_border->getBorderInsets(this);
	} else {
		return UInsets();
	}*/
	if (m_ui) {
		return getMargin() + m_ui->getBorderInsets(const_cast<UWidget*>(this));
	}
	return getMargin();
}

void
UWidget::dispatchEvent(UEvent * e) {
	// anything to do?
	e->reference();
	if (isEventEnabled(e->getType())) {
		processEvent(e);
	} else if (m_parent && dynamic_cast<UInputEvent*>(e)) {
#if 0
		// AB: events are propagated back to parents by the
		// abstractcontext now!
		//e->setSource(m_parent);
		m_parent->dispatchEvent(e);
#endif
	}
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
				sm_mouseFocusWidget = this;
				repaint();
			}
		break;
		case UEvent::MouseExited:
			if (hasMouseFocus()) {
				sm_mouseFocusWidget = NULL;
				repaint();
			}
		break;
		default:
		break;
	}
	/*
	if (!m_isActive && e->getType() == UEvent::MouseEntered) {
		m_isActive = true;
		repaint();
	} else if (m_isActive && e->getType() == UEvent::MouseExited) {
		m_isActive = false;
		repaint();
	}*/
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
		processKeyBindings(e);
	}
}

void
UWidget::processFocusEvent(UFocusEvent * e) {
	e->reference();
	fireFocusEvent(e);
	e->unreference();
}

void
UWidget::processWidgetEvent(UWidgetEvent * e) {
	e->reference();
	fireWidgetEvent(e);
	e->unreference();
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
		m_children.erase(pos);
		// range checking: if invalid position, push back
		if (index < 0 || (unsigned int)(index) >= m_children.size()) {
			m_children.push_back(w);
		} else {
			m_children.insert(m_children.begin() + index, w);
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
	if ( !contains(x, y) || !m_isVisible) {
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
UWidget::setMinimumSize(const UDimension & minimumSize ) {
	m_minimumSize = minimumSize;
	repaint();
}


UDimension
UWidget::getMinimumSize() const {
	return m_minimumSize;
}


void
UWidget::setPreferredSize(const UDimension & preferredSize) {
	m_preferredSize = preferredSize;
	repaint();
}


UDimension
UWidget::getPreferredSize() const {
	// check whether there is a explicetly requested preferred size
	if (m_preferredSize.isInvalid() == false) {
		return m_preferredSize;
	}
	// check whether this value is already cached
	if (m_cachedPreferredSize.isInvalid() == false && isValid()) {
		return m_cachedPreferredSize;
	} else if (m_ui) {
		// query the UI object for a preferred size
		m_cachedPreferredSize = m_ui->getPreferredSize(this);
	}
	if (m_cachedPreferredSize.isInvalid() && m_layout) {
		// try to query the layout manager
		m_cachedPreferredSize = m_layout->getPreferredLayoutSize(this);
	}
	if (m_cachedPreferredSize.isInvalid() && m_bgDrawable) {
		// try to query the background drawable
		// FIXME: Should this value be cached?
		m_cachedPreferredSize = UDimension(
			m_bgDrawable->getDrawableWidth(),
			m_bgDrawable->getDrawableHeight()
		);
	}

	if (m_cachedPreferredSize.isInvalid() == false) {
		return m_cachedPreferredSize;
	}
	return UDimension();
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
	UDimension ret(m_preferredSize);
	// check whether there is a explicetly requested preferred size
	if (ret.isInvalid() == false) {
		ret.clamp(maxSize);
		return ret;
	}
	// this time we do not cache the values
	if (m_ui) {
		// query the UI object for a preferred size
		ret = m_ui->getPreferredSize(this, maxSize);
	}
	if (ret.isInvalid() && m_layout) {
		// try to query the layout manager
		ret = m_layout->getPreferredLayoutSize(this, maxSize);
	}
	if (ret.isInvalid() && m_bgDrawable) {
		// try to query the background drawable
		// FIXME: Should this value be cached?
		ret = UDimension(
			m_bgDrawable->getDrawableWidth(),
			m_bgDrawable->getDrawableHeight()
		);
	}
	ret.clamp(maxSize);
	return ret;
	/*
	if (ret.isEmpty() == false) {
		ret.clamp(maxSize);
		return ret;
	}
	// FIXME: last resort
	return UDimension();//getSize();*/
}

void
UWidget::setMaximumSize(const UDimension & maximumSize) {
	m_maximumSize = maximumSize;
	repaint();
}


UDimension
UWidget::getMaximumSize() const {
	return m_maximumSize;
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
UWidget::setHorizontalAlignment(Alignment alignX) {
	m_horizontalAlignment = alignX;
}

Alignment
UWidget::getHorizontalAlignment() const {
	return m_horizontalAlignment;
}


void
UWidget::setVerticalAlignment(Alignment alignY) {
	m_verticalAlignment = alignY;
}

Alignment
UWidget::getVerticalAlignment() const {
	return m_verticalAlignment;
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

bool
UWidget::processKeyBindings(UKeyEvent * e) {
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
		UPoint mouse_pos;
		UDisplay::getDefault()->getMouseState(&mouse_pos.x, &mouse_pos.y);
		// getVisibleWidget does already necessary NULL return
		// if no widget is visible
		sm_mouseFocusWidget = root->getVisibleWidgetAt(mouse_pos);
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

void
UWidget::markUIAttribute(uint32_t attribute) {
	m_uiAttributes |= attribute;
}
void
UWidget::unmarkUIAttribute(uint32_t attribute) {
	m_uiAttributes &= ~attribute;
}
uint32_t
UWidget::getUIAttributesState() const {
	return m_uiAttributes;
}
