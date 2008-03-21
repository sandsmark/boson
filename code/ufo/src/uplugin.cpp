/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/uplugin.cpp
    begin             : Wed Apr 2 2003
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

#include "ufo/uplugin.hpp"

namespace ufo {

UFO_IMPLEMENT_ABSTRACT_CLASS(UPluginBase, UObject)

UFO_IMPLEMENT_ABSTRACT_CLASS(UFontPlugin, UPluginBase)
UFO_IMPLEMENT_ABSTRACT_CLASS(ULAFPlugin, UPluginBase)
UFO_IMPLEMENT_ABSTRACT_CLASS(UImageIOPlugin, UPluginBase)
UFO_IMPLEMENT_ABSTRACT_CLASS(UVideoPlugin, UPluginBase)


} // namespace ufo
