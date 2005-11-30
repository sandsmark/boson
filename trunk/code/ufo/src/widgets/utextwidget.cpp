/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/utextwidget.cpp
    begin             : Sa Apr 2 2005
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

#include "ufo/widgets/utextwidget.hpp"

#include "ufo/text/utextlayout.hpp"
#include "ufo/text/ucaret.hpp"
#include "ufo/text/udocument.hpp"
#include "ufo/text/udocumentfactory.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/events/umouseevent.hpp"

#include "ufo/umodel.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UTextWidget, UScrollableWidget)

std::string UTextWidget::m_textBuffer;
//UAttributeSet * UTextWidget::m_attributeBuffer = NULL;

UTextModel *
_createTextModel(UWidgetModel * model, UDocument * document, UTextLayout * layout) {
	UTextModel * textModel = new UTextModel();
	textModel->widgetState = model->widgetState;
	textModel->document = document;
	textModel->textLayout = layout;
	delete (model);
	return textModel;
}

UTextWidget::UTextWidget()
	: UScrollableWidget()
	//, std::streambuf()
	, m_doc(NULL)
	, m_textLayout(new UTextLayout())
	, m_type()
	, m_columns(0)
	, m_rows(0)
{
	setCssType("textwidget");
	m_model = _createTextModel(m_model, m_doc, m_textLayout);

	setContentType("text/plain");
	setText("");
	// FIXME: is this correct?
	setFocusable(false);

	if (m_doc) {
		m_doc->sigTextReplaced().connect(slot(*this, &UTextWidget::docChanged));
	}
}

UTextWidget::UTextWidget(const std::string & text)
	: UScrollableWidget()
	//, std::streambuf()
	, m_doc(NULL)
	, m_textLayout(new UTextLayout())
	, m_type()
	, m_columns(0)
	, m_rows(0)
{
	setCssType("textwidget");
	m_model = _createTextModel(m_model, m_doc, m_textLayout);

	setContentType("text/plain");
	setText(text);
	// FIXME: is this correct?
	setFocusable(false);

	if (m_doc) {
		m_doc->sigTextReplaced().connect(slot(*this, &UTextWidget::docChanged));
	}
}

UTextWidget::~UTextWidget() {
	delete (m_textLayout);
}

void
UTextWidget::setContentType(const std::string & type) {
	if (type == "text/plain") {
		setDocument(UDocumentFactory::createMimeDocument("text/plain"));
		m_type = "text/plain";
	}
}

std::string
UTextWidget::getContentType() const {
	return m_type;
}


void
UTextWidget::setDocument(UDocument * documentA) {
	if (!documentA) {
		return;
	}
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
		m_doc->sigTextReplaced().disconnect(slot(*this, &UTextWidget::docChanged));
		releasePointer(m_doc);
	}

	m_doc = documentA;
	trackPointer(m_doc);

	caret = getCaret();
	caret->setPosition(mark);
	caret->movePosition(position);
	getTextModel()->document = m_doc;

	m_doc->sigTextReplaced().connect(slot(*this, &UTextWidget::docChanged));
}

UDocument *
UTextWidget::getDocument() const {
	return m_doc;
}

UTextLayout *
UTextWidget::getTextLayout() const {
	return m_textLayout;
}

void
UTextWidget::cut() {
	unsigned int indexFrom, indexTo;
	UCaret * caret = getCaret();
	indexFrom = std::min(caret->getPosition(), caret->getMark());
	indexTo = std::max(caret->getPosition(), caret->getMark());

	if (indexFrom != indexTo) {
		m_textBuffer = std::string(m_doc->getText() + indexFrom, indexTo - indexFrom);
		m_doc->remove(indexFrom, indexTo - indexFrom);
	}
}

void
UTextWidget::copy() {
	unsigned int indexFrom, indexTo;
	UCaret * caret = getCaret();
	indexFrom = std::min(caret->getPosition(), caret->getMark());
	indexTo = std::max(caret->getPosition(), caret->getMark());

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
UTextWidget::paste() {
	m_doc->replaceSelection(m_textBuffer.data(), m_textBuffer.length());
	repaint();
}


void
UTextWidget::setText(const std::string & textA) {
	// TODO
	// thread safety

	m_doc->clear();//remove(0, m_doc->getLength());
	m_doc->insertString(0, textA.data(), textA.length());
	repaint();
}

std::string
UTextWidget::getText() const {
	return std::string(m_doc->getText(), m_doc->getLength());
}

//
// caret methods
//

void
UTextWidget::setCaretPosition(unsigned int positionA) {
	// a caret can be placed behind the last char
	if (positionA <= m_doc->getLength() ) {
		getCaret()->setPosition(positionA);
	}
	repaint();
}

void
UTextWidget::moveCaretPosition(unsigned int positionA) {
	// a caret can be placed behind the last char
	if (positionA <= m_doc->getLength() ) {
		getCaret()->movePosition(positionA);
	}
	repaint();
}

UCaret *
UTextWidget::getCaret() const {
	return m_doc->getCaret();//m_caret;
}

//
// size hints
//

void
UTextWidget::setColumns(unsigned int columnsA) {
	m_columns = columnsA;
}

unsigned int
UTextWidget::getColumns() const {
	return m_columns;
}

void
UTextWidget::setRows(unsigned int rowsA) {
	m_rows = rowsA;
}

unsigned int
UTextWidget::getRows() const {
	return m_rows;
}


//
// Overrides UWidget
//

void
UTextWidget::validateSelf() {
	UWidget::validateSelf();
	m_textLayout->setFont(getFont());
	m_textLayout->setMaximumSize(getSize());
	m_textLayout->setText(m_doc->getText(), m_doc->getLength());
	m_textLayout->layout();
}

UDimension
UTextWidget::getContentsSize(const UDimension & maxSize) const {
	//int width = textEdit->getFont()->getFontMetrics()->getMaxCharWidth();
	// max char width is too big
	int width = getFont().getFontMetrics()->getCharWidth('m');
	int height = getFont().getFontMetrics()->getHeight();

	unsigned int columns = getColumns();
	unsigned int rows = getRows();

	UDimension size;

	if (columns == 0 || rows == 0) {
		//size = getRenderer()->
		//	getPreferredSize(getDocument(), getFont());

		size = m_textLayout->getPreferredSize(maxSize);
	}

	if (columns) {
		width *= columns;
	} else if (size.w) {
		width = size.w;
	}

	if (rows) {
		height *= rows;
	} else if (size.h) {
		height = size.h;
	}

	// add one pixel in x so that the caret has one line to be
	// painted if it is in the last column
	return UDimension(
		width + 1,
		height
	);
	return UDimension::invalid;
}

void
UTextWidget::processMouseEvent(UMouseEvent * e) {
	if (isFocusable())
	switch (e->getType()) {
		case UEvent::MousePressed: {
			e->consume();
			UInsets insets = getInsets();
			UPoint pos = e->getLocation() - UPoint(insets.left, insets.top);

			getCaret()->setPosition(m_textLayout->viewToModel(pos));
			repaint();
		}
		break;
		case UEvent::MouseDragged: {
			e->consume();
			UInsets insets = getInsets();
			UPoint pos = e->getLocation() - UPoint(insets.left, insets.top);
			moveCaretPosition(m_textLayout->viewToModel(pos));
			repaint();
		}
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}

std::streambuf *
UTextWidget::rdbuf() {
	return this;
}

int
UTextWidget::overflow(int c){
	// FIXME
	// character by character is damn slow
	char ch = static_cast<char>(c);
	m_doc->insertString(m_doc->getLength(), &ch, 1);
	return 0;
}

// stream-like insertion operators
UTextWidget &
UTextWidget::operator<<(const std::string & str) {
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextWidget &
UTextWidget::operator<<(int i) {
	std::string str(UString::toString(i));
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextWidget &
UTextWidget::operator<<(long i) {
	std::string str(UString::toString(i));
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextWidget &
UTextWidget::operator<<(float f) {
	std::string str(UString::toString(f));
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextWidget &
UTextWidget::operator<<(double d) {
	std::string str(UString::toString(d));
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextWidget &
UTextWidget::operator<<(const char * cstr) {
	std::string str(cstr);
	m_doc->insertString(m_doc->getLength(), str.data(), str.length());
	return *this;
}

UTextModel *
UTextWidget::getTextModel() const {
	return static_cast<UTextModel*>(m_model);
}

void
UTextWidget::docChanged(UDocument*, unsigned int,
		unsigned int, unsigned int)
{
	m_textLayout->setText(m_doc->getText(), m_doc->getLength());
	invalidate();
}
