/*
    This file is part of the Boson game
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

#ifndef BOGL_2_0_DECL_P_H
#define BOGL_2_0_DECL_P_H

#include "bogl_1_5_decl_p.h"

/*
 additions in OpenGL 2.0:
 previously extensions and now directly included:
 * ARB_shader_objects
 * ARB_vertex_shader
 * ARB_fragment_shader
 * ARB_shading_language_100
 * ARB_draw_buffers
 * ARB_texture_non_power_of_two
 * ARB_point_sprite
 * ATI_separate_stencil, EXT_stencil_two_side
 * EXT_blend_equation_separate // AB: this is NOT listed in the spec in Appendix
                               //     I ("Version 2.0"). however this extension
                               //     is definitely included in OpenGL 2.0


 note that when promoting some shader extensions to GL core, some
 function names were changed! also GLuint is used instead of GLhandleARB

 changed define names:
   FOG_COORDINATE_ARRAY_BUFFER_BINDING -> FOG_COORD_ARRAY_BUFFER_BINDING
   define GL_BLEND_EQUATION -> GL_BLEND_EQUATION_RGB
*/


/*
 new functions:
 * ARB_shader_objects
   glIsShader()   // AB: not in the extension, but I assume it results from here
   glIsProgram()  // AB: not in the extension, but I assume it results from here
   glGetAttachedShaders();
   glCreateShader();
   glShaderSource();
   glCompileShader();
   glDeleteShader();
   glCreateProgram();
   glAttachShader();
   glDetachShader();
   glLinkProgram();
   glUseProgram();
   glDeleteProgram();
   glGetShaderInfoLog();
   glGetProgramInfoLog();
   glGetShaderSource();
   glGetUniformfv();
   glGetUniformiv();
   glGetProgramiv();
   glGetShaderiv();
   glUniform1f();
   glUniform2f();
   glUniform3f();
   glUniform4f();
   glUniform1i();
   glUniform2i();
   glUniform3i();
   glUniform4i();
   glUniform1fv();
   glUniform2fv();
   glUniform3fv();
   glUniform4fv();
   glUniform1iv();
   glUniform2iv();
   glUniform3iv();
   glUniform4iv();
   glUniformMatrix2fv();
   glUniformMatrix3fv();
   glUniformMatrix4fv();
   glValidateProgram();
   glGetUniformLocation();
   glGetActiveUniform();

  some function names were changed from the ARB extension to GL 2.0
  (simple glFooARB()->glFoo() not listed in the following)
    glAttachObjectARB() -> glAttachShader()
    glCreateProgramObjectARB() -> glCreateProgram()
    glCreateShaderObjectARB() -> glCreateShader()
    glDeleteObjectARB() -> glDeleteProgram()
                        -> glDeleteShader()
    glGetInfoLogARB() -> glGetShaderInfoLog()
                      -> glGetProgramInfoLog()
    glGetObjectParameterivARB() -> glGetProgramiv()
                                -> glGetShaderiv()
    glUseProgramObjectARB() -> glUseProgram()
    glDetachObjectARB() -> glDetachShader()
    glAttachObjectARB() -> glAttachShader()
    glGetAttachedObjectsARB() -> glGetAttachedShaders()
    glGetHandleARB() -> ??? seems to be dropped entirely

  other changes include:
    GLhandleARB -> GLuint
 * ARB_vertex_shader
    glVertexAttrib[1234][sfd]()
    glVertexAttrib[1234][sfd]v()
    glVertexAttrib4[bi ubusui]v()
    glVertexAttrib4Nub()
    glVertexAttrib4N[bsi ubusui]v()
    glVertexAttribPointer()
    glEnableVertexAttribArray()
    glDisableVertexAttribArray()
    glBindAttribLocation()
    glGetActiveAttrib()
    glGetAttribLocation()
    glGetVertexAttribdv()
    glGetVertexAttribfv()
    glGetVertexAttribiv()
    glGetVertexAttribPointerv()
 * ARB_fragment_shader
   none
 * ARB_shading_language_100
   none
 * ARB_draw_buffers
   glDrawBuffers()
 * ARB_texture_non_power_of_two
   none
 * ARB_point_sprite
   none
 * ATI_separate_stencil
   glStencilOpSeparate()
   glStencilFuncSeparate()
 * EXT_stencil_two_side
   none?
 * EXT_blend_equation_separate
   glBlendEquationSeparate()


 new defines:
 * ARB_shader_objects
   GL_DELETE_STATUS
   GL_COMPILE_STATUS
   GL_LINK_STATUS
   GL_VALIDATE_STATUS
   GL_INFO_LOG_LENGTH
   GL_ATTACHED_SHADERS
   GL_ACTIVE_UNIFORMS
   GL_ACTIVE_UNIFORM_MAX_LENGTH
   GL_SHADER_SOURCE_LENGTH
   GL_SHADER_TYPE
   GL_FLOAT_VEC2
   GL_FLOAT_VEC3
   GL_FLOAT_VEC4
   GL_INT_VEC2
   GL_INT_VEC3
   GL_INT_VEC4
   GL_BOOL
   GL_BOOL_VEC2
   GL_BOOL_VEC3
   GL_BOOL_VEC4
   GL_FLOAT_MAT2
   GL_FLOAT_MAT3
   GL_FLOAT_MAT4
   GL_SAMPLER_1D
   GL_SAMPLER_2D
   GL_SAMPLER_3D
   GL_SAMPLER_CUBE
   GL_SAMPLER_1D_SHADOW
   GL_SAMPLER_2D_SHADOW
   GL_CURRENT_PROGRAM

   defines in the ARB extension but not in GL 2.0:
     GL_PROGRAM_OBJECT_ARB
     GL_SAMPLER_2D_RECT_ARB
     GL_SAMPLER_2D_RECT_SHADOW_ARB
     GL_OBJECT_TYPE_ARB
     GL_SHADER_OBJECT_ARB

   some define names got changed from ARB to GL 2.0 (more than just the _ARB removed):
     GL_OBJECT_DELETE_STATUS_ARB        -> GL_DELETE_STATUS
     GL_OBJECT_COMPILE_STATUS_ARB       -> GL_COMPILE_STATUS
     GL_OBJECT_LINK_STATUS_ARB          -> GL_LINK_STATUS
     GL_OBJECT_VALIDATE_STATUS_ARB      -> GL_VALIDATE_STATUS
     GL_OBJECT_INFO_LOG_LENGTH_ARB      -> GL_INFO_LOG_LENGTH
     GL_OBJECT_ATTACHED_OBJECTS_ARB     -> GL_ATTACHED_SHADERS
     GL_OBJECT_ACTIVE_UNIFORMS_ARB      -> GL_ACTIVE_UNIFORMS
     GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB -> GL_ACTIVE_UNIFORM_MAX_LENGTH
     GL_OBJECT_SHADER_SOURCE_LENGTH_ARB -> GL_SHADER_SOURCE_LENGTH
     GL_OBJECT_SUBTYPE_ARB              -> GL_SHADER_TYPE

 * ARB_vertex_shader (some of them are also in ARB_vertex_shader)
   GL_VERTEX_SHADER
   GL_MAX_VERTEX_UNIFORM_COMPONENTS
   GL_MAX_VARYING_FLOATS
   GL_MAX_VERTEX_ATTRIBS
   GL_MAX_TEXTURE_IMAGE_UNITS
   GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
   GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
   GL_MAX_TEXTURE_COORDS
   GL_VERTEX_PROGRAM_POINT_SIZE
   GL_VERTEX_PROGRAM_TWO_SIDE
   GL_ACTIVE_ATTRIBUTES
   GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
   GL_VERTEX_ATTRIB_ARRAY_ENABLED
   GL_VERTEX_ATTRIB_ARRAY_SIZE
   GL_VERTEX_ATTRIB_ARRAY_STRIDE
   GL_VERTEX_ATTRIB_ARRAY_TYPE
   GL_VERTEX_ATTRIB_ARRAY_NORMALIZED
   GL_CURRENT_VERTEX_ATTRIB
   GL_VERTEX_ATTRIB_ARRAY_POINTER
   GL_FLOAT_VEC2
   GL_FLOAT_VEC3
   GL_FLOAT_VEC4
   GL_FLOAT_MAT2
   GL_FLOAT_MAT3
   GL_FLOAT_MAT4


   some define names got changed from ARB to GL 2.0 (more than just the _ARB removed):
     GL_OBJECT_ACTIVE_ATTRIBUTES -> GL_ACTIVE_ATTRIBUTES
     GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH -> GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
 * ARB_fragment_shader
   GL_FRAGMENT_SHADER
   GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
   GL_MAX_TEXTURE_COORDS
   GL_MAX_TEXTURE_IMAGE_UNITS
   GL_FRAGMENT_SHADER_DERIVATIVE_HINT
 * ARB_shading_language_100
   GL_SHADING_LANGUAGE_VERSION
 * ARB_draw_buffers
   GL_MAX_DRAW_BUFFERS
   GL_DRAW_BUFFER0
   GL_DRAW_BUFFER1
   GL_DRAW_BUFFER2
   GL_DRAW_BUFFER3
   GL_DRAW_BUFFER4
   GL_DRAW_BUFFER5
   GL_DRAW_BUFFER6
   GL_DRAW_BUFFER7
   GL_DRAW_BUFFER8
   GL_DRAW_BUFFER9
   GL_DRAW_BUFFER10
   GL_DRAW_BUFFER11
   GL_DRAW_BUFFER12
   GL_DRAW_BUFFER13
   GL_DRAW_BUFFER14
   GL_DRAW_BUFFER15
 * ARB_texture_non_power_of_two
   none
 * ARB_point_sprite
   GL_POINT_SPRITE
   GL_COORD_REPLACE
   GL_POINT_SPRITE_COORD_ORIGIN  // not in the extension, was added in GL 2.0
   GL_LOWER_LEFT                 // not in the extension, was added in GL 2.0
   GL_UPPER_LEFT                 // not in the extension, was added in GL 2.0
 * ATI_separate_stencil
   GL_STENCIL_BACK_FUNC                            0x8800
   GL_STENCIL_BACK_FAIL                            0x8801
   GL_STENCIL_BACK_PASS_DEPTH_FAIL                 0x8802
   GL_STENCIL_BACK_PASS_DEPTH_PASS                 0x8803
 * EXT_stencil_two_side
   none?
 * EXT_blend_equation_separate
   GL_BLEND_EQUATION_ALPHA
*/


typedef char GLchar;

extern "C" {
	// GL typedefs
	typedef GLvoid (*_glDrawBuffers)(GLsizei, const GLenum*);
	typedef GLvoid (*_glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum);
	typedef GLvoid (*_glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint);
	typedef GLboolean (*_glIsShader)(GLuint);
	typedef GLboolean (*_glIsProgram)(GLuint);
	typedef GLvoid (*_glGetAttachedShaders)(GLuint, GLsizei, GLsizei*, GLuint*);
	typedef GLuint (*_glCreateShader)(GLenum);
	typedef GLvoid (*_glShaderSource)(GLuint, GLsizei, const GLchar**, const GLint*);
	typedef GLvoid (*_glCompileShader)(GLuint);
	typedef GLvoid (*_glDeleteShader)(GLuint);
	typedef GLuint (*_glCreateProgram)(void);
	typedef GLvoid (*_glAttachShader)(GLuint, GLuint);
	typedef GLvoid (*_glDetachShader)(GLuint, GLuint);
	typedef GLvoid (*_glLinkProgram)(GLuint);
	typedef GLvoid (*_glUseProgram)(GLuint);
	typedef GLvoid (*_glDeleteProgram)(GLuint);
	typedef GLvoid (*_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
	typedef GLvoid (*_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
	typedef GLvoid (*_glGetShaderSource)(GLuint, GLsizei, GLsizei*, GLchar*);
	typedef GLvoid (*_glGetUniformfv)(GLuint, GLint, GLfloat*);
	typedef GLvoid (*_glGetUniformiv)(GLuint, GLint, GLint*);
	typedef GLvoid (*_glGetProgramiv)(GLuint, GLenum, GLint*);
	typedef GLvoid (*_glGetShaderiv)(GLuint, GLenum, GLint*);
	typedef GLvoid (*_glUniform1f)(GLint, GLfloat);
	typedef GLvoid (*_glUniform2f)(GLint, GLfloat, GLfloat);
	typedef GLvoid (*_glUniform3f)(GLint, GLfloat, GLfloat, GLfloat);
	typedef GLvoid (*_glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
	typedef GLvoid (*_glUniform1i)(GLint, GLint);
	typedef GLvoid (*_glUniform2i)(GLint, GLint, GLint);
	typedef GLvoid (*_glUniform3i)(GLint, GLint, GLint, GLint);
	typedef GLvoid (*_glUniform4i)(GLint, GLint, GLint, GLint, GLint);
	typedef GLvoid (*_glUniform1fv)(GLint, GLsizei, const GLfloat*);
	typedef GLvoid (*_glUniform2fv)(GLint, GLsizei, const GLfloat*);
	typedef GLvoid (*_glUniform3fv)(GLint, GLsizei, const GLfloat*);
	typedef GLvoid (*_glUniform4fv)(GLint, GLsizei, const GLfloat*);
	typedef GLvoid (*_glUniform1iv)(GLint, GLsizei, const GLint*);
	typedef GLvoid (*_glUniform2iv)(GLint, GLsizei, const GLint*);
	typedef GLvoid (*_glUniform3iv)(GLint, GLsizei, const GLint*);
	typedef GLvoid (*_glUniform4iv)(GLint, GLsizei, const GLint*);
	typedef GLvoid (*_glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat*);
	typedef GLvoid (*_glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat*);
	typedef GLvoid (*_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
	typedef GLvoid (*_glValidateProgram)(GLuint);
	typedef GLint (*_glGetUniformLocation)(GLuint, const GLchar*);
	typedef GLvoid (*_glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, const GLchar*);
	typedef GLvoid (*_glVertexAttrib1s)(GLuint, GLshort);
	typedef GLvoid (*_glVertexAttrib1f)(GLuint, GLfloat);
	typedef GLvoid (*_glVertexAttrib1d)(GLuint, GLdouble);
	typedef GLvoid (*_glVertexAttrib2s)(GLuint, GLshort, GLshort);
	typedef GLvoid (*_glVertexAttrib2f)(GLuint, GLfloat, GLfloat);
	typedef GLvoid (*_glVertexAttrib2d)(GLuint, GLdouble, GLdouble);
	typedef GLvoid (*_glVertexAttrib3s)(GLuint, GLshort, GLshort, GLshort);
	typedef GLvoid (*_glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat);
	typedef GLvoid (*_glVertexAttrib3d)(GLuint, GLdouble, GLdouble, GLdouble);
	typedef GLvoid (*_glVertexAttrib4s)(GLuint, GLshort, GLshort, GLshort, GLshort);
	typedef GLvoid (*_glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
	typedef GLvoid (*_glVertexAttrib4d)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
	typedef GLvoid (*_glVertexAttrib1sv)(GLuint, const GLshort*);
	typedef GLvoid (*_glVertexAttrib1fv)(GLuint, const GLfloat*);
	typedef GLvoid (*_glVertexAttrib1dv)(GLuint, const GLdouble*);
	typedef GLvoid (*_glVertexAttrib2sv)(GLuint, const GLshort*);
	typedef GLvoid (*_glVertexAttrib2fv)(GLuint, const GLfloat*);
	typedef GLvoid (*_glVertexAttrib2dv)(GLuint, const GLdouble*);
	typedef GLvoid (*_glVertexAttrib3sv)(GLuint, const GLshort*);
	typedef GLvoid (*_glVertexAttrib3fv)(GLuint, const GLfloat*);
	typedef GLvoid (*_glVertexAttrib3dv)(GLuint, const GLdouble*);
	typedef GLvoid (*_glVertexAttrib4sv)(GLuint, const GLshort*);
	typedef GLvoid (*_glVertexAttrib4fv)(GLuint, const GLfloat*);
	typedef GLvoid (*_glVertexAttrib4dv)(GLuint, const GLdouble*);
	typedef GLvoid (*_glVertexAttrib4bv)(GLuint, const GLbyte*);
	typedef GLvoid (*_glVertexAttrib4iv)(GLuint, const GLint*);
	typedef GLvoid (*_glVertexAttrib4ubv)(GLuint, const GLubyte*);
	typedef GLvoid (*_glVertexAttrib4usv)(GLuint, const GLushort*);
	typedef GLvoid (*_glVertexAttrib4uiv)(GLuint, const GLuint*);
	typedef GLvoid (*_glVertexAttrib4Nub)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
	typedef GLvoid (*_glVertexAttrib4Nbv)(GLuint, const GLbyte*);
	typedef GLvoid (*_glVertexAttrib4Nsv)(GLuint, const GLshort*);
	typedef GLvoid (*_glVertexAttrib4Niv)(GLuint, const GLint*);
	typedef GLvoid (*_glVertexAttrib4Nubv)(GLuint, const GLubyte*);
	typedef GLvoid (*_glVertexAttrib4Nusv)(GLuint, const GLushort*);
	typedef GLvoid (*_glVertexAttrib4Nuiv)(GLuint, const GLuint*);
	typedef GLvoid (*_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);
	typedef GLvoid (*_glEnableVertexAttribArray)(GLuint);
	typedef GLvoid (*_glDisableVertexAttribArray)(GLuint);
	typedef GLvoid (*_glBindAttribLocation)(GLuint, GLuint, const GLchar*);
	typedef GLvoid (*_glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
	typedef GLint (*_glGetAttribLocation)(GLuint, const GLchar*);
	typedef GLint (*_glGetVertexAttribdv)(GLuint, GLenum, GLdouble*);
	typedef GLint (*_glGetVertexAttribfv)(GLuint, GLenum, GLfloat*);
	typedef GLint (*_glGetVertexAttribiv)(GLuint, GLenum, GLint*);
	typedef GLint (*_glGetVertexAttribPointerv)(GLuint, GLenum, GLvoid**);
	typedef GLvoid (*_glBlendEquationSeparate)(GLenum, GLenum);





	// GL function pointers
	extern _glDrawBuffers bo_glDrawBuffers;
	extern _glStencilOpSeparate bo_glStencilOpSeparate;
	extern _glStencilFuncSeparate bo_glStencilFuncSeparate;
	extern _glIsShader bo_glIsShader;
	extern _glIsProgram bo_glIsProgram;
	extern _glGetAttachedShaders bo_glGetAttachedShaders;
	extern _glCreateShader bo_glCreateShader;
	extern _glShaderSource bo_glShaderSource;
	extern _glCompileShader bo_glCompileShader;
	extern _glDeleteShader bo_glDeleteShader;
	extern _glCreateProgram bo_glCreateProgram;
	extern _glAttachShader bo_glAttachShader;
	extern _glDetachShader bo_glDetachShader;
	extern _glLinkProgram bo_glLinkProgram;
	extern _glUseProgram bo_glUseProgram;
	extern _glDeleteProgram bo_glDeleteProgram;
	extern _glGetShaderInfoLog bo_glGetShaderInfoLog;
	extern _glGetProgramInfoLog bo_glGetProgramInfoLog;
	extern _glGetShaderSource bo_glGetShaderSource;
	extern _glGetUniformfv bo_glGetUniformfv;
	extern _glGetUniformiv bo_glGetUniformiv;
	extern _glGetProgramiv bo_glGetProgramiv;
	extern _glGetShaderiv bo_glGetShaderiv;
	extern _glUniform1f bo_glUniform1f;
	extern _glUniform2f bo_glUniform2f;
	extern _glUniform3f bo_glUniform3f;
	extern _glUniform4f bo_glUniform4f;
	extern _glUniform1i bo_glUniform1i;
	extern _glUniform2i bo_glUniform2i;
	extern _glUniform3i bo_glUniform3i;
	extern _glUniform4i bo_glUniform4i;
	extern _glUniform1fv bo_glUniform1fv;
	extern _glUniform2fv bo_glUniform2fv;
	extern _glUniform3fv bo_glUniform3fv;
	extern _glUniform4fv bo_glUniform4fv;
	extern _glUniform1iv bo_glUniform1iv;
	extern _glUniform2iv bo_glUniform2iv;
	extern _glUniform3iv bo_glUniform3iv;
	extern _glUniform4iv bo_glUniform4iv;
	extern _glUniformMatrix2fv bo_glUniformMatrix2fv;
	extern _glUniformMatrix3fv bo_glUniformMatrix3fv;
	extern _glUniformMatrix4fv bo_glUniformMatrix4fv;
	extern _glValidateProgram bo_glValidateProgram;
	extern _glGetUniformLocation bo_glGetUniformLocation;
	extern _glGetActiveUniform bo_glGetActiveUniform;
	extern _glVertexAttrib1s bo_glVertexAttrib1s;
	extern _glVertexAttrib1f bo_glVertexAttrib1f;
	extern _glVertexAttrib1d bo_glVertexAttrib1d;
	extern _glVertexAttrib2s bo_glVertexAttrib2s;
	extern _glVertexAttrib2f bo_glVertexAttrib2f;
	extern _glVertexAttrib2d bo_glVertexAttrib2d;
	extern _glVertexAttrib3s bo_glVertexAttrib3s;
	extern _glVertexAttrib3f bo_glVertexAttrib3f;
	extern _glVertexAttrib3d bo_glVertexAttrib3d;
	extern _glVertexAttrib4s bo_glVertexAttrib4s;
	extern _glVertexAttrib4f bo_glVertexAttrib4f;
	extern _glVertexAttrib4d bo_glVertexAttrib4d;
	extern _glVertexAttrib1sv bo_glVertexAttrib1sv;
	extern _glVertexAttrib1fv bo_glVertexAttrib1fv;
	extern _glVertexAttrib1dv bo_glVertexAttrib1dv;
	extern _glVertexAttrib2sv bo_glVertexAttrib2sv;
	extern _glVertexAttrib2fv bo_glVertexAttrib2fv;
	extern _glVertexAttrib2dv bo_glVertexAttrib2dv;
	extern _glVertexAttrib3sv bo_glVertexAttrib3sv;
	extern _glVertexAttrib3fv bo_glVertexAttrib3fv;
	extern _glVertexAttrib3dv bo_glVertexAttrib3dv;
	extern _glVertexAttrib4sv bo_glVertexAttrib4sv;
	extern _glVertexAttrib4fv bo_glVertexAttrib4fv;
	extern _glVertexAttrib4dv bo_glVertexAttrib4dv;
	extern _glVertexAttrib4bv bo_glVertexAttrib4bv;
	extern _glVertexAttrib4iv bo_glVertexAttrib4iv;
	extern _glVertexAttrib4ubv bo_glVertexAttrib4ubv;
	extern _glVertexAttrib4usv bo_glVertexAttrib4usv;
	extern _glVertexAttrib4uiv bo_glVertexAttrib4uiv;
	extern _glVertexAttrib4Nub bo_glVertexAttrib4Nub;
	extern _glVertexAttrib4Nbv bo_glVertexAttrib4Nbv;
	extern _glVertexAttrib4Nsv bo_glVertexAttrib4Nsv;
	extern _glVertexAttrib4Niv bo_glVertexAttrib4Niv;
	extern _glVertexAttrib4Nubv bo_glVertexAttrib4Nubv;
	extern _glVertexAttrib4Nusv bo_glVertexAttrib4Nusv;
	extern _glVertexAttrib4Nuiv bo_glVertexAttrib4Nuiv;
	extern _glVertexAttribPointer bo_glVertexAttribPointer;
	extern _glEnableVertexAttribArray bo_glEnableVertexAttribArray;
	extern _glDisableVertexAttribArray bo_glDisableVertexAttribArray;
	extern _glBindAttribLocation bo_glBindAttribLocation;
	extern _glGetActiveAttrib bo_glGetActiveAttrib;
	extern _glGetAttribLocation bo_glGetAttribLocation;
	extern _glGetVertexAttribdv bo_glGetVertexAttribdv;
	extern _glGetVertexAttribfv bo_glGetVertexAttribfv;
	extern _glGetVertexAttribiv bo_glGetVertexAttribiv;
	extern _glGetVertexAttribPointerv bo_glGetVertexAttribPointerv;
	extern _glBlendEquationSeparate bo_glBlendEquationSeparate;
}; // extern "C"


#define FOG_COORD_ARRAY_BUFFER_BINDING   FOG_COORDINATE_ARRAY_BUFFER_BINDING
#define GL_BLEND_EQUATION_RGB            GL_BLEND_EQUATION


// ARB_shader_objects
#define GL_DELETE_STATUS                                0x8B80
#define GL_COMPILE_STATUS                               0x8B81
#define GL_LINK_STATUS                                  0x8B82
#define GL_VALIDATE_STATUS                              0x8B83
#define GL_INFO_LOG_LENGTH                              0x8B84
#define GL_ATTACHED_SHADERS                             0x8B85
#define GL_ACTIVE_UNIFORMS                              0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH                    0x8B87
#define GL_SHADER_SOURCE_LENGTH                         0x8B88
#define GL_SHADER_TYPE                                  0x8B4F
#define GL_FLOAT_VEC2                                   0x8B50
#define GL_FLOAT_VEC3                                   0x8B51
#define GL_FLOAT_VEC4                                   0x8B52
#define GL_INT_VEC2                                     0x8B53
#define GL_INT_VEC3                                     0x8B54
#define GL_INT_VEC4                                     0x8B55
#define GL_BOOL                                         0x8B56
#define GL_BOOL_VEC2                                    0x8B57
#define GL_BOOL_VEC3                                    0x8B58
#define GL_BOOL_VEC4                                    0x8B59
#define GL_FLOAT_MAT2                                   0x8B5A
#define GL_FLOAT_MAT3                                   0x8B5B
#define GL_FLOAT_MAT4                                   0x8B5C
#define GL_SAMPLER_1D                                   0x8B5D
#define GL_SAMPLER_2D                                   0x8B5E
#define GL_SAMPLER_3D                                   0x8B5F
#define GL_SAMPLER_CUBE                                 0x8B60
#define GL_SAMPLER_1D_SHADOW                            0x8B61
#define GL_SAMPLER_2D_SHADOW                            0x8B62
#define GL_CURRENT_PROGRAM                              0x8B8D
// ARB_vertex_shader
#define GL_VERTEX_SHADER                                0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                0x8B4A
#define GL_MAX_VARYING_FLOATS                           0x8B4B
#define GL_MAX_VERTEX_ATTRIBS                           0x8869
#define GL_MAX_TEXTURE_IMAGE_UNITS                      0x8872
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS               0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS             0x8B4D
#define GL_MAX_TEXTURE_COORDS                           0x8871
#define GL_VERTEX_PROGRAM_POINT_SIZE                    0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE                      0x8643
#define GL_OBJECT_ACTIVE_ATTRIBUTES                     0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH           0x8B8A
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED                  0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE                     0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE                   0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE                     0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED               0x886A
#define GL_CURRENT_VERTEX_ATTRIB                        0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER                  0x8645
#define GL_FLOAT_VEC2                                   0x8B50
#define GL_FLOAT_VEC3                                   0x8B51
#define GL_FLOAT_VEC4                                   0x8B52
#define GL_FLOAT_MAT2                                   0x8B5A
#define GL_FLOAT_MAT3                                   0x8B5B
#define GL_FLOAT_MAT4                                   0x8B5C
// ARB_fragment_shader
#define GL_FRAGMENT_SHADER                              0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS              0x8B49
#define GL_MAX_TEXTURE_COORDS                           0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS                      0x8872
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT              0x8B8B
// ARB_shading_language_100
#define GL_SHADING_LANGUAGE_VERSION                     0x8B8C
// ARB_draw_buffers
#define GL_MAX_DRAW_BUFFERS                             0x8824
#define GL_DRAW_BUFFER0                                 0x8825
#define GL_DRAW_BUFFER1                                 0x8826
#define GL_DRAW_BUFFER2                                 0x8827
#define GL_DRAW_BUFFER3                                 0x8828
#define GL_DRAW_BUFFER4                                 0x8829
#define GL_DRAW_BUFFER5                                 0x882A
#define GL_DRAW_BUFFER6                                 0x882B
#define GL_DRAW_BUFFER7                                  0x882C
#define GL_DRAW_BUFFER8                                 0x882D
#define GL_DRAW_BUFFER9                                 0x882E
#define GL_DRAW_BUFFER10                                0x882F
#define GL_DRAW_BUFFER11                                0x8830
#define GL_DRAW_BUFFER12                                0x8831
#define GL_DRAW_BUFFER13                                0x8832
#define GL_DRAW_BUFFER14                                0x8833
#define GL_DRAW_BUFFER15                                0x8834
// ARB_point_sprite
#define GL_POINT_SPRITE                                 0x8861
#define GL_COORD_REPLACE                                0x8862
#define GL_POINT_SPRITE_COORD_ORIGIN                    0x8CA0
#define GL_LOWER_LEFT                                   0x8CA1
#define GL_UPPER_LEFT                                   0x8CA2
// ATI_separate_stencil
#define GL_STENCIL_BACK_FUNC                            0x8800
#define GL_STENCIL_BACK_FAIL                            0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL                 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS                 0x8803
// EXT_blend_equation_separate
#define GL_BLEND_EQUATION_ALPHA                         0x883D



#if BOGL_DO_DLOPEN
#define glDrawBuffers bo_glDrawBuffers
#define glStencilOpSeparate bo_glStencilOpSeparate
#define glStencilFuncSeparate bo_glStencilFuncSeparate
#define glIsShader bo_glIsShader
#define glIsProgram bo_glIsProgram
#define glGetAttachedShaders bo_glGetAttachedShaders
#define glCreateShader bo_glCreateShader
#define glShaderSource bo_glShaderSource
#define glCompileShader bo_glCompileShader
#define glDeleteShader bo_glDeleteShader
#define glCreateProgram bo_glCreateProgram
#define glAttachShader bo_glAttachShader
#define glDetachShader bo_glDetachShader
#define glLinkProgram bo_glLinkProgram
#define glUseProgram bo_glUseProgram
#define glDeleteProgram bo_glDeleteProgram
#define glGetShaderInfoLog bo_glGetShaderInfoLog
#define glGetProgramInfoLog bo_glGetProgramInfoLog
#define glGetShaderSource bo_glGetShaderSource
#define glGetUniformfv bo_glGetUniformfv
#define glGetUniformiv bo_glGetUniformiv
#define glGetProgramiv bo_glGetProgramiv
#define glGetShaderiv bo_glGetShaderiv
#define glUniform1f bo_glUniform1f
#define glUniform2f bo_glUniform2f
#define glUniform3f bo_glUniform3f
#define glUniform4f bo_glUniform4f
#define glUniform1i bo_glUniform1i
#define glUniform2i bo_glUniform2i
#define glUniform3i bo_glUniform3i
#define glUniform4i bo_glUniform4i
#define glUniform1fv bo_glUniform1fv
#define glUniform2fv bo_glUniform2fv
#define glUniform3fv bo_glUniform3fv
#define glUniform4fv bo_glUniform4fv
#define glUniform1iv bo_glUniform1iv
#define glUniform2iv bo_glUniform2iv
#define glUniform3iv bo_glUniform3iv
#define glUniform4iv bo_glUniform4iv
#define glUniformMatrix2fv bo_glUniformMatrix2fv
#define glUniformMatrix3fv bo_glUniformMatrix3fv
#define glUniformMatrix4fv bo_glUniformMatrix4fv
#define glValidateProgram bo_glValidateProgram
#define glGetUniformLocation bo_glGetUniformLocation
#define glGetActiveUniform bo_glGetActiveUniform
#define glVertexAttrib1s bo_glVertexAttrib1s
#define glVertexAttrib1f bo_glVertexAttrib1f
#define glVertexAttrib1d bo_glVertexAttrib1d
#define glVertexAttrib2s bo_glVertexAttrib2s
#define glVertexAttrib2f bo_glVertexAttrib2f
#define glVertexAttrib2d bo_glVertexAttrib2d
#define glVertexAttrib3s bo_glVertexAttrib3s
#define glVertexAttrib3f bo_glVertexAttrib3f
#define glVertexAttrib3d bo_glVertexAttrib3d
#define glVertexAttrib4s bo_glVertexAttrib4s
#define glVertexAttrib4f bo_glVertexAttrib4f
#define glVertexAttrib4d bo_glVertexAttrib4d
#define glVertexAttrib1sv bo_glVertexAttrib1sv
#define glVertexAttrib1fv bo_glVertexAttrib1fv
#define glVertexAttrib1dv bo_glVertexAttrib1dv
#define glVertexAttrib2sv bo_glVertexAttrib2sv
#define glVertexAttrib2fv bo_glVertexAttrib2fv
#define glVertexAttrib2dv bo_glVertexAttrib2dv
#define glVertexAttrib3sv bo_glVertexAttrib3sv
#define glVertexAttrib3fv bo_glVertexAttrib3fv
#define glVertexAttrib3dv bo_glVertexAttrib3dv
#define glVertexAttrib4sv bo_glVertexAttrib4sv
#define glVertexAttrib4fv bo_glVertexAttrib4fv
#define glVertexAttrib4dv bo_glVertexAttrib4dv
#define glVertexAttrib4bv bo_glVertexAttrib4bv
#define glVertexAttrib4iv bo_glVertexAttrib4iv
#define glVertexAttrib4ubv bo_glVertexAttrib4ubv
#define glVertexAttrib4usv bo_glVertexAttrib4usv
#define glVertexAttrib4uiv bo_glVertexAttrib4uiv
#define glVertexAttrib4Nub bo_glVertexAttrib4Nub
#define glVertexAttrib4Nbv bo_glVertexAttrib4Nbv
#define glVertexAttrib4Nsv bo_glVertexAttrib4Nsv
#define glVertexAttrib4Niv bo_glVertexAttrib4Niv
#define glVertexAttrib4Nubv bo_glVertexAttrib4Nubv
#define glVertexAttrib4Nusv bo_glVertexAttrib4Nusv
#define glVertexAttrib4Nuiv bo_glVertexAttrib4Nuiv
#define glVertexAttribPointer bo_glVertexAttribPointer
#define glEnableVertexAttribArray bo_glEnableVertexAttribArray
#define glDisableVertexAttribArray bo_glDisableVertexAttribArray
#define glBindAttribLocation bo_glBindAttribLocation
#define glGetActiveAttrib bo_glGetActiveAttrib
#define glGetAttribLocation bo_glGetAttribLocation
#define glGetVertexAttribdv bo_glGetVertexAttribdv
#define glGetVertexAttribfv bo_glGetVertexAttribfv
#define glGetVertexAttribiv bo_glGetVertexAttribiv
#define glGetVertexAttribPointerv bo_glGetVertexAttribPointerv
#define glBlendEquationSeparate bo_glBlendEquationSeparate
#endif // BOGL_DO_DLOPEN

#endif




