/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ulineedit.cpp
    begin             : Thu Sep 16 2004
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

#include "ufo/widgets/ulineedit.hpp"

#include "ufo/text/udocumentfactory.hpp"

#include "ufo/util/uinteger.hpp"
#include "ufo/events/ukeyevent.hpp"
#include "ufo/events/uactionevent.hpp"
#include <sstream>
using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(ULineEdit, UTextEdit)

ULineEdit::ULineEdit() {}

ULineEdit::ULineEdit(const std::string & text) : UTextEdit(text) {}
/*
void
ULineEdit::setInputToAll() {
	getDocument()->setDocumentFilter(NULL);
}

void
ULineEdit::setInputToInt() {
	getDocument()->setDocumentFilter(UDocumentFactory::createDigitFilter());
}

void
ULineEdit::setInputToDouble() {
}

bool
ULineEdit::isDoubleInput() {
	return true;
}

bool
ULineEdit::isIntInput() {
	return true;
}
*/
int
ULineEdit::getInt() const {
	return UString::toInt(getText());/*
	std::stringstream stream(getText());
	int ret = 0;
	stream >> ret;
	return ret;
	//UInteger::toInt(getText());*/
}

double
ULineEdit::getDouble() const {
	return UString::toDouble(getText());
	/*
	std::stringstream stream(getText());
	double ret = 0;
	stream >> ret;
	return ret;
	//UInteger::toInt(getText());*/
}

//
// protected methods
//

void
ULineEdit::processKeyEvent(UKeyEvent * e) {
	UWidget::processKeyEvent(e);

	if (e->getType() == UEvent::KeyPressed) {
		switch (e->getKeyCode()) {
			case UKey::UK_KP_ENTER:
			case UKey::UK_RETURN:
				m_sigActivated.emit(new UActionEvent(this, UEvent::Action,
					e->getModifiers(), getText()));
				return;
			break;
			default:
			break;
		}
	}
	UTextEdit::processKeyEvent(e);
}
