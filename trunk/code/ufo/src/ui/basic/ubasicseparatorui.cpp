/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasicseparatorui.cpp
    begin             : Mon Jul 22 2002
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

#include "ufo/ui/basic/ubasicseparatorui.hpp"

//#include "ufo/ufo_gl.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/ui/uuimanager.hpp"
#include "ufo/widgets/useparator.hpp"

#include "ufo/util/ucolor.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicSeparatorUI, USeparatorUI)

UBasicSeparatorUI * UBasicSeparatorUI::m_separatorUI = new UBasicSeparatorUI();

std::string UBasicSeparatorUI::m_lafId("USeparator");


UBasicSeparatorUI *
UBasicSeparatorUI::createUI(UWidget * w) {
	return m_separatorUI;
}

void
UBasicSeparatorUI::paint(UGraphics * g, UWidget * w) {
	USeparator * separator = dynamic_cast<USeparator *>(w);
	const UDimension & dim = separator->getSize();

	//const float * fg = w->getForegroundColor()->getFloat();
	//const float * bg = w->getBackgroundColor()->getFloat();
/*
	if (separator->getOrientation() == Horizontal) {
		glBegin(GL_LINES);
		glColor3fv(bg);
		glVertex2i(0, 1);
		glVertex2i(dim.w, 1);
		
		glColor3fv(fg);
		glVertex2i(dim.w, 2);
		glVertex2i(1, 2);
		glEnd();
	} else {
		glBegin(GL_LINES);
		glColor3fv(bg);
		glVertex2i(0, 0);
		glVertex2i(0, dim.h);

		glColor3fv(fg);
		glVertex2i(1, dim.h);
		glVertex2i(1, 1);
		glEnd();
	}
*/
	if (separator->getOrientation() == Horizontal) {
		g->setColor(w->getColorGroup().background());//->getBackgroundColor());
		g->drawLine(0, 0, dim.w, 0);
		
		g->setColor(w->getColorGroup().foreground());//->getForegroundColor());
		g->drawLine(dim.w, 1, 1, 1);
	} else {
		g->setColor(w->getColorGroup().background());//->getBackgroundColor());
		g->drawLine(0, 0, 0, dim.h);

		g->setColor(w->getColorGroup().foreground());//->getForegroundColor());
		g->drawLine(1, dim.h, 1, 1);
	}
}


const std::string &
UBasicSeparatorUI::getLafId() {
	return m_lafId;
}


UDimension
UBasicSeparatorUI::getPreferredSize(const UWidget * w) {
	const USeparator * separator = dynamic_cast<const USeparator *>(w);
	if (separator->getOrientation() == Horizontal) {
		return UDimension(0, 2);
	} else {
		return UDimension(2, 0);
	}
}
