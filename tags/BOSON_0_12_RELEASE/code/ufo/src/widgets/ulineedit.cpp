/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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
#include "ufo/text/ucaret.hpp"

#include "ufo/util/uinteger.hpp"
#include "ufo/events/ukeyevent.hpp"
#include "ufo/events/uactionevent.hpp"
#include <sstream>
#include "ufo/uvalidator.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(ULineEdit, UTextEdit)

ULineEdit::ULineEdit()
	: UTextEdit()
	, m_validator(NULL)
{}

ULineEdit::ULineEdit(const std::string & text)
	: UTextEdit(text)
	, m_validator(NULL)
{}

void
ULineEdit::setValidator(const UValidator * val) {
	swapPointers(m_validator, val);
	m_validator = val;
}

const UValidator *
ULineEdit::getValidator() const {
	return m_validator;
}

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

void
ULineEdit::setPrototype(const std::string & text) {
	m_prototype = text;
}

//
// protected methods
//

void
ULineEdit::processKeyEvent(UKeyEvent * e) {
/*
	if (e->getType() == UEvent::KeyPressed) {
		switch (e->getKeyCode()) {
			case UKey::UK_KP_ENTER:
			case UKey::UK_RETURN: {
				std::string text(getText());
				if (m_validator) {
					m_validator->fixup(&text);
					if (text != getText()) {
						setText(text);
					}
				}
				m_sigActivated.emit(new UActionEvent(this, UEvent::Action,
					e->getModifiers(), text));
				return;
			}
			break;
			default:
			break;
		}
	}
*/
	if (!isEditable()) {
		return;
	}
	if (e->isConsumed()) {
		UWidget::processKeyEvent(e);
	}
	UDocument * doc = getDocument();

	char keyChar[1];
	UCaret * caret = getCaret();
	unsigned int carPos = caret->getPosition();

	if (e->getType() == UEvent::KeyTyped) {
		if (m_validator) {
			std::string newText;
			newText += e->getKeyChar() & 0xFF;
			if (m_validator->validate(&newText, NULL) != UValidator::Invalid) {
				doc->replaceSelection(newText.data(), 1);
			}
		} else {
			keyChar[0] = e->getKeyChar() & 0xFF;
			doc->replaceSelection(keyChar, 1);
		}
		e->consume();
	} else if (e->getType() == UEvent::KeyPressed) {
		UKeyCode_t keyL = e->getKeyCode();
		switch (keyL) {
			case UKey::UK_C:
				if (e->getModifiers() & UMod::Ctrl) {
					copy();
					e->consume();
				}
			break;
			case UKey::UK_X:
				if (e->getModifiers() & UMod::Ctrl) {
					cut();
					e->consume();
				}
			break;
			case UKey::UK_V:
				if (e->getModifiers() & UMod::Ctrl) {
					paste();
					e->consume();
				}
			break;
			case UKey::UK_TAB:
				keyChar[0] = '\t';
				doc->replaceSelection(keyChar, 1);
				e->consume();
			break;
			case UKey::UK_KP_ENTER:
			case UKey::UK_RETURN: {
				std::string text(getText());
				if (m_validator) {
					m_validator->fixup(&text);
					if (text != getText()) {
						setText(text);
					}
				}
				m_sigActivated.emit(new UActionEvent(this, UEvent::Action,
					e->getModifiers(), text));
				e->consume();
			}
			break;
			case UKey::UK_BACKSPACE:
				if (hasSelection()) {
					doc->replaceSelection(NULL, 0);
				} else if (carPos > 0) {
					doc->remove(--carPos, 1);
					caret->setPosition(carPos);
				}
				e->consume();
			break;
			case UKey::UK_DELETE:
				if (hasSelection()) {
					doc->replaceSelection(NULL, 0);
				} else if (carPos < doc->getLength()) {
					doc->remove(carPos, 1);
				}
				e->consume();
			break;
			case UKey::UK_LEFT:
				if (carPos > 0) {
					caret->setPosition(--carPos);
					e->consume();
				}
			break;
			case UKey::UK_RIGHT:
				if (carPos < doc->getLength()) {
					caret->setPosition(++carPos);
					e->consume();
				}
			break;
			default:
			break;
		}
		/*
		// truncate to maximum text length
		if (m_maxLength && m_maxLength < int(doc->getLength())) {
			doc->remove(m_maxLength, doc->getLength() - m_maxLength);
		}
		*/
		repaint();
	}
	UWidget::processKeyEvent(e);
}

UDimension
ULineEdit::getContentsSize(const UDimension & maxSize) const {
	if (m_prototype != "") {
		return UDimension(
			getFont().getFontMetrics()->getStringWidth(m_prototype),
			getFont().getFontMetrics()->getHeight()
		);
	} else {
		return UTextWidget::getContentsSize(maxSize);
	}
}
