/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uinputmap.hpp
    begin             : Tue Feb 12 2002
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

#ifndef UINPUTMAP_HPP
#define UINPUTMAP_HPP

#include "uobject.hpp"

#include "ukeystroke.hpp"
#include "signals/ufo_signals.hpp"
#include "events/uactionevent.hpp"

namespace ufo {

/** @short An input map stores registered actions for certain key events
  * @ingroup events
  *
  * It is used by widgets and contexts.
  *
  * @see UContext::getInputMap
  * @see UWidget::getInputMap
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UInputMap : public UObject  {
	UFO_DECLARE_DYNAMIC_CLASS(UInputMap)
public:
	void put(const UKeyStroke & strokeA, const UActionSlot & listenerA);
	void remove(const UKeyStroke & strokeA);
	UActionSlot * get(const UKeyStroke & strokeA);

	void clear();
private:
	typedef std::map<UKeyStroke, UActionSlot> MapType;
	MapType m_map;
};


///
/// inline implementation
///

inline void
UInputMap::put(const UKeyStroke & strokeA, const UActionSlot & listenerA) {
	m_map[strokeA] = listenerA;
}

inline void
UInputMap::remove(const UKeyStroke & strokeA) {
	m_map.erase(strokeA);
}

inline UActionSlot *
UInputMap::get(const UKeyStroke & strokeA) {
	// we have to use == operator as map.find produces wrong results for
	// Shift, Alt and Ctrl
	for (MapType::iterator iter = m_map.begin();
			iter != m_map.end();
			++iter) {
		if ((*iter).first == strokeA) {
			return &((*iter).second);
		}
	}
	/*
	MapType::iterator iter;
	iter = m_map.find(strokeA);
	if (iter != m_map.end()) {
		return &((*iter).second);
	}*/
	return NULL;
}

inline void
UInputMap::clear() {
	m_map.clear();
}

} // namespace ufo

#endif // UINPUTMAP_HPP
