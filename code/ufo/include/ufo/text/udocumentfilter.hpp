/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/text/udocumentfilter.hpp
    begin             : Wed Dec 19 2001
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

#ifndef UDOCUMENTFILTER_HPP
#define UDOCUMENTFILTER_HPP

#include "../uobject.hpp"

namespace ufo {

/**A filter for text input.
  *@author Johannes Schmidt
  */

class UFO_EXPORT UDocumentFilter : public UObject  {
	UFO_DECLARE_ABSTRACT_CLASS(UDocumentFilter)
public:
	virtual std::string filterText(const std::string & input) = 0;
};

} // namespace ufo

#endif // UDOCUMENTFILTER_HPP
