/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/uspinbox.cpp
    begin             : Sat Apr 30 2005
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

#include "ufo/widgets/uspinbox.hpp"

#include "ufo/layouts/uboxlayout.hpp"
#include "ufo/widgets/ulineedit.hpp"

#include "ufo/events/umouseevent.hpp"

#include "ufo/umodel.hpp"
#include "ufo/uvalidator.hpp"

using namespace ufo;

UFO_IMPLEMENT_CLASS(USpinBox, UWidget)

USpinBoxModel*
_createSpinBoxModel(UWidgetModel * model, float min, float max, float val) {
	USpinBoxModel * c = new USpinBoxModel();
	c->widgetState = model->widgetState;
	c->subControls = UStyle::SubControls(
		UStyle::SC_SpinBoxUp | UStyle::SC_SpinBoxDown |
		UStyle::SC_SpinBoxFrame | UStyle::SC_SpinBoxEditField
	);
	c->minimum = min;
	c->maximum = max;
	if (c->maximum <= 0) {
		c->maximum = 1;
	}
	c->value = val;
	delete (model);
	return c;
}


USpinBox::USpinBox()
	: m_lineEdit(new ULineEdit())
{
	m_model = _createSpinBoxModel(m_model, 0, 100, 0);
	add(m_lineEdit);
	setLayout(new UBoxLayout(Horizontal));
	m_lineEdit->setValidator(new UDoubleValidator());
	m_lineEdit->sigActivated().connect(slot(*this, &USpinBox::valueChanged));

	std::string proto = UString::toString(getMaximum());
	// give it some extra space
	proto += ' ';
	m_lineEdit->setPrototype(proto);
}

USpinBox::~USpinBox()
{}


float
USpinBox::getMinimum() const {
	return getSpinBoxModel()->minimum;
}

void
USpinBox::setMinimum(float min) {
	getSpinBoxModel()->minimum = min;
	if (getValue() < min) {
		setValue(min);
	}
}

float
USpinBox::getMaximum() const {
	return getSpinBoxModel()->maximum;
}

void
USpinBox::setMaximum(float max) {
	getSpinBoxModel()->maximum = max;
	if (getValue() > max) {
		setValue(max);
	}
	std::string proto = UString::toString(max);
	// give it some extra space
	proto += ' ';
	m_lineEdit->setPrototype(proto);
}

void
USpinBox::setRange(float min, float max) {
	setMinimum(min);
	setMaximum(max);
}

float
USpinBox::getValue() const {
	return getSpinBoxModel()->value;
}

void
USpinBox::setValue(float newValue) {
	float val = std::max(getMinimum(), newValue);
	val = std::min(getMaximum(), val);
	getSpinBoxModel()->value = val;
	m_lineEdit->setText(UString::toString(val));
}

USpinBoxModel *
USpinBox::getSpinBoxModel() const {
	return static_cast<USpinBoxModel*>(m_model);
}

void
USpinBox::valueChanged(UActionEvent * e) {
	float val;
	UStringStream stream(m_lineEdit->getText());
	stream >> val;
	setValue(val);
}

void
USpinBox::processMouseEvent(UMouseEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed: {
			e->consume();
			UStyle::SubControls subctrl = getStyle()->getSubControlAt(
				UStyle::CE_SpinBox, getSize(), getStyleHints(),
				getModel(), e->getLocation()
			);
			getSpinBoxModel()->activeSubControls = subctrl;
			switch (subctrl) {
				case UStyle::SC_SpinBoxUp:
					setValue(m_lineEdit->getDouble() + 1);
					repaint();
				break;
				case UStyle::SC_SpinBoxDown:
					setValue(m_lineEdit->getDouble() - 1);
					repaint();
				break;
				default:
				break;
			}
		}
		break;
		case UEvent::MouseReleased:
			e->consume();
			getSpinBoxModel()->activeSubControls = UStyle::SC_None;
			repaint();
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}
