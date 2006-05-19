/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks <rivolaks@hot.ee>
    Copyright (C) 2006 Andreas Beckermann <b_mann@gmx.de>

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

#ifndef ARB_VERTEX_BUFFER_OBJECT_DECL_P_H
#define ARB_VERTEX_BUFFER_OBJECT_DECL_P_H

// note: compliant OpenGL implementations don't have to implement an extension!

#ifndef GL_ARB_vertex_buffer_object
typedef int GLsizeiptrARB;
typedef int GLintptrARB;
#endif

extern "C" {
	// GL typedefs
	typedef void (*_glBindBufferARB)(GLenum, GLuint);
	typedef void (*_glDeleteBuffersARB)(GLsizei, const GLuint*);
	typedef void (*_glGenBuffersARB)(GLsizei, GLuint*);
	typedef GLboolean (*_glIsBufferARB) (GLuint);
	typedef void (*_glBufferDataARB)(GLenum, GLsizeiptrARB, const GLvoid*, GLenum);
	typedef void (*_glBufferSubDataARB)(GLenum, GLintptrARB, GLsizeiptrARB, const GLvoid*);
	typedef GLvoid* (*_glMapBufferARB) (GLenum target, GLenum access);
	typedef GLboolean (* _glUnmapBufferARB) (GLenum target);
	
	typedef GLvoid (*_glGetBufferParameterivARB) (GLenum, GLenum, GLint*);
	typedef GLvoid (*_glGetBufferPointervARB) (GLenum, GLenum, GLvoid**);



	// GL function pointers
	extern _glBindBufferARB bo_glBindBufferARB;
	extern _glDeleteBuffersARB bo_glDeleteBuffersARB;
	extern _glGenBuffersARB bo_glGenBuffersARB;
	extern _glIsBufferARB bo_glIsBufferARB;
	extern _glBufferDataARB bo_glBufferDataARB;
	extern _glBufferSubDataARB bo_glBufferSubDataARB;
	extern _glMapBufferARB bo_glMapBufferARB;
	extern _glUnmapBufferARB bo_glUnmapBufferARB;
	extern _glGetBufferParameterivARB bo_glGetBufferParameterivARB;
	extern _glGetBufferPointervARB bo_glGetBufferPointervARB;
}; // extern "C"


#define GL_ARRAY_BUFFER_ARB                             0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB                     0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB                     0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB             0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB              0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB              0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB               0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB               0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB       0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB           0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB     0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB      0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB              0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB       0x889F
#define GL_STREAM_DRAW_ARB                              0x88E0
#define GL_STREAM_READ_ARB                              0x88E1
#define GL_STREAM_COPY_ARB                              0x88E2
#define GL_STATIC_DRAW_ARB                              0x88E4
#define GL_STATIC_READ_ARB                              0x88E5
#define GL_STATIC_COPY_ARB                              0x88E6
#define GL_DYNAMIC_DRAW_ARB                             0x88E8
#define GL_DYNAMIC_READ_ARB                             0x88E9
#define GL_DYNAMIC_COPY_ARB                             0x88EA
#define GL_READ_ONLY_ARB                                0x88B8
#define GL_WRITE_ONLY_ARB                               0x88B9
#define GL_READ_WRITE_ARB                               0x88BA
#define GL_STATIC_DRAW_ARB                              0x88E4
#define GL_DYNAMIC_DRAW_ARB                             0x88E8
#define GL_BUFFER_SIZE_ARB                              0x8764
#define GL_BUFFER_USAGE_ARB                             0x8765
#define GL_BUFFER_ACCESS_ARB                            0x88BB
#define GL_BUFFER_MAPPED_ARB                            0x88BC
#define GL_BUFFER_MAP_POINTER_ARB                       0x88BD



#if BOGL_DO_DLOPEN
#define glBindBufferARB bo_glBindBufferARB
#define glDeleteBuffersARB bo_glDeleteBuffersARB
#define glGenBuffersARB bo_glGenBuffersARB
#define glIsBufferARB bo_glIsBufferARB
#define glBufferDataARB bo_glBufferDataARB
#define glBufferSubDataARB bo_glBufferSubDataARB
#define glMapBufferARB bo_glMapBufferARB
#define glUnmapBufferARB bo_glUnmapBufferARB
#define glGetBufferParameterivARB bo_glGetBufferParameterivARB
#define glGetBufferPointervARB bo_glGetBufferPointervARB
#endif // BOGL_DO_DLOPEN


#endif
