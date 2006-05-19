/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ulistbox.cpp
    begin             : Tue Jun 18 2002
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

#include "ufo/widgets/ulistbox.hpp"

#include "ufo/widgets/uitem.hpp"
#include "ufo/events/umouseevent.hpp"
#include "ufo/events/ukeyevent.hpp"


using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UListBox, UScrollableWidget)


UListBox::UListBox()
	: m_selectionMode(SingleSelection)
	, m_indices()
	, m_visRowCount(5)
{
	setCssType("listbox");
}

UListBox::UListBox(const std::vector<UString> & listDataA)
	: m_selectionMode(SingleSelection)
	, m_indices()
	, m_visRowCount(5)
{
	setCssType("listbox");
}


//
// hides | overrides UScrollableWidget
//

int
UListBox::getUnitIncrement(Orientation orientation) const {
	int ret = 0;
	if (orientation == Vertical && m_items.size()) {
		ret = m_items[0]->getItemSize(getSize(), getStyleHints(), this).h;
	}
	return ret;
}

int
UListBox::getBlockIncrement(Orientation orientation) const {
	// FIXME
	return (getVisibleRowCount() - 1) * getUnitIncrement(orientation);
}

UDimension
UListBox::getPreferredViewportSize() const {
	int n = getVisibleRowCount();
	int i = 0;
	UDimension ret;
	for (std::vector<UItem*>::const_iterator iter = m_items.begin();
			i < n && iter != m_items.end();
			++iter) {
		UDimension size = (*iter)->getItemSize(getSize(), getStyleHints(), this);
		ret.h += size.h;
		ret.w = std::max(ret.w, size.w);
		++i;
	}
	return ret + getInsets();
}

//
// public methods
//

void
UListBox::addList(const std::vector<UString> & listData) {
	for (std::vector<UString>::const_iterator iter = listData.begin();
			iter != listData.end();
			++iter) {
		UStringItem * item = new UStringItem(*iter);
		addItemImpl(item, -1);
	}
}

void
UListBox::addItem(UItem * itemA, int index) {
	if (itemA) {
		addItemImpl(itemA, index);
	}
}

void
UListBox::addItem(const UString & itemA, int index) {
	UStringItem * item = new UStringItem(itemA);

	addItemImpl(item, index);
}

void
UListBox::addItem(UIcon * itemA, int index) {
	if (itemA) {
		UStringItem * item = new UStringItem(itemA);
		addItemImpl(item, index);
	}
}


void
UListBox::addItemImpl(UItem * itemA, int index) {
	trackPointer(itemA);
	if (index == -1) {
		m_items.push_back(itemA);
	} else {
		m_items.insert(m_items.begin() + index, itemA);
	}
	if (isInValidHierarchy())  {
		itemA->install(this);
	}
	repaint();
}

void
UListBox::paintWidget(UGraphics * g) {
	//UWidget::paintWidget(g);

	//UInsets insets = getInsets();
	// painting cells
/*
	int x = insets.left;
	int y = insets.top;
	const UColor & background = getBackgroundColor();
	const UColor & foreground = getForegroundColor();
*/
	const std::vector<UItem*> & items = getItems();
	const UStyleHints * hints = getStyleHints();
	URectangle rect = getInnerBounds();

	for (std::vector<UItem*>::const_iterator iter = items.begin();
			iter != items.end();
			++iter) {
		UDimension dim = (*iter)->getItemSize(getSize(), hints, this);
		rect.h = dim.h;
		if (isSelectedIndex(iter - items.begin())) {
			(*iter)->paintItem(g, rect, hints, WidgetSelected, this);

			/*g, this, x, y,
				true, // is selected
				false, // has focus
				getColorGroup().highlightedText(), // foreground
				getColorGroup().highlight() // background
			);
			*/
		} else {
			(*iter)->paintItem(g, rect, hints, WidgetNoState, this);
			/*
			(*iter)->paintItem(g, this, x, y,
				false, false,
				foreground, background
			);*/
		}
		rect.y += dim.h;//(*iter)->getItemSize(this).getHeight();
	}
}

UDimension
UListBox::getContentsSize(const UDimension & maxSize) const {
	UDimension ret;

	// iterate through all cells
	// FIXME: this is quite expensive
	const std::vector<UItem*> & items = getItems();
	for (std::vector<UItem*>::const_iterator iter = items.begin();
			iter != items.end();
			++iter) {
		const UDimension & wsize = (*iter)->getItemSize(getSize(), getStyleHints(), this);

		ret.h += wsize.h;
		ret.w = std::max(ret.w, wsize.w);
	}

	return ret;
}

void
UListBox::processMouseEvent(UMouseEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed:
			e->consume();
			setSelectedIndex(locationToIndex(e->getLocation()));
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}

void
UListBox::processKeyEvent(UKeyEvent * e) {
	if (e->isConsumed()) {
		UWidget::processKeyEvent(e);
		return;
	}

	int cur = getSelectedIndex();
	if (e->getType() == UEvent::KeyPressed) {
		switch (e->getKeyCode()) {
			case UKey::UK_KP_DOWN:
			case UKey::UK_DOWN:
				setSelectedIndex(cur + 1);
				e->consume();
			break;
			case UKey::UK_KP_UP:
			case UKey::UK_UP: {
				setSelectedIndex((cur) ? cur - 1 : 0);
				e->consume();
			}
			break;
			default:
			break;
		}
	}
	UWidget::processKeyEvent(e);
}

void
UListBox::processWidgetEvent(UWidgetEvent * e) {
	if (e->getType() == UEvent::WidgetAdded) {
		for (std::vector<UItem*>::const_iterator iter = m_items.begin();
				iter != m_items.end();
				++iter) {
			(*iter)->install(this);
		}
	} else if (e->getType() == UEvent::WidgetAdded) {
		for (std::vector<UItem*>::const_iterator iter = m_items.begin();
				iter != m_items.end();
				++iter) {
			(*iter)->uninstall(this);
		}
	}
}


void
UListBox::removeItem(unsigned int index) {
	if (index < m_items.size()) {
		UItem * item = m_items[index];
		item->uninstall(this);
		m_items.erase(m_items.begin() + index);
		releasePointer(item);
	}
	repaint();
}

void
UListBox::removeAllItems() {
	for (std::vector<UItem*>::const_iterator iter = m_items.begin();
			iter != m_items.end();
			++iter) {
		(*iter)->uninstall(this);
		releasePointer(*iter);
	}
	m_items.clear();
	repaint();
}

UItem *
UListBox::getItemAt(unsigned int n) const {
	if (n < m_items.size()) {
		return m_items[n];
	}
	return NULL;
}

const std::vector<UItem *> &
UListBox::getItems() const {
	return m_items;
}

unsigned int
UListBox::getItemCount() const {
	return m_items.size();
}

UListBox::SelectionMode
UListBox::getSelectionMode() const {
	return m_selectionMode;
}

void
UListBox::setSelectionMode(SelectionMode modeA) {
	m_selectionMode = modeA;
}

int
UListBox::getSelectedIndex() const {
	if (m_indices.size()) {
		return m_indices[0];
	} else {
		return -1;
	}
}

void
UListBox::setSelectedIndex(int indexA) {
	if (m_indices.size()) {
		int oldIndex = m_indices[0];

		if (indexA == -1) {
			m_indices.clear();
		} else {
			if (indexA >= m_items.size()) {
				indexA = (m_items.size()) ? m_items.size() - 1 : 0;
			}
			m_indices[0] = indexA;
		}

		if (indexA > oldIndex) {
			fireSelectionEvent(oldIndex, indexA);
		} else {
			fireSelectionEvent(indexA, oldIndex);
		}
	} else {
		m_indices.push_back(indexA);
		fireSelectionEvent(indexA, indexA);
	}
	repaint();
}


std::vector<unsigned int>
UListBox::getSelectedIndices() const {
	return m_indices;
}

void
UListBox::setSelectedIndices(const std::vector<unsigned int> & indicesA) {
	m_indices = indicesA;

	// FIXME:
	// damn lazyness!
	// must implement search for smallest and greatest index
	fireSelectionEvent(0, getItemCount());
	repaint();
}


bool
UListBox::isSelectedIndex(unsigned int indexA) {
	if (m_indices.size()) {
		return (indexA == m_indices[0]);
	} else {
		return false;
	}
}


UItem *
UListBox::getSelectedItem() {
	if (m_indices.size() && m_indices[0] < m_items.size()) {
		return m_items[m_indices[0]];
	} else {
		return NULL;
	}
}

std::vector<UItem*>
UListBox::getSelectedItems() {
	std::vector<UItem*> ret;

	for (std::vector<unsigned int>::const_iterator iter = m_indices.begin();
			iter != m_indices.end();
			iter++ ) {
		if (m_indices[*iter] < m_items.size()) {
			ret.push_back(m_items[m_indices[*iter]]);
		}
	}

	return ret;
}


int
UListBox::getVisibleRowCount() const {
	return m_visRowCount;
}

void
UListBox::setVisibleRowCount(int visibleRowCountA) {
	m_visRowCount = visibleRowCountA;
	repaint();
}


UPoint
UListBox::indexToLocation(unsigned int indexA) {
	UItem * item = getItemAt(0);
	int height = 0;
	if (item) {
		height = item->getItemSize(getSize(), getStyleHints(), this).h;
	}

	return UPoint(0, indexA * height);
	//return static_cast<UListBoxUI*>(getUI())->indexToLocation(this, indexA);
	//return UPoint();
}

int
UListBox::locationToIndex(const UPoint & locationA) {
	UItem * item = getItemAt(0);
	if (item) {
		int height = item->getItemSize(getSize(), getStyleHints(), this).h;
		return locationA.y / height;
	} else {
		// FIXME
		// what about -1 ?
		return 0;
	}
	//return static_cast<UListBoxUI*>(getUI())->locationToIndex(this, locationA);
}


void
UListBox::fireSelectionEvent(int firstIndexA, int lastIndexA) {
	m_sigSelectionChanged(this, firstIndexA, lastIndexA);
}
