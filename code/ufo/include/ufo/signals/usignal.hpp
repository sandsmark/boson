/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/signals/usignal.hpp
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

#ifndef USIGNAL_HPP
#define USIGNAL_HPP

// ripped from sigc++/signal.h (created by m4 macro)
// Copyright 2000, Karl Einar Nelson

//#include "uslotbase.h"
#include "uslot.hpp"

#include <list>

namespace ufo {

/**
  *@author Johannes Schmidt
  */

class USignalBase {
public:
	USignalBase() {}
	~USignalBase() {
		for (SlotIterator iter = _slots.begin();
				iter != _slots.end();
				++iter) {
			delete (*iter);
		}
		_slots.clear();
	}

public:
	void push_front(USlotBase * slot) {
		_slots.push_front(slot);
	}
	bool remove(const USlotBase * slot) {
		bool ret = false;
		for (SlotIterator iter = _slots.begin();
				iter != _slots.end();
				++iter) {
			if ((*iter)->equals(slot)) {
				(*iter)->node()->notify(false);
				ret = true;
				break;
			}
		}
		return ret;
	}
protected:
	bool final_remove(const USlotBase * slot) {
		bool ret = false;
		for (SlotIterator iter = _slots.begin();
				iter != _slots.end();
				++iter) {
			if ((*iter)->equals(slot)) {
				// remove garbage
				delete (*iter);
				_slots.erase(iter);
				ret = true;
				break;
			}
		}
		return ret;
	}
protected:
	std::list<USlotBase*> _slots;
	typedef std::list<USlotBase*>::iterator SlotIterator;
};

//
// signals
//


class USignal0 : public USignalBase {
public:
	typedef USlot0 InSlotType;

	void connect(const InSlotType & s) {
		push_front(new InSlotType(s));
	}
	bool disconnect(const InSlotType & s) {
		return remove(&s);
	}

	void operator()() {
		emit();
	}
	void emit() {
		for (SlotIterator iter = _slots.begin();iter != _slots.end();) {
			InSlotType * slot = static_cast<InSlotType*>(*iter);
			if (!slot->node()->died()) {
				(*slot)();
			}
			++iter;
			if (slot->node()->died()) {
				final_remove(slot);
			}
		}
	}
};

template <typename P1>
class USignal1 : public USignalBase {
public:
	typedef USlot1<P1> InSlotType;

	void connect(const InSlotType & s) {
		push_front(new InSlotType(s));
	}
	bool disconnect(const InSlotType & s) {
		return remove(&s);
	}

	void operator()(typename UTrait<P1>::ref p1) {
		emit(p1);
	}
	void emit(typename UTrait<P1>::ref p1) {
		for (SlotIterator iter = _slots.begin();iter != _slots.end();) {
			InSlotType * slot = static_cast<InSlotType*>(*iter);
			if (!slot->node()->died()) {
				(*slot)(p1);
			}
			++iter;
			if (slot->node()->died()) {
				final_remove(slot);
			}
		}
	}
};


template <typename P1,typename P2>
class USignal2 : public USignalBase {
public:
	typedef USlot2<P1, P2> InSlotType;

	void connect(const InSlotType & s) {
		push_front(new InSlotType(s));
	}
	bool disconnect(const InSlotType & s) {
		return remove(&s);
	}

	void operator()(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2) {
		emit(p1, p2);
	}
	void emit(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2) {
		for (SlotIterator iter = _slots.begin();iter != _slots.end();) {
			InSlotType * slot = static_cast<InSlotType*>(*iter);
			if (!slot->node()->died()) {
				(*slot)(p1, p2);
			}
			++iter;
			if (slot->node()->died()) {
				final_remove(slot);
			}
		}
	}
};


template <typename P1, typename P2, typename P3>
class USignal3 : public USignalBase {
public:
	typedef USlot3<P1, P2, P3> InSlotType;

	void connect(const InSlotType & s) {
		push_front(new InSlotType(s));
	}
	bool disconnect(const InSlotType & s) {
		return remove(&s);
	}

	void operator()(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3) {
		emit(p1, p2, p3);
	}
	void emit(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3) {
		for (SlotIterator iter = _slots.begin();iter != _slots.end();) {
			InSlotType * slot = static_cast<InSlotType*>(*iter);
			if (!slot->node()->died()) {
				(*slot)(p1, p2, p3);
			}
			++iter;
			if (slot->node()->died()) {
				final_remove(slot);
			}
		}
	}
};

template <typename P1, typename P2, typename P3, typename P4>
class USignal4 : public USignalBase {
public:
	typedef USlot4<P1, P2, P3, P4> InSlotType;

	void connect(const InSlotType & s) {
		push_front(new InSlotType(s));
	}
	bool disconnect(const InSlotType & s) {
		return remove(&s);
	}

	void operator()(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3, typename UTrait<P4>::ref p4) {
		emit(p1, p2, p3, p4);
	}
	void emit(typename UTrait<P1>::ref p1, typename UTrait<P2>::ref p2,
			typename UTrait<P3>::ref p3, typename UTrait<P4>::ref p4) {
		for (SlotIterator iter = _slots.begin();iter != _slots.end();) {
			InSlotType * slot = static_cast<InSlotType*>(*iter);
			if (!slot->node()->died()) {
				(*slot)(p1, p2, p3, p4);
			}
			++iter;
			if (slot->node()->died()) {
				final_remove(slot);
			}
		}
	}
};
} // namespace ufo

#endif // USIGNAL_HPP
