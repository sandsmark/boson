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

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include "bogl.h"
#include "boglx.h"

#include "bodebug.h"
#include "myqlibrary.h"

#include <qstringlist.h>

#include <stdlib.h>

// resolve given symbol
#define RESOLVE(a) \
	bo_##a = (_##a)gl.resolve( #a );

// resolve given symbol and return false, if function is 0
#define RESOLVE_CHECK(a) \
	bo_##a = (_##a)gl.resolve( #a ); \
	if (bo_##a == 0) { boError() << k_funcinfo << "unable to resolve symbol " << #a << endl; return false; }

// like RESOLVE(), but uses glXGetProcAddressARB()
#define RESOLVE_GL_SYMBOL(a) \
	bo_##a = (_##a)glXGetProcAddressARB( (const GLubyte*) #a );

// like RESOLVE_CHECK(), but uses glXGetProcAddressARB()
#define RESOLVE_GL_SYMBOL_CHECK(a) \
	bo_##a = (_##a)glXGetProcAddressARB( (const GLubyte*) #a ); \
	if (bo_##a == 0) { boError() << k_funcinfo << "unable to resolve symbol " << #a << endl; return false; }

// assign a function pointer from an extension (e.g. glActiveTextureARB) to a
// function pointer from an official GL version (e.g. glActiveTexture), if that
// pointer is not 0.
//
// this should be used so that we can use the "official" function names always
// and don't need to use the extension function if the GL version is not
// available.
#define ASSIGN_FROM_EXT(func, func_ext) \
	if (bo_##func == 0) { bo_##func = bo_##func_ext; }



static bool boglResolveGLXSymbols(MyQLibrary& gl);
static bool boglResolveOpenGL_1_1_Symbols(MyQLibrary& gl);
static bool boglResolveOpenGL_1_2_Symbols(MyQLibrary& gl);
static bool boglResolveOpenGL_1_2_1_Symbols(MyQLibrary& gl);
static bool boglResolveOpenGL_1_3_Symbols(MyQLibrary& gl);
static bool boglResolveOpenGL_1_4_Symbols(MyQLibrary& gl);
static bool boglResolveOpenGL_1_5_Symbols(MyQLibrary& gl);
static bool boglResolveOpenGL_2_0_Symbols(MyQLibrary& gl);
static bool boglResolveGLUSymbols(MyQLibrary& gl);
static bool boglResolveARB_multitexture_Symbols();
static bool boglResolveEXT_blend_color_Symbols();
static bool boglResolveEXT_polygon_offset_Symbols();
static bool boglResolveEXT_texture3d_Symbols();
static bool boglResolveARB_vertex_buffer_object_Symbols();
static bool boglResolveARB_shader_objects_Symbols();
static bool boglResolveEXT_framebuffer_object_Symbols();

bool boglResolveLibGLSymbols(MyQLibrary& gl)
{
 if (!boglResolveGLXSymbols(gl)) {
	return false;
 }

 // TODO: check for OpenGL version
 if (!boglResolveOpenGL_1_1_Symbols(gl)) {
	return false;
 }
 if (!boglResolveOpenGL_1_2_Symbols(gl)) {
	return false;
 }
 if (!boglResolveOpenGL_1_2_Symbols(gl)) {
	return false;
 }
 if (!boglResolveOpenGL_1_2_1_Symbols(gl)) {
	return false;
 }
 if (!boglResolveOpenGL_1_3_Symbols(gl)) {
	return false;
 }
 if (!boglResolveOpenGL_1_4_Symbols(gl)) {
	return false;
 }
 if (!boglResolveOpenGL_1_5_Symbols(gl)) {
	return false;
 }
 if (!boglResolveOpenGL_2_0_Symbols(gl)) {
	return false;
 }
 return true;
}

void boglResolveGLExtensionSymbols()
{
 QStringList extensions = boglGetOpenGLExtensions();

 if (extensions.count() == 0) {
	boError() << k_funcinfo << "extensions can not yet be loaded" << endl;
	return;
 }

 if (extensions.contains("GL_ARB_multitexture")) {
	boglResolveARB_multitexture_Symbols();
 }
 if (extensions.contains("GL_EXT_polygon_offset")) {
	boglResolveEXT_polygon_offset_Symbols();
 }
 if (extensions.contains("GL_EXT_texture3d")) {
	boglResolveEXT_texture3d_Symbols();
 }
 if (extensions.contains("GL_EXT_blend_color")) {
	boglResolveEXT_blend_color_Symbols();
 }
 if (extensions.contains("GL_ARB_vertex_buffer_object")) {
	boglResolveARB_vertex_buffer_object_Symbols();
 }
 if (extensions.contains("GL_ARB_shader_objects")) {
	boglResolveARB_shader_objects_Symbols();
 }
 if (extensions.contains("GL_EXT_framebuffer_object")) {
	boglResolveEXT_framebuffer_object_Symbols();
 }

}

bool boglResolveLibGLUSymbols(MyQLibrary& gl)
{
 if (!boglResolveGLUSymbols(gl)) {
	return false;
 }
 return true;
}


// GLU
extern "C" {
	_gluBeginCurve bo_gluBeginCurve;
	_gluBeginPolygon bo_gluBeginPolygon;
	_gluBeginSurface bo_gluBeginSurface;
	_gluBeginTrim bo_gluBeginTrim;
	_gluBuild1DMipmapLevels bo_gluBuild1DMipmapLevels;
	_gluBuild1DMipmaps bo_gluBuild1DMipmaps;
	_gluBuild2DMipmapLevels bo_gluBuild2DMipmapLevels;
	_gluBuild2DMipmaps bo_gluBuild2DMipmaps;
	_gluBuild3DMipmapLevels bo_gluBuild3DMipmapLevels;
	_gluBuild3DMipmaps bo_gluBuild3DMipmaps;
	_gluCheckExtension bo_gluCheckExtension;
	_gluCylinder bo_gluCylinder;
	_gluDeleteNurbsRenderer bo_gluDeleteNurbsRenderer;
	_gluDeleteQuadric bo_gluDeleteQuadric;
	_gluDeleteTess bo_gluDeleteTess;
	_gluDisk bo_gluDisk;
	_gluEndCurve bo_gluEndCurve;
	_gluEndPolygon bo_gluEndPolygon;
	_gluEndSurface bo_gluEndSurface;
	_gluEndTrim bo_gluEndTrim;
	_gluErrorString bo_gluErrorString;
	_gluGetNurbsProperty bo_gluGetNurbsProperty;
	_gluGetString bo_gluGetString;
	_gluGetTessProperty bo_gluGetTessProperty;
	_gluLoadSamplingMatrices bo_gluLoadSamplingMatrices;
	_gluLookAt bo_gluLookAt;
	_gluNewNurbsRenderer bo_gluNewNurbsRenderer;
	_gluNewQuadric bo_gluNewQuadric;
	_gluNewTess bo_gluNewTess;
	_gluNextContour bo_gluNextContour;
	_gluNurbsCallback bo_gluNurbsCallback;
	_gluNurbsCallbackData bo_gluNurbsCallbackData;
	_gluNurbsCallbackDataEXT bo_gluNurbsCallbackDataEXT;
	_gluNurbsCurve bo_gluNurbsCurve;
	_gluNurbsProperty bo_gluNurbsProperty;
	_gluNurbsSurface bo_gluNurbsSurface;
	_gluOrtho2D bo_gluOrtho2D;
	_gluPartialDisk bo_gluPartialDisk;
	_gluPerspective bo_gluPerspective;
	_gluPickMatrix bo_gluPickMatrix;
	_gluProject bo_gluProject;
	_gluPwlCurve bo_gluPwlCurve;
	_gluQuadricCallback bo_gluQuadricCallback;
	_gluQuadricDrawStyle bo_gluQuadricDrawStyle;
	_gluQuadricNormals bo_gluQuadricNormals;
	_gluQuadricOrientation bo_gluQuadricOrientation;
	_gluQuadricTexture bo_gluQuadricTexture;
	_gluScaleImage bo_gluScaleImage;
	_gluSphere bo_gluSphere;
	_gluTessBeginContour bo_gluTessBeginContour;
	_gluTessBeginPolygon bo_gluTessBeginPolygon;
	_gluTessCallback bo_gluTessCallback;
	_gluTessEndContour bo_gluTessEndContour;
	_gluTessEndPolygon bo_gluTessEndPolygon;
	_gluTessNormal bo_gluTessNormal;
	_gluTessProperty bo_gluTessProperty;
	_gluTessVertex bo_gluTessVertex;
	_gluUnProject bo_gluUnProject;
	_gluUnProject4 bo_gluUnProject4;
}; // "C"

bool boglResolveGLUSymbols(MyQLibrary& gl)
{
 RESOLVE_CHECK(gluBeginCurve);
 RESOLVE_CHECK(gluBeginPolygon);
 RESOLVE_CHECK(gluBeginSurface);
 RESOLVE_CHECK(gluBeginTrim);
 RESOLVE_CHECK(gluBuild1DMipmapLevels);
 RESOLVE_CHECK(gluBuild1DMipmaps);
 RESOLVE_CHECK(gluBuild2DMipmapLevels);
 RESOLVE_CHECK(gluBuild2DMipmaps);
 RESOLVE_CHECK(gluBuild3DMipmapLevels);
 RESOLVE_CHECK(gluBuild3DMipmaps);
 RESOLVE_CHECK(gluCheckExtension);
 RESOLVE_CHECK(gluCylinder);
 RESOLVE_CHECK(gluDeleteNurbsRenderer);
 RESOLVE_CHECK(gluDeleteQuadric);
 RESOLVE_CHECK(gluDeleteTess);
 RESOLVE_CHECK(gluDisk);
 RESOLVE_CHECK(gluEndCurve);
 RESOLVE_CHECK(gluEndPolygon);
 RESOLVE_CHECK(gluEndSurface);
 RESOLVE_CHECK(gluEndTrim);
 RESOLVE_CHECK(gluErrorString);
 RESOLVE_CHECK(gluGetNurbsProperty);
 RESOLVE_CHECK(gluGetString);
 RESOLVE_CHECK(gluGetTessProperty);
 RESOLVE_CHECK(gluLoadSamplingMatrices);
 RESOLVE_CHECK(gluLookAt);
 RESOLVE_CHECK(gluNewNurbsRenderer);
 RESOLVE_CHECK(gluNewQuadric);
 RESOLVE_CHECK(gluNewTess);
 RESOLVE_CHECK(gluNextContour);
 RESOLVE_CHECK(gluNurbsCallback);
 RESOLVE_CHECK(gluNurbsCallbackData);
 RESOLVE_CHECK(gluNurbsCallbackDataEXT);
 RESOLVE_CHECK(gluNurbsCurve);
 RESOLVE_CHECK(gluNurbsProperty);
 RESOLVE_CHECK(gluNurbsSurface);
 RESOLVE_CHECK(gluOrtho2D);
 RESOLVE_CHECK(gluPartialDisk);
 RESOLVE_CHECK(gluPerspective);
 RESOLVE_CHECK(gluPickMatrix);
 RESOLVE_CHECK(gluProject);
 RESOLVE_CHECK(gluPwlCurve);
 RESOLVE_CHECK(gluQuadricCallback);
 RESOLVE_CHECK(gluQuadricDrawStyle);
 RESOLVE_CHECK(gluQuadricNormals);
 RESOLVE_CHECK(gluQuadricOrientation);
 RESOLVE_CHECK(gluQuadricTexture);
 RESOLVE_CHECK(gluScaleImage);
 RESOLVE_CHECK(gluSphere);
 RESOLVE_CHECK(gluTessBeginContour);
 RESOLVE_CHECK(gluTessBeginPolygon);
 RESOLVE_CHECK(gluTessCallback);
 RESOLVE_CHECK(gluTessEndContour);
 RESOLVE_CHECK(gluTessEndPolygon);
 RESOLVE_CHECK(gluTessNormal);
 RESOLVE_CHECK(gluTessProperty);
 RESOLVE_CHECK(gluTessVertex);
 RESOLVE_CHECK(gluUnProject);
 RESOLVE_CHECK(gluUnProject4);
 return true;
}

// GLX
extern "C" {
	_glXChooseVisual bo_glXChooseVisual;
	_glXCopyContext bo_glXCopyContext;
	_glXCreateContext bo_glXCreateContext;
	_glXCreateGLXPixmap bo_glXCreateGLXPixmap;
	_glXDestroyContext bo_glXDestroyContext;
	_glXDestroyGLXPixmap bo_glXDestroyGLXPixmap;
	_glXGetConfig bo_glXGetConfig;
	_glXGetCurrentContext bo_glXGetCurrentContext;
	_glXGetCurrentDrawable bo_glXGetCurrentDrawable;
	_glXIsDirect bo_glXIsDirect;
	_glXMakeCurrent bo_glXMakeCurrent;
	_glXQueryExtension bo_glXQueryExtension;
	_glXQueryVersion bo_glXQueryVersion;
	_glXSwapBuffers bo_glXSwapBuffers;
	_glXUseXFont bo_glXUseXFont;
	_glXWaitGL bo_glXWaitGL;
	_glXWaitX bo_glXWaitX;
	_glXGetClientString bo_glXGetClientString;
	_glXQueryServerString bo_glXQueryServerString;
	_glXQueryExtensionsString bo_glXQueryExtensionsString;
	_glXGetFBConfigs bo_glXGetFBConfigs;
	_glXChooseFBConfig bo_glXChooseFBConfig;
	_glXGetFBConfigAttrib bo_glXGetFBConfigAttrib;
	_glXGetVisualFromFBConfig bo_glXGetVisualFromFBConfig;
	_glXCreateWindow bo_glXCreateWindow;
	_glXDestroyWindow bo_glXDestroyWindow;
	_glXCreatePixmap bo_glXCreatePixmap;
	_glXDestroyPixmap bo_glXDestroyPixmap;
	_glXCreatePbuffer bo_glXCreatePbuffer;
	_glXDestroyPbuffer bo_glXDestroyPbuffer;
	_glXQueryDrawable bo_glXQueryDrawable;
	_glXCreateNewContext bo_glXCreateNewContext;
	_glXMakeContextCurrent bo_glXMakeContextCurrent;
	_glXGetCurrentReadDrawable bo_glXGetCurrentReadDrawable;
	_glXGetCurrentDisplay bo_glXGetCurrentDisplay;
	_glXQueryContext bo_glXQueryContext;
	_glXSelectEvent bo_glXSelectEvent;
	_glXGetSelectedEvent bo_glXGetSelectedEvent;
	_glXGetProcAddress bo_glXGetProcAddress;
	_glXGetContextIDEXT bo_glXGetContextIDEXT;
	_glXImportContextEXT bo_glXImportContextEXT;
	_glXFreeContextEXT bo_glXFreeContextEXT;
	_glXQueryContextInfoEXT bo_glXQueryContextInfoEXT;
	_glXGetCurrentDisplayEXT bo_glXGetCurrentDisplayEXT;
	_glXGetProcAddressARB bo_glXGetProcAddressARB;
	_glXAllocateMemoryNV bo_glXAllocateMemoryNV;
	_glXFreeMemoryNV bo_glXFreeMemoryNV;
	_glXGetAGPOffsetMESA bo_glXGetAGPOffsetMESA;
	// GLX_SGIX_fbconfig
	_glXGetFBConfigAttribSGIX bo_glXGetFBConfigAttribSGIX;
	_glXChooseFBConfigSGIX bo_glXChooseFBConfigSGIX;
	_glXCreateGLXPixmapWithConfigSGIX bo_glXCreateGLXPixmapWithConfigSGIX;
	_glXCreateContextWithConfigSGIX bo_glXCreateContextWithConfigSGIX;
	_glXGetVisualFromFBConfigSGIX bo_glXGetVisualFromFBConfigSGIX;
	_glXGetFBConfigFromVisualSGIX bo_glXGetFBConfigFromVisualSGIX;
	//GLX_SGIX_pbuffer
	_glXCreateGLXPbufferSGIX bo_glXCreateGLXPbufferSGIX;
	_glXDestroyGLXPbufferSGIX bo_glXDestroyGLXPbufferSGIX;
	_glXQueryGLXPbufferSGIX bo_glXQueryGLXPbufferSGIX;
	_glXSelectEventSGIX bo_glXSelectEventSGIX;
	_glXGetSelectedEventSGIX bo_glXGetSelectedEventSGIX;
}; // extern "C"


bool boglResolveGLXSymbols(MyQLibrary& gl)
{
 RESOLVE_CHECK(glXChooseVisual);
 RESOLVE_CHECK(glXCopyContext);
 RESOLVE_CHECK(glXCreateContext);
 RESOLVE_CHECK(glXCreateGLXPixmap);
 RESOLVE_CHECK(glXDestroyContext);
 RESOLVE_CHECK(glXDestroyGLXPixmap);
 RESOLVE_CHECK(glXGetConfig);
 RESOLVE_CHECK(glXGetCurrentContext);
 RESOLVE_CHECK(glXGetCurrentDrawable);
 RESOLVE_CHECK(glXIsDirect);
 RESOLVE_CHECK(glXMakeCurrent);
 RESOLVE_CHECK(glXQueryExtension);
 RESOLVE_CHECK(glXQueryVersion);
 RESOLVE_CHECK(glXSwapBuffers);
 RESOLVE_CHECK(glXUseXFont);
 RESOLVE_CHECK(glXWaitGL);
 RESOLVE_CHECK(glXWaitX);
 RESOLVE_CHECK(glXGetClientString);
 RESOLVE_CHECK(glXQueryServerString);
 RESOLVE_CHECK(glXQueryExtensionsString);
 RESOLVE_CHECK(glXGetFBConfigs);
 RESOLVE_CHECK(glXChooseFBConfig);
 RESOLVE_CHECK(glXGetFBConfigAttrib);
 RESOLVE_CHECK(glXGetVisualFromFBConfig);
 RESOLVE_CHECK(glXCreateWindow);
 RESOLVE_CHECK(glXDestroyWindow);
 RESOLVE_CHECK(glXCreatePixmap);
 RESOLVE_CHECK(glXDestroyPixmap);
 RESOLVE_CHECK(glXCreatePbuffer);
 RESOLVE_CHECK(glXDestroyPbuffer);
 RESOLVE_CHECK(glXQueryDrawable);
 RESOLVE_CHECK(glXCreateNewContext);
 RESOLVE_CHECK(glXMakeContextCurrent);
 RESOLVE_CHECK(glXGetCurrentReadDrawable);
 RESOLVE_CHECK(glXGetCurrentDisplay);
 RESOLVE_CHECK(glXQueryContext);
 RESOLVE_CHECK(glXSelectEvent);
 RESOLVE_CHECK(glXGetSelectedEvent);


 // AB: note: glXGetProcAddressARB() is part of the linux OpenGL ABI
 //           glXGetProcAddress() is not.
 //           -> by using glXGetProcAddressARB() only, we can be sure that that
 //              function actually exists
 RESOLVE(glXGetProcAddress);
 RESOLVE(glXGetContextIDEXT);
 RESOLVE(glXImportContextEXT);
 RESOLVE(glXFreeContextEXT);
 RESOLVE(glXQueryContextInfoEXT);
 RESOLVE(glXGetCurrentDisplayEXT);
 RESOLVE(glXGetProcAddressARB);
 RESOLVE(glXAllocateMemoryNV);
 RESOLVE(glXFreeMemoryNV);
 RESOLVE(glXGetAGPOffsetMESA);
 RESOLVE(glXGetFBConfigAttribSGIX);
 RESOLVE(glXChooseFBConfigSGIX);
 RESOLVE(glXCreateGLXPixmapWithConfigSGIX);
 RESOLVE(glXCreateContextWithConfigSGIX);
 RESOLVE(glXGetVisualFromFBConfigSGIX);
 RESOLVE(glXGetFBConfigFromVisualSGIX);
 RESOLVE(glXCreateGLXPbufferSGIX);
 RESOLVE(glXDestroyGLXPbufferSGIX);
 RESOLVE(glXQueryGLXPbufferSGIX);
 RESOLVE(glXSelectEventSGIX);
 RESOLVE(glXGetSelectedEventSGIX);

 return true;
}



// OpenGL 1.1
extern "C" {
	_glGetError bo_glGetError;
	_glBegin bo_glBegin;
	_glEnd bo_glEnd;
	_glEdgeFlag bo_glEdgeFlag;
	_glEdgeFlagv bo_glEdgeFlagv;
	_glVertex2d bo_glVertex2d;
	_glVertex2f bo_glVertex2f;
	_glVertex2i bo_glVertex2i;
	_glVertex2s bo_glVertex2s;
	_glVertex3d bo_glVertex3d;
	_glVertex3f bo_glVertex3f;
	_glVertex3i bo_glVertex3i;
	_glVertex3s bo_glVertex3s;
	_glVertex4d bo_glVertex4d;
	_glVertex4f bo_glVertex4f;
	_glVertex4i bo_glVertex4i;
	_glVertex4s bo_glVertex4s;
	_glVertex2dv bo_glVertex2dv;
	_glVertex2fv bo_glVertex2fv;
	_glVertex2iv bo_glVertex2iv;
	_glVertex2sv bo_glVertex2sv;
	_glVertex3dv bo_glVertex3dv;
	_glVertex3fv bo_glVertex3fv;
	_glVertex3iv bo_glVertex3iv;
	_glVertex3sv bo_glVertex3sv;
	_glVertex4dv bo_glVertex4dv;
	_glVertex4fv bo_glVertex4fv;
	_glVertex4iv bo_glVertex4iv;
	_glVertex4sv bo_glVertex4sv;
	_glTexCoord1d bo_glTexCoord1d;
	_glTexCoord1f bo_glTexCoord1f;
	_glTexCoord1i bo_glTexCoord1i;
	_glTexCoord1s bo_glTexCoord1s;
	_glTexCoord2d bo_glTexCoord2d;
	_glTexCoord2f bo_glTexCoord2f;
	_glTexCoord2i bo_glTexCoord2i;
	_glTexCoord2s bo_glTexCoord2s;
	_glTexCoord3d bo_glTexCoord3d;
	_glTexCoord3f bo_glTexCoord3f;
	_glTexCoord3i bo_glTexCoord3i;
	_glTexCoord3s bo_glTexCoord3s;
	_glTexCoord4d bo_glTexCoord4d;
	_glTexCoord4f bo_glTexCoord4f;
	_glTexCoord4i bo_glTexCoord4i;
	_glTexCoord4s bo_glTexCoord4s;
	_glTexCoord1dv bo_glTexCoord1dv;
	_glTexCoord1fv bo_glTexCoord1fv;
	_glTexCoord1iv bo_glTexCoord1iv;
	_glTexCoord1sv bo_glTexCoord1sv;
	_glTexCoord2dv bo_glTexCoord2dv;
	_glTexCoord2fv bo_glTexCoord2fv;
	_glTexCoord2iv bo_glTexCoord2iv;
	_glTexCoord2sv bo_glTexCoord2sv;
	_glTexCoord3dv bo_glTexCoord3dv;
	_glTexCoord3fv bo_glTexCoord3fv;
	_glTexCoord3iv bo_glTexCoord3iv;
	_glTexCoord3sv bo_glTexCoord3sv;
	_glTexCoord4dv bo_glTexCoord4dv;
	_glTexCoord4fv bo_glTexCoord4fv;
	_glTexCoord4iv bo_glTexCoord4iv;
	_glTexCoord4sv bo_glTexCoord4sv;
	_glNormal3b bo_glNormal3b;
	_glNormal3d bo_glNormal3d;
	_glNormal3f bo_glNormal3f;
	_glNormal3i bo_glNormal3i;
	_glNormal3s bo_glNormal3s;
	_glNormal3bv bo_glNormal3bv;
	_glNormal3dv bo_glNormal3dv;
	_glNormal3fv bo_glNormal3fv;
	_glNormal3iv bo_glNormal3iv;
	_glNormal3sv bo_glNormal3sv;
	_glColor3b bo_glColor3b;
	_glColor3d bo_glColor3d;
	_glColor3f bo_glColor3f;
	_glColor3i bo_glColor3i;
	_glColor3s bo_glColor3s;
	_glColor3ub bo_glColor3ub;
	_glColor3ui bo_glColor3ui;
	_glColor3us bo_glColor3us;
	_glColor4b bo_glColor4b;
	_glColor4d bo_glColor4d;
	_glColor4f bo_glColor4f;
	_glColor4i bo_glColor4i;
	_glColor4s bo_glColor4s;
	_glColor4ub bo_glColor4ub;
	_glColor4ui bo_glColor4ui;
	_glColor4us bo_glColor4us;
	_glColor3bv bo_glColor3bv;
	_glColor3dv bo_glColor3dv;
	_glColor3fv bo_glColor3fv;
	_glColor3iv bo_glColor3iv;
	_glColor3sv bo_glColor3sv;
	_glColor3ubv bo_glColor3ubv;
	_glColor3uiv bo_glColor3uiv;
	_glColor3usv bo_glColor3usv;
	_glColor4bv bo_glColor4bv;
	_glColor4dv bo_glColor4dv;
	_glColor4fv bo_glColor4fv;
	_glColor4iv bo_glColor4iv;
	_glColor4sv bo_glColor4sv;
	_glColor4ubv bo_glColor4ubv;
	_glColor4uiv bo_glColor4uiv;
	_glColor4usv bo_glColor4usv;
	_glIndexd bo_glIndexd;
	_glIndexf bo_glIndexf;
	_glIndexi bo_glIndexi;
	_glIndexs bo_glIndexs;
	_glIndexub bo_glIndexub;
	_glIndexdv bo_glIndexdv;
	_glIndexfv bo_glIndexfv;
	_glIndexiv bo_glIndexiv;
	_glIndexsv bo_glIndexsv;
	_glIndexubv bo_glIndexubv;
	_glRectd bo_glRectd;
	_glRectf bo_glRectf;
	_glRecti bo_glRecti;
	_glRects bo_glRects;
	_glRectdv bo_glRectdv;
	_glRectfv bo_glRectfv;
	_glRectiv bo_glRectiv;
	_glRectsv bo_glRectsv;
	_glDepthRange bo_glDepthRange;
	_glViewport bo_glViewport;
	_glMatrixMode bo_glMatrixMode;
	_glLoadMatrixd bo_glLoadMatrixd;
	_glLoadMatrixf bo_glLoadMatrixf;
	_glMultMatrixd bo_glMultMatrixd;
	_glMultMatrixf bo_glMultMatrixf;
	_glLoadIdentity bo_glLoadIdentity;
	_glRotated bo_glRotated;
	_glRotatef bo_glRotatef;
	_glTranslated bo_glTranslated;
	_glTranslatef bo_glTranslatef;
	_glScaled bo_glScaled;
	_glScalef bo_glScalef;
	_glFrustum bo_glFrustum;
	_glOrtho bo_glOrtho;
	_glPushMatrix bo_glPushMatrix;
	_glPopMatrix bo_glPopMatrix;
	_glEnable bo_glEnable;
	_glDisable bo_glDisable;
	_glTexGend bo_glTexGend;
	_glTexGenf bo_glTexGenf;
	_glTexGeni bo_glTexGeni;
	_glTexGendv bo_glTexGendv;
	_glTexGenfv bo_glTexGenfv;
	_glTexGeniv bo_glTexGeniv;
	_glClipPlane bo_glClipPlane;
	_glRasterPos2d bo_glRasterPos2d;
	_glRasterPos2f bo_glRasterPos2f;
	_glRasterPos2i bo_glRasterPos2i;
	_glRasterPos2s bo_glRasterPos2s;
	_glRasterPos3d bo_glRasterPos3d;
	_glRasterPos3f bo_glRasterPos3f;
	_glRasterPos3i bo_glRasterPos3i;
	_glRasterPos3s bo_glRasterPos3s;
	_glRasterPos4d bo_glRasterPos4d;
	_glRasterPos4f bo_glRasterPos4f;
	_glRasterPos4i bo_glRasterPos4i;
	_glRasterPos4s bo_glRasterPos4s;
	_glRasterPos2dv bo_glRasterPos2dv;
	_glRasterPos2fv bo_glRasterPos2fv;
	_glRasterPos2iv bo_glRasterPos2iv;
	_glRasterPos2sv bo_glRasterPos2sv;
	_glRasterPos3dv bo_glRasterPos3dv;
	_glRasterPos3fv bo_glRasterPos3fv;
	_glRasterPos3iv bo_glRasterPos3iv;
	_glRasterPos3sv bo_glRasterPos3sv;
	_glRasterPos4dv bo_glRasterPos4dv;
	_glRasterPos4fv bo_glRasterPos4fv;
	_glRasterPos4iv bo_glRasterPos4iv;
	_glRasterPos4sv bo_glRasterPos4sv;
	_glMaterialf bo_glMaterialf;
	_glMateriali bo_glMateriali;
	_glMaterialfv bo_glMaterialfv;
	_glMaterialiv bo_glMaterialiv;
	_glFrontFace bo_glFrontFace;
	_glLightf bo_glLightf;
	_glLighti bo_glLighti;
	_glLightfv bo_glLightfv;
	_glLightiv bo_glLightiv;
	_glLightModelf bo_glLightModelf;
	_glLightModeli bo_glLightModeli;
	_glLightModelfv bo_glLightModelfv;
	_glLightModeliv bo_glLightModeliv;
	_glColorMaterial bo_glColorMaterial;
	_glShadeModel bo_glShadeModel;
	_glPointSize bo_glPointSize;
	_glLineWidth bo_glLineWidth;
	_glLineStipple bo_glLineStipple;
	_glCullFace bo_glCullFace;
	_glPolygonStipple bo_glPolygonStipple;
	_glPolygonMode bo_glPolygonMode;
	_glPolygonOffset bo_glPolygonOffset;
	_glPixelStoref bo_glPixelStoref;
	_glPixelStorei bo_glPixelStorei;
	_glPixelTransferf bo_glPixelTransferf;
	_glPixelTransferi bo_glPixelTransferi;
	_glPixelMapfv bo_glPixelMapfv;
	_glPixelMapuiv bo_glPixelMapuiv;
	_glPixelMapusv bo_glPixelMapusv;
	_glDrawPixels bo_glDrawPixels;
	_glPixelZoom bo_glPixelZoom;
	_glBitmap bo_glBitmap;
	_glTexImage1D bo_glTexImage1D;
	_glTexImage2D bo_glTexImage2D;
	_glCopyTexImage1D bo_glCopyTexImage1D;
	_glCopyTexImage2D bo_glCopyTexImage2D;
	_glTexSubImage1D bo_glTexSubImage1D;
	_glTexSubImage2D bo_glTexSubImage2D;
	_glCopyTexSubImage1D bo_glCopyTexSubImage1D;
	_glCopyTexSubImage2D bo_glCopyTexSubImage2D;
	_glTexParameterf bo_glTexParameterf;
	_glTexParameteri bo_glTexParameteri;
	_glTexParameterfv bo_glTexParameterfv;
	_glTexParameteriv bo_glTexParameteriv;
	_glBindTexture bo_glBindTexture;
	_glDeleteTextures bo_glDeleteTextures;
	_glGenTextures bo_glGenTextures;
	_glAreTexturesResident bo_glAreTexturesResident;
	_glPrioritizeTextures bo_glPrioritizeTextures;
	_glTexEnvf bo_glTexEnvf;
	_glTexEnvi bo_glTexEnvi;
	_glTexEnvfv bo_glTexEnvfv;
	_glTexEnviv bo_glTexEnviv;
	_glFogf bo_glFogf;
	_glFogi bo_glFogi;
	_glFogfv bo_glFogfv;
	_glFogiv bo_glFogiv;
	_glScissor bo_glScissor;
	_glAlphaFunc bo_glAlphaFunc;
	_glStencilFunc bo_glStencilFunc;
	_glStencilOp bo_glStencilOp;
	_glDepthFunc bo_glDepthFunc;
	_glBlendFunc bo_glBlendFunc;
	_glLogicOp bo_glLogicOp;
	_glDrawBuffer bo_glDrawBuffer;
	_glIndexMask bo_glIndexMask;
	_glColorMask bo_glColorMask;
	_glDepthMask bo_glDepthMask;
	_glStencilMask bo_glStencilMask;
	_glClear bo_glClear;
	_glClearColor bo_glClearColor;
	_glClearIndex bo_glClearIndex;
	_glClearDepth bo_glClearDepth;
	_glClearStencil bo_glClearStencil;
	_glClearAccum bo_glClearAccum;
	_glAccum bo_glAccum;
	_glReadPixels bo_glReadPixels;
	_glReadBuffer bo_glReadBuffer;
	_glCopyPixels bo_glCopyPixels;
	_glMap2d bo_glMap2d;
	_glMap2f bo_glMap2f;
	_glMap1d bo_glMap1d;
	_glMap1f bo_glMap1f;
	_glEvalCoord1d bo_glEvalCoord1d;
	_glEvalCoord1f bo_glEvalCoord1f;
	_glEvalCoord1dv bo_glEvalCoord1dv;
	_glEvalCoord1fv bo_glEvalCoord1fv;
	_glEvalCoord2d bo_glEvalCoord2d;
	_glEvalCoord2f bo_glEvalCoord2f;
	_glEvalCoord2dv bo_glEvalCoord2dv;
	_glEvalCoord2fv bo_glEvalCoord2fv;
	_glMapGrid1d bo_glMapGrid1d;
	_glMapGrid1f bo_glMapGrid1f;
	_glMapGrid2d bo_glMapGrid2d;
	_glMapGrid2f bo_glMapGrid2f;
	_glEvalMesh1 bo_glEvalMesh1;
	_glEvalMesh2 bo_glEvalMesh2;
	_glEvalPoint1 bo_glEvalPoint1;
	_glEvalPoint2 bo_glEvalPoint2;
	_glInitNames bo_glInitNames;
	_glLoadName bo_glLoadName;
	_glPushName bo_glPushName;
	_glPopName bo_glPopName;
	_glRenderMode bo_glRenderMode;
	_glSelectBuffer bo_glSelectBuffer;
	_glFeedbackBuffer bo_glFeedbackBuffer;
	_glPassThrough bo_glPassThrough;
	_glNewList bo_glNewList;
	_glEndList bo_glEndList;
	_glCallList bo_glCallList;
	_glCallLists bo_glCallLists;
	_glListBase bo_glListBase;
	_glGenLists bo_glGenLists;
	_glIsList bo_glIsList;
	_glDeleteLists bo_glDeleteLists;
	_glFlush bo_glFlush;
	_glFinish bo_glFinish;
	_glHint bo_glHint;
	_glGetBooleanv bo_glGetBooleanv;
	_glGetIntegerv bo_glGetIntegerv;
	_glGetDoublev bo_glGetDoublev;
	_glGetFloatv bo_glGetFloatv;
	_glIsEnabled bo_glIsEnabled;
	_glGetClipPlane bo_glGetClipPlane;
	_glGetLightfv bo_glGetLightfv;
	_glGetLightiv bo_glGetLightiv;
	_glGetMaterialfv bo_glGetMaterialfv;
	_glGetMaterialiv bo_glGetMaterialiv;
	_glGetTexEnvfv bo_glGetTexEnvfv;
	_glGetTexEnviv bo_glGetTexEnviv;
	_glGetTexGenfv bo_glGetTexGenfv;
	_glGetTexGeniv bo_glGetTexGeniv;
	_glGetTexGendv bo_glGetTexGendv;
	_glGetTexParameterfv bo_glGetTexParameterfv;
	_glGetTexParameteriv bo_glGetTexParameteriv;
	_glGetTexLevelParameterfv bo_glGetTexLevelParameterfv;
	_glGetTexLevelParameteriv bo_glGetTexLevelParameteriv;
	_glGetPixelMapfv bo_glGetPixelMapfv;
	_glGetPixelMapuiv bo_glGetPixelMapuiv;
	_glGetPixelMapusv bo_glGetPixelMapusv;
	_glGetMapdv bo_glGetMapdv;
	_glGetMapfv bo_glGetMapfv;
	_glGetMapiv bo_glGetMapiv;
	_glGetTexImage bo_glGetTexImage;
	_glIsTexture bo_glIsTexture;
	_glGetPolygonStipple bo_glGetPolygonStipple;
	_glGetPointerv bo_glGetPointerv;
	_glGetString bo_glGetString;
	_glPushAttrib bo_glPushAttrib;
	_glPopAttrib bo_glPopAttrib;
	_glPushClientAttrib bo_glPushClientAttrib;
	_glPopClientAttrib bo_glPopClientAttrib;

	_glEdgeFlagPointer bo_glEdgeFlagPointer;
	_glTexCoordPointer bo_glTexCoordPointer;
	_glColorPointer bo_glColorPointer;
	_glIndexPointer bo_glIndexPointer;
	_glNormalPointer bo_glNormalPointer;
	_glVertexPointer bo_glVertexPointer;
	_glEnableClientState bo_glEnableClientState;
	_glDisableClientState bo_glDisableClientState;
	_glArrayElement bo_glArrayElement;
	_glDrawArrays bo_glDrawArrays;
	_glDrawElements bo_glDrawElements;
	_glInterleavedArrays bo_glInterleavedArrays;

}; // extern "C"

bool boglResolveOpenGL_1_1_Symbols(MyQLibrary& gl)
{
 RESOLVE_CHECK(glGetError);
 RESOLVE_CHECK(glBegin);
 RESOLVE_CHECK(glEnd);
 RESOLVE_CHECK(glEdgeFlag);
 RESOLVE_CHECK(glEdgeFlagv);
 RESOLVE_CHECK(glVertex2d);
 RESOLVE_CHECK(glVertex2f);
 RESOLVE_CHECK(glVertex2i);
 RESOLVE_CHECK(glVertex2s);
 RESOLVE_CHECK(glVertex3d);
 RESOLVE_CHECK(glVertex3f);
 RESOLVE_CHECK(glVertex3i);
 RESOLVE_CHECK(glVertex3s);
 RESOLVE_CHECK(glVertex4d);
 RESOLVE_CHECK(glVertex4f);
 RESOLVE_CHECK(glVertex4i);
 RESOLVE_CHECK(glVertex4s);
 RESOLVE_CHECK(glVertex2dv);
 RESOLVE_CHECK(glVertex2fv);
 RESOLVE_CHECK(glVertex2iv);
 RESOLVE_CHECK(glVertex2sv);
 RESOLVE_CHECK(glVertex3dv);
 RESOLVE_CHECK(glVertex3fv);
 RESOLVE_CHECK(glVertex3iv);
 RESOLVE_CHECK(glVertex3sv);
 RESOLVE_CHECK(glVertex4dv);
 RESOLVE_CHECK(glVertex4fv);
 RESOLVE_CHECK(glVertex4iv);
 RESOLVE_CHECK(glVertex4sv);
 RESOLVE_CHECK(glTexCoord1d);
 RESOLVE_CHECK(glTexCoord1f);
 RESOLVE_CHECK(glTexCoord1i);
 RESOLVE_CHECK(glTexCoord1s);
 RESOLVE_CHECK(glTexCoord2d);
 RESOLVE_CHECK(glTexCoord2f);
 RESOLVE_CHECK(glTexCoord2i);
 RESOLVE_CHECK(glTexCoord2s);
 RESOLVE_CHECK(glTexCoord3d);
 RESOLVE_CHECK(glTexCoord3f);
 RESOLVE_CHECK(glTexCoord3i);
 RESOLVE_CHECK(glTexCoord3s);
 RESOLVE_CHECK(glTexCoord4d);
 RESOLVE_CHECK(glTexCoord4f);
 RESOLVE_CHECK(glTexCoord4i);
 RESOLVE_CHECK(glTexCoord4s);
 RESOLVE_CHECK(glTexCoord1dv);
 RESOLVE_CHECK(glTexCoord1fv);
 RESOLVE_CHECK(glTexCoord1iv);
 RESOLVE_CHECK(glTexCoord1sv);
 RESOLVE_CHECK(glTexCoord2dv);
 RESOLVE_CHECK(glTexCoord2fv);
 RESOLVE_CHECK(glTexCoord2iv);
 RESOLVE_CHECK(glTexCoord2sv);
 RESOLVE_CHECK(glTexCoord3dv);
 RESOLVE_CHECK(glTexCoord3fv);
 RESOLVE_CHECK(glTexCoord3iv);
 RESOLVE_CHECK(glTexCoord3sv);
 RESOLVE_CHECK(glTexCoord4dv);
 RESOLVE_CHECK(glTexCoord4fv);
 RESOLVE_CHECK(glTexCoord4iv);
 RESOLVE_CHECK(glTexCoord4sv);
 RESOLVE_CHECK(glNormal3b);
 RESOLVE_CHECK(glNormal3d);
 RESOLVE_CHECK(glNormal3f);
 RESOLVE_CHECK(glNormal3i);
 RESOLVE_CHECK(glNormal3s);
 RESOLVE_CHECK(glNormal3bv);
 RESOLVE_CHECK(glNormal3dv);
 RESOLVE_CHECK(glNormal3fv);
 RESOLVE_CHECK(glNormal3iv);
 RESOLVE_CHECK(glNormal3sv);
 RESOLVE_CHECK(glColor3b);
 RESOLVE_CHECK(glColor3d);
 RESOLVE_CHECK(glColor3f);
 RESOLVE_CHECK(glColor3i);
 RESOLVE_CHECK(glColor3s);
 RESOLVE_CHECK(glColor3ub);
 RESOLVE_CHECK(glColor3ui);
 RESOLVE_CHECK(glColor3us);
 RESOLVE_CHECK(glColor4b);
 RESOLVE_CHECK(glColor4d);
 RESOLVE_CHECK(glColor4f);
 RESOLVE_CHECK(glColor4i);
 RESOLVE_CHECK(glColor4s);
 RESOLVE_CHECK(glColor4ub);
 RESOLVE_CHECK(glColor4ui);
 RESOLVE_CHECK(glColor4us);
 RESOLVE_CHECK(glColor3bv);
 RESOLVE_CHECK(glColor3dv);
 RESOLVE_CHECK(glColor3fv);
 RESOLVE_CHECK(glColor3iv);
 RESOLVE_CHECK(glColor3sv);
 RESOLVE_CHECK(glColor3ubv);
 RESOLVE_CHECK(glColor3uiv);
 RESOLVE_CHECK(glColor3usv);
 RESOLVE_CHECK(glColor4bv);
 RESOLVE_CHECK(glColor4dv);
 RESOLVE_CHECK(glColor4fv);
 RESOLVE_CHECK(glColor4iv);
 RESOLVE_CHECK(glColor4sv);
 RESOLVE_CHECK(glColor4ubv);
 RESOLVE_CHECK(glColor4uiv);
 RESOLVE_CHECK(glColor4usv);
 RESOLVE_CHECK(glIndexd);
 RESOLVE_CHECK(glIndexf);
 RESOLVE_CHECK(glIndexi);
 RESOLVE_CHECK(glIndexs);
 RESOLVE_CHECK(glIndexub);
 RESOLVE_CHECK(glIndexdv);
 RESOLVE_CHECK(glIndexfv);
 RESOLVE_CHECK(glIndexiv);
 RESOLVE_CHECK(glIndexsv);
 RESOLVE_CHECK(glIndexubv);
 RESOLVE_CHECK(glRectd);
 RESOLVE_CHECK(glRectf);
 RESOLVE_CHECK(glRecti);
 RESOLVE_CHECK(glRects);
 RESOLVE_CHECK(glRectdv);
 RESOLVE_CHECK(glRectfv);
 RESOLVE_CHECK(glRectiv);
 RESOLVE_CHECK(glRectsv);
 RESOLVE_CHECK(glDepthRange);
 RESOLVE_CHECK(glViewport);
 RESOLVE_CHECK(glMatrixMode);
 RESOLVE_CHECK(glLoadMatrixd);
 RESOLVE_CHECK(glLoadMatrixf);
 RESOLVE_CHECK(glMultMatrixd);
 RESOLVE_CHECK(glMultMatrixf);
 RESOLVE_CHECK(glLoadIdentity);
 RESOLVE_CHECK(glRotated);
 RESOLVE_CHECK(glRotatef);
 RESOLVE_CHECK(glTranslated);
 RESOLVE_CHECK(glTranslatef);
 RESOLVE_CHECK(glScaled);
 RESOLVE_CHECK(glScalef);
 RESOLVE_CHECK(glFrustum);
 RESOLVE_CHECK(glOrtho);
 RESOLVE_CHECK(glPushMatrix);
 RESOLVE_CHECK(glPopMatrix);
 RESOLVE_CHECK(glEnable);
 RESOLVE_CHECK(glDisable);
 RESOLVE_CHECK(glTexGend);
 RESOLVE_CHECK(glTexGenf);
 RESOLVE_CHECK(glTexGeni);
 RESOLVE_CHECK(glTexGendv);
 RESOLVE_CHECK(glTexGenfv);
 RESOLVE_CHECK(glTexGeniv);
 RESOLVE_CHECK(glClipPlane);
 RESOLVE_CHECK(glRasterPos2d);
 RESOLVE_CHECK(glRasterPos2f);
 RESOLVE_CHECK(glRasterPos2i);
 RESOLVE_CHECK(glRasterPos2s);
 RESOLVE_CHECK(glRasterPos3d);
 RESOLVE_CHECK(glRasterPos3f);
 RESOLVE_CHECK(glRasterPos3i);
 RESOLVE_CHECK(glRasterPos3s);
 RESOLVE_CHECK(glRasterPos4d);
 RESOLVE_CHECK(glRasterPos4f);
 RESOLVE_CHECK(glRasterPos4i);
 RESOLVE_CHECK(glRasterPos4s);
 RESOLVE_CHECK(glRasterPos2dv);
 RESOLVE_CHECK(glRasterPos2fv);
 RESOLVE_CHECK(glRasterPos2iv);
 RESOLVE_CHECK(glRasterPos2sv);
 RESOLVE_CHECK(glRasterPos3dv);
 RESOLVE_CHECK(glRasterPos3fv);
 RESOLVE_CHECK(glRasterPos3iv);
 RESOLVE_CHECK(glRasterPos3sv);
 RESOLVE_CHECK(glRasterPos4dv);
 RESOLVE_CHECK(glRasterPos4fv);
 RESOLVE_CHECK(glRasterPos4iv);
 RESOLVE_CHECK(glRasterPos4sv);
 RESOLVE_CHECK(glMaterialf);
 RESOLVE_CHECK(glMateriali);
 RESOLVE_CHECK(glMaterialfv);
 RESOLVE_CHECK(glMaterialiv);
 RESOLVE_CHECK(glFrontFace);
 RESOLVE_CHECK(glLightf);
 RESOLVE_CHECK(glLighti);
 RESOLVE_CHECK(glLightfv);
 RESOLVE_CHECK(glLightiv);
 RESOLVE_CHECK(glLightModelf);
 RESOLVE_CHECK(glLightModeli);
 RESOLVE_CHECK(glLightModelfv);
 RESOLVE_CHECK(glLightModeliv);
 RESOLVE_CHECK(glColorMaterial);
 RESOLVE_CHECK(glShadeModel);
 RESOLVE_CHECK(glPointSize);
 RESOLVE_CHECK(glLineWidth);
 RESOLVE_CHECK(glLineStipple);
 RESOLVE_CHECK(glCullFace);
 RESOLVE_CHECK(glPolygonStipple);
 RESOLVE_CHECK(glPolygonMode);
 RESOLVE_CHECK(glPolygonOffset);
 RESOLVE_CHECK(glPixelStoref);
 RESOLVE_CHECK(glPixelStorei);
 RESOLVE_CHECK(glPixelTransferf);
 RESOLVE_CHECK(glPixelTransferi);
 RESOLVE_CHECK(glPixelMapfv);
 RESOLVE_CHECK(glPixelMapuiv);
 RESOLVE_CHECK(glPixelMapusv);
 RESOLVE_CHECK(glDrawPixels);
 RESOLVE_CHECK(glPixelZoom);
 RESOLVE_CHECK(glBitmap);
 RESOLVE_CHECK(glTexImage1D);
 RESOLVE_CHECK(glTexImage2D);
 RESOLVE_CHECK(glCopyTexImage1D);
 RESOLVE_CHECK(glCopyTexImage2D);
 RESOLVE_CHECK(glTexSubImage1D);
 RESOLVE_CHECK(glTexSubImage2D);
 RESOLVE_CHECK(glCopyTexSubImage1D);
 RESOLVE_CHECK(glCopyTexSubImage2D);
 RESOLVE_CHECK(glTexParameterf);
 RESOLVE_CHECK(glTexParameteri);
 RESOLVE_CHECK(glTexParameterfv);
 RESOLVE_CHECK(glTexParameteriv);
 RESOLVE_CHECK(glBindTexture);
 RESOLVE_CHECK(glDeleteTextures);
 RESOLVE_CHECK(glGenTextures);
 RESOLVE_CHECK(glAreTexturesResident);
 RESOLVE_CHECK(glPrioritizeTextures);
 RESOLVE_CHECK(glTexEnvf);
 RESOLVE_CHECK(glTexEnvi);
 RESOLVE_CHECK(glTexEnvfv);
 RESOLVE_CHECK(glTexEnviv);
 RESOLVE_CHECK(glFogf);
 RESOLVE_CHECK(glFogi);
 RESOLVE_CHECK(glFogfv);
 RESOLVE_CHECK(glFogiv);
 RESOLVE_CHECK(glScissor);
 RESOLVE_CHECK(glAlphaFunc);
 RESOLVE_CHECK(glStencilFunc);
 RESOLVE_CHECK(glStencilOp);
 RESOLVE_CHECK(glDepthFunc);
 RESOLVE_CHECK(glBlendFunc);
 RESOLVE_CHECK(glLogicOp);
 RESOLVE_CHECK(glDrawBuffer);
 RESOLVE_CHECK(glIndexMask);
 RESOLVE_CHECK(glColorMask);
 RESOLVE_CHECK(glDepthMask);
 RESOLVE_CHECK(glStencilMask);
 RESOLVE_CHECK(glClear);
 RESOLVE_CHECK(glClearColor);
 RESOLVE_CHECK(glClearIndex);
 RESOLVE_CHECK(glClearDepth);
 RESOLVE_CHECK(glClearStencil);
 RESOLVE_CHECK(glClearAccum);
 RESOLVE_CHECK(glAccum);
 RESOLVE_CHECK(glReadPixels);
 RESOLVE_CHECK(glReadBuffer);
 RESOLVE_CHECK(glCopyPixels);
 RESOLVE_CHECK(glMap2d);
 RESOLVE_CHECK(glMap2f);
 RESOLVE_CHECK(glMap1d);
 RESOLVE_CHECK(glMap1f);
 RESOLVE_CHECK(glEvalCoord1d);
 RESOLVE_CHECK(glEvalCoord1f);
 RESOLVE_CHECK(glEvalCoord1dv);
 RESOLVE_CHECK(glEvalCoord1fv);
 RESOLVE_CHECK(glEvalCoord2d);
 RESOLVE_CHECK(glEvalCoord2f);
 RESOLVE_CHECK(glEvalCoord2dv);
 RESOLVE_CHECK(glEvalCoord2fv);
 RESOLVE_CHECK(glMapGrid1d);
 RESOLVE_CHECK(glMapGrid1f);
 RESOLVE_CHECK(glMapGrid2d);
 RESOLVE_CHECK(glMapGrid2f);
 RESOLVE_CHECK(glEvalMesh1);
 RESOLVE_CHECK(glEvalMesh2);
 RESOLVE_CHECK(glEvalPoint1);
 RESOLVE_CHECK(glEvalPoint2);
 RESOLVE_CHECK(glInitNames);
 RESOLVE_CHECK(glLoadName);
 RESOLVE_CHECK(glPushName);
 RESOLVE_CHECK(glPopName);
 RESOLVE_CHECK(glRenderMode);
 RESOLVE_CHECK(glSelectBuffer);
 RESOLVE_CHECK(glFeedbackBuffer);
 RESOLVE_CHECK(glPassThrough);
 RESOLVE_CHECK(glNewList);
 RESOLVE_CHECK(glEndList);
 RESOLVE_CHECK(glCallList);
 RESOLVE_CHECK(glCallLists);
 RESOLVE_CHECK(glListBase);
 RESOLVE_CHECK(glGenLists);
 RESOLVE_CHECK(glIsList);
 RESOLVE_CHECK(glDeleteLists);
 RESOLVE_CHECK(glFlush);
 RESOLVE_CHECK(glFinish);
 RESOLVE_CHECK(glHint);
 RESOLVE_CHECK(glGetBooleanv);
 RESOLVE_CHECK(glGetIntegerv);
 RESOLVE_CHECK(glGetDoublev);
 RESOLVE_CHECK(glGetFloatv);
 RESOLVE_CHECK(glIsEnabled);
 RESOLVE_CHECK(glGetClipPlane);
 RESOLVE_CHECK(glGetLightfv);
 RESOLVE_CHECK(glGetLightiv);
 RESOLVE_CHECK(glGetMaterialfv);
 RESOLVE_CHECK(glGetMaterialiv);
 RESOLVE_CHECK(glGetTexEnvfv);
 RESOLVE_CHECK(glGetTexEnviv);
 RESOLVE_CHECK(glGetTexGenfv);
 RESOLVE_CHECK(glGetTexGeniv);
 RESOLVE_CHECK(glGetTexGendv);
 RESOLVE_CHECK(glGetTexParameterfv);
 RESOLVE_CHECK(glGetTexParameteriv);
 RESOLVE_CHECK(glGetTexLevelParameterfv);
 RESOLVE_CHECK(glGetTexLevelParameteriv);
 RESOLVE_CHECK(glGetPixelMapfv);
 RESOLVE_CHECK(glGetPixelMapuiv);
 RESOLVE_CHECK(glGetPixelMapusv);
 RESOLVE_CHECK(glGetMapdv);
 RESOLVE_CHECK(glGetMapfv);
 RESOLVE_CHECK(glGetMapiv);
 RESOLVE_CHECK(glGetTexImage);
 RESOLVE_CHECK(glIsTexture);
 RESOLVE_CHECK(glGetPolygonStipple);
 RESOLVE_CHECK(glGetPointerv);
 RESOLVE_CHECK(glGetString);
 RESOLVE_CHECK(glPushAttrib);
 RESOLVE_CHECK(glPopAttrib);
 RESOLVE_CHECK(glPushClientAttrib);
 RESOLVE_CHECK(glPopClientAttrib);

 RESOLVE_CHECK(glEdgeFlagPointer);
 RESOLVE_CHECK(glTexCoordPointer);
 RESOLVE_CHECK(glColorPointer);
 RESOLVE_CHECK(glIndexPointer);
 RESOLVE_CHECK(glNormalPointer);
 RESOLVE_CHECK(glVertexPointer);
 RESOLVE_CHECK(glEnableClientState);
 RESOLVE_CHECK(glDisableClientState);
 RESOLVE_CHECK(glArrayElement);
 RESOLVE_CHECK(glDrawArrays);
 RESOLVE_CHECK(glDrawElements);
 RESOLVE_CHECK(glInterleavedArrays);

 return true;
}


// OpenGL 1.2
extern "C" {
	_glTexImage3D bo_glTexImage3D;
	_glTexSubImage3D bo_glTexSubImage3D;
	_glCopyTexSubImage3D bo_glCopyTexSubImage3D;
	_glDrawRangeElements bo_glDrawRangeElements;
	_glColorTable bo_glColorTable;
	_glCopyColorTable bo_glCopyColorTable;
	_glColorTableParameteriv bo_glColorTableParameteriv;
	_glColorTableParameterfv bo_glColorTableParameterfv;
	_glGetColorTable bo_glGetColorTable;
	_glGetColorTableParameterfv bo_glGetColorTableParameterfv;
	_glGetColorTableParameteriv bo_glGetColorTableParameteriv;
	_glColorSubTable bo_glColorSubTable;
	_glCopyColorSubTable bo_glCopyColorSubTable;
	_glConvolutionFilter1D bo_glConvolutionFilter1D;
	_glConvolutionFilter2D bo_glConvolutionFilter2D;
	_glCopyConvolutionFilter1D bo_glCopyConvolutionFilter1D;
	_glCopyConvolutionFilter2D bo_glCopyConvolutionFilter2D;
	_glGetConvolutionFilter bo_glGetConvolutionFilter;
	_glSeparableFilter2D bo_glSeparableFilter2D;
	_glGetSeparableFilter bo_glGetSeparableFilter;
	_glConvolutionParameterf bo_glConvolutionParameterf;
	_glConvolutionParameterfv bo_glConvolutionParameterfv;
	_glConvolutionParameteri bo_glConvolutionParameteri;
	_glConvolutionParameteriv bo_glConvolutionParameteriv;
	_glGetConvolutionParameterfv bo_glGetConvolutionParameterfv;
	_glGetConvolutionParameteriv bo_glGetConvolutionParameteriv;
	_glHistogram bo_glHistogram;
	_glResetHistogram bo_glResetHistogram;
	_glGetHistogram bo_glGetHistogram;
	_glGetHistogramParameterfv bo_glGetHistogramParameterfv;
	_glGetHistogramParameteriv bo_glGetHistogramParameteriv;
	_glMinmax bo_glMinmax;
	_glResetMinmax bo_glResetMinmax;
	_glGetMinmax bo_glGetMinmax;
	_glGetMinmaxParameterfv bo_glGetMinmaxParameterfv;
	_glGetMinmaxParameteriv bo_glGetMinmaxParameteriv;
	_glBlendColor bo_glBlendColor;
	_glBlendEquation bo_glBlendEquation;
}; // "C"

bool boglResolveOpenGL_1_2_Symbols(MyQLibrary& gl)
{
 RESOLVE_CHECK(glTexImage3D);
 RESOLVE_CHECK(glTexSubImage3D);
 RESOLVE_CHECK(glCopyTexSubImage3D);
 RESOLVE_CHECK(glDrawRangeElements);

 QStringList extensions = boglGetOpenGLExtensions();
 if (extensions.contains("GL_ARB_imaging")) {
	RESOLVE_CHECK(glColorTable);
	RESOLVE_CHECK(glCopyColorTable);
	RESOLVE_CHECK(glColorTableParameteriv);
	RESOLVE_CHECK(glColorTableParameterfv);
	RESOLVE_CHECK(glGetColorTable);
	RESOLVE_CHECK(glGetColorTableParameterfv);
	RESOLVE_CHECK(glGetColorTableParameteriv);
	RESOLVE_CHECK(glColorSubTable);
	RESOLVE_CHECK(glCopyColorSubTable);
	RESOLVE_CHECK(glConvolutionFilter1D);
	RESOLVE_CHECK(glConvolutionFilter2D);
	RESOLVE_CHECK(glCopyConvolutionFilter1D);
	RESOLVE_CHECK(glCopyConvolutionFilter2D);
	RESOLVE_CHECK(glGetConvolutionFilter);
	RESOLVE_CHECK(glSeparableFilter2D);
	RESOLVE_CHECK(glGetSeparableFilter);
	RESOLVE_CHECK(glConvolutionParameterf);
	RESOLVE_CHECK(glConvolutionParameterfv);
	RESOLVE_CHECK(glConvolutionParameteri);
	RESOLVE_CHECK(glConvolutionParameteriv);
	RESOLVE_CHECK(glGetConvolutionParameterfv);
	RESOLVE_CHECK(glGetConvolutionParameteriv);
	RESOLVE_CHECK(glHistogram);
	RESOLVE_CHECK(glResetHistogram);
	RESOLVE_CHECK(glGetHistogram);
	RESOLVE_CHECK(glGetHistogramParameterfv);
	RESOLVE_CHECK(glGetHistogramParameteriv);
	RESOLVE_CHECK(glMinmax);
	RESOLVE_CHECK(glResetMinmax);
	RESOLVE_CHECK(glGetMinmax);
	RESOLVE_CHECK(glGetMinmaxParameterfv);
	RESOLVE_CHECK(glGetMinmaxParameteriv);
	RESOLVE_CHECK(glBlendColor);
	RESOLVE_CHECK(glBlendEquation);
 }
 return true;
}

bool boglResolveOpenGL_1_2_1_Symbols(MyQLibrary& gl)
{
 // OpenGL 1.2.1 does not introduce new functions
 Q_UNUSED(gl);
 return true;
}


// OpenGL 1.3
extern "C" {
	_glCompressedTexImage1D bo_glCompressedTexImage1D;
	_glCompressedTexImage2D bo_glCompressedTexImage2D;
	_glCompressedTexImage3D bo_glCompressedTexImage3D;
	_glCompressedTexSubImage1D bo_glCompressedTexSubImage1D;
	_glCompressedTexSubImage2D bo_glCompressedTexSubImage2D;
	_glCompressedTexSubImage3D bo_glCompressedTexSubImage3D;
	_glGetCompressedTexImage bo_glGetCompressedTexImage;
	_glSampleCoverage bo_glSampleCoverage;
	_glMultiTexCoord1d bo_glMultiTexCoord1d;
	_glMultiTexCoord1dv bo_glMultiTexCoord1dv;
	_glMultiTexCoord1f bo_glMultiTexCoord1f;
	_glMultiTexCoord1fv bo_glMultiTexCoord1fv;
	_glMultiTexCoord1i bo_glMultiTexCoord1i;
	_glMultiTexCoord1iv bo_glMultiTexCoord1iv;
	_glMultiTexCoord1s bo_glMultiTexCoord1s;
	_glMultiTexCoord1sv bo_glMultiTexCoord1sv;
	_glMultiTexCoord2d bo_glMultiTexCoord2d;
	_glMultiTexCoord2dv bo_glMultiTexCoord2dv;
	_glMultiTexCoord2f bo_glMultiTexCoord2f;
	_glMultiTexCoord2fv bo_glMultiTexCoord2fv;
	_glMultiTexCoord2i bo_glMultiTexCoord2i;
	_glMultiTexCoord2iv bo_glMultiTexCoord2iv;
	_glMultiTexCoord2s bo_glMultiTexCoord2s;
	_glMultiTexCoord2sv bo_glMultiTexCoord2sv;
	_glMultiTexCoord3d bo_glMultiTexCoord3d;
	_glMultiTexCoord3dv bo_glMultiTexCoord3dv;
	_glMultiTexCoord3f bo_glMultiTexCoord3f;
	_glMultiTexCoord3fv bo_glMultiTexCoord3fv;
	_glMultiTexCoord3i bo_glMultiTexCoord3i;
	_glMultiTexCoord3iv bo_glMultiTexCoord3iv;
	_glMultiTexCoord3s bo_glMultiTexCoord3s;
	_glMultiTexCoord3sv bo_glMultiTexCoord3sv;
	_glMultiTexCoord4d bo_glMultiTexCoord4d;
	_glMultiTexCoord4dv bo_glMultiTexCoord4dv;
	_glMultiTexCoord4f bo_glMultiTexCoord4f;
	_glMultiTexCoord4fv bo_glMultiTexCoord4fv;
	_glMultiTexCoord4i bo_glMultiTexCoord4i;
	_glMultiTexCoord4iv bo_glMultiTexCoord4iv;
	_glMultiTexCoord4s bo_glMultiTexCoord4s;
	_glMultiTexCoord4sv bo_glMultiTexCoord4sv;
	_glClientActiveTexture bo_glClientActiveTexture;
	_glActiveTexture bo_glActiveTexture;
	_glLoadTransposeMatrixd bo_glLoadTransposeMatrixd;
	_glLoadTransposeMatrixf bo_glLoadTransposeMatrixf;
	_glMultTransposeMatrixd bo_glMultTransposeMatrixd;
	_glMultTransposeMatrixf bo_glMultTransposeMatrixf;
};

bool boglResolveOpenGL_1_3_Symbols(MyQLibrary& gl)
{
 RESOLVE(glCompressedTexImage1D);
 RESOLVE(glCompressedTexImage2D);
 RESOLVE(glCompressedTexImage3D);
 RESOLVE(glCompressedTexSubImage1D);
 RESOLVE(glCompressedTexSubImage2D);
 RESOLVE(glCompressedTexSubImage3D);
 RESOLVE(glGetCompressedTexImage);
 RESOLVE(glSampleCoverage);
 RESOLVE(glMultiTexCoord1d);
 RESOLVE(glMultiTexCoord1dv);
 RESOLVE(glMultiTexCoord1f);
 RESOLVE(glMultiTexCoord1fv);
 RESOLVE(glMultiTexCoord1i);
 RESOLVE(glMultiTexCoord1iv);
 RESOLVE(glMultiTexCoord1s);
 RESOLVE(glMultiTexCoord1sv);
 RESOLVE(glMultiTexCoord2d);
 RESOLVE(glMultiTexCoord2dv);
 RESOLVE(glMultiTexCoord2f);
 RESOLVE(glMultiTexCoord2fv);
 RESOLVE(glMultiTexCoord2i);
 RESOLVE(glMultiTexCoord2iv);
 RESOLVE(glMultiTexCoord2s);
 RESOLVE(glMultiTexCoord2sv);
 RESOLVE(glMultiTexCoord3d);
 RESOLVE(glMultiTexCoord3dv);
 RESOLVE(glMultiTexCoord3f);
 RESOLVE(glMultiTexCoord3fv);
 RESOLVE(glMultiTexCoord3i);
 RESOLVE(glMultiTexCoord3iv);
 RESOLVE(glMultiTexCoord3s);
 RESOLVE(glMultiTexCoord3sv);
 RESOLVE(glMultiTexCoord4d);
 RESOLVE(glMultiTexCoord4dv);
 RESOLVE(glMultiTexCoord4f);
 RESOLVE(glMultiTexCoord4fv);
 RESOLVE(glMultiTexCoord4i);
 RESOLVE(glMultiTexCoord4iv);
 RESOLVE(glMultiTexCoord4s);
 RESOLVE(glMultiTexCoord4sv);
 RESOLVE(glClientActiveTexture);
 RESOLVE(glActiveTexture);
 RESOLVE(glLoadTransposeMatrixd);
 RESOLVE(glLoadTransposeMatrixf);
 RESOLVE(glMultTransposeMatrixd);
 RESOLVE(glMultTransposeMatrixf);
 return true;
}

// OpenGL 1.4
extern "C" {
	_glFogCoordPointer bo_glFogCoordPointer;
	_glFogCoordf bo_glFogCoordf;
	_glFogCoordd bo_glFogCoordd;
	_glFogCoordfv bo_glFogCoordfv;
	_glFogCoorddv bo_glFogCoorddv;
	_glMultiDrawArrays bo_glMultiDrawArrays;
	_glMultiDrawElements bo_glMultiDrawElements;
	_glPointParameterf bo_glPointParameterf;
	_glPointParameterfv bo_glPointParameterfv;
	_glPointParameteri bo_glPointParameteri;
	_glPointParameteriv bo_glPointParameteriv;
	_glSecondaryColor3b bo_glSecondaryColor3b;
	_glSecondaryColor3s bo_glSecondaryColor3s;
	_glSecondaryColor3i bo_glSecondaryColor3i;
	_glSecondaryColor3f bo_glSecondaryColor3f;
	_glSecondaryColor3d bo_glSecondaryColor3d;
	_glSecondaryColor3bv bo_glSecondaryColor3bv;
	_glSecondaryColor3sv bo_glSecondaryColor3sv;
	_glSecondaryColor3iv bo_glSecondaryColor3iv;
	_glSecondaryColor3fv bo_glSecondaryColor3fv;
	_glSecondaryColor3dv bo_glSecondaryColor3dv;
	_glSecondaryColor3ub bo_glSecondaryColor3ub;
	_glSecondaryColor3us bo_glSecondaryColor3us;
	_glSecondaryColor3ui bo_glSecondaryColor3ui;
	_glSecondaryColor3ubv bo_glSecondaryColor3ubv;
	_glSecondaryColor3usv bo_glSecondaryColor3usv;
	_glSecondaryColor3uiv bo_glSecondaryColor3uiv;
	_glSecondaryColorPointer bo_glSecondaryColorPointer;
	_glBlendFuncSeparate bo_glBlendFuncSeparate;
	_glWindowPos2d bo_glWindowPos2d;
	_glWindowPos2f bo_glWindowPos2f;
	_glWindowPos2i bo_glWindowPos2i;
	_glWindowPos2s bo_glWindowPos2s;
	_glWindowPos2dv bo_glWindowPos2dv;
	_glWindowPos2fv bo_glWindowPos2fv;
	_glWindowPos2iv bo_glWindowPos2iv;
	_glWindowPos2sv bo_glWindowPos2sv;
	_glWindowPos3d bo_glWindowPos3d;
	_glWindowPos3f bo_glWindowPos3f;
	_glWindowPos3i bo_glWindowPos3i;
	_glWindowPos3s bo_glWindowPos3s;
	_glWindowPos3dv bo_glWindowPos3dv;
	_glWindowPos3fv bo_glWindowPos3fv;
	_glWindowPos3iv bo_glWindowPos3iv;
	_glWindowPos3sv bo_glWindowPos3sv;
}; // extern "C"

bool boglResolveOpenGL_1_4_Symbols(MyQLibrary& gl)
{
 RESOLVE(glFogCoordPointer);
 RESOLVE(glFogCoordf);
 RESOLVE(glFogCoordd);
 RESOLVE(glFogCoordfv);
 RESOLVE(glFogCoorddv);
 RESOLVE(glMultiDrawArrays);
 RESOLVE(glMultiDrawElements);
 RESOLVE(glPointParameterf);
 RESOLVE(glPointParameterfv);
 RESOLVE(glPointParameteri);
 RESOLVE(glPointParameteriv);
 RESOLVE(glSecondaryColor3b);
 RESOLVE(glSecondaryColor3s);
 RESOLVE(glSecondaryColor3i);
 RESOLVE(glSecondaryColor3f);
 RESOLVE(glSecondaryColor3d);
 RESOLVE(glSecondaryColor3bv);
 RESOLVE(glSecondaryColor3sv);
 RESOLVE(glSecondaryColor3iv);
 RESOLVE(glSecondaryColor3fv);
 RESOLVE(glSecondaryColor3dv);
 RESOLVE(glSecondaryColor3ub);
 RESOLVE(glSecondaryColor3us);
 RESOLVE(glSecondaryColor3ui);
 RESOLVE(glSecondaryColor3ubv);
 RESOLVE(glSecondaryColor3usv);
 RESOLVE(glSecondaryColor3uiv);
 RESOLVE(glSecondaryColorPointer);
 RESOLVE(glBlendFuncSeparate);
 RESOLVE(glWindowPos2d);
 RESOLVE(glWindowPos2f);
 RESOLVE(glWindowPos2i);
 RESOLVE(glWindowPos2s);
 RESOLVE(glWindowPos2dv);
 RESOLVE(glWindowPos2fv);
 RESOLVE(glWindowPos2iv);
 RESOLVE(glWindowPos2sv);
 RESOLVE(glWindowPos3d);
 RESOLVE(glWindowPos3f);
 RESOLVE(glWindowPos3i);
 RESOLVE(glWindowPos3s);
 RESOLVE(glWindowPos3dv);
 RESOLVE(glWindowPos3fv);
 RESOLVE(glWindowPos3iv);
 RESOLVE(glWindowPos3sv);


 return true;
}


// OpenGL 1.5
extern "C" {
	_glBindBuffer bo_glBindBuffer;
	_glDeleteBuffers bo_glDeleteBuffers;
	_glGenBuffers bo_glGenBuffers;
	_glIsBuffer bo_glIsBuffer;
	_glBufferData bo_glBufferData;
	_glBufferSubData bo_glBufferSubData;
	_glMapBuffer bo_glMapBuffer;
	_glUnmapBuffer bo_glUnmapBuffer;
	_glGetBufferParameteriv bo_glGetBufferParameteriv;
	_glGetBufferPointerv bo_glGetBufferPointerv;
	_glGenQueries bo_glGenQueries;
	_glDeleteQueries bo_glDeleteQueries;
	_glIsQuery bo_glIsQuery;
	_glBeginQuery bo_glBeginQuery;
	_glEndQuery bo_glEndQuery;
	_glGetQueryiv bo_glGetQueryiv;
	_glGetQueryObjectiv bo_glGetQueryObjectiv;
	_glGetQueryObjectuiv bo_glGetQueryObjectuiv;
}; // extern "C"

bool boglResolveOpenGL_1_5_Symbols(MyQLibrary& gl)
{
 RESOLVE(glBindBuffer);
 RESOLVE(glDeleteBuffers);
 RESOLVE(glGenBuffers);
 RESOLVE(glIsBuffer);
 RESOLVE(glBufferData);
 RESOLVE(glBufferSubData);
 RESOLVE(glMapBuffer);
 RESOLVE(glUnmapBuffer);
 RESOLVE(glGetBufferParameteriv);
 RESOLVE(glGetBufferPointerv);
 RESOLVE(glGenQueries);
 RESOLVE(glDeleteQueries);
 RESOLVE(glIsQuery);
 RESOLVE(glBeginQuery);
 RESOLVE(glEndQuery);
 RESOLVE(glGetQueryiv);
 RESOLVE(glGetQueryObjectiv);
 RESOLVE(glGetQueryObjectuiv);

 return true;
}


// OpenGL 2.0
extern "C" {
	_glDrawBuffers bo_glDrawBuffers;
	_glStencilOpSeparate bo_glStencilOpSeparate;
	_glStencilFuncSeparate bo_glStencilFuncSeparate;
	_glIsShader bo_glIsShader;
	_glIsProgram bo_glIsProgram;
	_glGetAttachedShaders bo_glGetAttachedShaders;
	_glCreateShader bo_glCreateShader;
	_glShaderSource bo_glShaderSource;
	_glCompileShader bo_glCompileShader;
	_glDeleteShader bo_glDeleteShader;
	_glCreateProgram bo_glCreateProgram;
	_glAttachShader bo_glAttachShader;
	_glDetachShader bo_glDetachShader;
	_glLinkProgram bo_glLinkProgram;
	_glUseProgram bo_glUseProgram;
	_glDeleteProgram bo_glDeleteProgram;
	_glGetShaderInfoLog bo_glGetShaderInfoLog;
	_glGetProgramInfoLog bo_glGetProgramInfoLog;
	_glGetShaderSource bo_glGetShaderSource;
	_glGetUniformfv bo_glGetUniformfv;
	_glGetUniformiv bo_glGetUniformiv;
	_glGetProgramiv bo_glGetProgramiv;
	_glGetShaderiv bo_glGetShaderiv;
	_glUniform1f bo_glUniform1f;
	_glUniform2f bo_glUniform2f;
	_glUniform3f bo_glUniform3f;
	_glUniform4f bo_glUniform4f;
	_glUniform1i bo_glUniform1i;
	_glUniform2i bo_glUniform2i;
	_glUniform3i bo_glUniform3i;
	_glUniform4i bo_glUniform4i;
	_glUniform1fv bo_glUniform1fv;
	_glUniform2fv bo_glUniform2fv;
	_glUniform3fv bo_glUniform3fv;
	_glUniform4fv bo_glUniform4fv;
	_glUniform1iv bo_glUniform1iv;
	_glUniform2iv bo_glUniform2iv;
	_glUniform3iv bo_glUniform3iv;
	_glUniform4iv bo_glUniform4iv;
	_glUniformMatrix2fv bo_glUniformMatrix2fv;
	_glUniformMatrix3fv bo_glUniformMatrix3fv;
	_glUniformMatrix4fv bo_glUniformMatrix4fv;
	_glValidateProgram bo_glValidateProgram;
	_glGetUniformLocation bo_glGetUniformLocation;
	_glGetActiveUniform bo_glGetActiveUniform;
	_glVertexAttrib1s bo_glVertexAttrib1s;
	_glVertexAttrib1f bo_glVertexAttrib1f;
	_glVertexAttrib1d bo_glVertexAttrib1d;
	_glVertexAttrib2s bo_glVertexAttrib2s;
	_glVertexAttrib2f bo_glVertexAttrib2f;
	_glVertexAttrib2d bo_glVertexAttrib2d;
	_glVertexAttrib3s bo_glVertexAttrib3s;
	_glVertexAttrib3f bo_glVertexAttrib3f;
	_glVertexAttrib3d bo_glVertexAttrib3d;
	_glVertexAttrib4s bo_glVertexAttrib4s;
	_glVertexAttrib4f bo_glVertexAttrib4f;
	_glVertexAttrib4d bo_glVertexAttrib4d;
	_glVertexAttrib1sv bo_glVertexAttrib1sv;
	_glVertexAttrib1fv bo_glVertexAttrib1fv;
	_glVertexAttrib1dv bo_glVertexAttrib1dv;
	_glVertexAttrib2sv bo_glVertexAttrib2sv;
	_glVertexAttrib2fv bo_glVertexAttrib2fv;
	_glVertexAttrib2dv bo_glVertexAttrib2dv;
	_glVertexAttrib3sv bo_glVertexAttrib3sv;
	_glVertexAttrib3fv bo_glVertexAttrib3fv;
	_glVertexAttrib3dv bo_glVertexAttrib3dv;
	_glVertexAttrib4sv bo_glVertexAttrib4sv;
	_glVertexAttrib4fv bo_glVertexAttrib4fv;
	_glVertexAttrib4dv bo_glVertexAttrib4dv;
	_glVertexAttrib4bv bo_glVertexAttrib4bv;
	_glVertexAttrib4iv bo_glVertexAttrib4iv;
	_glVertexAttrib4ubv bo_glVertexAttrib4ubv;
	_glVertexAttrib4usv bo_glVertexAttrib4usv;
	_glVertexAttrib4uiv bo_glVertexAttrib4uiv;
	_glVertexAttrib4Nub bo_glVertexAttrib4Nub;
	_glVertexAttrib4Nbv bo_glVertexAttrib4Nbv;
	_glVertexAttrib4Nsv bo_glVertexAttrib4Nsv;
	_glVertexAttrib4Niv bo_glVertexAttrib4Niv;
	_glVertexAttrib4Nubv bo_glVertexAttrib4Nubv;
	_glVertexAttrib4Nusv bo_glVertexAttrib4Nusv;
	_glVertexAttrib4Nuiv bo_glVertexAttrib4Nuiv;
	_glVertexAttribPointer bo_glVertexAttribPointer;
	_glEnableVertexAttribArray bo_glEnableVertexAttribArray;
	_glDisableVertexAttribArray bo_glDisableVertexAttribArray;
	_glBindAttribLocation bo_glBindAttribLocation;
	_glGetActiveAttrib bo_glGetActiveAttrib;
	_glGetAttribLocation bo_glGetAttribLocation;
	_glGetVertexAttribdv bo_glGetVertexAttribdv;
	_glGetVertexAttribfv bo_glGetVertexAttribfv;
	_glGetVertexAttribiv bo_glGetVertexAttribiv;
	_glGetVertexAttribPointerv bo_glGetVertexAttribPointerv;
	_glBlendEquationSeparate bo_glBlendEquationSeparate;
}; // extern "C"

bool boglResolveOpenGL_2_0_Symbols(MyQLibrary& gl)
{
 RESOLVE(glDrawBuffers);
 RESOLVE(glStencilOpSeparate);
 RESOLVE(glStencilFuncSeparate);
 RESOLVE(glIsShader);
 RESOLVE(glIsProgram);
 RESOLVE(glGetAttachedShaders);
 RESOLVE(glCreateShader);
 RESOLVE(glShaderSource);
 RESOLVE(glCompileShader);
 RESOLVE(glDeleteShader);
 RESOLVE(glCreateProgram);
 RESOLVE(glAttachShader);
 RESOLVE(glDetachShader);
 RESOLVE(glLinkProgram);
 RESOLVE(glUseProgram);
 RESOLVE(glDeleteProgram);
 RESOLVE(glGetShaderInfoLog);
 RESOLVE(glGetProgramInfoLog);
 RESOLVE(glGetShaderSource);
 RESOLVE(glGetUniformfv);
 RESOLVE(glGetUniformiv);
 RESOLVE(glGetProgramiv);
 RESOLVE(glGetShaderiv);
 RESOLVE(glUniform1f);
 RESOLVE(glUniform2f);
 RESOLVE(glUniform3f);
 RESOLVE(glUniform4f);
 RESOLVE(glUniform1i);
 RESOLVE(glUniform2i);
 RESOLVE(glUniform3i);
 RESOLVE(glUniform4i);
 RESOLVE(glUniform1fv);
 RESOLVE(glUniform2fv);
 RESOLVE(glUniform3fv);
 RESOLVE(glUniform4fv);
 RESOLVE(glUniform1iv);
 RESOLVE(glUniform2iv);
 RESOLVE(glUniform3iv);
 RESOLVE(glUniform4iv);
 RESOLVE(glUniformMatrix2fv);
 RESOLVE(glUniformMatrix3fv);
 RESOLVE(glUniformMatrix4fv);
 RESOLVE(glValidateProgram);
 RESOLVE(glGetUniformLocation);
 RESOLVE(glGetActiveUniform);
 RESOLVE(glVertexAttrib1s);
 RESOLVE(glVertexAttrib1f);
 RESOLVE(glVertexAttrib1d);
 RESOLVE(glVertexAttrib2s);
 RESOLVE(glVertexAttrib2f);
 RESOLVE(glVertexAttrib2d);
 RESOLVE(glVertexAttrib3s);
 RESOLVE(glVertexAttrib3f);
 RESOLVE(glVertexAttrib3d);
 RESOLVE(glVertexAttrib4s);
 RESOLVE(glVertexAttrib4f);
 RESOLVE(glVertexAttrib4d);
 RESOLVE(glVertexAttrib1sv);
 RESOLVE(glVertexAttrib1fv);
 RESOLVE(glVertexAttrib1dv);
 RESOLVE(glVertexAttrib2sv);
 RESOLVE(glVertexAttrib2fv);
 RESOLVE(glVertexAttrib2dv);
 RESOLVE(glVertexAttrib3sv);
 RESOLVE(glVertexAttrib3fv);
 RESOLVE(glVertexAttrib3dv);
 RESOLVE(glVertexAttrib4sv);
 RESOLVE(glVertexAttrib4fv);
 RESOLVE(glVertexAttrib4dv);
 RESOLVE(glVertexAttrib4bv);
 RESOLVE(glVertexAttrib4iv);
 RESOLVE(glVertexAttrib4ubv);
 RESOLVE(glVertexAttrib4usv);
 RESOLVE(glVertexAttrib4uiv);
 RESOLVE(glVertexAttrib4Nub);
 RESOLVE(glVertexAttrib4Nbv);
 RESOLVE(glVertexAttrib4Nsv);
 RESOLVE(glVertexAttrib4Niv);
 RESOLVE(glVertexAttrib4Nubv);
 RESOLVE(glVertexAttrib4Nusv);
 RESOLVE(glVertexAttrib4Nuiv);
 RESOLVE(glVertexAttribPointer);
 RESOLVE(glEnableVertexAttribArray);
 RESOLVE(glDisableVertexAttribArray);
 RESOLVE(glBindAttribLocation);
 RESOLVE(glGetActiveAttrib);
 RESOLVE(glGetAttribLocation);
 RESOLVE(glGetVertexAttribdv);
 RESOLVE(glGetVertexAttribfv);
 RESOLVE(glGetVertexAttribiv);
 RESOLVE(glGetVertexAttribPointerv);
 RESOLVE(glBlendEquationSeparate);

 return true;
}






// ARB_multitexture
extern "C" {
	_glMultiTexCoord1dARB bo_glMultiTexCoord1dARB;
	_glMultiTexCoord1dvARB bo_glMultiTexCoord1dvARB;
	_glMultiTexCoord1fARB bo_glMultiTexCoord1fARB;
	_glMultiTexCoord1fvARB bo_glMultiTexCoord1fvARB;
	_glMultiTexCoord1iARB bo_glMultiTexCoord1iARB;
	_glMultiTexCoord1ivARB bo_glMultiTexCoord1ivARB;
	_glMultiTexCoord1sARB bo_glMultiTexCoord1sARB;
	_glMultiTexCoord1svARB bo_glMultiTexCoord1svARB;
	_glMultiTexCoord2dARB bo_glMultiTexCoord2dARB;
	_glMultiTexCoord2dvARB bo_glMultiTexCoord2dvARB;
	_glMultiTexCoord2fARB bo_glMultiTexCoord2fARB;
	_glMultiTexCoord2fvARB bo_glMultiTexCoord2fvARB;
	_glMultiTexCoord2iARB bo_glMultiTexCoord2iARB;
	_glMultiTexCoord2ivARB bo_glMultiTexCoord2ivARB;
	_glMultiTexCoord2sARB bo_glMultiTexCoord2sARB;
	_glMultiTexCoord2svARB bo_glMultiTexCoord2svARB;
	_glMultiTexCoord3dARB bo_glMultiTexCoord3dARB;
	_glMultiTexCoord3dvARB bo_glMultiTexCoord3dvARB;
	_glMultiTexCoord3fARB bo_glMultiTexCoord3fARB;
	_glMultiTexCoord3fvARB bo_glMultiTexCoord3fvARB;
	_glMultiTexCoord3iARB bo_glMultiTexCoord3iARB;
	_glMultiTexCoord3ivARB bo_glMultiTexCoord3ivARB;
	_glMultiTexCoord3sARB bo_glMultiTexCoord3sARB;
	_glMultiTexCoord3svARB bo_glMultiTexCoord3svARB;
	_glMultiTexCoord4dARB bo_glMultiTexCoord4dARB;
	_glMultiTexCoord4dvARB bo_glMultiTexCoord4dvARB;
	_glMultiTexCoord4fARB bo_glMultiTexCoord4fARB;
	_glMultiTexCoord4fvARB bo_glMultiTexCoord4fvARB;
	_glMultiTexCoord4iARB bo_glMultiTexCoord4iARB;
	_glMultiTexCoord4ivARB bo_glMultiTexCoord4ivARB;
	_glMultiTexCoord4sARB bo_glMultiTexCoord4sARB;
	_glMultiTexCoord4svARB bo_glMultiTexCoord4svARB;
	_glClientActiveTextureARB bo_glClientActiveTextureARB;
	_glActiveTextureARB bo_glActiveTextureARB;
}; // extern "C"

bool boglResolveARB_multitexture_Symbols()
{
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord1dARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord1dvARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord1fARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord1fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord1iARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord1ivARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord1sARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord1svARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord2dARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord2dvARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord2fARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord2fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord2iARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord2ivARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord2sARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord2svARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord3dARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord3dvARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord3fARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord3fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord3iARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord3ivARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord3sARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord3svARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord4dARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord4dvARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord4fARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord4fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord4iARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord4ivARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord4sARB);
 RESOLVE_GL_SYMBOL_CHECK(glMultiTexCoord4svARB);
 RESOLVE_GL_SYMBOL_CHECK(glClientActiveTextureARB);
 RESOLVE_GL_SYMBOL_CHECK(glActiveTextureARB);

 ASSIGN_FROM_EXT(glMultiTexCoord1d, glMultiTexCoord1dARB);
 ASSIGN_FROM_EXT(glMultiTexCoord1dv, glMultiTexCoord1dvARB);
 ASSIGN_FROM_EXT(glMultiTexCoord1f, glMultiTexCoord1fARB);
 ASSIGN_FROM_EXT(glMultiTexCoord1fv, glMultiTexCoord1fvARB);
 ASSIGN_FROM_EXT(glMultiTexCoord1i, glMultiTexCoord1iARB);
 ASSIGN_FROM_EXT(glMultiTexCoord1iv, glMultiTexCoord1ivARB);
 ASSIGN_FROM_EXT(glMultiTexCoord1s, glMultiTexCoord1sARB);
 ASSIGN_FROM_EXT(glMultiTexCoord1sv, glMultiTexCoord1svARB);
 ASSIGN_FROM_EXT(glMultiTexCoord2d, glMultiTexCoord2dARB);
 ASSIGN_FROM_EXT(glMultiTexCoord2dv, glMultiTexCoord2dvARB);
 ASSIGN_FROM_EXT(glMultiTexCoord2f, glMultiTexCoord2fARB);
 ASSIGN_FROM_EXT(glMultiTexCoord2fv, glMultiTexCoord2fvARB);
 ASSIGN_FROM_EXT(glMultiTexCoord2i, glMultiTexCoord2iARB);
 ASSIGN_FROM_EXT(glMultiTexCoord2iv, glMultiTexCoord2ivARB);
 ASSIGN_FROM_EXT(glMultiTexCoord2s, glMultiTexCoord2sARB);
 ASSIGN_FROM_EXT(glMultiTexCoord2sv, glMultiTexCoord2svARB);
 ASSIGN_FROM_EXT(glMultiTexCoord3d, glMultiTexCoord3dARB);
 ASSIGN_FROM_EXT(glMultiTexCoord3dv, glMultiTexCoord3dvARB);
 ASSIGN_FROM_EXT(glMultiTexCoord3f, glMultiTexCoord3fARB);
 ASSIGN_FROM_EXT(glMultiTexCoord3fv, glMultiTexCoord3fvARB);
 ASSIGN_FROM_EXT(glMultiTexCoord3i, glMultiTexCoord3iARB);
 ASSIGN_FROM_EXT(glMultiTexCoord3iv, glMultiTexCoord3ivARB);
 ASSIGN_FROM_EXT(glMultiTexCoord3s, glMultiTexCoord3sARB);
 ASSIGN_FROM_EXT(glMultiTexCoord3sv, glMultiTexCoord3svARB);
 ASSIGN_FROM_EXT(glMultiTexCoord4d, glMultiTexCoord4dARB);
 ASSIGN_FROM_EXT(glMultiTexCoord4dv, glMultiTexCoord4dvARB);
 ASSIGN_FROM_EXT(glMultiTexCoord4f, glMultiTexCoord4fARB);
 ASSIGN_FROM_EXT(glMultiTexCoord4fv, glMultiTexCoord4fvARB);
 ASSIGN_FROM_EXT(glMultiTexCoord4i, glMultiTexCoord4iARB);
 ASSIGN_FROM_EXT(glMultiTexCoord4iv, glMultiTexCoord4ivARB);
 ASSIGN_FROM_EXT(glMultiTexCoord4s, glMultiTexCoord4sARB);
 ASSIGN_FROM_EXT(glMultiTexCoord4sv, glMultiTexCoord4svARB);
 ASSIGN_FROM_EXT(glClientActiveTexture, glClientActiveTextureARB);
 ASSIGN_FROM_EXT(glActiveTexture, glActiveTextureARB);

 return true;
}


// EXT_blend_color
extern "C" {
	_glBlendColorEXT bo_glBlendColorEXT;
}; // extern "C"

bool boglResolveEXT_blend_color_Symbols()
{
 RESOLVE_GL_SYMBOL_CHECK(glBlendColorEXT);

 if (bo_glBlendColor == 0) {
	bo_glBlendColor = bo_glBlendColorEXT;
 }
 return true;
}



// EXT_polygon_offset
extern "C" {
	_glPolygonOffsetEXT bo_glPolygonOffsetEXT;
}; // extern "C"

bool boglResolveEXT_polygon_offset_Symbols()
{
 RESOLVE_GL_SYMBOL_CHECK(glPolygonOffsetEXT);
 return true;
}


// EXT_texture3d
extern "C" {
	_glTexImage3DEXT bo_glTexImage3DEXT;
	_glTexSubImage3DEXT bo_glTexSubImage3DEXT;
	_glCopyTexSubImage3DEXT bo_glCopyTexSubImage3DEXT;
}; // extern "C"

bool boglResolveEXT_texture3d_Symbols()
{
 RESOLVE_GL_SYMBOL_CHECK(glTexImage3DEXT);
 RESOLVE_GL_SYMBOL_CHECK(glTexSubImage3DEXT);
 RESOLVE_GL_SYMBOL_CHECK(glCopyTexSubImage3DEXT);
 return true;
}


// GL_ARB_vertex_buffer_object
extern "C" {
	_glBindBufferARB bo_glBindBufferARB;
	_glDeleteBuffersARB bo_glDeleteBuffersARB;
	_glGenBuffersARB bo_glGenBuffersARB;
	_glIsBufferARB bo_glIsBufferARB;
	_glBufferDataARB bo_glBufferDataARB;
	_glBufferSubDataARB bo_glBufferSubDataARB;
	_glMapBufferARB bo_glMapBufferARB;
	_glUnmapBufferARB bo_glUnmapBufferARB;
	_glGetBufferParameterivARB bo_glGetBufferParameterivARB;
	_glGetBufferPointervARB bo_glGetBufferPointervARB;
}; // extern "C"

bool boglResolveARB_vertex_buffer_object_Symbols()
{
 RESOLVE_GL_SYMBOL_CHECK(glBindBufferARB);
 RESOLVE_GL_SYMBOL_CHECK(glDeleteBuffersARB);
 RESOLVE_GL_SYMBOL_CHECK(glGenBuffersARB);
 RESOLVE_GL_SYMBOL_CHECK(glIsBufferARB);
 RESOLVE_GL_SYMBOL_CHECK(glBufferDataARB);
 RESOLVE_GL_SYMBOL_CHECK(glBufferSubDataARB);
 RESOLVE_GL_SYMBOL_CHECK(glMapBufferARB);
 RESOLVE_GL_SYMBOL_CHECK(glUnmapBufferARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetBufferParameterivARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetBufferPointervARB);

 ASSIGN_FROM_EXT(glBindBuffer, glBindBufferARB);
 ASSIGN_FROM_EXT(glDeleteBuffers, glDeleteBuffersARB);
 ASSIGN_FROM_EXT(glGenBuffers, glGenBuffersARB);
 ASSIGN_FROM_EXT(glIsBuffer, glIsBufferARB);
 ASSIGN_FROM_EXT(glBufferData, glBufferDataARB);
 ASSIGN_FROM_EXT(glBufferSubData, glBufferSubDataARB);
 ASSIGN_FROM_EXT(glMapBuffer, glMapBufferARB);
 ASSIGN_FROM_EXT(glUnmapBuffer, glUnmapBufferARB);
 ASSIGN_FROM_EXT(glGetBufferParameteriv, glGetBufferParameterivARB);
 ASSIGN_FROM_EXT(glGetBufferPointerv, glGetBufferPointervARB);

 return true;
}


// GL_ARB_shader_objects
extern "C" {
	_glDeleteObjectARB bo_glDeleteObjectARB;
	_glGetHandleARB bo_glGetHandleARB;
	_glDetachObjectARB bo_glDetachObjectARB;
	_glCreateShaderObjectARB bo_glCreateShaderObjectARB;
	_glShaderSourceARB bo_glShaderSourceARB;
	_glCompileShaderARB bo_glCompileShaderARB;
	_glCreateProgramObjectARB bo_glCreateProgramObjectARB;
	_glAttachObjectARB bo_glAttachObjectARB;
	_glLinkProgramARB bo_glLinkProgramARB;
	_glUseProgramObjectARB bo_glUseProgramObjectARB;
	_glValidateProgramARB bo_glValidateProgramARB;
	_glUniform1fARB bo_glUniform1fARB;
	_glUniform2fARB bo_glUniform2fARB;
	_glUniform3fARB bo_glUniform3fARB;
	_glUniform4fARB bo_glUniform4fARB;
	_glUniform1iARB bo_glUniform1iARB;
	_glUniform2iARB bo_glUniform2iARB;
	_glUniform3iARB bo_glUniform3iARB;
	_glUniform4iARB bo_glUniform4iARB;
	_glUniform1fvARB bo_glUniform1fvARB;
	_glUniform2fvARB bo_glUniform2fvARB;
	_glUniform3fvARB bo_glUniform3fvARB;
	_glUniform4fvARB bo_glUniform4fvARB;
	_glUniform1ivARB bo_glUniform1ivARB;
	_glUniform2ivARB bo_glUniform2ivARB;
	_glUniform3ivARB bo_glUniform3ivARB;
	_glUniform4ivARB bo_glUniform4ivARB;
	_glUniformMatrix2fvARB bo_glUniformMatrix2fvARB;
	_glUniformMatrix3fvARB bo_glUniformMatrix3fvARB;
	_glUniformMatrix4fvARB bo_glUniformMatrix4fvARB;
	_glGetObjectParameterfvARB bo_glGetObjectParameterfvARB;
	_glGetObjectParameterivARB bo_glGetObjectParameterivARB;
	_glGetInfoLogARB bo_glGetInfoLogARB;
	_glGetAttachedObjectsARB bo_glGetAttachedObjectsARB;
	_glGetUniformLocationARB bo_glGetUniformLocationARB;
	_glGetActiveUniformARB bo_glGetActiveUniformARB;
	_glGetUniformfvARB bo_glGetUniformfvARB;
	_glGetUniformivARB bo_glGetUniformivARB;
	_glGetShaderSourceARB bo_glGetShaderSourceARB;
}; // "C"

bool boglResolveARB_shader_objects_Symbols()
{
 RESOLVE_GL_SYMBOL_CHECK(glDeleteObjectARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetHandleARB);
 RESOLVE_GL_SYMBOL_CHECK(glDetachObjectARB);
 RESOLVE_GL_SYMBOL_CHECK(glCreateShaderObjectARB);
 RESOLVE_GL_SYMBOL_CHECK(glShaderSourceARB);
 RESOLVE_GL_SYMBOL_CHECK(glCompileShaderARB);
 RESOLVE_GL_SYMBOL_CHECK(glCreateProgramObjectARB);
 RESOLVE_GL_SYMBOL_CHECK(glAttachObjectARB);
 RESOLVE_GL_SYMBOL_CHECK(glLinkProgramARB);
 RESOLVE_GL_SYMBOL_CHECK(glUseProgramObjectARB);
 RESOLVE_GL_SYMBOL_CHECK(glValidateProgramARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform1fARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform2fARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform3fARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform4fARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform1iARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform2iARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform3iARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform4iARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform1fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform2fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform3fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform4fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform1ivARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform2ivARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform3ivARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniform4ivARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniformMatrix2fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniformMatrix3fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glUniformMatrix4fvARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetObjectParameterfvARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetObjectParameterivARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetInfoLogARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetAttachedObjectsARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetUniformLocationARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetActiveUniformARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetUniformfvARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetUniformivARB);
 RESOLVE_GL_SYMBOL_CHECK(glGetShaderSourceARB);

 ASSIGN_FROM_EXT(glDeleteProgram, glDeleteObjectARB);
 ASSIGN_FROM_EXT(glDeleteShader, glDeleteObjectARB);
// ASSIGN_FROM_EXT(glGetHandleARB, glGetHandleARB);
 ASSIGN_FROM_EXT(glDetachShader, glDetachObjectARB);
 ASSIGN_FROM_EXT(glCreateShader, glCreateShaderObjectARB);
 ASSIGN_FROM_EXT(glShaderSource, glShaderSourceARB);
 ASSIGN_FROM_EXT(glCompileShader, glCompileShaderARB);
 ASSIGN_FROM_EXT(glCreateProgram, glCreateProgramObjectARB);
 ASSIGN_FROM_EXT(glAttachShader, glAttachObjectARB);
 ASSIGN_FROM_EXT(glLinkProgram, glLinkProgramARB);
 ASSIGN_FROM_EXT(glUseProgram, glUseProgramObjectARB);
 ASSIGN_FROM_EXT(glValidateProgram, glValidateProgramARB);
 ASSIGN_FROM_EXT(glUniform1f, glUniform1fARB);
 ASSIGN_FROM_EXT(glUniform2f, glUniform2fARB);
 ASSIGN_FROM_EXT(glUniform3f, glUniform3fARB);
 ASSIGN_FROM_EXT(glUniform4f, glUniform4fARB);
 ASSIGN_FROM_EXT(glUniform1i, glUniform1iARB);
 ASSIGN_FROM_EXT(glUniform2i, glUniform2iARB);
 ASSIGN_FROM_EXT(glUniform3i, glUniform3iARB);
 ASSIGN_FROM_EXT(glUniform4i, glUniform4iARB);
 ASSIGN_FROM_EXT(glUniform1fv, glUniform1fvARB);
 ASSIGN_FROM_EXT(glUniform2fv, glUniform2fvARB);
 ASSIGN_FROM_EXT(glUniform3fv, glUniform3fvARB);
 ASSIGN_FROM_EXT(glUniform4fv, glUniform4fvARB);
 ASSIGN_FROM_EXT(glUniform1iv, glUniform1ivARB);
 ASSIGN_FROM_EXT(glUniform2iv, glUniform2ivARB);
 ASSIGN_FROM_EXT(glUniform3iv, glUniform3ivARB);
 ASSIGN_FROM_EXT(glUniform4iv, glUniform4ivARB);
 ASSIGN_FROM_EXT(glUniformMatrix2fv, glUniformMatrix2fvARB);
 ASSIGN_FROM_EXT(glUniformMatrix3fvARB, glUniformMatrix3fvARB);
 ASSIGN_FROM_EXT(glUniformMatrix4fvARB, glUniformMatrix4fvARB);
// ASSIGN_FROM_EXT(glGetObjectParameterfvARB, glGetObjectParameterfvARB);
 ASSIGN_FROM_EXT(glGetProgramiv, glGetObjectParameterivARB);
 ASSIGN_FROM_EXT(glGetShaderiv, glGetObjectParameterivARB);
 ASSIGN_FROM_EXT(glGetShaderInfoLog, glGetInfoLogARB);
 ASSIGN_FROM_EXT(glGetProgramInfoLog, glGetInfoLogARB);
 ASSIGN_FROM_EXT(glGetAttachedShaders, glGetAttachedObjectsARB);
 ASSIGN_FROM_EXT(glGetUniformLocation, glGetUniformLocationARB);
 ASSIGN_FROM_EXT(glGetActiveUniform, glGetActiveUniformARB);
 ASSIGN_FROM_EXT(glGetUniformfv, glGetUniformfvARB);
 ASSIGN_FROM_EXT(glGetUniformiv, glGetUniformivARB);
 ASSIGN_FROM_EXT(glGetShaderSource, glGetShaderSourceARB);

 // AB: note: there appers to be no equivalent to glIsShader() in the ARB
 //           extension
 // AB: note: there appers to be no equivalent to glIsProgram() in the ARB
 //           extension

 if (bo_glIsShader == 0) {
	boWarning() << k_funcinfo << "have no glIsShader() function in the ARB extension. dont use that function" << endl;
 }
 if (bo_glIsProgram == 0) {
	boWarning() << k_funcinfo << "have no glIsProgram() function in the ARB extension. dont use that function" << endl;
 }


 return true;
}


// GL_EXT_framebuffer_object
extern "C" {
	_glIsRenderbufferEXT bo_glIsRenderbufferEXT;
	_glBindRenderbufferEXT bo_glBindRenderbufferEXT;
	_glDeleteRenderbuffersEXT bo_glDeleteRenderbuffersEXT;
	_glGenRenderbuffersEXT bo_glGenRenderbuffersEXT;
	_glRenderbufferStorageEXT bo_glRenderbufferStorageEXT;
	_glGetRenderbufferParameterivEXT bo_glGetRenderbufferParameterivEXT;
	_glIsFramebufferEXT bo_glIsFramebufferEXT;
	_glBindFramebufferEXT bo_glBindFramebufferEXT;
	_glDeleteFramebuffersEXT bo_glDeleteFramebuffersEXT;
	_glGenFramebuffersEXT bo_glGenFramebuffersEXT;
	_glCheckFramebufferStatusEXT bo_glCheckFramebufferStatusEXT;
	_glFramebufferTexture1DEXT bo_glFramebufferTexture1DEXT;
	_glFramebufferTexture2DEXT bo_glFramebufferTexture2DEXT;
	_glFramebufferTexture3DEXT bo_glFramebufferTexture3DEXT;
	_glFramebufferRenderbufferEXT bo_glFramebufferRenderbufferEXT;
	_glGetFramebufferAttachmentParameterivEXT bo_glGetFramebufferAttachmentParameterivEXT;
	_glGenerateMipmapEXT bo_glGenerateMipmapEXT;
}; // "C"

bool boglResolveEXT_framebuffer_object_Symbols()
{
 RESOLVE_GL_SYMBOL_CHECK(glIsRenderbufferEXT);
 RESOLVE_GL_SYMBOL_CHECK(glBindRenderbufferEXT);
 RESOLVE_GL_SYMBOL_CHECK(glDeleteRenderbuffersEXT);
 RESOLVE_GL_SYMBOL_CHECK(glGenRenderbuffersEXT);
 RESOLVE_GL_SYMBOL_CHECK(glRenderbufferStorageEXT);
 RESOLVE_GL_SYMBOL_CHECK(glGetRenderbufferParameterivEXT);
 RESOLVE_GL_SYMBOL_CHECK(glIsFramebufferEXT);
 RESOLVE_GL_SYMBOL_CHECK(glBindFramebufferEXT);
 RESOLVE_GL_SYMBOL_CHECK(glDeleteFramebuffersEXT);
 RESOLVE_GL_SYMBOL_CHECK(glGenFramebuffersEXT);
 RESOLVE_GL_SYMBOL_CHECK(glCheckFramebufferStatusEXT);
 RESOLVE_GL_SYMBOL_CHECK(glFramebufferTexture1DEXT);
 RESOLVE_GL_SYMBOL_CHECK(glFramebufferTexture2DEXT);
 RESOLVE_GL_SYMBOL_CHECK(glFramebufferTexture3DEXT);
 RESOLVE_GL_SYMBOL_CHECK(glFramebufferRenderbufferEXT);
 RESOLVE_GL_SYMBOL_CHECK(glGetFramebufferAttachmentParameterivEXT);
 RESOLVE_GL_SYMBOL_CHECK(glGenerateMipmapEXT);

 return true;
}

