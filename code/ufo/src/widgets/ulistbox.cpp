/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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

#include "ufo/ui/ulistboxui.hpp"

//#include "ufo/ui/uuimanager.hpp"

// used by string list box item
//#include "ufo/widgets/ulabel.hpp"


namespace ufo {

/** private class for string items and icon items.
  * @author Johannes Schmidt
  */
  /*
class UStringListBoxItem : public UListBoxItem {
	UFO_DECLARE_ABSTRACT_CLASS(UStringListBoxItem)
public:
	UStringListBoxItem(const UString & s);
	UStringListBoxItem(UIcon * i);
	UStringListBoxItem(const UString & s, UIcon * icon);

public: // Implements UListBoxItem
	virtual void paintItem(UListBox * listBoxA, int x, int y,
		bool isSelectedA, bool hasFocusA);
	virtual UDimension getItemSize(const UListBox * listBoxA) const;
	virtual void notify(UListBox * listBox, bool b);
private: // Private attributes
	UIcon * m_icon;
	std::string m_text;
};
UFO_IMPLEMENT_DYNAMIC_CLASS(UStringListBoxItem, UListBoxItem)

UFO_IMPLEMENT_DYNAMIC_CLASS(UListBoxItem, "")
*/

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UListBox, UScrollableWidget)


UListBox::UListBox()
	: m_selectionMode(SingleSelection)
	, m_indices()
	, m_visRowCount(5)
{}

UListBox::UListBox(const std::vector<UString> & listDataA)
	: m_selectionMode(SingleSelection)
	, m_indices()
	, m_visRowCount(5)
{}


//*
//* hides | overrides UWidget
//*
/*
void
UListBox::setUI(UListBoxUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
UListBox::getUI() const {
	return static_cast<UListBoxUI*>(UWidget::getUI());
}

void
UListBox::updateUI() {
	setUI(static_cast<UListBoxUI*>(getUIManager()->getUI(this)));
}
*/

//*
//* hides | overrides UScrollableWidget
//*

int
UListBox::getUnitIncrement(const URectangle & visibleRectA,
		Orientation orientationA, Direction directionA) const {
	int ret = 0;
	if (m_items.size()) {
		ret = m_items[0]->getItemSize(this).h;
	}
	if (directionA == Up) {
		ret = -ret;
	}
	return ret;
}

int
UListBox::getBlockIncrement(const URectangle & visibleRectA,
		Orientation orientationA, Direction directionA) const {
	// FIXME
	return (getVisibleRowCount() - 1) *
		getUnitIncrement(visibleRectA, orientationA, directionA);
}

UDimension
UListBox::getPreferredViewportSize() const {
	int n = getVisibleRowCount();
	int i = 0;
	UDimension ret;
	UInsets insets = getInsets();
	for (std::vector<UItem*>::const_iterator iter = m_items.begin();
			i < n && iter != m_items.end();
			++iter) {
		UDimension size = (*iter)->getItemSize(this);
		ret.h += size.h;
		ret.w = std::max(ret.w, size.w);
		++i;
	}
	ret.w += insets.left + insets.right;
	ret.h += insets.top + insets.bottom;
	return ret;
}
//*
//* public methods
//*

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
UListBox::addedToHierarchy() {
	UWidget::addedToHierarchy();
	for (std::vector<UItem*>::const_iterator iter = m_items.begin();
			iter != m_items.end();
			++iter) {
		(*iter)->install(this);
	}
}

void
UListBox::removedFromHierarchy() {
	UWidget::removedFromHierarchy();
	for (std::vector<UItem*>::const_iterator iter = m_items.begin();
			iter != m_items.end();
			++iter) {
		(*iter)->uninstall(this);
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
	return static_cast<UListBoxUI*>(getUI())->indexToLocation(this, indexA);
}

int
UListBox::locationToIndex(const UPoint & locationA) {
	return static_cast<UListBoxUI*>(getUI())->locationToIndex(this, locationA);
}


void
UListBox::fireSelectionEvent(int firstIndexA, int lastIndexA) {
	m_sigSelectionChanged(this, firstIndexA, lastIndexA);
}

} // namespace ufo
/*
#include "ufo/uicon.hpp"
#include "ufo/ugraphics.hpp"
#include "ufo/ufo_gl.hpp"
#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"
namespace ufo {

//
// class UStringListBoxItem
//

UStringListBoxItem::UStringListBoxItem(const UString & s)
	: m_icon(NULL)
	, m_text(s.str())
{}
UStringListBoxItem::UStringListBoxItem(UIcon * i)
	: m_icon(i)
	, m_text("")
{}
UStringListBoxItem::UStringListBoxItem(const UString & s, UIcon * icon)
	: m_icon(icon)
	, m_text(s.str())
{}

void
UStringListBoxItem::paintItem(UListBox * listBoxA, int x, int y,
		bool isSelectedA, bool hasFocusA) {
*/
	/*if (isSelectedA) {
		setForegroundColor(listBoxA->getSelectionForeground());
		setBackgroundColor(listBoxA->getSelectionBackground());
	} else {
		setForegroundColor(listBoxA->getForegroundColor());
		setBackgroundColor(listBoxA->getBackgroundColor());
	}*/
/*
	//setSize(getPreferredSize());

	// FIXME:
	// is this a hack?
	//UDimension dim = listBoxA->getInnerSize();
	//setBounds(x, y, dim.getWidth(), getHeight());

	//paint();

	UGraphics * graphics = listBoxA->getGraphics();
	UDimension dim = listBoxA->getInnerSize();

	// background
	if (isSelectedA) {
		glColor3fv(listBoxA->getSelectionBackground()->getFloat());
	} else {
		glColor3fv(listBoxA->getBackgroundColor()->getFloat());
	}
	glRecti(x, y, x + dim.getWidth(), y + dim.getHeight());

	// foreground
	if (isSelectedA) {
		glColor3fv(listBoxA->getSelectionForeground()->getFloat());
	} else {
		glColor3fv(listBoxA->getForegroundColor()->getFloat());
	}
	if (m_icon) {
		m_icon->paintIcon(listBoxA, x, y);
		x += m_icon->getIconWidth() + 4;
	}
	graphics->drawText(m_text, x, y);
}

UDimension
UStringListBoxItem::getItemSize(const UListBox * listBoxA) const {
	UDimension ret;
	const UFontMetrics * metrics = listBoxA->getFont()->getFontMetrics();
	ret.w = metrics->getTextWidth(m_text);
	ret.h = metrics->getHeight();
	if (m_icon) {
		ret.w += m_icon->getIconWidth() + 4;
		ret.h = std::max(m_icon->getIconHeight(), ret.h);
	}
	return ret;
	//return ULabel::getSize();
}

void
UStringListBoxItem::notify(UListBox * listBox, bool b) {
*/
	/*setContext(listBox->getContext());
	//updateUI();
	validate();
	setSize(getPreferredSize());
	setVisible(true);*/
/*
}

} // namespace ufo
*/
