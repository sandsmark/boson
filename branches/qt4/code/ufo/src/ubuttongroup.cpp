/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ubuttongroup.cpp
    begin             : Mon Dec 22 2003
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

#include "ufo/ubuttongroup.hpp"

#include "ufo/widgets/ubutton.hpp"
// special case for radio buttons
#include "ufo/widgets/uradiobutton.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UButtonGroup, UObject)

UButtonGroup::UButtonGroup()
	: m_buttons()
	, m_selectedButton(NULL)
{}

void
UButtonGroup::addButton(UButton * button) {
	m_buttons.push_back(button);
	button->setButtonGroup(this);

	if (button->isSelected()) {
		if (m_selectedButton == NULL) {
			m_selectedButton = button;
		} else {
			button->setSelected(false);
		}
	}
}

void
UButtonGroup::removeButton(UButton * button) {
	//m_buttons.push_back(button);
	// FIXME
	if (button == m_selectedButton) {
		button->setSelected(false);
		m_selectedButton = NULL;
	}
	m_buttons.erase(std::find(m_buttons.begin(), m_buttons.end(), button));
}

void
UButtonGroup::setSelectedButton(UButton * button, bool selected) {
	if (!selected) {
		if (button == m_selectedButton && !dynamic_cast<URadioButton*>(button)) {
			m_selectedButton->setSelected(false);
			m_selectedButton = NULL;
		}
	} else
	if (button) {
		if (m_selectedButton && m_selectedButton != button) {
			m_selectedButton->setSelected(false);
		}
		m_selectedButton = button;
		if (!m_selectedButton->isSelected()) {
			m_selectedButton->setSelected(true);
		}
	}
}

UButton *
UButtonGroup::getSelectedButton() const {
	return m_selectedButton;
}
