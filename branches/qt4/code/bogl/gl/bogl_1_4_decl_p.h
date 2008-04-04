// This file was generated from bogl_1_4_decl_p.boglc
// Do not edit this file (all changes will be lost)!

#ifndef BOGL_1_4_DECL_P_H
#define BOGL_1_4_DECL_P_H

#ifndef BOGL_H
#error Never include this file directly! Include bogl.h instead!
#endif

#include "bogl/bogl_do_dlopen.h"

#include "bogl_1_3_decl_p.h"







#define GL_GENERATE_MIPMAP              0x8191
#define GL_GENERATE_MIPMAP_HINT         0x8192
#define GL_DEPTH_COMPONENT              0x1902
#define GL_DEPTH_COMPONENT16            0x81A5
#define GL_DEPTH_COMPONENT24            0x81A6
#define GL_DEPTH_COMPONENT32            0x81A7
#define GL_TEXTURE_DEPTH_SIZE           0x884A
#define GL_DEPTH_TEXTURE_MODE           0x884B
#define GL_TEXTURE_COMPARE_MODE         0x884C
#define GL_TEXTURE_COMPARE_FUNC         0x884D
#define GL_COMPARE_R_TO_TEXTURE         0x884E
#define GL_FOG_COORDINATE_SOURCE        0x8450
#define GL_FOG_COORDINATE               0x8451
#define GL_FRAGMENT_DEPTH               0x8452
#define GL_CURRENT_FOG_COORDINATE       0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE    0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE  0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#define GL_FOG_COORDINATE_ARRAY         0x8457
#define GL_POINT_SIZE_MIN               0x8126
#define GL_POINT_SIZE_MAX               0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE    0x8128
#define GL_POINT_DISTANCE_ATTENUATION   0x8129
#define GL_COLOR_SUM                    0x8458
#define GL_CURRENT_SECONDARY_COLOR      0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE   0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE   0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY        0x845E
#define GL_BLEND_DST_RGB                0x80C8
#define GL_BLEND_SRC_RGB                0x80C9
#define GL_BLEND_DST_ALPHA              0x80CA
#define GL_BLEND_SRC_ALPHA              0x80CB
#define GL_INCR_WRAP                    0x8507
#define GL_DECR_WRAP                    0x8508
#define GL_TEXTURE_FILTER_CONTROL       0x8500
#define GL_TEXTURE_LOD_BIAS             0x8501
#define GL_MAX_TEXTURE_LOD_BIAS         0x84FD
#define GL_MIRRORED_REPEAT_ARB          0x8370




extern "C" {
	// typedefs
	typedef void (*_glFogCoordf)(GLfloat);
	typedef void (*_glFogCoordd)(GLdouble);
	typedef void (*_glFogCoordfv)(const GLfloat*);
	typedef void (*_glFogCoorddv)(const GLdouble*);
	typedef void (*_glFogCoordPointer)(GLenum, GLsizei, const GLvoid*);
	typedef void (*_glMultiDrawArrays)(GLenum, GLint*, GLsizei*, GLsizei);
	typedef void (*_glMultiDrawElements)(GLenum, GLsizei*, GLenum, const GLvoid**, GLsizei);
	typedef void (*_glPointParameterf)(GLenum, GLfloat);
	typedef void (*_glPointParameterfv)(GLenum, const GLfloat*);
	typedef void (*_glPointParameteri)(GLenum, GLint);
	typedef void (*_glPointParameteriv)(GLenum, const GLint*);
	typedef void (*_glSecondaryColor3b)(GLbyte, GLbyte, GLbyte);
	typedef void (*_glSecondaryColor3s)(GLshort, GLshort, GLshort);
	typedef void (*_glSecondaryColor3i)(GLint, GLint, GLint);
	typedef void (*_glSecondaryColor3f)(GLfloat, GLfloat, GLfloat);
	typedef void (*_glSecondaryColor3d)(GLdouble, GLdouble, GLdouble);
	typedef void (*_glSecondaryColor3bv)(const GLbyte*);
	typedef void (*_glSecondaryColor3sv)(const GLshort*);
	typedef void (*_glSecondaryColor3iv)(const GLint*);
	typedef void (*_glSecondaryColor3fv)(const GLfloat*);
	typedef void (*_glSecondaryColor3dv)(const GLdouble*);
	typedef void (*_glSecondaryColor3ub)(GLubyte, GLubyte, GLubyte);
	typedef void (*_glSecondaryColor3us)(GLushort, GLushort, GLushort);
	typedef void (*_glSecondaryColor3ui)(GLuint, GLuint, GLuint);
	typedef void (*_glSecondaryColor3ubv)(const GLubyte*);
	typedef void (*_glSecondaryColor3usv)(const GLushort*);
	typedef void (*_glSecondaryColor3uiv)(const GLuint*);
	typedef void (*_glSecondaryColorPointer)(GLint, GLenum, GLsizei, const GLvoid*);
	typedef void (*_glBlendFuncSeparate)(GLenum, GLenum, GLenum,GLenum);
	typedef void (*_glWindowPos2d)(GLdouble, GLdouble);
	typedef void (*_glWindowPos2f)(GLfloat, GLfloat);
	typedef void (*_glWindowPos2i)(GLint, GLint);
	typedef void (*_glWindowPos2s)(GLshort, GLshort);
	typedef void (*_glWindowPos2dv)(const GLdouble*);
	typedef void (*_glWindowPos2fv)(const GLfloat*);
	typedef void (*_glWindowPos2iv)(const GLint*);
	typedef void (*_glWindowPos2sv)(const GLshort*);
	typedef void (*_glWindowPos3d)(GLdouble, GLdouble, GLdouble);
	typedef void (*_glWindowPos3f)(GLfloat, GLfloat, GLfloat);
	typedef void (*_glWindowPos3i)(GLint, GLint, GLint);
	typedef void (*_glWindowPos3s)(GLshort, GLshort, GLshort);
	typedef void (*_glWindowPos3dv)(const GLdouble*);
	typedef void (*_glWindowPos3fv)(const GLfloat*);
	typedef void (*_glWindowPos3iv)(const GLint*);
	typedef void (*_glWindowPos3sv)(const GLshort*);


	// Function pointers
	extern _glFogCoordf bo_glFogCoordf;
	extern _glFogCoordd bo_glFogCoordd;
	extern _glFogCoordfv bo_glFogCoordfv;
	extern _glFogCoorddv bo_glFogCoorddv;
	extern _glFogCoordPointer bo_glFogCoordPointer;
	extern _glMultiDrawArrays bo_glMultiDrawArrays;
	extern _glMultiDrawElements bo_glMultiDrawElements;
	extern _glPointParameterf bo_glPointParameterf;
	extern _glPointParameterfv bo_glPointParameterfv;
	extern _glPointParameteri bo_glPointParameteri;
	extern _glPointParameteriv bo_glPointParameteriv;
	extern _glSecondaryColor3b bo_glSecondaryColor3b;
	extern _glSecondaryColor3s bo_glSecondaryColor3s;
	extern _glSecondaryColor3i bo_glSecondaryColor3i;
	extern _glSecondaryColor3f bo_glSecondaryColor3f;
	extern _glSecondaryColor3d bo_glSecondaryColor3d;
	extern _glSecondaryColor3bv bo_glSecondaryColor3bv;
	extern _glSecondaryColor3sv bo_glSecondaryColor3sv;
	extern _glSecondaryColor3iv bo_glSecondaryColor3iv;
	extern _glSecondaryColor3fv bo_glSecondaryColor3fv;
	extern _glSecondaryColor3dv bo_glSecondaryColor3dv;
	extern _glSecondaryColor3ub bo_glSecondaryColor3ub;
	extern _glSecondaryColor3us bo_glSecondaryColor3us;
	extern _glSecondaryColor3ui bo_glSecondaryColor3ui;
	extern _glSecondaryColor3ubv bo_glSecondaryColor3ubv;
	extern _glSecondaryColor3usv bo_glSecondaryColor3usv;
	extern _glSecondaryColor3uiv bo_glSecondaryColor3uiv;
	extern _glSecondaryColorPointer bo_glSecondaryColorPointer;
	extern _glBlendFuncSeparate bo_glBlendFuncSeparate;
	extern _glWindowPos2d bo_glWindowPos2d;
	extern _glWindowPos2f bo_glWindowPos2f;
	extern _glWindowPos2i bo_glWindowPos2i;
	extern _glWindowPos2s bo_glWindowPos2s;
	extern _glWindowPos2dv bo_glWindowPos2dv;
	extern _glWindowPos2fv bo_glWindowPos2fv;
	extern _glWindowPos2iv bo_glWindowPos2iv;
	extern _glWindowPos2sv bo_glWindowPos2sv;
	extern _glWindowPos3d bo_glWindowPos3d;
	extern _glWindowPos3f bo_glWindowPos3f;
	extern _glWindowPos3i bo_glWindowPos3i;
	extern _glWindowPos3s bo_glWindowPos3s;
	extern _glWindowPos3dv bo_glWindowPos3dv;
	extern _glWindowPos3fv bo_glWindowPos3fv;
	extern _glWindowPos3iv bo_glWindowPos3iv;
	extern _glWindowPos3sv bo_glWindowPos3sv;
}


#define glFogCoordf bo_glFogCoordf
#define glFogCoordd bo_glFogCoordd
#define glFogCoordfv bo_glFogCoordfv
#define glFogCoorddv bo_glFogCoorddv
#define glFogCoordPointer bo_glFogCoordPointer
#define glMultiDrawArrays bo_glMultiDrawArrays
#define glMultiDrawElements bo_glMultiDrawElements
#define glPointParameterf bo_glPointParameterf
#define glPointParameterfv bo_glPointParameterfv
#define glPointParameteri bo_glPointParameteri
#define glPointParameteriv bo_glPointParameteriv
#define glSecondaryColor3b bo_glSecondaryColor3b
#define glSecondaryColor3s bo_glSecondaryColor3s
#define glSecondaryColor3i bo_glSecondaryColor3i
#define glSecondaryColor3f bo_glSecondaryColor3f
#define glSecondaryColor3d bo_glSecondaryColor3d
#define glSecondaryColor3bv bo_glSecondaryColor3bv
#define glSecondaryColor3sv bo_glSecondaryColor3sv
#define glSecondaryColor3iv bo_glSecondaryColor3iv
#define glSecondaryColor3fv bo_glSecondaryColor3fv
#define glSecondaryColor3dv bo_glSecondaryColor3dv
#define glSecondaryColor3ub bo_glSecondaryColor3ub
#define glSecondaryColor3us bo_glSecondaryColor3us
#define glSecondaryColor3ui bo_glSecondaryColor3ui
#define glSecondaryColor3ubv bo_glSecondaryColor3ubv
#define glSecondaryColor3usv bo_glSecondaryColor3usv
#define glSecondaryColor3uiv bo_glSecondaryColor3uiv
#define glSecondaryColorPointer bo_glSecondaryColorPointer
#define glBlendFuncSeparate bo_glBlendFuncSeparate
#define glWindowPos2d bo_glWindowPos2d
#define glWindowPos2f bo_glWindowPos2f
#define glWindowPos2i bo_glWindowPos2i
#define glWindowPos2s bo_glWindowPos2s
#define glWindowPos2dv bo_glWindowPos2dv
#define glWindowPos2fv bo_glWindowPos2fv
#define glWindowPos2iv bo_glWindowPos2iv
#define glWindowPos2sv bo_glWindowPos2sv
#define glWindowPos3d bo_glWindowPos3d
#define glWindowPos3f bo_glWindowPos3f
#define glWindowPos3i bo_glWindowPos3i
#define glWindowPos3s bo_glWindowPos3s
#define glWindowPos3dv bo_glWindowPos3dv
#define glWindowPos3fv bo_glWindowPos3fv
#define glWindowPos3iv bo_glWindowPos3iv
#define glWindowPos3sv bo_glWindowPos3sv

#endif // BOGL_1_4_DECL_P_H