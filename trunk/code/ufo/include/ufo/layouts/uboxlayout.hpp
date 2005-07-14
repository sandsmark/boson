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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UBOXLAYOUT_HPP
#define UBOXLAYOUT_HPP

#include "ulayoutmanager.hpp"

namespace ufo {

/**
  * @short This layout manager lays out all children in one direction: XAxis or YAxis
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UBoxLayout : public ULayoutManager {
	UFO_DECLARE_DYNAMIC_CLASS(UBoxLayout)
public:  // Public attributes
	enum {
		XAxis,
		YAxis
	};

public:
	UBoxLayout();
	UBoxLayout(int axis);
	UBoxLayout(int hgap, int vgap);
	UBoxLayout(int axis, int hgap, int vgap);
	virtual ~UBoxLayout();

public: // Implements ULayoutManager
	virtual void layoutContainer(const UWidget * parent);
	virtual UDimension getPreferredLayoutSize(const UWidget * parent,
		const UDimension & maxSize) const;

protected:  // Protected attributes
	/**  */
	int m_axis;
	/**  horizontal gap between widgets*/
	int m_hgap;
	/**  vertical gap between widgets*/
	int m_vgap;
};

} // namespace ufo

#endif // UBOXLAYOUT_HPP