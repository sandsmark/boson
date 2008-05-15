/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/uobject.cpp
    begin             : Tue May 8 2001
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

#include "ufo/uobject.hpp"

#include "ufo/signals/ufo_signals.hpp"
#include "ufo/ufo_types.hpp"

#include <algorithm>

using namespace ufo;

#ifdef UFO_RTTI

//
//
// UCLassInfo
//


UClassInfo * UClassInfo::sm_first = NULL;
std::map<std::string, UClassInfo*> UClassInfo::sm_classTable;

void
UClassInfo::initClassInfo() {
	// FIXME
}

UClassInfo::~UClassInfo() {
	// remove this object from the linked list of all class infos: if we don't
	// do it, loading/unloading a DLL containing static UClassInfo objects is
	// not going to work
	if (this == sm_first) {
		sm_first = m_next;
	} else {
		UClassInfo * info = sm_first;
		while (info) {
			if (info->m_next == this) {
				info->m_next = m_next;
				break;
			}

			info = info->m_next;
		}
	}
	//sm_classTable[m_className] = NULL;
}

// searches for the class info struct in the single linked list and registers
// the result at the hash map
UClassInfo *
UClassInfo::findClass(const std::string & className) {
	// we are caching the results in a static hash map
	if (sm_classTable[className]) {
		return sm_classTable[className];
	} else {
		for (UClassInfo * info = sm_first; info ; info = info->m_next) {
			if (info->getClassName() == className) {
				sm_classTable[className] = info;
				return info;
			}
		}

		return NULL;
	}
}

#endif // UFO_RTTI

//
//
// UObject implementation
//
//

// UObject is the root class for the class info structures
UFO_IMPLEMENT_DYNAMIC_CLASS(UObject, "")

static int nAllocated = 0;
static int nFreed = 0;

UObject::UObject() {
	nAllocated++;
}
UObject::UObject(const UObject & obj) : UCollectable() {
	nAllocated++;
}
UObject::~UObject() {
	for (std::list<UObjectSlotNode*>::iterator iter = m_objectSlots.begin();
			iter != m_objectSlots.end();
			++iter) {
		(*iter)->notify(true);
	}
	m_sigDestroyed(this);
	releaseAllPointers();
	nFreed++;
}

unsigned int
UObject::hashCode() const {
	long ret = reinterpret_cast<long>(this);
	return (unsigned int)(ret);
}

// not to be used
unsigned int
UObject::objCount() {
	return nAllocated - nFreed;
}

bool
UObject::equals(const UObject * obj) const {
	return (this == obj);
}


UObject *
UObject::clone() const {
	return new UObject();
}

std::string
UObject::toString() const {
	UStringStream stream;
#ifdef UFO_RTTI
	if (getClassInfo()) {
		stream << getClassInfo()->getClassName();
	}
#endif
	stream <<  '[';
	paramString(stream);
	stream << ']';

	return stream.str();
}

UCollectable *
UObject::trackPointer(UCollectable * c) {
	if (c) {
		m_pointers.push_back(c);
		c->reference();
	}
	return c;
}

const UCollectable *
UObject::trackPointer(const UCollectable * c) {
	if (c) {
		m_pointers.push_back(c);
		c->reference();
	}
	return c;
}

bool
UObject::releasePointer(UCollectable * c) {
	// removes the first occurence of c and unreferences it
	std::list<const UCollectable*>::iterator iter;
	iter = std::find(m_pointers.begin(), m_pointers.end(), c);
	if (iter != m_pointers.end()) {
		c->unreference();
		m_pointers.erase(iter);
		return true;
	}
	return false;
}

bool
UObject::releasePointer(const UCollectable * c) {
	// removes the first occurence of c and unreferences it
	std::list<const UCollectable*>::iterator iter;
	iter = std::find(m_pointers.begin(), m_pointers.end(), c);
	if (iter != m_pointers.end()) {
		c->unreference();
		m_pointers.erase(iter);
		return true;
	}
	return false;
}

void
UObject::swapPointers(const UCollectable * oldObj, const UCollectable * newObj) {
	if (oldObj != newObj) {
		releasePointer(oldObj);
		trackPointer(newObj);
	}
}

void
UObject::releaseAllPointers() {
	std::list<const UCollectable*>::iterator iter;
	for (iter = m_pointers.begin(); iter != m_pointers.end(); ++iter) {
		(*iter)->unreference();
	}
	m_pointers.clear();
}

//
// protected methods
//

std::ostream &
UObject::paramString(std::ostream & os) const {
	if (!m_name.empty()) {
		return os <<  "name=" << m_name << ";";
	} else {
		return os;
	}
}


//
// class info implementation of abstract classes
//
// FIXME
// this is not really a good place
//

#include "ufo/ucontext.hpp"
#include "ufo/udisplay.hpp"
#include "ufo/udrawable.hpp"
#include "ufo/ugraphics.hpp"
#include "ufo/uicon.hpp"
#include "ufo/utoolkit.hpp"
#include "ufo/uinputmap.hpp"
#include "ufo/uvideodevice.hpp"
#include "ufo/uvideodriver.hpp"

UFO_IMPLEMENT_ABSTRACT_CLASS(UContext, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UDisplay, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UDrawable, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UGraphics, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UIcon, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UToolkit, UObject)
UFO_IMPLEMENT_DYNAMIC_CLASS(UInputMap, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UVideoDevice, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UVideoDriver, UObject)
