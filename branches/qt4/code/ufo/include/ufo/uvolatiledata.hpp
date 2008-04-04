/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uvolatiledata.hpp
    begin             : Tue Feb 24 2004
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

#ifndef UVOLATILEDATA_HPP
#define UVOLATILEDATA_HPP

#include "uobject.hpp"

namespace ufo {

class UDisplay;

/** @short An interface for volatile data, i.e. for data which have to be recreated
  *  after an OpenGL context was destroyed and recreated.
  * @ingroup misc
  *
  * @author Johannes Schmidt
  */
class UVolatileData : public virtual UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UVolatileData)
public: // constructor
	UVolatileData(UDisplay * display = NULL);
	virtual ~UVolatileData();

public: // Public virtual methods
	virtual void refresh() = 0;

public: // Public methods
	/** Returns the context group which stores the volatile data. */
	UDisplay * getDisplay() const;

	/** Returns true when the volatile data needs a refresh. */
	bool needsRefresh() const;

	/** Returns the time stamp of the last refresh.
	  */
	uint32_t getLastRefreshTime() const;

protected: // Protected methods
	/** Sets the context group. */
	void setDisplay(UDisplay * display);
	/** Updates the time stamp. Should be called at a call to refresh. */
	void updateRefreshTime();

private: // Private attributes
	UDisplay * m_display;
	uint32_t m_refreshTime;
};

//
// inline implementation
//


inline uint32_t
UVolatileData::getLastRefreshTime() const {
	return m_refreshTime;
}

inline UDisplay *
UVolatileData::getDisplay() const {
	return m_display;
}

} // namespace ufo

#endif // UVOLATILEDATA_HPP
