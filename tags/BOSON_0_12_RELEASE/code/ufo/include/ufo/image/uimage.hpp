/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/image/uimage.hpp
    begin             : Sat Oct 11 2003
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

#ifndef UIMAGE_HPP
#define UIMAGE_HPP

#include "../udrawable.hpp"
#include "../uvolatiledata.hpp"

namespace ufo {

class UGraphics;
class UContextGroup;

/** @short A abstract image representation
  * @ingroup drawing
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UImage : public UDrawable, public UVolatileData {
	UFO_DECLARE_ABSTRACT_CLASS(UImage)
public:
	/** @return The size of the image. */
	virtual UDimension getImageSize() const = 0;

	/** @return The number of bytes used for one pixel. */
	virtual int getImageComponents() const = 0;

public: // Implements UVolatileData
	/** May be overriden for auto refreshing after context recreation. */
	virtual void refresh();

public: // Implements UDrawable
	virtual void paintDrawable(UGraphics * g, const URectangle & rect);
	virtual UDimension getDrawableSize() const;

public: // System dependent
	/** Returns the system dependent image peer object, if any.
	  * This may be SDL surface for SDL 2D backends.
	  * @return The system dependent image object or NULL.
	  */
	virtual void * peer() const { return NULL; }
	/** Returns the system dependent image handle, if any.
	  * This may be the OpenGL index for OpenGL backends.
	  * @return The system dependent image handle or 0.
	  */
	virtual unsigned long handle() const { return 0; }
};

} // namespace ufo

#endif // UIMAGE_HPP
