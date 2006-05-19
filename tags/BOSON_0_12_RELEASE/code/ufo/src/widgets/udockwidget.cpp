/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/udockwidget.cpp
    begin             : Tue Apr 5 2005
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

#include "ufo/widgets/udockwidget.hpp"

#include "ufo/layouts/uboxlayout.hpp"

using namespace ufo;

UFO_IMPLEMENT_CLASS(UDockWidget, UWidget)

UDockWidget::UDockWidget()
	: m_allowedAreas(AllDockWidgetAreas)
	, m_isFloating(false)
{
	setLayout(new UBoxLayout(0, 0));
	setBorder(BottomLineBorder);
}

void
UDockWidget::setAllowedAreas(DockWidgetArea areas) {
	m_allowedAreas = areas;
}

DockWidgetArea
UDockWidget::getAllowedAreas() const {
	return m_allowedAreas;
}


bool
UDockWidget::isFloating() const {
	return m_isFloating;
}

void
UDockWidget::setFloating(bool b) {
	m_isFloating = b;
}
