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

#ifndef BOGL_1_4_DECL_P_H
#define BOGL_1_4_DECL_P_H

#include "bogl_1_3_decl_p.h"

/*
 additions in OpenGL 1.4:
 previously extensions and now directly included:
 * SGIS_generate_mipmap
 * NV_blend_square
 * ARB_depth_texture
 * ARB_shadow
 * EXT_fog_coord
 * EXT_multi_draw_arrays
 * ARB_point_parameters
 * EXT_secondary_color
 * EXT_blend_func_separate
 * EXT_stencil_wrap
 * ARB_texture_crossbar
*/

/*
 new functions:
 * SGIS_generate_mipmap
   (TODO)
 * NV_blend_square
   (TODO)
 * ARB_depth_texture
   (TODO)
 * ARB_shadow
   (TODO)
 * EXT_fog_coord
   (TODO)
 * EXT_multi_draw_arrays
   (TODO)
 * ARB_point_parameters
   (TODO)
 * EXT_secondary_color
   (TODO)
 * EXT_blend_func_separate
   (TODO)
 * EXT_stencil_wrap
   (TODO)
 * ARB_texture_crossbar
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

