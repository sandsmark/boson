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

#define QT_CLEAN_NAMESPACE

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
// TODO: glx

#undef GLsizeiptrARB


// bogl variables
extern bool bogl_inited;

// bogl functions
void boglInit();
unsigned int boglGetOpenGLVersion();
QString boglGetOpenGLVersionString();
QString boglGetOpenGLVendorString();
QString boglGetOpenGLRendererString();
QStringList boglGetOpenGLExtensions();
QString boglGetGLUVersionString();
QStringList boglGetGLUExtensions();



// Typedefs
// This type was added for vbo extension.
typedef int GLsizeiptrARB;

// Function prototypes
typedef void (*_boglBlendColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (*_boglDeleteBuffers)(GLsizei, const GLuint*);
typedef void (*_boglGenBuffers)(GLsizei, GLuint*);
typedef void (*_boglBindBuffer)(GLenum, GLuint);
typedef void (*_boglBufferData)(GLenum, GLsizeiptrARB, const GLvoid*, GLenum);
typedef GLvoid* (* _boglMapBuffer) (GLenum target, GLenum access);
typedef GLboolean (* _boglUnmapBuffer) (GLenum target);
typedef void (*_boglActiveTexture)(GLenum);


// Function pointers for extensions
// Blendcolor
extern _boglBlendColor boglBlendColor;
// VBO
extern _boglDeleteBuffers boglDeleteBuffers;
extern _boglGenBuffers boglGenBuffers;
extern _boglBindBuffer boglBindBuffer;
extern _boglBufferData boglBufferData;
extern _boglMapBuffer boglMapBuffer;
extern _boglUnmapBuffer boglUnmapBuffer;
// Textures
extern _boglActiveTexture boglActiveTexture;


// Defines
#define GL_CONSTANT_COLOR                 0x8001

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_WRITE_ONLY                     0x88B9
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
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


/**
 * This header includes more or less complete support for following extensions:
 * @li GL_multitexture_ARB (GL 1.3) - first 4 texture units
 * @li GL_blend_color_EXT / GL_ARB_imaging() - boglBlendColor()
 * @li GL_vertex_buffer_object_ARB (GL 1.5) - necessary VBO functionality
 * ...
 **/

#endif
