/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasictexteditui.cpp
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

#include "ufo/ui/basic/ubasictexteditui.hpp"

#include "ufo/ugraphics.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/widgets/utextedit.hpp"

#include "ufo/ui/uuimanager.hpp"
#include "ufo/ui/ustyle.hpp"

#include "ufo/util/ucolor.hpp"
#include "ufo/events/umouseevent.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicTextEditUI, UTextUI)

UBasicTextEditUI *
UBasicTextEditUI::m_textEditUI = new UBasicTextEditUI();

std::string UBasicTextEditUI::m_lafId("UTextEdit");

UBasicTextEditUI::UBasicTextEditUI() {}
UBasicTextEditUI::~UBasicTextEditUI() {}


UBasicTextEditUI *
UBasicTextEditUI::createUI(UWidget * w) {
	return m_textEditUI;
}


void
UBasicTextEditUI::paint(UGraphics * g, UWidget * w) {
	UWidgetUI::paint(g, w);

	UTextEdit * textEdit = dynamic_cast<UTextEdit*>(w);

	const UFont * font = w->getFont();

	// translate using the widget's insets
	UInsets insets = w->getInsets();
	//glTranslatef(insets.left, insets.top, 0);
	g->translate(insets.left, insets.top);

	UStyle * style = w->getUIManager()->getStyle();
	if (textEdit->hasSelection()) {
		UCaret * caret = textEdit->getDocument()->getCaret();
		URectangle position = modelToView(textEdit, caret->getPosition());
		URectangle mark = modelToView(textEdit, caret->getMark());

		style->paintTextSelection(g, textEdit, 0, textEdit->getInnerSize().w,
			position, mark);
	}


	//glColor3fv(w->getForegroundColor()->getFloat());
	//g->setColor(w->getForegroundColor());
	g->setColor(w->getColorGroup().text());//getForegroundColor());


	textEdit->getRenderer()->render(
		g,
		textEdit->getDocument(),
		textEdit->getInnerSize(),
		font
	);

	if (textEdit->isFocused() && textEdit->getCaret()) {
		UCaret * caret = textEdit->getCaret();
		style->paintCaret(g, textEdit,
			modelToView(textEdit, caret->getPosition()),
			caret);
	}
	//glTranslatef(-insets.left, -insets.top, 0);
	g->translate(-insets.left, -insets.top);
}


const std::string &
UBasicTextEditUI::getLafId() {
	return m_lafId;
}



void
UBasicTextEditUI::installUI(UWidget * w) {
	UTextEditUI::installUI(w);
	installSignals(dynamic_cast<UTextEdit*>(w));
}
void
UBasicTextEditUI::uninstallUI(UWidget * w) {
	UTextEditUI::uninstallUI(w);
	uninstallSignals(dynamic_cast<UTextEdit*>(w));
}


UDimension
UBasicTextEditUI::getPreferredSize(const UWidget * w) {
	/*const UTextEdit * textL = dynamic_cast<const UTextEdit*>(w);
	if (! textL) {
		return UDimension();
	}
	const UInsets & in = textL->getInsets();
	UDimension sizeL = textL->getRenderer()->
		getPreferredSize(textL->getDocument(), textL->getFont());

	// add one pixel in x so that the caret has one line to be
	// painted if it is in the last column
	sizeL.w += 1 + in.left + in.right;
	sizeL.h += in.top + in.bottom;

	return sizeL;
	*/
	const UTextEdit * textEdit = dynamic_cast<const UTextEdit*>(w);
	if (!textEdit) {
		return UDimension();
	}
	const UInsets & in = textEdit->getInsets();

	//int width = textEdit->getFont()->getFontMetrics()->getMaxCharWidth();
	// max char width is too big
	int width = textEdit->getFont()->getFontMetrics()->getCharWidth('m');
	int height = textEdit->getFont()->getFontMetrics()->getHeight();

	unsigned int columns = textEdit->getColumns();
	unsigned int rows = textEdit->getRows();

	UDimension size;

	if (columns == 0 || rows == 0) {
		size = textEdit->getRenderer()->
			getPreferredSize(textEdit->getDocument(), textEdit->getFont());
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
		width + in.left + in.right + 1,
		height + in.top + in.bottom
	);
}


static void
focus_listener_slot(UMouseEvent * e) {
	if (UTextEdit * textEdit = dynamic_cast<UTextEdit*>(e->getSource()) ) {
		e->consume();
		textEdit->requestFocus();

		// TODO
		// type safe ui
		UTextEditUI * ui = static_cast<UTextEditUI *>(textEdit->getUI());
		UInsets insets = textEdit->getInsets();
		UPoint pos = e->getLocation() - UPoint(insets.left, insets.top);
		textEdit->getCaret()->setPosition(ui->viewToModel(textEdit, pos));
		textEdit->repaint();
	}
}

static void
mouse_selection_listener_slot(UMouseEvent * e) {
	if (UTextEdit * textEdit = dynamic_cast<UTextEdit*>(e->getSource()) ) {
		e->consume();
		// TODO
		// type safe ui
		UTextEditUI * ui = static_cast<UTextEditUI *>(textEdit->getUI());
		UInsets insets = textEdit->getInsets();
		UPoint pos = e->getLocation() - UPoint(insets.left, insets.top);
		textEdit->moveCaretPosition(ui->viewToModel(textEdit, pos));
		textEdit->repaint();
	}
}

void
UBasicTextEditUI::installSignals(UTextEdit * textEdit) {
	textEdit->sigMousePressed().connect(slot(&focus_listener_slot));
	textEdit->sigMouseDragged().connect(slot(&mouse_selection_listener_slot));
}

void
UBasicTextEditUI::uninstallSignals(UTextEdit * textEdit) {
	textEdit->sigMouseDragged().disconnect(slot(&mouse_selection_listener_slot));
	textEdit->sigMousePressed().disconnect(slot(&focus_listener_slot));
}
