/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/usharedptr.hpp
    begin             : Thu Aug 22 2002
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

#ifndef USHAREDPTR_HPP
#define USHAREDPTR_HPP

#include "config/ufo_config.hpp"

namespace ufo {

/** Shared reference counting smart pointer.
  *
  * @author Johannes Schmidt
  */
template<typename T>
class USharedPtr {
private:
	T * m_object; // shared object
	int * m_refCount; // shared counter

public:
	explicit USharedPtr() : m_object(NULL), m_refCount(NULL) {}

	explicit USharedPtr(const USharedPtr<T> & right) {
		if (right.m_object) {
			m_object = right.m_object;
			m_refCount = right.m_refCount;
			++*m_refCount;
		} else {
			m_object = NULL;
			m_refCount = NULL;
		}
	}
	
	explicit USharedPtr(T * right) : m_object(NULL), m_refCount(NULL) {
		assign(right);
	}
	/*
	explicit USharedPtr(const T * right) : m_object(NULL), m_refCount(NULL) {
		assign(const_cast<T*>(right));
	}
	*/
	~USharedPtr() {
		release();
	}

	USharedPtr<T>& operator =(const USharedPtr<T>& right) {
		if (right.m_object) {
			++*right.m_refCount;
		}

		release();

		m_object = right.m_object;
		m_refCount = right.m_refCount;

		return *this;
	}

	USharedPtr<T>& operator =(T* right) {
		assign(right);

		return *this;
	}
	/*
	USharedPtr<T>& operator =(const T* right) {
		assign(const_cast<T*>(right));

		return *this;
	}
	*/
	bool valid() {
		return (m_object != NULL);
	}

	T * operator ->() const {
		return m_object;
	}

	T & operator *() const {
		return *m_object;
	}

	operator T*() const {
		return m_object;
	}

	operator T() const {
		return *m_object;
	}

	int refCount() {
		return *m_refCount;
	}
private: // Private methods
	void assign(T * t) {
		if (t == m_object) { return; }

		release();
		if (t) {
			m_object = t;
			m_refCount = new int;
			*m_refCount = 1;
		}
	}
	void release() {
		if (m_object) {
			if (*m_refCount == 1) {
				delete m_refCount;
				delete m_object;
			} else {
				--*m_refCount;
			}
			m_object = NULL;
			m_refCount = NULL;
		}
	}
};

} // namespace ufo

#endif // USHAREDPTR_HPP
