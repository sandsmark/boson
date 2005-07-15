/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ugroupbox.cpp
    begin             : Fri May 27 2005
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

#include "ufo/widgets/ugroupbox.hpp"

#include "ufo/umodel.hpp"

using namespace ufo;

UFO_IMPLEMENT_CLASS(UGroupBox, UWidget)

UGroupBoxModel *
ufo_createGroupBoxModel(UWidgetModel * model, const std::string & text, UIcon * icon) {
	UGroupBoxModel * c = new UGroupBoxModel();
	c->widgetState = model->widgetState;
	c->icon = icon;
	c->text = text;
	c->acceleratorIndex = -1;
	delete (model);
	return c;
}


UGroupBox::UGroupBox() {
	m_model = ufo_createGroupBoxModel(m_model, "", NULL);
}
UGroupBox::UGroupBox(const std::string & title) {
	m_model = ufo_createGroupBoxModel(m_model, title, NULL);
}

void
UGroupBox::setTitle(const std::string & title) {
	(static_cast<UGroupBoxModel*>(m_model))->text = title;
}

std::string
UGroupBox::getTitle() const {
	return (static_cast<UGroupBoxModel*>(m_model))->text;
}
