/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

#include "ubosonstyle.h"
#include "ubolabelui.h"
#include <ufo/util/udimension.hpp>
#include <ufo/umodel.hpp>

using namespace ufo;

UBosonStyle::UBosonStyle()
	: UBasicStyle() {
}

UBosonStyle::~UBosonStyle() {
}

void UBosonStyle::paintComponent(UGraphics * g,
		ComponentElement elem,
		const URectangle & rect,
		const UStyleHints * hints,
		const UWidgetModel * model,
		UWidget * w) {
	switch (elem) {
		case CE_Label:
		{
#if 0
			// AB: libufo code
			paintPrimitive(g, PE_PanelWidget, rect, hints, model->widgetState);
			paintComponent(g, CE_Compound, rect - insets, hints, model);
#if 0
			// AB: the paintComponent() call causes this code:
			const UCompoundModel * compound = static_cast<const UCompoundModel*>(model);
			paintCompound(g, hints, compound->text, compound->icon, rect, model->widgetState, compound->acceleratorIndex);
#endif
#else
			paintPrimitive(g, PE_PanelWidget, rect, hints, model->widgetState);

			const UCompoundModel * compound = static_cast<const UCompoundModel*>(model);
			paintBoUfoLabel(g, hints, compound->text, compound->icon, rect, model->widgetState, compound->acceleratorIndex);


#endif
			break;
		}
		default:
		{
			UBasicStyle::paintComponent(g, elem, rect, hints, model, w);
			break;
		}
	}
}

void UBosonStyle::paintBoUfoLabel(
		UGraphics * g,
		const UStyleHints * hints,
		const std::string & text,
		UIcon * icon,
		const URectangle & rect,
		uint32_t widgetState,
		int acceleratorIndex) {
	UBoLabelUI ui;
	ui.paint(g, hints, text, icon, rect, widgetState, acceleratorIndex);
}


