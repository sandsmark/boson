/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/utexteditui.cpp
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


#include "ufo/ui/utexteditui.hpp"

#include "ufo/util/upoint.hpp"
#include "ufo/util/urectangle.hpp"

#include "ufo/widgets/utextedit.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UTextEditUI, UWidgetUI)


URectangle
UTextEditUI::modelToView(const UTextEdit * textA, int offsA) const {
	URectangle ret = textA->getRenderer()->
		modelToView(textA->getDocument(), offsA, textA->getFont());

	//const UInsets & in = textA->getInsets();
	//ret.x += in.left;
	//ret.y += in.top;

	return ret;
}


int
UTextEditUI::viewToModel(const UTextEdit * textA, const UPoint & posA) const {
//	const UInsets & in = textA->getInsets();

//	UPoint modelPos = posA;
//	modelPos.translate(UPoint(-in.left, -in.left));

	return textA->getRenderer()->
		viewToModel(textA->getDocument(), posA, textA->getFont());
}
