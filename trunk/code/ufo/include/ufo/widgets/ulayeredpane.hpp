/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ulayeredpane.hpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef ULAYEREDPANE_HPP
#define ULAYEREDPANE_HPP

#include "uwidget.hpp"

#include "../util/uinteger.hpp"

namespace ufo {

/** @short A panel with different layers.
  * @ingroup internal
  *
  * Widgets in the highest layer are painted on top of the widgets in lower
  * layers. Widgets should be added with add( widget, layer, positionInLayer).
  * If the layer isn´t specified, the default layer is used.
  * example: <code>add( new ULabel("hello"), new UInteger(5), 0) </code>+
  *
    *@author Johannes Schmidt
  */

class UFO_EXPORT ULayeredPane : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(ULayeredPane)
public:
	ULayeredPane();

	/** Sets a property in the property of w
	  * @param w
	  *	The widget that should moved to the given layer
	  * @param layer
	  *	The new layer
	  */
	void putLayerProperty(UWidget * w, int layer);

	/** Sets new layer and position.
	  * Postion will be the end of the given layer.
	  */
	void setLayer(UWidget * w, int layer);
	/** Sets new layer and position.
	  * @param w
	  *	The widget that should moved to the given layer and position
	  * @param layer
	  *	The new layer
	  * @param position
	  *	The new position relative to the layer
	  */
	void setLayer(UWidget * w, int layer, int position);
	/**
	  * @return The layer property of the widget.
	  *	-1 if w isn´t a valid child of this layered pane.
	  */
	int getLayer(const UWidget * w) const;

	/**
	  *
	  */
	void setPosition(UWidget * w, int position);
	/**
	  * @return The relative position to its layer of the widget
	  */
	int getPosition(const UWidget * w) const;

	/** Moves widget w to the front of the child widget hierarchie.
	  */
	void moveToFront(UWidget * w);
	/** Moves widget w to the back of the child widget hierarchie.
	  */
	void moveToBack(UWidget * w);


	/** @return The (child vector) index at which a <strong>newly</strong>
	  *  inserted widget is the top most widget within that layer.
	  */
	int getLayerBegin(int layer) const;
	/** @return The (child vector) index at which a <strong>newly</strong>
	  *  inserted widget is the bottom most widget within that layer.
	  */
	int getLayerEnd(int layer) const;
	/** @return The (child vector) index for a widget which should be added,
	  *  relative to the whole children vector
	  * @param layer The desired layer
	  * @param position The new position relative to the layer.
	  *  Use -1 for appending
	  */
	int indexForLayer(int layer, int position) const;


protected:
	/** add a sub widget at the specified index
	  * @param w the widget, which should be added
	  * @param index if index is -1 or isn´t set, the widget will be
	  *   added at the end of the layer
	  * @param constraints the layer
	  * @see put
	 */
	void addImpl(UWidget * w, UObject * constraints, int index);


public:  // Public attributes
	static const UInteger * DefaultLayer;
	static const UInteger * FrameLayer;
	static const UInteger * PopupLayer;
	// FIXME
	// drag and drop is not supported
	static const UInteger * DragLayer;

	static const UInteger * RootPaneLayer;

private:
	static const std::string LayerProperty;
};

} // namespace ufo

#endif // ULAYEREDPANE_HPP
