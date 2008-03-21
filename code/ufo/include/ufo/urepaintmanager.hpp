/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/urepaintmanager.hpp
    begin             : Fri Nov 1 2002
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

#ifndef UREPAINTMANAGER_HPP
#define UREPAINTMANAGER_HPP

#include "uobject.hpp"

#include "util/urectangle.hpp"

namespace ufo {

class UWidget;
class UGraphics;

/** @short The repaint manager is a very simple controller which keeps track
  *  of the union of all dirty regions.
  * @ingroup drawing
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT URepaintManager : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(URepaintManager)
public:
	URepaintManager();
	virtual ~URepaintManager();

	/** Adds the specified subrectangle of the given widget to be dirty. */
	virtual void addDirtyRegion(UWidget * widget, int x, int y, int w, int h);
	/** Marks the entire widget to be dirty. */
	virtual void addDirtyWidget(UWidget * widget);

	/** Returns the unioned dirty region for the whole context. */
	virtual URectangle getDirtyRegion() const;

	/** In general, this method repaints the whole buffer.
	  * This method may be overriden to repaint only parts of the given
	  * buffer.
	  */
	//virtual void paintDirtyRegions(UGraphics * g);
	/** Returns true when there are dirty regions. */
	virtual bool isDirty() const;

	/** Cleares any marked regions. */
	virtual void clearDirtyRegions();
	/** Cleares any marked regions. */
	virtual void clearDirtyRegion(UWidget * widget, int x, int y, int w, int h);

private: // Private attributes
	URectangle m_dirtyRegion;
};

} // namespace ufo

#endif // UREPAINTMANAGER_HPP
