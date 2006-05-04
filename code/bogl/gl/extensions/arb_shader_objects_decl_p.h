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

#ifndef ARB_SHADER_OBJECTS_DECL_P_H
#define ARB_SHADER_OBJECTS_DECL_P_H

// note: compliant OpenGL implementations don't have to implement an extension!


#ifndef GL_ARB_shader_objects
typedef char GLcharARB;
typedef unsigned int GLhandleARB;
#endif

extern "C" {
	// GL typedefs
	typedef GLvoid (*_glDeleteObjectARB) (GLhandleARB);
	typedef GLhandleARB (*_glGetHandleARB) (GLenum);
	typedef GLvoid (*_glDetachObjectARB) (GLhandleARB, GLhandleARB);
	typedef GLhandleARB (*_glCreateShaderObjectARB) (GLenum);
	typedef GLvoid (*_glShaderSourceARB) (GLhandleARB, GLsizei, const GLcharARB**, const GLint*);
	typedef GLvoid (*_glCompileShaderARB) (GLhandleARB);
	typedef GLhandleARB (*_glCreateProgramObjectARB) (GLvoid);
	typedef GLvoid (*_glAttachObjectARB) (GLhandleARB, GLhandleARB);
	typedef GLvoid (*_glLinkProgramARB) (GLhandleARB);
	typedef GLvoid (*_glUseProgramObjectARB) (GLhandleARB);
	typedef GLvoid (*_glValidateProgramARB) (GLhandleARB);
	typedef GLvoid (*_glUniform1fARB) (GLint, GLfloat);
	typedef GLvoid (*_glUniform2fARB) (GLint, GLfloat, GLfloat);
	typedef GLvoid (*_glUniform3fARB) (GLint, GLfloat, GLfloat, GLfloat);
	typedef GLvoid (*_glUniform4fARB) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
	typedef GLvoid (*_glUniform1iARB) (GLint, GLint);
	typedef GLvoid (*_glUniform2iARB) (GLint, GLint, GLint);
	typedef GLvoid (*_glUniform3iARB) (GLint, GLint, GLint, GLint);
	typedef GLvoid (*_glUniform4iARB) (GLint, GLint, GLint, GLint, GLfloat);
	typedef GLvoid (*_glUniform1fvARB) (GLint, GLsizei, const GLfloat*);
	typedef GLvoid (*_glUniform2fvARB) (GLint, GLsizei, const GLfloat*);
	typedef GLvoid (*_glUniform3fvARB) (GLint, GLsizei, const GLfloat*);
	typedef GLvoid (*_glUniform4fvARB) (GLint, GLsizei, const GLfloat*);
	typedef GLvoid (*_glUniform1ivARB) (GLint, GLsizei, const GLint*);
	typedef GLvoid (*_glUniform2ivARB) (GLint, GLsizei, const GLint*);
	typedef GLvoid (*_glUniform3ivARB) (GLint, GLsizei, const GLint*);
	typedef GLvoid (*_glUniform4ivARB) (GLint, GLsizei, const GLint*);
	typedef GLvoid (*_glUniformMatrix2fvARB) (GLint, GLsizei, GLboolean, const GLfloat*);
	typedef GLvoid (*_glUniformMatrix3fvARB) (GLint, GLsizei, GLboolean, const GLfloat*);
	typedef GLvoid (*_glUniformMatrix4fvARB) (GLint, GLsizei, GLboolean, const GLfloat*);
	typedef GLvoid (*_glGetObjectParameterfvARB) (GLhandleARB, GLenum, GLfloat*);
	typedef GLvoid (*_glGetObjectParameterivARB) (GLhandleARB, GLenum, GLint*);
	typedef GLvoid (*_glGetInfoLogARB) (GLhandleARB, GLsizei, GLsizei*, GLcharARB*);
	typedef GLvoid (*_glGetAttachedObjectsARB) (GLhandleARB, GLsizei, GLsizei*, GLhandleARB*);
	typedef GLint (*_glGetUniformLocationARB) (GLhandleARB, const GLcharARB*);
	typedef GLvoid (*_glGetActiveUniformARB) (GLhandleARB, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, const GLcharARB*);
	typedef GLvoid (*_glGetActiveUniformfvARB) (GLhandleARB, GLint, GLfloat*);
	typedef GLvoid (*_glGetActiveUniformivARB) (GLhandleARB, GLint, GLint*);
	typedef GLvoid (*_glGetShaderSourceARB) (GLhandleARB, GLsizei, GLsizei*, GLcharARB*);



	// GL function pointers
	extern _glDeleteObjectARB bo_glDeleteObjectARB;
	extern _glGetHandleARB bo_glGetHandleARB;
	extern _glDetachObjectARB bo_glDetachObjectARB;
	extern _glCreateShaderObjectARB bo_glCreateShaderObjectARB;
	extern _glShaderSourceARB bo_glShaderSourceARB;
	extern _glCompileShaderARB bo_glCompileShaderARB;
	extern _glCreateProgramObjectARB bo_glCreateProgramObjectARB;
	extern _glAttachObjectARB bo_glAttachObjectARB;
	extern _glLinkProgramARB bo_glLinkProgramARB;
	extern _glUseProgramObjectARB bo_glUseProgramObjectARB;
	extern _glValidateProgramARB bo_glValidateProgramARB;
	extern _glUniform1fARB bo_glUniform1fARB;
	extern _glUniform2fARB bo_glUniform2fARB;
	extern _glUniform3fARB bo_glUniform3fARB;
	extern _glUniform4fARB bo_glUniform4fARB;
	extern _glUniform1iARB bo_glUniform1iARB;
	extern _glUniform2iARB bo_glUniform2iARB;
	extern _glUniform3iARB bo_glUniform3iARB;
	extern _glUniform4iARB bo_glUniform4iARB;
	extern _glUniform1fvARB bo_glUniform1fvARB;
	extern _glUniform2fvARB bo_glUniform2fvARB;
	extern _glUniform3fvARB bo_glUniform3fvARB;
	extern _glUniform4fvARB bo_glUniform4fvARB;
	extern _glUniform1ivARB bo_glUniform1ivARB;
	extern _glUniform2ivARB bo_glUniform2ivARB;
	extern _glUniform3ivARB bo_glUniform3ivARB;
	extern _glUniform4ivARB bo_glUniform4ivARB;
	extern _glUniformMatrix2fvARB bo_glUniformMatrix2fvARB;
	extern _glUniformMatrix3fvARB bo_glUniformMatrix3fvARB;
	extern _glUniformMatrix4fvARB bo_glUniformMatrix4fvARB;
	extern _glGetObjectParameterfvARB bo_glGetObjectParameterfvARB;
	extern _glGetObjectParameterivARB bo_glGetObjectParameterivARB;
	extern _glGetInfoLogARB bo_glGetInfoLogARB;
	extern _glGetAttachedObjectsARB bo_glGetAttachedObjectsARB;
	extern _glGetUniformLocationARB bo_glGetUniformLocationARB;
	extern _glGetActiveUniformARB bo_glGetActiveUniformARB;
	extern _glGetActiveUniformfvARB bo_glGetActiveUniformfvARB;
	extern _glGetActiveUniformivARB bo_glGetActiveUniformivARB;
	extern _glGetShaderSourceARB bo_glGetShaderSourceARB;

}; // extern "C"


#define GL_PROGRAM_OBJECT_ARB                              0x8B40
#define GL_OBJECT_TYPE_ARB                                 0x8B4E
#define GL_OBJECT_SUBTYPE_ARB                              0x8B4F
#define GL_OBJECT_DELETE_STATUS_ARB                        0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB                       0x8B81
#define GL_OBJECT_LINK_STATUS_ARB                          0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB                      0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB                      0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB                     0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB                      0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB            0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB                 0x8B88
#define GL_SHADER_OBJECT_ARB                               0x8B48
#define GL_FLOAT                                           0x1406
#define GL_FLOAT_VEC2_ARB                                  0x8B50
#define GL_FLOAT_VEC3_ARB                                  0x8B51
#define GL_FLOAT_VEC4_ARB                                  0x8B52
#define GL_INT                                             0x1404
#define GL_INT_VEC2_ARB                                    0x8B53
#define GL_INT_VEC3_ARB                                    0x8B54
#define GL_INT_VEC4_ARB                                    0x8B55
#define GL_BOOL_ARB                                        0x8B56
#define GL_BOOL_VEC2_ARB                                   0x8B57
#define GL_BOOL_VEC3_ARB                                   0x8B58
#define GL_BOOL_VEC4_ARB                                   0x8B59
#define GL_FLOAT_MAT2_ARB                                  0x8B5A
#define GL_FLOAT_MAT3_ARB                                  0x8B5B
#define GL_FLOAT_MAT4_ARB                                  0x8B5C
#define GL_SAMPLER_1D_ARB                                  0x8B5D
#define GL_SAMPLER_2D_ARB                                  0x8B5E
#define GL_SAMPLER_3D_ARB                                  0x8B5F
#define GL_SAMPLER_CUBE_ARB                                0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB                           0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB                           0x8B62
#define GL_SAMPLER_2D_RECT_ARB                             0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB                      0x8B64


#ifdef BOGL_DO_DLOPEN
#define glDeleteObjectARB bo_glDeleteObjectARB
#define glGetHandleARB bo_glGetHandleARB
#define glDetachObjectARB bo_glDetachObjectARB
#define glCreateShaderObjectARB bo_glCreateShaderObjectARB
#define glShaderSourceARB bo_glShaderSourceARB
#define glCompileShaderARB bo_glCompileShaderARB
#define glCreateProgramObjectARB bo_glCreateProgramObjectARB
#define glAttachObjectARB bo_glAttachObjectARB
#define glLinkProgramARB bo_glLinkProgramARB
#define glUseProgramObjectARB bo_glUseProgramObjectARB
#define glValidateProgramARB bo_glValidateProgramARB
#define glUniform1fARB bo_glUniform1fARB
#define glUniform2fARB bo_glUniform2fARB
#define glUniform3fARB bo_glUniform3fARB
#define glUniform4fARB bo_glUniform4fARB
#define glUniform1iARB bo_glUniform1iARB
#define glUniform2iARB bo_glUniform2iARB
#define glUniform3iARB bo_glUniform3iARB
#define glUniform4iARB bo_glUniform4iARB
#define glUniform1fvARB bo_glUniform1fvARB
#define glUniform2fvARB bo_glUniform2fvARB
#define glUniform3fvARB bo_glUniform3fvARB
#define glUniform4fvARB bo_glUniform4fvARB
#define glUniform1ivARB bo_glUniform1ivARB
#define glUniform2ivARB bo_glUniform2ivARB
#define glUniform3ivARB bo_glUniform3ivARB
#define glUniform4ivARB bo_glUniform4ivARB
#define glUniformMatrix2fvARB bo_glUniformMatrix2fvARB
#define glUniformMatrix3fvARB bo_glUniformMatrix3fvARB
#define glUniformMatrix4fvARB bo_glUniformMatrix4fvARB
#define glGetObjectParameterfvARB bo_glGetObjectParameterfvARB
#define glGetObjectParameterivARB bo_glGetObjectParameterivARB
#define glGetInfoLogARB bo_glGetInfoLogARB
#define glGetAttachedObjectsARB bo_glGetAttachedObjectsARB
#define glGetUniformLocationARB bo_glGetUniformLocationARB
#define glGetActiveUniformARB bo_glGetActiveUniformARB
#define glGetActiveUniformfvARB bo_glGetActiveUniformfvARB
#define glGetActiveUniformivARB bo_glGetActiveUniformivARB
#define glGetShaderSourceARB bo_glGetShaderSourceARB
#endif // BOGL_DO_DLOPEN

#endif
