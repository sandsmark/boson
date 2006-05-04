/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks <rivolaks@hot.ee>

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

#ifndef BOGL_H
#define BOGL_H

class QString;
class QStringList;

// macro that is used for versions returned by bogl (such as OpenGL version)
#define MAKE_VERSION_BOGL(a,b,c) ( ((a) << 16) | ((b) << 8) | (c) )

// Defines
#define GL_GLEXT_LEGACY

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#ifdef __gl_h_
#error You should include "bogl.h" before <GL/gl.h>
#endif

// Some gl headers define GLsizeiptrARB type while others do not and there
//  doesn't seem to be any good way to find out if it's defined. So we replace
//  OpenGL type's name
#define GLsizeiptrARB GLsizeiptrARB_ORIGINAL

// Include OpenGL headers
#include <GL/gl.h>
#include <GL/glu.h>

// AB: the Qt includes are required to avoid conflicts with GLX
//#include <qnamespace.h>
//#include <qglobal.h>
//#include <qevent.h>
//#include <GL/glx.h>


#include <bogl_decl_p.h>

#undef GLsizeiptrARB


// bogl variables
extern bool bogl_inited;

// bogl functions
void boglInit();
bool boglResolveGLSymbols();
unsigned int boglGetOpenGLVersion();
QString boglGetOpenGLVersionString();
QString boglGetOpenGLVendorString();
QString boglGetOpenGLRendererString();
QStringList boglGetOpenGLExtensions();
QString boglGetGLUVersionString();
QStringList boglGetGLUExtensions();


// internal
class QLibrary;
bool boglResolveLibGLSymbols(QLibrary& gl);
bool boglResolveLibGLUSymbols(QLibrary& glu);



// Typedefs
// This type was added for vbo extension.
// TODO: why do we use "ARB" here?
typedef int GLsizeiptrARB;
#ifndef GL_ARB_shader_objects
/* GL types for handling shader object handles and characters */
typedef char GLcharARB;		/* native character */
#endif

// Function prototypes
typedef void (*_boglActiveTexture)(GLenum);

typedef void (*_boglAttachShader) (GLuint, GLuint);
typedef void (*_boglCompileShader) (GLuint);
typedef GLuint (*_boglCreateProgram) (void);
typedef GLuint (*_boglCreateShader) (GLenum);
typedef void (*_boglDeleteProgram) (GLuint);
typedef void (*_boglDeleteShader) (GLuint);
typedef void (*_boglGetProgramInfoLog) (GLuint, GLsizei, GLsizei *, GLcharARB *);
typedef void (*_boglGetProgramiv) (GLuint, GLenum, GLint *);
typedef void (*_boglGetShaderInfoLog) (GLuint, GLsizei, GLsizei *, GLcharARB *);
typedef void (*_boglGetShaderiv) (GLuint, GLenum, GLint *);
typedef GLint (*_boglGetUniformLocation) (GLuint, const GLcharARB *);
typedef void (*_boglLinkProgram) (GLuint);
typedef void (*_boglShaderSource) (GLuint, GLsizei, const GLcharARB* *, const GLint *);
typedef void (*_boglUniform1f) (GLint, GLfloat);
typedef void (*_boglUniform1i) (GLint, GLint);
typedef void (*_boglUniform2fv) (GLint, GLsizei, const GLfloat *);
typedef void (*_boglUniform3fv) (GLint, GLsizei, const GLfloat *);
typedef void (*_boglUniform4fv) (GLint, GLsizei, const GLfloat *);
typedef void (*_boglUseProgram) (GLuint);

typedef void (*_boglBindRenderbuffer) (GLenum, GLuint);
typedef void (*_boglDeleteRenderbuffers) (GLsizei, const GLuint *);
typedef void (*_boglGenRenderbuffers) (GLsizei, GLuint *);
typedef void (*_boglRenderbufferStorage) (GLenum, GLenum, GLsizei, GLsizei);
typedef void (*_boglBindFramebuffer) (GLenum, GLuint);
typedef void (*_boglDeleteFramebuffers) (GLsizei, const GLuint *);
typedef void (*_boglGenFramebuffers) (GLsizei, GLuint *);
typedef GLenum (*_boglCheckFramebufferStatus) (GLenum);
typedef void (*_boglFramebufferTexture2D) (GLenum, GLenum, GLenum, GLuint, GLint);
typedef void (*_boglFramebufferRenderbuffer) (GLenum, GLenum, GLenum, GLuint);
typedef void (*_boglGenerateMipmap) (GLenum);



// Function pointers for extensions
// Textures
extern _boglActiveTexture boglActiveTexture;
// Shaders
extern _boglAttachShader boglAttachShader;
extern _boglCompileShader boglCompileShader;
extern _boglCreateProgram boglCreateProgram;
extern _boglCreateShader boglCreateShader;
extern _boglDeleteProgram boglDeleteProgram;
extern _boglDeleteShader boglDeleteShader;
extern _boglGetProgramInfoLog boglGetProgramInfoLog;
extern _boglGetProgramiv boglGetProgramiv;
extern _boglGetShaderInfoLog boglGetShaderInfoLog;
extern _boglGetShaderiv boglGetShaderiv;
extern _boglGetUniformLocation boglGetUniformLocation;
extern _boglLinkProgram boglLinkProgram;
extern _boglShaderSource boglShaderSource;
extern _boglUniform1f boglUniform1f;
extern _boglUniform1i boglUniform1i;
extern _boglUniform2fv boglUniform2fv;
extern _boglUniform3fv boglUniform3fv;
extern _boglUniform4fv boglUniform4fv;
extern _boglUseProgram boglUseProgram;
// FBO
extern _boglBindRenderbuffer boglBindRenderbuffer;
extern _boglDeleteRenderbuffers boglDeleteRenderbuffers;
extern _boglGenRenderbuffers boglGenRenderbuffers;
extern _boglRenderbufferStorage boglRenderbufferStorage;
extern _boglBindFramebuffer boglBindFramebuffer;
extern _boglDeleteFramebuffers boglDeleteFramebuffers;
extern _boglGenFramebuffers boglGenFramebuffers;
extern _boglCheckFramebufferStatus boglCheckFramebufferStatus;
extern _boglFramebufferTexture2D boglFramebufferTexture2D;
extern _boglFramebufferRenderbuffer boglFramebufferRenderbuffer;
extern _boglGenerateMipmap boglGenerateMipmap;



#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE          0x84E1
#define GL_MAX_TEXTURE_UNITS              0x84E2

#define GL_REFLECTION_MAP                 0x8512
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

#define GL_GENERATE_MIPMAP                0x8191

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_EXT_texture_lod_bias
#define GL_MAX_TEXTURE_LOD_BIAS_EXT       0x84FD
#define GL_TEXTURE_FILTER_CONTROL_EXT     0x8500
#define GL_TEXTURE_LOD_BIAS_EXT           0x8501
#endif

#define GL_COMBINE                        0x8570
#define GL_COMBINE_RGB                    0x8571
#define GL_COMBINE_ALPHA                  0x8572
#define GL_SOURCE0_RGB                    0x8580
#define GL_SOURCE1_RGB                    0x8581
#define GL_SOURCE2_RGB                    0x8582
#define GL_SOURCE0_ALPHA                  0x8588
#define GL_SOURCE1_ALPHA                  0x8589
#define GL_SOURCE2_ALPHA                  0x858A
#define GL_OPERAND0_RGB                   0x8590
#define GL_OPERAND1_RGB                   0x8591
#define GL_OPERAND2_RGB                   0x8592
#define GL_OPERAND0_ALPHA                 0x8598
#define GL_OPERAND1_ALPHA                 0x8599
#define GL_OPERAND2_ALPHA                 0x859A
#define GL_RGB_SCALE                      0x8573
#define GL_ADD_SIGNED                     0x8574
#define GL_INTERPOLATE                    0x8575
#define GL_SUBTRACT                       0x84E7
#define GL_CONSTANT                       0x8576
#define GL_PRIMARY_COLOR                  0x8577
#define GL_PREVIOUS                       0x8578
#define GL_DOT3_RGB                       0x86AE
#define GL_DOT3_RGBA                      0x86AF

#define GL_PROGRAM_OBJECT                 0x8B40
#define GL_SHADER_OBJECT                  0x8B48
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30

#ifndef GL_EXT_framebuffer_object
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT 0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS 0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS 0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#endif

#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_TEXTURE_MODE             0x884B
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_COMPARE_R_TO_TEXTURE           0x884E


// AB: defines for extensions:
//     (note: the above defines are partially for extensions too, one day we
//      should sort this properly)
// GL_ARB_multitexture (part of OpenGL 1.2.1)
#define GL_MAX_TEXTURE_UNITS_ARB          0x84E2



/**
 * This header includes more or less complete support for following extensions:
 * @li GL_multitexture_ARB (GL 1.3) - first 4 texture units
 * @li GL_texture_env_combine_ARB (GL 1.3) - full support
 * @li GL_texture_env_dot3_ARB (GL 1.3) - full support
 * @li GL_shader_object_ARB and other shader extensions (GL 2.0) - necessary stuff
 * ...
 **/

#endif
