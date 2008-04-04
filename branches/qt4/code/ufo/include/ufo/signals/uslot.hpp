/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/signals/uslot.hpp
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

#ifndef USLOT_HPP
#define USLOT_HPP

// ripped from sigc++/slot.h (created by m4 macro)
// Copyright 2000, Karl Einar Nelson

#include "../ufo_global.hpp"
#include "../utrait.hpp"
#include "../usharedptr.hpp"

namespace ufo {

// implementation
typedef void (*ProxyPtr)(void*);
typedef void (*FuncPtr)(void*);

struct UFO_EXPORT USlotNode {
	USlotNode() : m_died(false) {}
	USlotNode(FuncPtr proxy) : _proxy(proxy), m_died(false) {}
	virtual ~USlotNode() {}

	// message from child that it has died and we should start
	// our shut down.  If from_child is true, we do not need
	// to clean up the child links.
	virtual void notify(bool /* from_child */) {
		m_died = true;
	}
	virtual bool died() { return m_died; }

	virtual bool equals(const USlotNode * /* node */) const { return false; }

	bool connected() { return _proxy != NULL; }

public: // Public attributes
	/** A proxy func which does the actual callback calling.
	  * FIXME: make this private.
	  */
	ProxyPtr _proxy;
private: // Private attributes
	bool m_died;
};

/** Base class for slot templates.
  * @author Johannes Schmidt
  */

class UFO_EXPORT USlotBase {
public:
	USlotBase() : _node(0) {}
	USlotBase(USlotNode * node) : _node(node) {}
	~USlotBase() {}

	/** returns weak pointer */
	USlotNode * node() { return _node; }
	const USlotNode * node() const { return _node; }

	bool operator ==(const USlotBase & slot) const {
		//return (*_impl == *(slot._impl));
		return _node->equals(slot._node);
	}
	bool equals(const USlotBase * slot) const {
		return _node->equals(slot->_node);
	}

	void assign(USlotNode * node) {
		_node = node;
	}

private:
	/** shared implementation */
	USharedPtr<USlotNode> _node;
};


//
// only void returns are supported
//

class UFO_EXPORT USlot0 : public USlotBase {
public:
	//typedef typename Trait<R>::type RType;
	typedef void (*Callback)();
	typedef void (*Proxy)(void*);

	void operator()() {
		if (!node()) return;
		//if (!node_->valid()) { clear(); return RType(); }
		((Proxy)(node()->_proxy))(node());
	}

	USlot0(USlotNode * node = NULL) : USlotBase(node) {}
};

template <typename P1>
class UFO_EXPORT USlot1 : public USlotBase {
public:
	typedef void (*Proxy)(typename UTrait<P1>::ref, void*);

	void operator ()(typename UTrait<P1>::ref p1) {
		if (!node()) return;
		//if (!node_->valid()) { clear(); return RType(); }
		((Proxy)(node()->_proxy))(p1, node());
	}

	USlot1(USlotNode * node = NULL) : USlotBase(node) {}
};

template <typename P1, typename P2>
class UFO_EXPORT USlot2 : public USlotBase {
public:
	typedef void (*Proxy)(typename UTrait<P1>::ref, typename UTrait<P2>::ref, void*);

	void operator ()(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2) {
		if (!node()) return;
		//if (!node_->valid()) { clear(); return RType(); }
		((Proxy)(node()->_proxy))(p1, p2, node());
	}

	USlot2(USlotNode * node = NULL) : USlotBase(node) {}
};


template <typename P1, typename P2, typename P3>
class UFO_EXPORT USlot3 : public USlotBase {
public:
	typedef void (*Proxy)(typename UTrait<P1>::ref, typename UTrait<P2>::ref,
		typename UTrait<P3>::ref, void*);

	void operator ()(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3) {
		if (!node()) return;
		//if (!node_->valid()) { clear(); return RType(); }
		((Proxy)(node()->_proxy))(p1, p2, p3, node());
	}

	USlot3(USlotNode * node = NULL) : USlotBase(node) {}
};


template <typename P1, typename P2, typename P3, typename P4>
class UFO_EXPORT USlot4 : public USlotBase {
public:
	typedef void (*Proxy)(typename UTrait<P1>::ref, typename UTrait<P2>::ref,
		typename UTrait<P3>::ref, typename UTrait<P4>::ref, void*);

	void operator ()(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3, typename UTrait<P4>::ref p4) {
		if (!node()) return;
		//if (!node_->valid()) { clear(); return RType(); }
		((Proxy)(node()->_proxy))(p1, p2, p3, p4, node());
	}

	USlot4(USlotNode * node = NULL) : USlotBase(node) {}
};

} // namespace ufo

#endif // USLOT_HPP
