/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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
#include "ufo/text/udefaultdocumentrenderer.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/ui/utexteditui.hpp"

#include "ufo/events/ukeyevent.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UTextEdit, UWidget)

std::string UTextEdit::m_textBuffer;
//UAttributeSet * UTextEdit::m_attributeBuffer = NULL;

UTextEdit::UTextEdit()
	: UWidget()
	//, std::streambuf()
	, m_type()
	, m_isEditable(true)
	, m_doc(NULL)
	, m_docRenderer(NULL)
//	, m_caret(NULL)
	, m_columns(0)
	, m_rows(0)
{
	setEventState(UEvent::KeyTyped, true);
	setContentType("text/plain");

	setClipping(true);
}

UTextEdit::UTextEdit(const std::string & text)
	: UWidget()
	//, std::streambuf()
	, m_type()
	, m_isEditable(true)
	, m_doc(NULL)
	, m_docRenderer(NULL)
//	, m_caret(NULL)
	, m_maxLength(0)
	, m_columns(0)
	, m_rows(0)
{
	setEventState(UEvent::KeyTyped, true);
	setContentType("text/plain");
	setText(text);

	setClipping(true);
}


void
UTextEdit::setEditable(bool b) {
	if (m_isEditable != b) {
		// TODO
		// fire property change
		// change cursor

		m_isEditable = b;
	}
}
bool
UTextEdit::isEditable() const {
	return m_isEditable;
}


void
UTextEdit::setContentType(const std::string & type) {
	if (type == "text/plain") {
		setDocument(UDocumentFactory::createMimeDocument("text/plain"));
		setRenderer(new UDefaultDocumentRenderer());
		m_type = "text/plain";
	}
}

std::string
UTextEdit::getContentType() const {
	return m_type;
}


void
UTextEdit::setDocument(UDocument * documentA) {
	// FIXME
	// thread safety?
	// fire property change

	unsigned int position = 0;
	unsigned int mark = 0;
	UCaret * caret;
	if (m_doc) {
		caret = getCaret();
		position = caret->getPosition();
		mark = caret->getMark();
		releasePointer(m_doc);
	}

	// no NULL pointers accepted
	if (documentA) {
		m_doc = documentA;
		trackPointer(m_doc);

		// set new line filter if necessary
		//if (!m_isMultiLine && (m_doc->getDocumentFilter() == NULL)) {
		//	m_doc->setDocumentFilter(UDocumentFactory::createNewLineFilter());
		//}
		caret = getCaret();
		caret->setPosition(mark);
		caret->movePosition(position);
	}
}

UDocument *
UTextEdit::getDocument() const {
	return m_doc;
}

void
UTextEdit::setRenderer(UDocumentRenderer * documentRendererA) {
	// FIXME
	// thread safety?
	// fire property change

	// no NULL pointers accepted
	if (documentRendererA) {
		swapPointers(m_docRenderer, documentRendererA);
		m_docRenderer = documentRendererA;
	}
}

UDocumentRenderer *
UTextEdit::getRenderer() const {
	return m_docRenderer;
}


void
UTextEdit::cut() {
	unsigned int indexFrom, indexTo;
	getSelection(&indexFrom, &indexTo);

	if (indexFrom != indexTo) {
		m_textBuffer = std::string(m_doc->getText() + indexFrom, indexTo - indexFrom);
		m_doc->remove(indexFrom, indexTo - indexFrom);
	}
}

void
UTextEdit::copy() {
	unsigned int indexFrom, indexTo;
	getSelection(&indexFrom, &indexTo);

	if (indexFrom != indexTo) {
		m_textBuffer = std::string(m_doc->getText() + indexFrom, indexTo - indexFrom);

		// if this document doesn ´t support styles,
		// don ´t request for styles
		//if (m_doc->isStyledDocument()) {
		//m_attributeBuffer = m_doc->getAttributes(startL, endL);
		//}
	}
}

void
UTextEdit::paste() {
	m_doc->replaceSelection(m_textBuffer.data(), m_textBuffer.length());
	repaint();
}


void
UTextEdit::setText(const std::string & textA) {
	// TODO
	// thread safety

	m_doc->clear();//remove(0, m_doc->getLength());
	m_doc->insertString(0, textA.data(), textA.length());
	repaint();
}

std::string
UTextEdit::getText() const {
	return std::string(m_doc->getText(), m_doc->getLength());
}

std::string
UTextEdit::getSelectedText() const {
	unsigned int indexFrom, indexTo;
	getSelection(&indexFrom, &indexTo);

	if (indexFrom != indexTo) {
		return std::string(m_doc->getText() + indexFrom, indexTo - indexFrom);
	}
	return "";
}


//
// selection methods
//



void
UTextEdit::setSelection(unsigned int indexFrom, unsigned int indexTo) {
	unsigned int length = m_doc->getLength();

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
	if (positionA <= m_doc->getLength() ) {
		getCaret()->setPosition(positionA);
	}
	repaint();
}

void
UTextEdit::moveCaretPosition(unsigned int positionA) {
	// a caret can be placed behind the last char
	if (positionA <= m_doc->getLength() ) {
		getCaret()->movePosition(positionA);
	}
	repaint();
}
/*
void
UTextEdit::setCaret(UCaret * caretA) {
	if (m_caret) {
		m_caret->uninstall(this);
		//m_doc->removeDocumentListener(m_caret);
		removeChild(m_caret);
	}
	//swapPointers(m_caret, caretA);
	m_caret = caretA;
	if (m_caret) {
		addChild(m_caret);
		m_caret->install(this);
		//m_doc->addDocumentListener(m_caret);
	}
	repaint();
}
*/
UCaret *
UTextEdit::getCaret() const {
	return m_doc->getCaret();//m_caret;
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
// size hints
//

void
UTextEdit::setColumns(unsigned int columnsA) {
	m_columns = columnsA;
}

unsigned int
UTextEdit::getColumns() const {
	return m_columns;
}

void
UTextEdit::setRows(unsigned int rowsA) {
	m_rows = rowsA;
}

unsigned int
UTextEdit::getRows() const {
	return m_rows;
}


//
// Overrides UWidget
//

bool
UTextEdit::isActive() const {
	return isFocused();
}

void
UTextEdit::processKeyEvent(UKeyEvent * e) {
	UWidget::processKeyEvent(e);

	if (!isEditable()) {
		return;
	}

	char keyChar[1];
	if (!e->isConsumed()) {
		UCaret * caret = getCaret();
		unsigned int carPos = caret->getPosition();

		if (e->getType() == UEvent::KeyTyped) {
			keyChar[0] = e->getKeyChar() & 0xFF;
			m_doc->replaceSelection(keyChar, 1);
		} else if (e->getType() == UEvent::KeyPressed) {
			UKeyCode_t keyL = e->getKeyCode();
			UTextEditUI * uiL = NULL;

			switch (keyL) {
				case UKey::UK_C:
					if (e->getModifiers() & UMod::Ctrl) {
						copy();
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
					m_doc->replaceSelection(keyChar, 1);
					break;
				case UKey::UK_KP_ENTER:
				case UKey::UK_RETURN:
					keyChar[0] = '\n';
					m_doc->replaceSelection(keyChar, 1);
					break;
				case UKey::UK_BACKSPACE:
					if (hasSelection()) {
						m_doc->replaceSelection(NULL, 0);
					} else if (carPos > 0) {
						m_doc->remove(--carPos, 1);
						caret->setPosition(carPos);
					}
					break;
				case UKey::UK_DELETE:
					if (hasSelection()) {
						m_doc->replaceSelection(NULL, 0);
					} else if (carPos < m_doc->getLength()) {
						m_doc->remove(carPos, 1);
					}
					break;
				case UKey::UK_UP: {
						uiL = static_cast<UTextEditUI*>(getUI());
						const URectangle & rect = uiL->modelToView(this, carPos);
						const UFontMetrics * metrics = getFont()->getFontMetrics();
						UPoint pos(rect.x, rect.y + rect.h / 2 - metrics->getHeight());
						int newPos = uiL->viewToModel(this, pos);
						caret->setPosition(newPos);
					}
					break;
				case UKey::UK_DOWN: {
						uiL = static_cast<UTextEditUI*>(getUI());
						const URectangle & rect = uiL->modelToView(this, carPos);
						const UFontMetrics * metrics = getFont()->getFontMetrics();
						UPoint pos(rect.x, rect.y + rect.h / 2 + metrics->getHeight());
						int newPos = uiL->viewToModel(this, pos);
						caret->setPosition(newPos);
					}
					break;
				case UKey::UK_LEFT:
					if (carPos > 0) {
						caret->setPosition(--carPos);
					}
					break;
				case UKey::UK_RIGHT:
					if (carPos < m_doc->getLength()) {
						caret->setPosition(++carPos);
					}
					break;
				default:
					break;
			}
		}
		// truncate to maximum text length
		if (m_maxLength && m_maxLength < (int)m_doc->getLength()) {
			m_doc->remove(m_maxLength, m_doc->getLength() - m_maxLength);
		}
		// should we revalidate?
		invalidate();
		repaint();
	}
}


std::streambuf *
UTextEdit::rdbuf() {
	return this;
}

int
UTextEdit::overflow(int c){
	// FIXME
	// character by character is damn slow
	char ch = static_cast<char>(c);
	m_doc->insertString(m_doc->getLength(), &ch, 1);
	return 0;
}

// stream-like insertion operators
UTextEdit &
UTextEdit::operator<<(const std::string & str) {
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextEdit &
UTextEdit::operator<<(int i) {
	std::string str(UString::toString(i));
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextEdit &
UTextEdit::operator<<(long i) {
	std::string str(UString::toString(i));
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextEdit &
UTextEdit::operator<<(float f) {
	std::string str(UString::toString(f));
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextEdit &
UTextEdit::operator<<(double d) {
	std::string str(UString::toString(d));
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextEdit &
UTextEdit::operator<<(const char * cstr) {
	std::string str(cstr);
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}
