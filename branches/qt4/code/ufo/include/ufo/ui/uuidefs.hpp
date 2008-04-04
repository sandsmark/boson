/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/uuidefs.hpp
    begin             : Wed Jul 10 2002
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

#ifndef UUIDEFS_HPP
#define UUIDEFS_HPP

// macros and type definitions for ui and related classes

//#include "uobjectdefs.h"

//#include "../util/uhashmap.hpp"
#include <map>

namespace ufo {

class UWidgetUI;
class UWidget;


typedef UWidgetUI * ( *UI_HANDLER) (UWidget * widget);


typedef std::map<std::string, UI_HANDLER> UUIMap;

typedef std::map<std::string, UObject*> UThemeMap;

} // namespace ufo

#endif // UUIDEFS_HPP
