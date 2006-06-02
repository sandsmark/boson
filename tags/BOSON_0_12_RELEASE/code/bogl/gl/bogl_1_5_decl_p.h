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

 some token names were changed (old ones remain valid):
  new                     old
  -----------------------------------------------------
  FOG_COORD_SRC           FOG_COORDINATE_SOURCE
  FOG_COORD               FOG_COORDINATE
  CURRENT_FOG_COORD       CURRENT_FOG_COORDINATE
  FOG_COORD_ARRAY_TYPE    FOG_COORDINATE_ARRAY_TYPE
  FOG_COORD_ARRAY_STRIDE  FOG_COORDINATE_ARRAY_STRIDE
  FOG_COORD_ARRAY_POINTER FOG_COORDINATE_ARRAY_POINTER
  FOG_COORD_ARRAY         FOG_COORDINATE_ARRAY
  SRC0_RGB                SOURCE0_RGB
  SRC1_RGB                SOURCE1_RGB
  SRC2_RGB                SOURCE2_RGB
  SRC0_ALPHA              SOURCE0_ALPHA
  SRC1_ALPHA              SOURCE1_ALPHA
  SRC2_ALPHA              SOURCE2_ALPHA
*/


/*
 new functions:
 * ARB_vertex_buffer_object
   glBindBuffer()
   glDeleteBuffers()
   glGenBuffers()
   glIsBuffer()
   glBufferData()
   glBufferSubData()
   glMapBuffer()
   glUnmapBuffer()
   glGetBufferParameteriv()
   glGetBufferPointerv()
 * ARB_occlusion_query
   GL_SAMPLES_PASSED
   GL_QUERY_COUNTER_BITS
   GL_CURRENT_QUERY
   GL_QUERY_RESULT
   GL_QUERY_RESULT_AVAILABLE
 * EXT_shadow_funcs
   none


 new defines:
 * ARB_vertex_buffer_object
   GL_ARRAY_BUFFER
   GL_ELEMENT_ARRAY_BUFFER
   GL_ARRAY_BUFFER_BINDING
   GL_ELEMENT_ARRAY_BUFFER_BINDING
   GL_VERTEX_ARRAY_BUFFER_BINDING
   GL_NORMAL_ARRAY_BUFFER_BINDING
   GL_COLOR_ARRAY_BUFFER_BINDING
   GL_INDEX_ARRAY_BUFFER_BINDING
   GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING
   GL_EDGE_FLAG_ARRAY_BUFFER_BINDING
   GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING
   GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
   GL_WEIGHT_ARRAY_BUFFER_BINDING
   GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING
   GL_STREAM_DRAW
   GL_STREAM_READ
   GL_STREAM_COPY
   GL_STATIC_DRAW
   GL_STATIC_READ
   GL_STATIC_COPY
   GL_DYNAMIC_DRAW
   GL_DYNAMIC_READ
   GL_DYNAMIC_COPY
   GL_READ_ONLY
   GL_WRITE_ONLY
   GL_READ_WRITE
   GL_STATIC_DRAW
   GL_DYNAMIC_DRAW
   GL_BUFFER_SIZE
   GL_BUFFER_USAGE
   GL_BUFFER_ACCESS
   GL_BUFFER_MAPPED
   GL_BUFFER_MAP_POINTER
 * ARB_occlusion_query
   glGenQueries()
   glDeleteQueries()
   glIsQuery()
   glBeginQuery()
   glEndQuery()
   glGetQueryiv()
   glGetQueryObjectiv()
   glGetQueryObjectuiv()
 * EXT_shadow_funcs
   none
*/


typedef int GLsizeiptr;
typedef int GLintptr;


extern "C" {
	// GL typedefs
	typedef void (*_glBindBuffer)(GLenum, GLuint);
	typedef void (*_glDeleteBuffers)(GLsizei, const GLuint*);
	typedef void (*_glGenBuffers)(GLsizei, GLuint*);
	typedef GLboolean (*_glIsBuffer) (GLuint);
	typedef void (*_glBufferData)(GLenum, GLsizeiptr, const GLvoid*, GLenum);
	typedef void (*_glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid*);
	typedef GLvoid* (*_glMapBuffer) (GLenum target, GLenum access);
	typedef GLboolean (* _glUnmapBuffer) (GLenum target);
	
	typedef GLvoid (*_glGetBufferParameteriv) (GLenum, GLenum, GLint*);
	typedef GLvoid (*_glGetBufferPointerv) (GLenum, GLenum, GLvoid**);
	typedef GLvoid (*_glGenQueries)(GLsizei n, GLuint *ids);
	typedef GLvoid (*_glDeleteQueries)(GLsizei, const GLuint*);
	typedef GLvoid (*_glIsQuery)(GLuint);
	typedef GLvoid (*_glBeginQuery)(GLenum, GLuint);
	typedef GLvoid (*_glEndQuery)(GLenum);
	typedef GLvoid (*_glGetQueryiv)(GLenum, GLenum, GLint*);
	typedef GLvoid (*_glGetQueryObjectiv)(GLuint, GLenum, GLint*);
	typedef GLvoid (*_glGetQueryObjectuiv)(GLuint, GLenum, GLuint*);


	// GL function pointers
	extern _glBindBuffer bo_glBindBuffer;
	extern _glDeleteBuffers bo_glDeleteBuffers;
	extern _glGenBuffers bo_glGenBuffers;
	extern _glIsBuffer bo_glIsBuffer;
	extern _glBufferData bo_glBufferData;
	extern _glBufferSubData bo_glBufferSubData;
	extern _glMapBuffer bo_glMapBuffer;
	extern _glUnmapBuffer bo_glUnmapBuffer;
	extern _glGetBufferParameteriv bo_glGetBufferParameteriv;
	extern _glGetBufferPointerv bo_glGetBufferPointerv;
	extern _glGenQueries bo_glGenQueries;
	extern _glDeleteQueries bo_glDeleteQueries;
	extern _glIsQuery bo_glIsQuery;
	extern _glBeginQuery bo_glBeginQuery;
	extern _glEndQuery bo_glEndQuery;
	extern _glGetQueryiv bo_glGetQueryiv;
	extern _glGetQueryObjectiv bo_glGetQueryObjectiv;
	extern _glGetQueryObjectuiv bo_glGetQueryObjectuiv;
}; // extern "C"


// names changed in GL 1.5 (old ones remain valid)
#define GL_FOG_COORD_SRC           0x8450
#define GL_FOG_COORD               0x8451
#define GL_CURRENT_FOG_COORD       0x8453
#define GL_FOG_COORD_ARRAY_TYPE    0x8454
#define GL_FOG_COORD_ARRAY_STRIDE  0x8455
#define GL_FOG_COORD_ARRAY_POINTER 0x8456
#define GL_FOG_COORD_ARRAY         0x8457
#define GL_SRC0_RGB                0x8580
#define GL_SRC1_RGB                0x8581
#define GL_SRC2_RGB                0x8582
#define GL_SRC0_ALPHA              0x8588
#define GL_SRC1_ALPHA              0x8589
#define GL_SRC2_ALPHA              0x858A

// ARB_vertex_buffer_object
#define GL_ARRAY_BUFFER                             0x8892
#define GL_ELEMENT_ARRAY_BUFFER                     0x8893
#define GL_ARRAY_BUFFER_BINDING                     0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING             0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING              0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING              0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING               0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING               0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING       0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING           0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING     0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING      0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING              0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING       0x889F
#define GL_STREAM_DRAW                              0x88E0
#define GL_STREAM_READ                              0x88E1
#define GL_STREAM_COPY                              0x88E2
#define GL_STATIC_DRAW                              0x88E4
#define GL_STATIC_READ                              0x88E5
#define GL_STATIC_COPY                              0x88E6
#define GL_DYNAMIC_DRAW                             0x88E8
#define GL_DYNAMIC_READ                             0x88E9
#define GL_DYNAMIC_COPY                             0x88EA
#define GL_READ_ONLY                                0x88B8
#define GL_WRITE_ONLY                               0x88B9
#define GL_READ_WRITE                               0x88BA
#define GL_STATIC_DRAW                              0x88E4
#define GL_DYNAMIC_DRAW                             0x88E8
#define GL_BUFFER_SIZE                              0x8764
#define GL_BUFFER_USAGE                             0x8765
#define GL_BUFFER_ACCESS                            0x88BB
#define GL_BUFFER_MAPPED                            0x88BC
#define GL_BUFFER_MAP_POINTER                       0x88BD
// ARB_occlusion_query
#define GL_SAMPLES_PASSED                           0x8914
#define GL_QUERY_COUNTER_BITS                       0x8864
#define GL_CURRENT_QUERY                            0x8865
#define GL_QUERY_RESULT                             0x8866
#define GL_QUERY_RESULT_AVAILABLE                   0x8867




// GL defines
#if BOGL_DO_DLOPEN
#define glBindBuffer bo_glBindBuffer
#define glDeleteBuffers bo_glDeleteBuffers
#define glGenBuffers bo_glGenBuffers
#define glIsBuffer bo_glIsBuffer
#define glBufferData bo_glBufferData
#define glBufferSubData bo_glBufferSubData
#define glMapBuffer bo_glMapBuffer
#define glUnmapBuffer bo_glUnmapBuffer
#define glGetBufferParameteriv bo_glGetBufferParameteriv
#define glGetBufferPointerv bo_glGetBufferPointerv
#define glGenQueries bo_glGenQueries
#define glDeleteQueries bo_glDeleteQueries
#define glIsQuery bo_glIsQuery
#define glBeginQuery bo_glBeginQuery
#define glEndQuery bo_glEndQuery
#define glGetQueryiv bo_glGetQueryiv
#define glGetQueryObjectiv bo_glGetQueryObjectiv
#define glGetQueryObjectuiv bo_glGetQueryObjectuiv
#endif // BOGL_DO_DLOPEN

#endif
