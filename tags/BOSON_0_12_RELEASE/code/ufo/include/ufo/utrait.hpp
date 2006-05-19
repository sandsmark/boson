/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/utrait.hpp
    begin             : Thu Jul 25 2002
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

#ifndef UTRAIT_HPP
#define UTRAIT_HPP

namespace ufo {

// thanks to libsigc++ (http://libsigc.sf.net)
// gratefully taken from trait.h

#undef UFO_SPECIALIZE_REFERENCES
// nock - 2002-10-06
// Using non-const references does not accept temporarily created objects.
// As all UFO signals use pointers or basic types
// copying shouldn't be a problem

#ifdef UFO_SPECIALIZE_REFERENCES

template <typename T>
struct UTrait {
	typedef T type;
	typedef T& ref;
	//typedef T* ptr;
};

// FIXME
// this needs partial template specialization
template <typename T>
struct UTrait<T&> {
	typedef T& type;
	typedef T& ref;
	//typedef T* ptr;
};

template <typename T>
struct UTrait<T*> {
	typedef T* type;
	typedef T* ref;
	//typedef T* ptr;
};

#else

// for really dumb compilers, we have to copy rather than reference
template <class T>
struct UTrait {
	typedef T  type;
	typedef T  ref;
};

#endif // UFO_SPECIALIZE_REFERENCES

} // namespace ufo

#endif // UTRAIT_HPP
