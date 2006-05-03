/*
    This file is part of the Boson game
    Copyright (C) 2005-2006 Andreas Beckermann <b_mann@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef BOGL_1_5_DECL_P_H
#define BOGL_1_5_DECL_P_H

#include "bogl_1_4_decl_p.h"

/*
 additions in OpenGL 1.5:
 previously extensions and now directly included:
 * ARB_vertex_buffer_object
 * ARB_occlusion_query
 * EXT_shadow_funcs
*/

/*
 new functions:
 * ARB_vertex_buffer_object
   (TODO)
 * ARB_occlusion_query
   (TODO)
 * EXT_shadow_funcs
   (TODO)
*/


extern "C" {
	// GL typedefs
	// (TODO)

	// GL function pointers
	// (TODO)
}; // extern "C"


// GL defines
#if BOGL_DO_DLOPEN
// (TODO)
#endif // BOGL_DO_DLOPEN

#endif

