/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/utabwidget.hpp
    begin             : Sun Sep 25 2005
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

#include "ufo/widgets/utabwidget.hpp"

#include "ufo/widgets/ustackwidget.hpp"
#include "ufo/widgets/utabbar.hpp"

#include "ufo/layouts/uborderlayout.hpp"

using namespace ufo;

UFO_IMPLEMENT_CLASS(UTabWidget, UWidget)

UTabWidget::UTabWidget()
	: m_stackWidget(new UStackWidget())
	, m_tabBar(new UTabBar())
{
	setLayout(new UBorderLayout(0, 0));
	add(m_stackWidget);
	add(m_tabBar, UBorderLayout::North);
	m_stackWidget->setBorder(LineBorder);

	m_tabBar->sigSelectionChanged().connect(slot(*this, &UTabWidget::slotTabSelected));
}

UTabWidget::~UTabWidget() {
}

void
UTabWidget::addTab(UWidget * child, const std::string & label) {
	m_stackWidget->add(child);
	m_tabBar->addTab(label);
	if (getTabCount() == 1) {
		setSelectedIndex(0);
	}
}

void
UTabWidget::removeTab(int index) {
	// FIXME:: not yet implemented
	m_tabBar->removeTab(index);
	m_stackWidget->remove(index);
}

std::string
UTabWidget::getTabText(int index) const {
	return m_tabBar->getTabText(index);
}

UWidget *
UTabWidget::getTabWidget(int index) const {
	return m_stackWidget->getWidget(index);
}

int
UTabWidget::getTabIndex(const std::string & label) const {
	return m_tabBar->getTabIndex(label);
}

int
UTabWidget::getTabIndex(UWidget * child) const {
	return m_stackWidget->getIndexOf(child);
}

static bool ufo_is_recursive =  false;
void
UTabWidget::setSelectedIndex(int index) {
	if (!ufo_is_recursive) {
		ufo_is_recursive = true;
		m_stackWidget->setSelectedIndex(index);
		m_tabBar->setSelectedIndex(index);
		ufo_is_recursive = false;
	}
}

int
UTabWidget::getSelectedIndex() const {
	return m_stackWidget->getSelectedIndex();
}

int
UTabWidget::getTabCount() const {
	return m_stackWidget->getWidgetCount();
}


void
UTabWidget::slotTabSelected(UTabBar * bar) {
	setSelectedIndex(bar->getSelectedIndex());
}
