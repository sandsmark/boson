/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uprogressbar.cpp
    begin             : Tue Mar 8 2005
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

#include "ufo/widgets/uprogressbar.hpp"

#include "ufo/umodel.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UProgressBar, UWidget)

UProgressBar::UProgressBar()
{
	UProgressBarModel * pModel = new UProgressBarModel();
	pModel->widgetState = m_model->widgetState;
	pModel->minimum = 0;
	pModel->maximum = 100;
	pModel->value = 0;
	pModel->text = "";
	pModel->textVisible = true;
	delete (m_model);
	m_model = pModel;
}

int
UProgressBar::getMaximum() const {
	return getProgressBarModel()->maximum;
}

int
UProgressBar::getMinimum() const {
	return getProgressBarModel()->minimum;
}

void
UProgressBar::setMaximum(int max) {
	getProgressBarModel()->maximum = max;
	if (getValue() > max) {
		setValue(max);
	}
	updateText();
}

void
UProgressBar::setMinimum(int min){
	getProgressBarModel()->minimum = min;
	if (getValue() < min) {
		setValue(min);
	}
	updateText();
}

void
UProgressBar::setRange(int min, int max) {
	setMinimum(min);
	setMaximum(max);
}

int
UProgressBar::getValue() const {
	return getProgressBarModel()->value;
}

void
UProgressBar::setValue(int value) {
	int newValue = std::max(getMinimum(), value);
	newValue = std::min(getMaximum(), newValue);
	getProgressBarModel()->value = newValue;
	updateText();
	repaint();
}

std::string
UProgressBar::getText() const {
	// progress == value - min
	float percentage = float(getValue() - getMinimum()) /
		(getMaximum() - getMinimum());
	std::string ret = UString::toString(percentage * 100);
	ret += '%';
	return ret;
}


void
UProgressBar::setTextVisible(bool vis) {
	if (getProgressBarModel()->textVisible == vis) {
		return;
	}
	getProgressBarModel()->textVisible = vis;
	repaint();
}

bool
UProgressBar::isTextVisible() const {
	return getProgressBarModel()->textVisible;
}

UDimension
UProgressBar::getContentsSize(const UDimension & maxSize) const {
	return UDimension(100, 16);
}

UProgressBarModel *
UProgressBar::getProgressBarModel() const {
	return static_cast<UProgressBarModel*>(m_model);
}

void
UProgressBar::updateText() {
	getProgressBarModel()->text = getText();
}
