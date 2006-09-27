/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uvertexarray.hpp
    begin             : Tue 26 Apr 2005
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

#ifndef UVERTEXARRAY
#define UVERTEXARRAY

#include "uobject.hpp"

#include "util/ucolor.hpp"

#include <vector>

namespace ufo {

/** An abstraction for vertex arrays with otional colors.
  * Warning: This API may change and is not yet stable
  * @author Johannes Schmidt
  */
class UFO_EXPORT UVertexArray : public UObject {
	UFO_DECLARE_CLASS(UVertexArray)
public:
	/** This type is used to determine the array returned
	  * by getArray.
	  * @see getArray
	  */
	enum Type {
		NoType = 0,
		/** 3 floats for vertices. */
		V3F,
		/** 3 floats for color and 3 floats for vertices. */
		C3F_V3F,
		/** argb color and 3 floats for vertices */
		CARGB_V3F,
		/** argb color and 3 floats for vertices */
		CRGBA_V2F
	};
public:
	UVertexArray();
	virtual ~UVertexArray();

public:
	/** Sets an offset for all following points (not yet implemented). */
	void setOffset(float x, float y);
	/** Adds a new vertex to the array. */
	void add(float x, float y);

	/** Sets a new color for all following vertices. */
	void setColor(const UColor & col);

	/** @return The total count of vertices. */
	int getCount() const;

	/** @return The type of vertices. 
	  * @see getArray
	  */
	int getType() const;
	/** Explicitely sets the type of the vertices. This effects the output
	  * of getArray.
	  * @see getArray
	  */
	void setType(Type t);
	/** @return A void pointer array of the vertices in the given type.
	  *  The returned array is automatically freed by UVertexArray on
	  *  destruction or if a new type was set.
	  */
	void * getArray();
	
	/** @return A vector with plain coordinates of the vertices. */
	std::vector<std::pair<float, float> > getVertices() const;
	/** @return A vector with colors for each vertex. */
	std::vector<UColor> getColors() const;

protected: // Protected methods
	/** Deletes allocated memory for the returned vertex array. */
	void dispose();

private:
	Type m_type;
	std::vector<std::pair<float, float> > m_vertices;
	std::vector<UColor> m_colors;
	void * m_array;
};

} // namespace ufo

#endif // UVERTEXARRAY
