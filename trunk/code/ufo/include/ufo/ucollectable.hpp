/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ucollectable.hpp
    begin             : Sat Jan 26 2002
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

#ifndef UCOLLECTABLE_HPP
#define UCOLLECTABLE_HPP

#include <cstddef>

#include "ufo_global.hpp"

namespace ufo {

/**
  * @short Base class for all garbage collected objects.
  * @ingroup core
  *
  * This class simply adds a reference counter and deletes the object
  * if it was created dynamically and the reference count drops to 0.
  * <p>
  * An instance is created dynamically if it was created with
  * the new operator.
  * <em>Please note:</em>
  * The reference count starts with a reference count, that means no
  * garbage collection activated.
  *
  * @author Johannes Schmidt
  */

//class UFO_EXPORT UCollectable : public gc_cleanup {
//};
class UFO_EXPORT UCollectable {
public:
	UCollectable();
	virtual ~UCollectable();

	void setDynamic(bool b);
	bool isDynamic();

	void reference() const;
	void unreference() const;

	unsigned int getReferenceCount() const;

	// FIXME
	// hm, evil
	void * operator new(std::size_t size);
	inline void operator delete(void * p, std::size_t /* size */);
private:
	mutable unsigned int m_refCount  : 31;
	unsigned int m_isDynamic :  1;
	static bool m_dynamicAlloc;
};

//
// inline implementation
//


inline void
UCollectable::setDynamic(bool b) { m_isDynamic = b; }

inline bool
UCollectable::isDynamic() { return m_isDynamic; }

inline void
UCollectable::reference() const { m_refCount++; }

inline void
UCollectable::unreference() const {
	if (m_refCount) { // do nothing if ref() has never been called
		m_refCount--;
		if (m_refCount == 0 && m_isDynamic) { delete this; }
	}
}

inline unsigned int
UCollectable::getReferenceCount() const {
	return m_refCount;
}

inline void
UCollectable::operator delete(void * p, std::size_t /* size */) {
	::operator delete(p);
}

} // namespace ufo

#endif // UCOLLECTABLE_HPP
