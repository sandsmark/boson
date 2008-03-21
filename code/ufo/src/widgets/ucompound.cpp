/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ucompound.cpp
    begin             : Fri Mar 7 2003
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

#include "ufo/widgets/ucompound.hpp"

#include "ufo/umodel.hpp"
#include "ufo/ui/ustylehints.hpp"

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UCompound, UWidget)

UCompoundModel *
_createCompoundModel(UWidgetModel * model, const std::string & text, UIcon * icon) {
	UCompoundModel * c = new UCompoundModel();
	c->widgetState = model->widgetState;
	c->icon = icon;
	c->text = text;
	c->acceleratorIndex = -1;
	//c->buttonFeatures = UCompoundModel::None;
	delete (model);
	return c;
}

UCompound::UCompound()
	: m_icon(NULL)
	, m_disabledIcon(NULL)
	, m_iconTextGap(4)
{
	m_model = _createCompoundModel(m_model, "", m_icon);
	trackPointer(m_icon);
	trackPointer(m_disabledIcon);
}

UCompound::UCompound(const std::string & text)
	: m_icon(NULL)
	, m_disabledIcon(NULL)
	, m_iconTextGap(4)
{
	m_model = _createCompoundModel(m_model, "", NULL);
	setText(text);
	trackPointer(m_icon);
	trackPointer(m_disabledIcon);
}

UCompound::UCompound(UIcon * icon)
	: m_icon(icon)
	, m_disabledIcon(icon)
	, m_iconTextGap(4)
{
	m_model = _createCompoundModel(m_model, "", icon);
	trackPointer(m_icon);
	trackPointer(m_disabledIcon);
}

UCompound::UCompound(const std::string & text, UIcon * icon)
	: m_icon(icon)
	, m_disabledIcon(icon)
	, m_iconTextGap(4)
{
	m_model = _createCompoundModel(m_model, "", icon);
	setText(text);
	trackPointer(m_icon);
	trackPointer(m_disabledIcon);
}

void
UCompound::setText(const std::string & text) {
	if (getCompoundModel()->acceleratorIndex != -1) {
		std::string accel("Alt");
		accel += '+';
		accel += getCompoundModel()->text[getCompoundModel()->acceleratorIndex];
		// release mnemonic shortcut
		releaseShortcut(accel);
	}
	// oops, we have to filter & character for accelerator indices
	std::string::size_type index = text.find('&');
	if (index < text.length() - 1 && text[index + 1] != '&') {
		std::string newtext(text);
		newtext.erase(index, 1);

		getCompoundModel()->text = newtext;
		getCompoundModel()->acceleratorIndex = index;

		updateMnemonic();
	} else {
		getCompoundModel()->text = text;
		getCompoundModel()->acceleratorIndex = -1;
	}
	invalidate();
	repaint();
}

std::string
UCompound::getText() const {
	return getCompoundModel()->text;
}

void
UCompound::setIcon(UIcon * icon) {
	swapPointers(m_icon, icon);
	m_icon = icon;
	getCompoundModel()->icon = icon;
	repaint();
}

UIcon *
UCompound::getDefaultIcon() const {
	if (m_icon) {
		return m_icon;
	} else {
		return getStyleHints()->icon;
	}
}


UIcon *
UCompound::getIcon() const {
	if (!isEnabled() && m_disabledIcon) {
		return m_disabledIcon;
	}
	return getDefaultIcon();
}

void
UCompound::setDisabledIcon(UIcon * icon) {
	swapPointers(m_disabledIcon, icon);
	m_disabledIcon = icon;
	repaint();
}

void
UCompound::setIconTextGap(int iconTextGap) {
	m_iconTextGap = iconTextGap;
	invalidate();
	repaint();
}

void
UCompound::processStateChangeEvent(uint32_t state) {
	UWidget::processStateChangeEvent(state);
	if (isVisible() || state & WidgetVisible) {
		(static_cast<UCompoundModel*>(m_model))->icon = getIcon();
	}
	if (state & WidgetVisible) {
		updateMnemonic();
	}
}

void
UCompound::processStyleHintChange(uint32_t styleHint) {
	if (styleHint == UStyleHints::AllHints ||
			styleHint == UStyleHints::IconHint) {
		(static_cast<UCompoundModel*>(m_model))->icon = getIcon();
	}
	UWidget::processStyleHintChange(styleHint);
}

std::ostream &
UCompound::paramString(std::ostream & os) const {
	os << "\"" << getText() << "\"";

	return UWidget::paramString(os);
}

UCompoundModel *
UCompound::getCompoundModel() const {
	return static_cast<UCompoundModel*>(m_model);
}

void
UCompound::updateMnemonic() {
	if (getCompoundModel()->acceleratorIndex != -1) {
		std::string accel("Alt");
		accel += '+';
		accel += getText()[getCompoundModel()->acceleratorIndex];

		if (isVisible()) {
			grabShortcut(accel);
		} else {
			releaseShortcut(accel);
		}
	}
}
