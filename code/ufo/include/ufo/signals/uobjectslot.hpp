/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/signals/uobjectslots.hpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UOBJECTSLOTS_HPP
#define UOBJECTSLOTS_HPP

// ripped from sigc++/objectslot.h
// Copyright 2000, Karl Einar Nelson

#include "uslot.hpp"

namespace ufo {

class UObject;

// (internal)
struct UObjectSlotNode : public USlotNode {
	typedef void (UObject::*Method)(void);
	//typedef void (*Method)(void);

	void * m_object;
	Method m_method;

	UObjectSlotNode(ProxyPtr proxy, void * object, Method method)
	: USlotNode(proxy) {
		m_object = object;
		m_method = method;
	}

	virtual bool equals(const USlotNode * node) const {
		if (const UObjectSlotNode * onode = dynamic_cast<const UObjectSlotNode*>(node)) {
			return ((m_object == onode->m_object) &&
				(m_method == onode->m_method));/* &&
				(_proxy == onode->_proxy));*/
		} else {
			return false;
		}
	}

	virtual ~UObjectSlotNode() {}
};


//
// 0 params
//

template <typename Obj>
struct UObjectSlot0 {
	typedef void (Obj::*Method)();

	static void proxy(void * s) {
		UObjectSlotNode* os = (UObjectSlotNode*) s;
		((Obj*)(os->m_object)->*(reinterpret_cast<Method>(os->m_method)))();
	}
};

template <typename Obj>
USlot0
slot(Obj & obj,void (Obj::*method)()) {
	typedef UObjectSlot0<Obj> SType;

	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}

template <typename Obj>
USlot0
slot(Obj & obj,void (Obj::*method)() const) {
	typedef UObjectSlot0<Obj> SType;

	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}


//
// 1 param
//

template <typename P1,typename Obj>
struct UObjectSlot1 {
	typedef void (Obj::*Method)(P1);

	static void proxy(typename UTrait<P1>::ref p1,void * s) {
		UObjectSlotNode* os = (UObjectSlotNode*)s;
		((Obj*)(os->m_object)->*(reinterpret_cast<Method>(os->m_method)))(p1);
	}
};

template <typename P1, typename Obj>
USlot1<P1>
slot(Obj & obj,void (Obj::*method)(P1)) {
	typedef UObjectSlot1<P1,Obj> SType;
	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}

template <typename P1,typename Obj>
USlot1<P1>
slot(Obj & obj,void (Obj::*method)(P1) const) {
	typedef UObjectSlot1<P1,Obj> SType;
	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}

//
// 2 params
//


template <typename P1, typename P2, typename Obj>
struct UObjectSlot2 {
	typedef void (Obj::*Method)(P1, P2);

	static void proxy(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2,void * s) {
		UObjectSlotNode* os = (UObjectSlotNode*)s;
		((Obj*)(os->m_object)->*(reinterpret_cast<Method>(os->m_method)))(p1, p2);
	}
};

template <typename P1, typename P2, typename Obj>
USlot2<P1, P2>
slot(Obj & obj,void (Obj::*method)(P1, P2)) {
	typedef UObjectSlot2<P1, P2, Obj> SType;
	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}

template <typename P1, typename P2, typename Obj>
USlot2<P1, P2>
slot(Obj & obj,void (Obj::*method)(P1, P2) const) {
	typedef UObjectSlot2<P1, P2, Obj> SType;
	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}


//
// 3 params
//


template <typename P1, typename P2, typename P3, typename Obj>
struct UObjectSlot3 {
	typedef void (Obj::*Method)(P1, P2, P3);

	static void proxy(
			typename UTrait<P1>::ref p1,
			typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3,
			void * s) {
		UObjectSlotNode * os = (UObjectSlotNode*)s;
		((Obj*)(os->m_object)->*(reinterpret_cast<Method>(os->m_method)))(p1, p2, p3);
	}
};

template <typename P1, typename P2, typename P3, typename Obj>
USlot3<P1, P2, P3>
slot(Obj & obj, void (Obj::*method)(P1, P2, P3)) {
	typedef UObjectSlot3<P1, P2, P3, Obj> SType;
	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}

template <typename P1, typename P2, typename P3, typename Obj>
USlot3<P1, P2, P3>
slot(Obj & obj,void (Obj::*method)(P1, P2, P3) const) {
	typedef UObjectSlot3<P1, P2, P3, Obj> SType;
	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}


//
// 4 params
//


template <typename P1, typename P2, typename P3, typename P4, typename Obj>
struct UObjectSlot4 {
	typedef void (Obj::*Method)(P1, P2, P3, P4);

	static void proxy(
			typename UTrait<P1>::ref p1,
			typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3,
			typename UTrait<P4>::ref p4,
			void * s) {
		UObjectSlotNode * os = (UObjectSlotNode*)s;
		((Obj*)(os->m_object)->*(reinterpret_cast<Method>(os->m_method)))(p1, p2, p3, p4);
	}
};

template <typename P1, typename P2, typename P3, typename P4, typename Obj>
USlot4<P1, P2, P3, P4>
slot(Obj & obj, void (Obj::*method)(P1, P2, P3, P4)) {
	typedef UObjectSlot4<P1, P2, P3, P4, Obj> SType;
	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}

template <typename P1, typename P2, typename P3, typename P4, typename Obj>
USlot4<P1, P2, P3, P4>
slot(Obj & obj,void (Obj::*method)(P1, P2, P3, P4) const) {
	typedef UObjectSlot4<P1, P2, P3, P4, Obj> SType;
	return new UObjectSlotNode((FuncPtr)(&SType::proxy),
		&obj,
		reinterpret_cast<UObjectSlotNode::Method>(method));
}

} // namespace ufo

#endif // UOBJECTSLOTS_HPP
