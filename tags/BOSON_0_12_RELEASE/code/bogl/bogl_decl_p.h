/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann <b_mann@gmx.de>

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

#ifndef BOGLDECL_P_H
#define BOGLDECL_P_H

#ifndef BOGL_H
#error Never include this file directly! Include bogl.h instead!
#endif

// OpenGL
#include "gl/bogl_2_0_decl_p.h"

// GLU
#include "gl/boglu_decl_p.h"

// OpenGL extensions
#include "gl/extensions/arb_multitexture_decl_p.h"
#include "gl/extensions/ext_blend_color_decl_p.h"
#include "gl/extensions/ext_polygon_offset_decl_p.h"
#include "gl/extensions/ext_texture3d_decl_p.h"
#include "gl/extensions/arb_vertex_buffer_object_decl_p.h"
#include "gl/extensions/arb_shader_objects_decl_p.h"
#include "gl/extensions/ext_framebuffer_object_decl_p.h"

#endif // BOGLDECL_P_H
