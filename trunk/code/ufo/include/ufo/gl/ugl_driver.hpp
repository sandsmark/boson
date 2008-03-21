/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/gl/ugl_driver.hpp
    begin             : Sat Nov 1 2003
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

#ifndef UGL_DRIVER_HPP
#define UGL_DRIVER_HPP

#include "../uobject.hpp"
// for data primitives like GLenum
#include "../ufo_gl.hpp"

namespace ufo {

class USharedLib;

/** @short A simple OpenGL driver which loads OpenGL at runtime.
  * @ingroup internal
  * @ingroup opengl
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UGL_Driver {
public:
	UGL_Driver(const char * glPath = "");

public: // gl methods
//#ifdef UFO_TARGET_OPENGL
#ifndef UFO_OS_WIN32
#define WINAPI
#endif
#define UFO_GL_PROC(ret,func,params) ret (WINAPI *func) params;
#include "ugl_prototypes.hpp"
#undef UFO_GL_PROC
//#endif // UFO_TARGET_OPENGL

public: // init
	void * getProcAddress(const char* proc);

public: // static attributes
	USharedLib * m_glLib;
};

extern UFO_EXPORT UGL_Driver * ugl_driver;// = new ufo::UGL_Driver("");

} // namespace ufo

#endif // UGL_DRIVER_HPP
