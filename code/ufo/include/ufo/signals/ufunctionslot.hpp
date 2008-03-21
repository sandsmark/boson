/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/signals/ufunctionslots.hpp
    begin             : Thu Jul 18 2002
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

// ripped from sigc++/slot.h
// Copyright 2000, Karl Einar Nelson

#ifndef UFUNCTIONSLOTS_HPP
#define UFUNCTIONSLOTS_HPP

#include "uslot.hpp"

namespace ufo {

struct UFuncSlotNode : public USlotNode {
	FuncPtr _func;
	UFuncSlotNode(ProxyPtr proxy, FuncPtr func) : USlotNode(proxy), _func(func) {}
	virtual ~UFuncSlotNode() {}

	virtual bool equals(const USlotNode * node) const {
		if (const UFuncSlotNode * fnode = dynamic_cast<const UFuncSlotNode*>(node)) {
			return (_func == fnode->_func);
		} else {
			return false;
		}
	}
};

//
// 0 params
//

struct UFuncSlot0 {
	typedef void (*Callback)();

	static void proxy(void *s) {
		((Callback)(((UFuncSlotNode*)s)->_func))();
	}
};

inline USlot0 slot(void (*func)()) {
	return new UFuncSlotNode((ProxyPtr) &UFuncSlot0::proxy, (FuncPtr) func);
}

//
// 1 param
//

template <typename P1>
struct UFuncSlot1 {
	typedef void (*Callback)(P1 p1);

	static void proxy(typename UTrait<P1>::ref p1, void *s) {
		((Callback)(((UFuncSlotNode*)s)->_func))(p1);
	}
};

template <typename P1>
USlot1<P1> slot(void (*func)(P1)) {
	return new UFuncSlotNode((ProxyPtr) &UFuncSlot1<P1>::proxy, (FuncPtr) func);
}


//
// 2 params
//

template <typename P1, typename P2>
struct UFuncSlot2 {
	typedef void (*Callback)(P1 p1, P2 p2);

	static void proxy(
			typename UTrait<P1>::ref p1,
			typename UTrait<P2>::ref p2,
			void *s) {
		((Callback)(((UFuncSlotNode*)s)->_func))(p1, p2);
	}
};

template <typename P1, typename P2>
USlot2<P1, P2> slot(void (*func)(P1, P2)) {
	return new UFuncSlotNode((ProxyPtr) &UFuncSlot2<P1, P2>::proxy, (FuncPtr) func);
}



//
// 3 params
//

template <typename P1, typename P2, typename P3>
struct UFuncSlot3 {
	typedef void (*Callback)(P1 p1, P2 p2, P3 p3);

	static void proxy(
			typename UTrait<P1>::ref p1,
			typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3,
			void *s) {
		((Callback)(((UFuncSlotNode*)s)->_func))(p1, p2, p3);
	}
};

template <typename P1, typename P2, typename P3>
USlot3<P1, P2, P3> slot(void (*func)(P1, P2, P3)) {
	return new UFuncSlotNode((ProxyPtr) &UFuncSlot3<P1, P2, P3>::proxy, (FuncPtr) func);
}


//
// 4 params
//

template <typename P1, typename P2, typename P3, typename P4>
struct UFuncSlot4 {
	typedef void (*Callback)(P1 p1, P2 p2, P3 p3, P4 p4);

	static void proxy(
			typename UTrait<P1>::ref p1,
			typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3,
			typename UTrait<P4>::ref p4,
			void *s) {
		((Callback)(((UFuncSlotNode*)s)->_func))(p1, p2, p3, p4);
	}
};

template <typename P1, typename P2, typename P3, typename P4>
USlot4<P1, P2, P3, P4> slot(void (*func)(P1, P2, P3, P4)) {
	return new UFuncSlotNode((ProxyPtr) &UFuncSlot4<P1, P2, P3, P4>::proxy, (FuncPtr) func);
}

} // namespace ufo

#endif // UFUNCTIONSLOTS_HPP
