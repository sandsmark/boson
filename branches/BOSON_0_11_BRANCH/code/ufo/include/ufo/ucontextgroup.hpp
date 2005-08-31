/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ucontextgroup.hpp
    begin             : Thu Feb 19 2004
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


#ifndef UCONTEXTGROUP_HPP
#define UCONTEXTGROUP_HPP

#include "uobject.hpp"

#include <vector>

namespace ufo {

class UContext;
class UVolatileData;

/** A context group represents all (OpenGL)/UFO context which share
  * memory, i.e. may use the same images and font renderers.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UContextGroup : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UContextGroup)
public: // constructor
	UContextGroup();
	virtual ~UContextGroup();
public:
	/** Returns all contexts which belong to this context groups. */
	std::vector<UContext*> getContexts();

	/** Returns true if the given context belongs to this groups. */
	bool belongsToGroup(UContext * context);

	/** Should be called when the (OpenGL) data should be refreshed.
	  * Refreshes all registered contexts.
	  */
	void refresh();

	/** Gets the time stamp of the last refresh. This may be used to
	  * avoid unnecessary refreshes.
	  */
	uint32_t getLastRefreshTime() const;

	void addVolatileData(UVolatileData * vdata);
	void removeVolatileData(UVolatileData * vdata);

public: // Shouldn't be used by client code
	void addContext(UContext * context);
	void removeContext(UContext * context);

private:
	std::vector<UContext*> m_contexts;
	std::vector<UVolatileData*> m_volatileData;
	uint32_t m_refreshTime;
};

//
// inline implementation
//


inline uint32_t
UContextGroup::getLastRefreshTime() const {
	return m_refreshTime;
}

} // namespace ufo

#endif // UCONTEXTGROUP_HPP
