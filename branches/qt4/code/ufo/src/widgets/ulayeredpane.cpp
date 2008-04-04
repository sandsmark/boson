/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ulayeredpane.cpp
    begin             : Mon Aug 13 2001
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/widgets/ulayeredpane.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(ULayeredPane, UWidget)

const UInteger * ULayeredPane::DefaultLayer = new UInteger(0);
const UInteger * ULayeredPane::FrameLayer = new UInteger(100);
const UInteger * ULayeredPane::PopupLayer = new UInteger(200);
const UInteger * ULayeredPane::DragLayer = new UInteger(300);
const UInteger * ULayeredPane::RootPaneLayer = new UInteger(-500000);

const std::string ULayeredPane::LayerProperty = "layer";

ULayeredPane::ULayeredPane() : UWidget() {
	// layered panes don´t have layouts
	setLayout(NULL);
}


void
ULayeredPane::addImpl(UWidget * w, UObject * constraints, int index) {
	int layer;

	if (UInteger * intObj = dynamic_cast<UInteger *>(constraints) ) {
		layer = intObj->toInt();
	} else {
		layer = DefaultLayer->toInt();
	}

	putLayerProperty(w, layer);

	int position = indexForLayer(layer, index);

	UWidget::addImpl(w, NULL, position);
}


void
ULayeredPane::putLayerProperty(UWidget * w, int layer) {
	UInteger * newLayer = new UInteger(layer);

	w->put(LayerProperty, newLayer);
}

void
ULayeredPane::setLayer(UWidget * w, int layer) {
	setLayer(w, layer, -1);
}

void
ULayeredPane::setLayer(UWidget * w, int layer, int position) {
	if ( (layer == getLayer(w)) && (position == getPosition(w)) ) {
		// no change
		return ;
	}

	putLayerProperty(w, layer);
	// at first, move the widget to the very end of the container.
	// this way, we do not have to factor in the case when we move
	// within the same layer (might give wrong results)
	// Note: This violates the order of child widgets
	//setIndexOf(w, -1);
	int index = indexForLayer(layer, position);
	setIndexOf(w, index);
}

int
ULayeredPane::getLayer(const UWidget * w) const {
	if (UInteger * ret = dynamic_cast<UInteger *>(w->get(LayerProperty))) {
		return ret->toInt();
	}
	return DefaultLayer->toInt();
}


void
ULayeredPane::setPosition(UWidget * w, int position) {
	setLayer(w, getLayer(w), position);
}

int
ULayeredPane::getPosition(const UWidget * w) const {
	int index = getIndexOf(w);
	int layerBegin = getLayerBegin(getLayer(w));

	return index - layerBegin;
}


void
ULayeredPane::moveToFront(UWidget * w) {
	setPosition(w, 0);
}

void
ULayeredPane::moveToBack(UWidget * w) {
	int layer = getLayer(w);
	// the number of widgets in this layer
	int count = getLayerEnd(layer) - getLayerBegin(layer);

	setPosition(w, count);
}

int
ULayeredPane::getLayerBegin(int layer) const {
	int layerBegin = -1;
	int current;

	int nWidgets = getWidgetCount();
	for (int i = 0;i < nWidgets;i++) {
		current = getLayer(getWidget(i));

		if ( layer >= current) {
			layerBegin = i;
			break;
		}
	}
	if (layerBegin == -1)
		layerBegin = nWidgets;
	return layerBegin;
}


int
ULayeredPane::getLayerEnd(int layer) const {
	int layerEnd = -1;
	int current;

	int nWidgets = getWidgetCount();
	for (int i = 0;i < nWidgets;i++) {
		current = getLayer(getWidget(i));

		// DEBUG
		//if (current > 500000)
		//	std::cerr << "WARNING :: property was garbage collected\n";
		// end DEBUG

		if (layer > current) {
			layerEnd = i;
			break;
		}
	}
	// this happens when we have only one layer
	if (layerEnd == -1) {
		layerEnd = nWidgets;
	}
	return layerEnd;
}

int
ULayeredPane::indexForLayer(int layer, int position) const {
	int layerBegin = getLayerBegin(layer);
	int layerEnd = getLayerEnd(layer);

	int ret = 0;

	if (position > -1 && position <= layerEnd - layerBegin) {
		ret = layerBegin + position;
	} else {
		// insert at the end of the given layer
		ret = layerBegin;
	}

	// clamp
	if (layerEnd == layerBegin) {
		ret = layerEnd;
	} else if (ret > layerEnd) {
		ret = layerEnd;
	}

	//
	if (ret >= getWidgetCount()) {
		return -1;
	}

	// else, position == -1, ..
	return ret;
}
