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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#define GLX_GLXEXT_PROTOTYPES

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



static bool boglResolveGLUSymbols(MyQLibrary* gl);

// from gl/bogl_resolve_symbols.cpp
extern bool bogl_resolveSymbols_GL();
extern bool bogl_resolveSymbols_GLU();

bool boglResolveLibGLSymbols(MyQLibrary* gl_)
{
 // TODO: GLU functions must be resolved manually

 // AB: we use glXGetProcAddressARB() to resolve all other GL and GLX functions,
 //     so we must resolve this one manually before doing anything else.
 //     sidenote: glXGetProcAddressARB() is part of the Linux OpenGL ABI,
 //     glXGetProcAddress() is not, so we can be sure glXGetProcAddressARB() is
 //     always present.
#if BOGL_DO_DLOPEN
 if (!gl_) {
	boError() << k_funcinfo << "NULL library provided (internal error)" << endl;
	return false;
 }
 MyQLibrary& gl = *gl_;
 RESOLVE(glXGetProcAddressARB);
#else // BOGL_DO_DLOPEN
#undef glXGetProcAddressARB
 bo_glXGetProcAddressARB = (_glXGetProcAddressARB)glXGetProcAddressARB((const GLubyte*)"glXGetProcAddressARB");
#endif // BOGL_DO_DLOPEN

 // resolve GL, GL extension and GLX functions
 if (!bogl_resolveSymbols_GL()) {
	return false;
 }

 // resolve GLU functions
 if (!boglResolveGLUSymbols(gl_)) {
	return false;
 }

 return true;
}

bool boglResolveLibGLUSymbols(MyQLibrary* gl)
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

// TODO: we could let boglc (the bogl compiler) compile a .cpp file for GLU
//       functions, i.e. one that contains
//       a) the definitions of the function pointers (as above in the "extern "C""
//           part)
//       b) resolve code for dlopen()
//       c) resolve code for non-dlopen()
//
//       however since GLU is very unlikely to change, I think it is totally
//       sufficient to have that code here, even though it is inconsistent with
//       the GL/GLX functions.
bool boglResolveGLUSymbols(MyQLibrary* gl_)
{
#if BOGL_DO_DLOPEN
 if (!gl_) {
	boError() << k_funcinfo << "NULL library provided (internal error)" << endl;
	return false;
 }
 MyQLibrary& gl = *gl_;

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
#else // BOGL_DO_DLOPEN
 Q_UNUSED(gl_);

 // first revert the "#define gluXXX bo_gluXXX" from bogl.h
 #undef gluBeginCurve
 #undef gluBeginPolygon
 #undef gluBeginSurface
 #undef gluBeginTrim
 #undef gluBuild1DMipmapLevels
 #undef gluBuild1DMipmaps
 #undef gluBuild2DMipmapLevels
 #undef gluBuild2DMipmaps
 #undef gluBuild3DMipmapLevels
 #undef gluBuild3DMipmaps
 #undef gluCheckExtension
 #undef gluCylinder
 #undef gluDeleteNurbsRenderer
 #undef gluDeleteQuadric
 #undef gluDeleteTess
 #undef gluDisk
 #undef gluEndCurve
 #undef gluEndPolygon
 #undef gluEndSurface
 #undef gluEndTrim
 #undef gluErrorString
 #undef gluGetNurbsProperty
 #undef gluGetString
 #undef gluGetTessProperty
 #undef gluLoadSamplingMatrices
 #undef gluLookAt
 #undef gluNewNurbsRenderer
 #undef gluNewQuadric
 #undef gluNewTess
 #undef gluNextContour
 #undef gluNurbsCallback
 #undef gluNurbsCallbackData
 #undef gluNurbsCallbackDataEXT
 #undef gluNurbsCurve
 #undef gluNurbsProperty
 #undef gluNurbsSurface
 #undef gluOrtho2D
 #undef gluPartialDisk
 #undef gluPerspective
 #undef gluPickMatrix
 #undef gluProject
 #undef gluPwlCurve
 #undef gluQuadricCallback
 #undef gluQuadricDrawStyle
 #undef gluQuadricNormals
 #undef gluQuadricOrientation
 #undef gluQuadricTexture
 #undef gluScaleImage
 #undef gluSphere
 #undef gluTessBeginContour
 #undef gluTessBeginPolygon
 #undef gluTessCallback
 #undef gluTessEndContour
 #undef gluTessEndPolygon
 #undef gluTessNormal
 #undef gluTessProperty
 #undef gluTessVertex
 #undef gluUnProject
 #undef gluUnProject4

#define RESOLVE_DUMMY(a) \
	bo_##a = a
 RESOLVE_DUMMY(gluBeginCurve);
 RESOLVE_DUMMY(gluBeginPolygon);
 RESOLVE_DUMMY(gluBeginSurface);
 RESOLVE_DUMMY(gluBeginTrim);
 RESOLVE_DUMMY(gluBuild1DMipmapLevels);
 RESOLVE_DUMMY(gluBuild1DMipmaps);
 RESOLVE_DUMMY(gluBuild2DMipmapLevels);
 RESOLVE_DUMMY(gluBuild2DMipmaps);
 RESOLVE_DUMMY(gluBuild3DMipmapLevels);
 RESOLVE_DUMMY(gluBuild3DMipmaps);
 RESOLVE_DUMMY(gluCheckExtension);
 RESOLVE_DUMMY(gluCylinder);
 RESOLVE_DUMMY(gluDeleteNurbsRenderer);
 RESOLVE_DUMMY(gluDeleteQuadric);
 RESOLVE_DUMMY(gluDeleteTess);
 RESOLVE_DUMMY(gluDisk);
 RESOLVE_DUMMY(gluEndCurve);
 RESOLVE_DUMMY(gluEndPolygon);
 RESOLVE_DUMMY(gluEndSurface);
 RESOLVE_DUMMY(gluEndTrim);
 RESOLVE_DUMMY(gluErrorString);
 RESOLVE_DUMMY(gluGetNurbsProperty);
 RESOLVE_DUMMY(gluGetString);
 RESOLVE_DUMMY(gluGetTessProperty);
 RESOLVE_DUMMY(gluLoadSamplingMatrices);
 RESOLVE_DUMMY(gluLookAt);
 RESOLVE_DUMMY(gluNewNurbsRenderer);
 RESOLVE_DUMMY(gluNewQuadric);
 RESOLVE_DUMMY(gluNewTess);
 RESOLVE_DUMMY(gluNextContour);
 RESOLVE_DUMMY(gluNurbsCallback);
 RESOLVE_DUMMY(gluNurbsCallbackData);
 RESOLVE_DUMMY(gluNurbsCallbackDataEXT);
 RESOLVE_DUMMY(gluNurbsCurve);
 RESOLVE_DUMMY(gluNurbsProperty);
 RESOLVE_DUMMY(gluNurbsSurface);
 RESOLVE_DUMMY(gluOrtho2D);
 RESOLVE_DUMMY(gluPartialDisk);
 RESOLVE_DUMMY(gluPerspective);
 RESOLVE_DUMMY(gluPickMatrix);
 RESOLVE_DUMMY(gluProject);
 RESOLVE_DUMMY(gluPwlCurve);
 RESOLVE_DUMMY(gluQuadricCallback);
 RESOLVE_DUMMY(gluQuadricDrawStyle);
 RESOLVE_DUMMY(gluQuadricNormals);
 RESOLVE_DUMMY(gluQuadricOrientation);
 RESOLVE_DUMMY(gluQuadricTexture);
 RESOLVE_DUMMY(gluScaleImage);
 RESOLVE_DUMMY(gluSphere);
 RESOLVE_DUMMY(gluTessBeginContour);
 RESOLVE_DUMMY(gluTessBeginPolygon);
 RESOLVE_DUMMY(gluTessCallback);
 RESOLVE_DUMMY(gluTessEndContour);
 RESOLVE_DUMMY(gluTessEndPolygon);
 RESOLVE_DUMMY(gluTessNormal);
 RESOLVE_DUMMY(gluTessProperty);
 RESOLVE_DUMMY(gluTessVertex);
 RESOLVE_DUMMY(gluUnProject);
 RESOLVE_DUMMY(gluUnProject4);
#undef RESOLVE_DUMMY
#endif // BOGL_DO_DLOPEN
 return true;
}


