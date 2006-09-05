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

#include <GL/gl.h>
#include <GL/glu.h>

#include <bogl_decl_p.h>

#undef GLsizeiptrARB



class BoGLPrivate;
class BoGL
{
public:
	~BoGL();

	/**
	 * @return The global BoGL object. If no object has been created yet, it
	 * creates a new object and returns it.
	 **/
	static BoGL* bogl();

	/**
	 * Try to resolve the OpenGL symbols. For this we first try to load a
	 * libGL.so and libGLU.so (using dlopen()) and then resolve the required
	 * functions.
	 *
	 * @return The value of @ref isResolved, i.e. TRUE if resolving has been
	 * successful, otherwise FALSE.
	 **/
	bool resolveGLSymbols();
	bool isResolved() const;

	/**
	 * Manually initialize the internal data structures.
	 *
	 * This method expects that @ref isResolved is TRUE. Additionally there
	 * must be a current GLX context available (as OpenGL functions are only
	 * guaranteed to work correctly once a context has been created).
	 *
	 * Calling this method again after initializing has been successful is a
	 * noop and results in no error.
	 *
	 * @return The value of @ref isInitialized
	 **/
	bool initialize();
	bool isInitialized() const;

	/**
	 * @return The (absolute) filename of libGL.so that is being used, i.e.
	 * that was picked by @ref resolveGLSymbols. @ref QString::null if @ref
	 * resolveGLSymbols has not been called yet.
	 **/
	const QString& OpenGLFile() const;
	/**
	 * @return The (absolute) filename of libGLU.so that is being used, i.e.
	 * that was picked by @ref resolveGLSymbols. @ref QString::null if @ref
	 * resolveGLSymbols has not been called yet.
	 **/
	const QString& GLUFile() const;

	/**
	 * Must be called @em after the context has been created/made current.
	 * Put it into the initializeGL() calll.
	 *
	 * It is necessary to create the context first, because apparently
	 * glGetString(GL_EXTENSIONS) depends on it, which is required.
	 **/
	void resolveGLExtensionSymbols();

	/**
	 * @return The OpenGL version that is installed. See the @ref
	 * MAKE_VERSION_BOGL macro.
	 *
	 * This method automatically calls @ref initialize.
	 **/
	unsigned int OpenGLVersion() const;
	/**
	 * @return The version string that is returned by the OpenGL
	 * installation that is being used.
	 *
	 * This method automatically calls @ref initialize.
	 **/
	QString OpenGLVersionString() const;
	/**
	 * @return The vendor string that is returned by the OpenGL
	 * installation that is being used.
	 *
	 * This method automatically calls @ref initialize.
	 **/
	QString OpenGLVendorString() const;
	/**
	 * @return The renderer string that is returned by the OpenGL
	 * installation that is being used.
	 *
	 * This method automatically calls @ref initialize.
	 **/
	QString OpenGLRendererString() const;
	/**
	 * @return The extensions that are returned by the OpenGL
	 * installation that is being used.
	 *
	 * This method automatically calls @ref initialize.
	 **/
	QStringList OpenGLExtensions() const;
	/**
	 * @return The version string that is returned by the GLU
	 * installation that is being used.
	 *
	 * This method automatically calls @ref initialize.
	 **/
	QString GLUVersionString() const;
	/**
	 * @return The extensions that are returned by the GLU
	 * installation that is being used.
	 *
	 * This method automatically calls @ref initialize.
	 **/
	QStringList GLUExtensions() const;

protected:
	BoGL();

private:
	static BoGL* mBoGL;
	BoGLPrivate* d;
};


/**
 * bogl functions
 *
 * This is a C style interface to @ref BoGL
 **/

/**
 * See @ref BoGL::resolveGLSymbols
 **/
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

/**
 * @internal
 *
 * This function is called by @ref BoGL::initialize automatically. You don't
 * need to call this.
 *
 * Must be called @em after the context has been created/made current. Put it
 * into the initializeGL() calll.
 *
 * It is necessary to create the context first, because apparently
 * glGetString(GL_EXTENSIONS) depends on it, which is required.
 **/
void boglResolveGLExtensionSymbols();



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

#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_TEXTURE_MODE             0x884B
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_COMPARE_R_TO_TEXTURE           0x884E


#endif
