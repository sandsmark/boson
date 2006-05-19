/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/utextedit.cpp
    begin             : Wed Mar 26 2003
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

#include "ufo/widgets/utextedit.hpp"

#include "ufo/text/udocumentfactory.hpp"
#include "ufo/text/ucaret.hpp"
#include "ufo/text/utextlayout.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"


#include "ufo/events/ukeyevent.hpp"
#include "ufo/events/umouseevent.hpp"

#include "ufo/umodel.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UTextEdit, UTextWidget)

//std::string UTextEdit::m_textBuffer;

UTextEdit::UTextEdit()
	: UTextWidget()
	, m_maxLength(0)
{
	setCssType("textedit");
	setEditable(true);
	setFocusable(true);
}

UTextEdit::UTextEdit(const std::string & text)
	: UTextWidget(text)
	, m_maxLength(0)
{
	setCssType("textedit");
	setEditable(true);
	setFocusable(true);
}


void
UTextEdit::setEditable(bool b) {
	setState(WidgetEditable, b);
}
bool
UTextEdit::isEditable() const {
	return testState(WidgetEditable);
}

std::string
UTextEdit::getSelectedText() const {
	unsigned int indexFrom, indexTo;
	getSelection(&indexFrom, &indexTo);

	if (indexFrom != indexTo) {
		return std::string(getDocument()->getText() + indexFrom, indexTo - indexFrom);
	}
	return "";
}


//
// selection methods
//



void
UTextEdit::setSelection(unsigned int indexFrom, unsigned int indexTo) {
	unsigned int length = getDocument()->getLength();

	if (indexFrom > length) {
		indexFrom = indexTo = length;
	}
	if (indexTo > length) {
		indexTo = length;
	}
	if (indexFrom > indexTo) {
		indexTo = indexFrom;
	}

	setCaretPosition(indexFrom);
	moveCaretPosition(indexTo);
}

void
UTextEdit::getSelection(unsigned int * indexFrom, unsigned int * indexTo) const {
	UCaret * caret = getCaret();
	if (indexFrom) {
		*indexFrom = std::min(caret->getPosition(), caret->getMark());
	}
	if (indexTo) {
		*indexTo = std::max(caret->getPosition(), caret->getMark());
	}
}


bool
UTextEdit::hasSelection() const {
	UCaret * caret = getCaret();
	return (caret->getPosition() != caret->getMark());
}

//
// caret methods
//

void
UTextEdit::setCaretPosition(unsigned int positionA) {
	// a caret can be placed behind the last char
	if (positionA <= getDocument()->getLength() ) {
		getCaret()->setPosition(positionA);
	}
	repaint();
}

void
UTextEdit::moveCaretPosition(unsigned int positionA) {
	// a caret can be placed behind the last char
	if (positionA <= getDocument()->getLength() ) {
		getCaret()->movePosition(positionA);
	}
	repaint();
}

void
UTextEdit::setMaxLength(int maxLength) {
	m_maxLength = maxLength;
}

int
UTextEdit::getMaxLength() const {
	return m_maxLength;
}


//
// Overrides UWidget
//

void
UTextEdit::processKeyEvent(UKeyEvent * e) {
	if (!isEditable()) {
		return;
	}
	UDocument * doc = getDocument();
	UTextLayout * textLayout = getTextLayout();

	char keyChar[1];
	if (!e->isConsumed()) {
		UCaret * caret = getCaret();
		unsigned int carPos = caret->getPosition();

		if (e->getType() == UEvent::KeyTyped) {
			keyChar[0] = e->getKeyChar() & 0xFF;
			doc->replaceSelection(keyChar, 1);
		} else if (e->getType() == UEvent::KeyPressed) {
			UKeyCode_t keyL = e->getKeyCode();
			//UTextEditUI * uiL = NULL;

			switch (keyL) {
				case UKey::UK_C:
					if (e->getModifiers() & UMod::Ctrl) {
						copy();
						std::cerr << "copying\n";
					}
					break;
				case UKey::UK_X:
					if (e->getModifiers() & UMod::Ctrl) {
						cut();
					}
					break;
				case UKey::UK_V:
					if (e->getModifiers() & UMod::Ctrl) {
						paste();
					}
					break;
				case UKey::UK_TAB:
					keyChar[0] = '\t';
					doc->replaceSelection(keyChar, 1);
					break;
				case UKey::UK_KP_ENTER:
				case UKey::UK_RETURN:
					keyChar[0] = '\n';
					doc->replaceSelection(keyChar, 1);
					break;
				case UKey::UK_BACKSPACE:
					if (hasSelection()) {
						doc->replaceSelection(NULL, 0);
					} else if (carPos > 0) {
						doc->remove(--carPos, 1);
						caret->setPosition(carPos);
					}
					break;
				case UKey::UK_DELETE:
					if (hasSelection()) {
						doc->replaceSelection(NULL, 0);
					} else if (carPos < doc->getLength()) {
						doc->remove(carPos, 1);
					}
					break;
				case UKey::UK_UP: {
						//uiL = static_cast<UTextEditUI*>(getUI());
						//const URectangle & rect = uiL->modelToView(this, carPos);
						URectangle rect = //getRenderer()->modelToView(m_doc,
							//caret->getPosition(), getFont());
							textLayout->modelToView(caret->getPosition());
						const UFontMetrics * metrics = getFont().getFontMetrics();
						UPoint pos(rect.x, rect.y + rect.h / 2 - metrics->getHeight());
						//int newPos = uiL->viewToModel(this, pos);
						int newPos =// getRenderer()->viewToModel(m_doc,
							//pos, getFont());
							textLayout->viewToModel(pos);
						caret->setPosition(newPos);
					}
					break;
				case UKey::UK_DOWN: {
						//uiL = static_cast<UTextEditUI*>(getUI());
						//const URectangle & rect = uiL->modelToView(this, carPos);
						URectangle rect = //getRenderer()->modelToView(m_doc,
							//caret->getPosition(), getFont());
							textLayout->modelToView(caret->getPosition());
						const UFontMetrics * metrics = getFont().getFontMetrics();
						UPoint pos(rect.x, rect.y + rect.h / 2 + metrics->getHeight());
						//int newPos = uiL->viewToModel(this, pos);
						int newPos =// getRenderer()->viewToModel(m_doc,
							//pos, getFont());
							textLayout->viewToModel(pos);
						caret->setPosition(newPos);
					}
					break;
				case UKey::UK_LEFT:
					if (carPos > 0) {
						caret->setPosition(--carPos);
					}
					break;
				case UKey::UK_RIGHT:
					if (carPos < doc->getLength()) {
						caret->setPosition(++carPos);
					}
					break;
				default:
					break;
			}
		}
		// truncate to maximum text length
		if (m_maxLength && m_maxLength < int(doc->getLength())) {
			doc->remove(m_maxLength, doc->getLength() - m_maxLength);
		}
		repaint();
	}
	UWidget::processKeyEvent(e);
}

void
UTextEdit::processMouseEvent(UMouseEvent * e) {
	if (isFocusable())
	switch (e->getType()) {
		case UEvent::MousePressed: {
			e->consume();
			requestFocus();

			UInsets insets = getInsets();
			UPoint pos = e->getLocation() - UPoint(insets.left, insets.top);

			//getCaret()->setPosition(getRenderer()->viewToModel(m_doc, pos, getFont()));
			getCaret()->setPosition(getTextLayout()->viewToModel(pos));
			repaint();
		}
		break;
		case UEvent::MouseDragged: {
			e->consume();
			UInsets insets = getInsets();
			UPoint pos = e->getLocation() - UPoint(insets.left, insets.top);
			moveCaretPosition(getTextLayout()->viewToModel(pos));//getRenderer()->viewToModel(m_doc, pos, getFont()));
			repaint();
		}
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}
