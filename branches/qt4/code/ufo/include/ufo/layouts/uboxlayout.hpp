/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/layouts/upopuplayout.hpp
    begin             : Thu May 31 2001
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

#ifndef UBOXLAYOUT_HPP
#define UBOXLAYOUT_HPP

#include "ulayoutmanager.hpp"

namespace ufo {

/**
  * @short This layout manager lays out all children in horizontal or vertical
  *  orientation
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBoxLayout : public ULayoutManager {
	UFO_DECLARE_DYNAMIC_CLASS(UBoxLayout)
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
	UBoxLayout();
	/** Creates a new box layout with the given orientation.
	  * Vertical and horizontal spacing between child
	  * widgets is 2.
	  */
	UBoxLayout(int orientation);
	UBoxLayout(int hgap, int vgap);
	UBoxLayout(int orientation, int hgap, int vgap);
	virtual ~UBoxLayout();

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

#endif // UBOXLAYOUT_HPP
