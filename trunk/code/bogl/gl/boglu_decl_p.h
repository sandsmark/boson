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

#ifndef BOGLU_DECL_P_H
#define BOGLU_DECL_P_H

// AB: I won't separate this file up into GLU versions.
//     that's a lot of work and I don't think it's so important for GLU.


#ifndef BOGL_H
#error Never include this file directly! Include bogl.h instead!
#endif

extern "C" {
	// GLU typedefs
	typedef void (*_gluBeginCurve)(GLUnurbs* nurb);
	typedef void (*_gluBeginPolygon)(GLUtesselator* tess);
	typedef void (*_gluBeginSurface)(GLUnurbs* nurb);
	typedef void (*_gluBeginTrim)(GLUnurbs* nurb);
	typedef GLint (*_gluBuild1DMipmapLevels)(GLenum target, GLint internalFormat, GLsizei width, GLenum format, GLenum type, GLint level, GLint base, GLint max, const void* data);
	typedef GLint (*_gluBuild1DMipmaps)(GLenum target, GLint internalFormat, GLsizei width, GLenum format, GLenum type, const void *data);
	typedef GLint (*_gluBuild2DMipmapLevels)(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint level, GLint base, GLint max, const void *data);
	typedef GLint (*_gluBuild2DMipmaps)(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data);
	typedef GLint (*_gluBuild3DMipmapLevels)(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLint level, GLint base, GLint max, const void *data);
	typedef GLint (*_gluBuild3DMipmaps)(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
	typedef GLboolean (*_gluCheckExtension)(const GLubyte *extName, const GLubyte *extString);
	typedef void (*_gluCylinder)(GLUquadric* quad, GLdouble base, GLdouble top, GLdouble height, GLint slices, GLint stacks);
	typedef void (*_gluDeleteNurbsRenderer)(GLUnurbs* nurb);
	typedef void (*_gluDeleteQuadric)(GLUquadric* quad);
	typedef void (*_gluDeleteTess)(GLUtesselator* tess);
	typedef void (*_gluDisk)(GLUquadric* quad, GLdouble inner, GLdouble outer, GLint slices, GLint loops);
	typedef void (*_gluEndCurve)(GLUnurbs* nurb);
	typedef void (*_gluEndPolygon)(GLUtesselator* tess);
	typedef void (*_gluEndSurface)(GLUnurbs* nurb);
	typedef void (*_gluEndTrim)(GLUnurbs* nurb);
	typedef const GLubyte* (*_gluErrorString)(GLenum error);
	typedef void (*_gluGetNurbsProperty)(GLUnurbs* nurb, GLenum property, GLfloat* data);
	typedef const GLubyte* (*_gluGetString)(GLenum name);
	typedef void (*_gluGetTessProperty)(GLUtesselator* tess, GLenum which, GLdouble* data);
	typedef void (*_gluLoadSamplingMatrices)(GLUnurbs* nurb, const GLfloat *model, const GLfloat *perspective, const GLint *view);
	typedef void (*_gluLookAt)(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ);
	typedef GLUnurbs* (*_gluNewNurbsRenderer)();
	typedef GLUquadric* (*_gluNewQuadric)();
	typedef GLUtesselator* (*_gluNewTess)();
	typedef void (*_gluNextContour)(GLUtesselator* tess, GLenum type);
	typedef void (*_gluNurbsCallback)(GLUnurbs* nurb, GLenum which, _GLUfuncptr CallBackFunc);
	typedef void (*_gluNurbsCallbackData)(GLUnurbs* nurb, GLvoid* userData);
	typedef void (*_gluNurbsCallbackDataEXT)(GLUnurbs* nurb, GLvoid* userData);
	typedef void (*_gluNurbsCurve)(GLUnurbs* nurb, GLint knotCount, GLfloat *knots, GLint stride, GLfloat *control, GLint order, GLenum type);
	typedef void (*_gluNurbsProperty)(GLUnurbs* nurb, GLenum property, GLfloat value);
	typedef void (*_gluNurbsSurface)(GLUnurbs* nurb, GLint sKnotCount, GLfloat* sKnots, GLint tKnotCount, GLfloat* tKnots, GLint sStride, GLint tStride, GLfloat* control, GLint sOrder, GLint tOrder, GLenum type);
	typedef void (*_gluOrtho2D)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top);
	typedef void (*_gluPartialDisk)(GLUquadric* quad, GLdouble inner, GLdouble outer, GLint slices, GLint loops, GLdouble start, GLdouble sweep);
	typedef void (*_gluPerspective)(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
	typedef void (*_gluPickMatrix)(GLdouble x, GLdouble y, GLdouble delX, GLdouble delY, GLint *viewport);
	typedef GLint (*_gluProject)(GLdouble objX, GLdouble objY, GLdouble objZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* winX, GLdouble* winY, GLdouble* winZ);
	typedef void (*_gluPwlCurve)(GLUnurbs* nurb, GLint count, GLfloat* data, GLint stride, GLenum type);
	typedef void (*_gluQuadricCallback)(GLUquadric* quad, GLenum which, _GLUfuncptr CallBackFunc);
	typedef void (*_gluQuadricDrawStyle)(GLUquadric* quad, GLenum draw);
	typedef void (*_gluQuadricNormals)(GLUquadric* quad, GLenum normal);
	typedef void (*_gluQuadricOrientation)(GLUquadric* quad, GLenum orientation);
	typedef void (*_gluQuadricTexture)(GLUquadric* quad, GLboolean texture);
	typedef GLint (*_gluScaleImage)(GLenum format, GLsizei wIn, GLsizei hIn, GLenum typeIn, const void *dataIn, GLsizei wOut, GLsizei hOut, GLenum typeOut, GLvoid* dataOut);
	typedef void (*_gluSphere)(GLUquadric* quad, GLdouble radius, GLint slices, GLint stacks);
	typedef void (*_gluTessBeginContour)(GLUtesselator* tess);
	typedef void (*_gluTessBeginPolygon)(GLUtesselator* tess, GLvoid* data);
	typedef void (*_gluTessCallback)(GLUtesselator* tess, GLenum which, _GLUfuncptr CallBackFunc);
	typedef void (*_gluTessEndContour)(GLUtesselator* tess);
	typedef void (*_gluTessEndPolygon)(GLUtesselator* tess);
	typedef void (*_gluTessNormal)(GLUtesselator* tess, GLdouble valueX, GLdouble valueY, GLdouble valueZ);
	typedef void (*_gluTessProperty)(GLUtesselator* tess, GLenum which, GLdouble data);
	typedef void (*_gluTessVertex)(GLUtesselator* tess, GLdouble *location, GLvoid* data);
	typedef GLint (*_gluUnProject)(GLdouble winX, GLdouble winY, GLdouble winZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* objX, GLdouble* objY, GLdouble* objZ);
	typedef GLint (*_gluUnProject4)(GLdouble winX, GLdouble winY, GLdouble winZ, GLdouble clipW, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble near, GLdouble far, GLdouble* objX, GLdouble* objY, GLdouble* objZ, GLdouble* objW);



// GLU function pointers
	extern _gluBeginCurve bo_gluBeginCurve;
	extern _gluBeginPolygon bo_gluBeginPolygon;
	extern _gluBeginSurface bo_gluBeginSurface;
	extern _gluBeginTrim bo_gluBeginTrim;
	extern _gluBuild1DMipmapLevels bo_gluBuild1DMipmapLevels;
	extern _gluBuild1DMipmaps bo_gluBuild1DMipmaps;
	extern _gluBuild2DMipmapLevels bo_gluBuild2DMipmapLevels;
	extern _gluBuild2DMipmaps bo_gluBuild2DMipmaps;
	extern _gluBuild3DMipmapLevels bo_gluBuild3DMipmapLevels;
	extern _gluBuild3DMipmaps bo_gluBuild3DMipmaps;
	extern _gluCheckExtension bo_gluCheckExtension;
	extern _gluCylinder bo_gluCylinder;
	extern _gluDeleteNurbsRenderer bo_gluDeleteNurbsRenderer;
	extern _gluDeleteQuadric bo_gluDeleteQuadric;
	extern _gluDeleteTess bo_gluDeleteTess;
	extern _gluDisk bo_gluDisk;
	extern _gluEndCurve bo_gluEndCurve;
	extern _gluEndPolygon bo_gluEndPolygon;
	extern _gluEndSurface bo_gluEndSurface;
	extern _gluEndTrim bo_gluEndTrim;
	extern _gluErrorString bo_gluErrorString;
	extern _gluGetNurbsProperty bo_gluGetNurbsProperty;
	extern _gluGetString bo_gluGetString;
	extern _gluGetTessProperty bo_gluGetTessProperty;
	extern _gluLoadSamplingMatrices bo_gluLoadSamplingMatrices;
	extern _gluLookAt bo_gluLookAt;
	extern _gluNewNurbsRenderer bo_gluNewNurbsRenderer;
	extern _gluNewQuadric bo_gluNewQuadric;
	extern _gluNewTess bo_gluNewTess;
	extern _gluNextContour bo_gluNextContour;
	extern _gluNurbsCallback bo_gluNurbsCallback;
	extern _gluNurbsCallbackData bo_gluNurbsCallbackData;
	extern _gluNurbsCallbackDataEXT bo_gluNurbsCallbackDataEXT;
	extern _gluNurbsCurve bo_gluNurbsCurve;
	extern _gluNurbsProperty bo_gluNurbsProperty;
	extern _gluNurbsSurface bo_gluNurbsSurface;
	extern _gluOrtho2D bo_gluOrtho2D;
	extern _gluPartialDisk bo_gluPartialDisk;
	extern _gluPerspective bo_gluPerspective;
	extern _gluPickMatrix bo_gluPickMatrix;
	extern _gluProject bo_gluProject;
	extern _gluPwlCurve bo_gluPwlCurve;
	extern _gluQuadricCallback bo_gluQuadricCallback;
	extern _gluQuadricDrawStyle bo_gluQuadricDrawStyle;
	extern _gluQuadricNormals bo_gluQuadricNormals;
	extern _gluQuadricOrientation bo_gluQuadricOrientation;
	extern _gluQuadricTexture bo_gluQuadricTexture;
	extern _gluScaleImage bo_gluScaleImage;
	extern _gluSphere bo_gluSphere;
	extern _gluTessBeginContour bo_gluTessBeginContour;
	extern _gluTessBeginPolygon bo_gluTessBeginPolygon;
	extern _gluTessCallback bo_gluTessCallback;
	extern _gluTessEndContour bo_gluTessEndContour;
	extern _gluTessEndPolygon bo_gluTessEndPolygon;
	extern _gluTessNormal bo_gluTessNormal;
	extern _gluTessProperty bo_gluTessProperty;
	extern _gluTessVertex bo_gluTessVertex;
	extern _gluUnProject bo_gluUnProject;
	extern _gluUnProject4 bo_gluUnProject4;
}; // extern "C"



// GLU defines
#if BOGL_DO_DLOPEN

#define gluBeginCurve bo_gluBeginCurve
#define gluBeginPolygon bo_gluBeginPolygon
#define gluBeginSurface bo_gluBeginSurface
#define gluBeginTrim bo_gluBeginTrim
#define gluBuild1DMipmapLevels bo_gluBuild1DMipmapLevels
#define gluBuild1DMipmaps bo_gluBuild1DMipmaps
#define gluBuild2DMipmapLevels bo_gluBuild2DMipmapLevels
#define gluBuild2DMipmaps bo_gluBuild2DMipmaps
#define gluBuild3DMipmapLevels bo_gluBuild3DMipmapLevels
#define gluBuild3DMipmaps bo_gluBuild3DMipmaps
#define gluCheckExtension bo_gluCheckExtension
#define gluCylinder bo_gluCylinder
#define gluDeleteNurbsRenderer bo_gluDeleteNurbsRenderer
#define gluDeleteQuadric bo_gluDeleteQuadric
#define gluDeleteTess bo_gluDeleteTess
#define gluDisk bo_gluDisk
#define gluEndCurve bo_gluEndCurve
#define gluEndPolygon bo_gluEndPolygon
#define gluEndSurface bo_gluEndSurface
#define gluEndTrim bo_gluEndTrim
#define gluErrorString bo_gluErrorString
#define gluGetNurbsProperty bo_gluGetNurbsProperty
#define gluGetString bo_gluGetString
#define gluGetTessProperty bo_gluGetTessProperty
#define gluLoadSamplingMatrices bo_gluLoadSamplingMatrices
#define gluLookAt bo_gluLookAt
#define gluNewNurbsRenderer bo_gluNewNurbsRenderer
#define gluNewQuadric bo_gluNewQuadric
#define gluNewTess bo_gluNewTess
#define gluNextContour bo_gluNextContour
#define gluNurbsCallback bo_gluNurbsCallback
#define gluNurbsCallbackData bo_gluNurbsCallbackData
#define gluNurbsCallbackDataEXT bo_gluNurbsCallbackDataEXT
#define gluNurbsCurve bo_gluNurbsCurve
#define gluNurbsProperty bo_gluNurbsProperty
#define gluNurbsSurface bo_gluNurbsSurface
#define gluOrtho2D bo_gluOrtho2D
#define gluPartialDisk bo_gluPartialDisk
#define gluPerspective bo_gluPerspective
#define gluPickMatrix bo_gluPickMatrix
#define gluProject bo_gluProject
#define gluPwlCurve bo_gluPwlCurve
#define gluQuadricCallback bo_gluQuadricCallback
#define gluQuadricDrawStyle bo_gluQuadricDrawStyle
#define gluQuadricNormals bo_gluQuadricNormals
#define gluQuadricOrientation bo_gluQuadricOrientation
#define gluQuadricTexture bo_gluQuadricTexture
#define gluScaleImage bo_gluScaleImage
#define gluSphere bo_gluSphere
#define gluTessBeginContour bo_gluTessBeginContour
#define gluTessBeginPolygon bo_gluTessBeginPolygon
#define gluTessCallback bo_gluTessCallback
#define gluTessEndContour bo_gluTessEndContour
#define gluTessEndPolygon bo_gluTessEndPolygon
#define gluTessNormal bo_gluTessNormal
#define gluTessProperty bo_gluTessProperty
#define gluTessVertex bo_gluTessVertex
#define gluUnProject bo_gluUnProject
#define gluUnProject4 bo_gluUnProject4

#endif // BOGL_DO_DLOPEN

#endif // BOGLDECL_P_H
