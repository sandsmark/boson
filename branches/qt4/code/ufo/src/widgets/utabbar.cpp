/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/utabbar.hpp
    begin             : Mon Sep 26 2005
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

#include "ufo/widgets/utabbar.hpp"

#include "ufo/widgets/ucompound.hpp"
#include "ufo/events/umouseevent.hpp"
#include "ufo/events/ushortcutevent.hpp"
#include "ufo/layouts/uboxlayout.hpp"

using namespace ufo;

UFO_IMPLEMENT_CLASS(UTabBar, UWidget)

class UTabBarTab : public UCompound {
	UFO_STYLE_TYPE(UStyle::CE_TabBarTab)
public:
	UTabBarTab(const std::string & text) : UCompound(text) {}

	void deselect() {
		setState(WidgetSelected, false);
	}
	void select() {
		setState(WidgetSelected);
	}
protected:
	void processMouseEvent(UMouseEvent * e) {
		switch (e->getType()) {
			case UEvent::MousePressed:
				if (UTabBar * bar = dynamic_cast<UTabBar*>(getParent())) {
					bar->setSelectedIndex(bar->getIndexOf(this));
				} else {
					select();
				}
				e->consume();
			break;
			default:
			break;
		}
	}
	void processShortcutEvent(UShortcutEvent * e) {
		if (isEnabled()) {
			if (UTabBar * bar = dynamic_cast<UTabBar*>(getParent())) {
				bar->setSelectedIndex(bar->getIndexOf(this));
			} else {
				select();
			}
			e->consume();
		}
		UWidget::processShortcutEvent(e);
	}


	UDimension getContentsSize(const UDimension & maxSize) const {
	UDimension ret(getStyle()->getCompoundPreferredSize(
		getStyleHints(),
		getText(),
		getIcon())
	);

	if (ret.isValid()) {
		ret.clamp(maxSize);
		return ret;
	}
	return UDimension::invalid;
}
};

UTabBar::UTabBar()
	: m_selectedIndex(-1)
{
	setOrientation(Horizontal);
	setLayout(new UBoxLayout(Horizontal, 0, 0));
}

UTabBar::~UTabBar() {
}

void
UTabBar::addTab(const std::string & label) {
	UTabBarTab * tab = new UTabBarTab(label);
	add(tab);
}

void
UTabBar::removeTab(int index) {
	remove(index);
	if (index == m_selectedIndex) {
		setSelectedIndex(0);
	}
}

void
UTabBar::setSelectedIndex(int index) {
	if (index == m_selectedIndex) {
		return;
	}
	if (UTabBarTab * tab = dynamic_cast<UTabBarTab*>(getWidget(m_selectedIndex))) {
		tab->deselect();
	}
	m_selectedIndex = index;
	if (UTabBarTab * tab = dynamic_cast<UTabBarTab*>(getWidget(index))) {
		tab->select();
	}
	m_sigSelectionChanged(this);
}

int
UTabBar::getSelectedIndex() const {
	return m_selectedIndex;
}

std::string
UTabBar::getTabText(int index) const {
	if (UTabBarTab * tab = dynamic_cast<UTabBarTab*>(getWidget(index))) {
		return tab->getText();
	}
	return "";
}

int
UTabBar::getTabIndex(const std::string & label) const {
	for (unsigned int i = 0; i < getWidgetCount(); ++i) {
		if (UTabBarTab * tab = dynamic_cast<UTabBarTab*>(getWidget(i))) {
			if (tab->getText() == label) {
				return i;
			}
		}
	}
	return -1;
}

int
UTabBar::getTabCount() const {
	return getWidgetCount();
}
