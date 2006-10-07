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

// AB: large parts of this file are copied from libufo
// ufo/include/ufo/layouts/uboxlayout.hpp
// we reimplement this code because UBoxLayout is really too spartanic for us
// (and our code won't make it to official libufo cvs, according to the author)

#ifndef UBOBOXLAYOUT_H
#define UBOBOXLAYOUT_H

#include <ufo/layouts/ulayoutmanager.hpp>

namespace ufo {

/**
  * @short This layout manager lays out all children in horizontal or vertical
  *  orientation
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBoBoxLayout : public ULayoutManager {
	UFO_DECLARE_DYNAMIC_CLASS(UBoBoxLayout)
public:  // Public attributes
	enum {
		XAxis = Horizontal,
		YAxis = Vertical
	};

public:
	/** Creates a new box layout which uses the orientation of the container
	  * widget as orientation. Vertical and horizontal spacing between child
	  * widgets is 2.
	  */
	UBoBoxLayout();
	/** Creates a new box layout with the given orientation.
	  * Vertical and horizontal spacing between child
	  * widgets is 2.
	  */
	UBoBoxLayout(int orientation);
	UBoBoxLayout(int hgap, int vgap);
	UBoBoxLayout(int orientation, int hgap, int vgap);
	virtual ~UBoBoxLayout();

public: // Implements ULayoutManager
	virtual void layoutContainer(const UWidget * container);
	virtual UDimension getPreferredLayoutSize(const UWidget * container,
		const UDimension & maxSize) const;

protected: // Protected methods
	virtual int getTotalFlex(const UWidget * container);
protected:  // Protected attributes
	/**  */
	int m_orientation;
	/**  horizontal gap between widgets*/
	int m_hgap;
	/**  vertical gap between widgets*/
	int m_vgap;
};

} // namespace ufo

#endif // UBOBOXLAYOUT_H
