// This file was generated from arb_multitexture_decl_p.boglc
// Do not edit this file (all changes will be lost)!

#ifndef ARB_MULTITEXTURE_DECL_P_H
#define ARB_MULTITEXTURE_DECL_P_H

#ifndef BOGL_H
#error Never include this file directly! Include bogl.h instead!
#endif

#include "bogl/bogl_do_dlopen.h"








#define GL_TEXTURE0_ARB                       0x84C0
#define GL_TEXTURE1_ARB                       0x84C1
#define GL_TEXTURE2_ARB                       0x84C2
#define GL_TEXTURE3_ARB                       0x84C3
#define GL_TEXTURE4_ARB                       0x84C4
#define GL_TEXTURE5_ARB                       0x84C5
#define GL_TEXTURE6_ARB                       0x84C6
#define GL_TEXTURE7_ARB                       0x84C7
#define GL_TEXTURE8_ARB                       0x84C8
#define GL_TEXTURE9_ARB                       0x84C9
#define GL_CLIENT_ACTIVE_TEXTURE_ARB          0x84E1
#define GL_ACTIVE_TEXTURE_ARB                 0x84E0
#define GL_MAX_TEXTURE_UNITS_ARB              0x84E2



extern "C" {
	// typedefs
	typedef void (*_glMultiTexCoord1dARB)(GLenum target, GLdouble s);
	typedef void (*_glMultiTexCoord1dvARB)(GLenum target, const GLdouble* v);
	typedef void (*_glMultiTexCoord1fARB)(GLenum target, GLfloat s);
	typedef void (*_glMultiTexCoord1fvARB)(GLenum target, const GLfloat* v);
	typedef void (*_glMultiTexCoord1iARB)(GLenum target, GLint s);
	typedef void (*_glMultiTexCoord1ivARB)(GLenum target, const GLint* v);
	typedef void (*_glMultiTexCoord1sARB)(GLenum target, GLshort s);
	typedef void (*_glMultiTexCoord1svARB)(GLenum target, const GLshort* v);
	typedef void (*_glMultiTexCoord2dARB)(GLenum target, GLdouble s, GLdouble t);
	typedef void (*_glMultiTexCoord2dvARB)(GLenum target, const GLdouble* v);
	typedef void (*_glMultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t);
	typedef void (*_glMultiTexCoord2fvARB)(GLenum target, const GLfloat* v);
	typedef void (*_glMultiTexCoord2iARB)(GLenum target, GLint s, GLint t);
	typedef void (*_glMultiTexCoord2ivARB)(GLenum target, const GLint* v);
	typedef void (*_glMultiTexCoord2sARB)(GLenum target, GLshort s, GLshort t);
	typedef void (*_glMultiTexCoord2svARB)(GLenum target, const GLshort* v);
	typedef void (*_glMultiTexCoord3dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
	typedef void (*_glMultiTexCoord3dvARB)(GLenum target, const GLdouble* v);
	typedef void (*_glMultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
	typedef void (*_glMultiTexCoord3fvARB)(GLenum target, const GLfloat* v);
	typedef void (*_glMultiTexCoord3iARB)(GLenum target, GLint s, GLint t, GLint r);
	typedef void (*_glMultiTexCoord3ivARB)(GLenum target, const GLint* v);
	typedef void (*_glMultiTexCoord3sARB)(GLenum target, GLshort s, GLshort t, GLshort r);
	typedef void (*_glMultiTexCoord3svARB)(GLenum target, const GLshort* v);
	typedef void (*_glMultiTexCoord4dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	typedef void (*_glMultiTexCoord4dvARB)(GLenum target, const GLdouble* v);
	typedef void (*_glMultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	typedef void (*_glMultiTexCoord4fvARB)(GLenum target, const GLfloat* v);
	typedef void (*_glMultiTexCoord4iARB)(GLenum target, GLint s, GLint t, GLint r, GLint q);
	typedef void (*_glMultiTexCoord4ivARB)(GLenum target, const GLint* v);
	typedef void (*_glMultiTexCoord4sARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
	typedef void (*_glMultiTexCoord4svARB)(GLenum target, const GLshort* v);
	typedef void (*_glClientActiveTextureARB)(GLenum texture);
	typedef void (*_glActiveTextureARB)(GLenum texture);


	// Function pointers
	extern _glMultiTexCoord1dARB bo_glMultiTexCoord1dARB;
	extern _glMultiTexCoord1dvARB bo_glMultiTexCoord1dvARB;
	extern _glMultiTexCoord1fARB bo_glMultiTexCoord1fARB;
	extern _glMultiTexCoord1fvARB bo_glMultiTexCoord1fvARB;
	extern _glMultiTexCoord1iARB bo_glMultiTexCoord1iARB;
	extern _glMultiTexCoord1ivARB bo_glMultiTexCoord1ivARB;
	extern _glMultiTexCoord1sARB bo_glMultiTexCoord1sARB;
	extern _glMultiTexCoord1svARB bo_glMultiTexCoord1svARB;
	extern _glMultiTexCoord2dARB bo_glMultiTexCoord2dARB;
	extern _glMultiTexCoord2dvARB bo_glMultiTexCoord2dvARB;
	extern _glMultiTexCoord2fARB bo_glMultiTexCoord2fARB;
	extern _glMultiTexCoord2fvARB bo_glMultiTexCoord2fvARB;
	extern _glMultiTexCoord2iARB bo_glMultiTexCoord2iARB;
	extern _glMultiTexCoord2ivARB bo_glMultiTexCoord2ivARB;
	extern _glMultiTexCoord2sARB bo_glMultiTexCoord2sARB;
	extern _glMultiTexCoord2svARB bo_glMultiTexCoord2svARB;
	extern _glMultiTexCoord3dARB bo_glMultiTexCoord3dARB;
	extern _glMultiTexCoord3dvARB bo_glMultiTexCoord3dvARB;
	extern _glMultiTexCoord3fARB bo_glMultiTexCoord3fARB;
	extern _glMultiTexCoord3fvARB bo_glMultiTexCoord3fvARB;
	extern _glMultiTexCoord3iARB bo_glMultiTexCoord3iARB;
	extern _glMultiTexCoord3ivARB bo_glMultiTexCoord3ivARB;
	extern _glMultiTexCoord3sARB bo_glMultiTexCoord3sARB;
	extern _glMultiTexCoord3svARB bo_glMultiTexCoord3svARB;
	extern _glMultiTexCoord4dARB bo_glMultiTexCoord4dARB;
	extern _glMultiTexCoord4dvARB bo_glMultiTexCoord4dvARB;
	extern _glMultiTexCoord4fARB bo_glMultiTexCoord4fARB;
	extern _glMultiTexCoord4fvARB bo_glMultiTexCoord4fvARB;
	extern _glMultiTexCoord4iARB bo_glMultiTexCoord4iARB;
	extern _glMultiTexCoord4ivARB bo_glMultiTexCoord4ivARB;
	extern _glMultiTexCoord4sARB bo_glMultiTexCoord4sARB;
	extern _glMultiTexCoord4svARB bo_glMultiTexCoord4svARB;
	extern _glClientActiveTextureARB bo_glClientActiveTextureARB;
	extern _glActiveTextureARB bo_glActiveTextureARB;
}


#define glMultiTexCoord1dARB bo_glMultiTexCoord1dARB
#define glMultiTexCoord1dvARB bo_glMultiTexCoord1dvARB
#define glMultiTexCoord1fARB bo_glMultiTexCoord1fARB
#define glMultiTexCoord1fvARB bo_glMultiTexCoord1fvARB
#define glMultiTexCoord1iARB bo_glMultiTexCoord1iARB
#define glMultiTexCoord1ivARB bo_glMultiTexCoord1ivARB
#define glMultiTexCoord1sARB bo_glMultiTexCoord1sARB
#define glMultiTexCoord1svARB bo_glMultiTexCoord1svARB
#define glMultiTexCoord2dARB bo_glMultiTexCoord2dARB
#define glMultiTexCoord2dvARB bo_glMultiTexCoord2dvARB
#define glMultiTexCoord2fARB bo_glMultiTexCoord2fARB
#define glMultiTexCoord2fvARB bo_glMultiTexCoord2fvARB
#define glMultiTexCoord2iARB bo_glMultiTexCoord2iARB
#define glMultiTexCoord2ivARB bo_glMultiTexCoord2ivARB
#define glMultiTexCoord2sARB bo_glMultiTexCoord2sARB
#define glMultiTexCoord2svARB bo_glMultiTexCoord2svARB
#define glMultiTexCoord3dARB bo_glMultiTexCoord3dARB
#define glMultiTexCoord3dvARB bo_glMultiTexCoord3dvARB
#define glMultiTexCoord3fARB bo_glMultiTexCoord3fARB
#define glMultiTexCoord3fvARB bo_glMultiTexCoord3fvARB
#define glMultiTexCoord3iARB bo_glMultiTexCoord3iARB
#define glMultiTexCoord3ivARB bo_glMultiTexCoord3ivARB
#define glMultiTexCoord3sARB bo_glMultiTexCoord3sARB
#define glMultiTexCoord3svARB bo_glMultiTexCoord3svARB
#define glMultiTexCoord4dARB bo_glMultiTexCoord4dARB
#define glMultiTexCoord4dvARB bo_glMultiTexCoord4dvARB
#define glMultiTexCoord4fARB bo_glMultiTexCoord4fARB
#define glMultiTexCoord4fvARB bo_glMultiTexCoord4fvARB
#define glMultiTexCoord4iARB bo_glMultiTexCoord4iARB
#define glMultiTexCoord4ivARB bo_glMultiTexCoord4ivARB
#define glMultiTexCoord4sARB bo_glMultiTexCoord4sARB
#define glMultiTexCoord4svARB bo_glMultiTexCoord4svARB
#define glClientActiveTextureARB bo_glClientActiveTextureARB
#define glActiveTextureARB bo_glActiveTextureARB

#endif // ARB_MULTITEXTURE_DECL_P_H
