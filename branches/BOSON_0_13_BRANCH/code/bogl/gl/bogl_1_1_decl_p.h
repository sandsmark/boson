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

#ifndef BOGL_1_1_DECL_P_H
#define BOGL_1_1_DECL_P_H

// AB: this is for OpenGL 1.0 and 1.1, since I don't have a 1.0 spec

#ifndef BOGL_H
#error Never include this file directly! Include bogl.h instead!
#endif

#include "../bogl_do_dlopen.h"

extern "C" {
	// GL typedefs
	typedef GLenum (*_glGetError)();
	typedef void (*_glBegin)(GLenum mode);
	typedef void (*_glEnd)();
	typedef void (*_glEdgeFlag)(GLboolean flag);
	typedef void (*_glEdgeFlagv)(const GLboolean *flag);
	typedef void (*_glVertex2d)(GLdouble x, GLdouble y);
	typedef void (*_glVertex2f)(GLfloat x, GLfloat y);
	typedef void (*_glVertex2i)(GLint x, GLint y);
	typedef void (*_glVertex2s)(GLshort x, GLshort y);
	typedef void (*_glVertex3d)(GLdouble x, GLdouble y, GLdouble z);
	typedef void (*_glVertex3f)(GLfloat x, GLfloat y, GLfloat z);
	typedef void (*_glVertex3i)(GLint x, GLint y, GLint z);
	typedef void (*_glVertex3s)(GLshort x, GLshort y, GLshort z);
	typedef void (*_glVertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	typedef void (*_glVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	typedef void (*_glVertex4i)(GLint x, GLint y, GLint z, GLint w);
	typedef void (*_glVertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
	typedef void (*_glVertex2dv)(const GLdouble *v);
	typedef void (*_glVertex2fv)(const GLfloat *v);
	typedef void (*_glVertex2iv)(const GLint *v);
	typedef void (*_glVertex2sv)(const GLshort *v);
	typedef void (*_glVertex3dv)(const GLdouble *v);
	typedef void (*_glVertex3fv)(const GLfloat *v);
	typedef void (*_glVertex3iv)(const GLint *v);
	typedef void (*_glVertex3sv)(const GLshort *v);
	typedef void (*_glVertex4dv)(const GLdouble *v);
	typedef void (*_glVertex4fv)(const GLfloat *v);
	typedef void (*_glVertex4iv)(const GLint *v);
	typedef void (*_glVertex4sv)(const GLshort *v);
	typedef void (*_glTexCoord1d)(GLdouble s);
	typedef void (*_glTexCoord1f)(GLfloat s);
	typedef void (*_glTexCoord1i)(GLint s);
	typedef void (*_glTexCoord1s)(GLshort s);
	typedef void (*_glTexCoord2d)(GLdouble s, GLdouble t);
	typedef void (*_glTexCoord2f)(GLfloat s, GLfloat t);
	typedef void (*_glTexCoord2i)(GLint s, GLint t);
	typedef void (*_glTexCoord2s)(GLshort s, GLshort t);
	typedef void (*_glTexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
	typedef void (*_glTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
	typedef void (*_glTexCoord3i)(GLint s, GLint t, GLint r);
	typedef void (*_glTexCoord3s)(GLshort s, GLshort t, GLshort r);
	typedef void (*_glTexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	typedef void (*_glTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	typedef void (*_glTexCoord4i)(GLint s, GLint t, GLint r, GLint q);
	typedef void (*_glTexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
	typedef void (*_glTexCoord1dv)(const GLdouble *v);
	typedef void (*_glTexCoord1fv)(const GLfloat *v);
	typedef void (*_glTexCoord1iv)(const GLint *v);
	typedef void (*_glTexCoord1sv)(const GLshort *v);
	typedef void (*_glTexCoord2dv)(const GLdouble *v);
	typedef void (*_glTexCoord2fv)(const GLfloat *v);
	typedef void (*_glTexCoord2iv)(const GLint *v);
	typedef void (*_glTexCoord2sv)(const GLshort *v);
	typedef void (*_glTexCoord3dv)(const GLdouble *v);
	typedef void (*_glTexCoord3fv)(const GLfloat *v);
	typedef void (*_glTexCoord3iv)(const GLint *v);
	typedef void (*_glTexCoord3sv)(const GLshort *v);
	typedef void (*_glTexCoord4dv)(const GLdouble *v);
	typedef void (*_glTexCoord4fv)(const GLfloat *v);
	typedef void (*_glTexCoord4iv)(const GLint *v);
	typedef void (*_glTexCoord4sv)(const GLshort *v);
	typedef void (*_glNormal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
	typedef void (*_glNormal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
	typedef void (*_glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
	typedef void (*_glNormal3i)(GLint nx, GLint ny, GLint nz);
	typedef void (*_glNormal3s)(GLshort nx, GLshort ny, GLshort nz);
	typedef void (*_glNormal3bv)(const GLbyte *v);
	typedef void (*_glNormal3dv)(const GLdouble *v);
	typedef void (*_glNormal3fv)(const GLfloat *v);
	typedef void (*_glNormal3iv)(const GLint *v);
	typedef void (*_glNormal3sv)(const GLshort *v);
	typedef void (*_glColor3b)(GLbyte red, GLbyte green, GLbyte blue);
	typedef void (*_glColor3d)(GLdouble red, GLdouble green, GLdouble blue);
	typedef void (*_glColor3f)(GLfloat red, GLfloat green, GLfloat blue);
	typedef void (*_glColor3i)(GLint red, GLint green, GLint blue);
	typedef void (*_glColor3s)(GLshort red, GLshort green, GLshort blue);
	typedef void (*_glColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
	typedef void (*_glColor3ui)(GLuint red, GLuint green, GLuint blue);
	typedef void (*_glColor3us)(GLushort red, GLushort green, GLushort blue);
	typedef void (*_glColor4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
	typedef void (*_glColor4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
	typedef void (*_glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	typedef void (*_glColor4i)(GLint red, GLint green, GLint blue, GLint alpha);
	typedef void (*_glColor4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
	typedef void (*_glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
	typedef void (*_glColor4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
	typedef void (*_glColor4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
	typedef void (*_glColor3bv)(const GLbyte *v);
	typedef void (*_glColor3dv)(const GLdouble *v);
	typedef void (*_glColor3fv)(const GLfloat *v);
	typedef void (*_glColor3iv)(const GLint *v);
	typedef void (*_glColor3sv)(const GLshort *v);
	typedef void (*_glColor3ubv)(const GLubyte *v);
	typedef void (*_glColor3uiv)(const GLuint *v);
	typedef void (*_glColor3usv)(const GLushort *v);
	typedef void (*_glColor4bv)(const GLbyte *v);
	typedef void (*_glColor4dv)(const GLdouble *v);
	typedef void (*_glColor4fv)(const GLfloat *v);
	typedef void (*_glColor4iv)(const GLint *v);
	typedef void (*_glColor4sv)(const GLshort *v);
	typedef void (*_glColor4ubv)(const GLubyte *v);
	typedef void (*_glColor4uiv)(const GLuint *v);
	typedef void (*_glColor4usv)(const GLushort *v);
	typedef void (*_glIndexd)(GLdouble c);
	typedef void (*_glIndexf)(GLfloat c);
	typedef void (*_glIndexi)(GLint c);
	typedef void (*_glIndexs)(GLshort c);
	typedef void (*_glIndexub)(GLubyte c);
	typedef void (*_glIndexdv)(const GLdouble func);
	typedef void (*_glIndexfv)(const GLfloat func);
	typedef void (*_glIndexiv)(const GLint func);
	typedef void (*_glIndexsv)(const GLshort func);
	typedef void (*_glIndexubv)(const GLubyte func);
	typedef void (*_glRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
	typedef void (*_glRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	typedef void (*_glRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
	typedef void (*_glRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
	typedef void (*_glRectdv)(const GLdouble *v1, const GLdouble *v2);
	typedef void (*_glRectfv)(const GLfloat *v1, const GLfloat *v2);
	typedef void (*_glRectiv)(const GLint *v1, const GLint *v2);
	typedef void (*_glRectsv)(const GLshort *v1, const GLshort *v2);
	typedef void (*_glDepthRange)(GLclampd near_val, GLclampd far_val);
	typedef void (*_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
	typedef void (*_glMatrixMode)(GLenum mode);
	typedef void (*_glLoadMatrixd)(const GLdouble *m);
	typedef void (*_glLoadMatrixf)(const GLfloat *m);
	typedef void (*_glMultMatrixd)(const GLdouble *m);
	typedef void (*_glMultMatrixf)(const GLfloat *m);
	typedef void (*_glLoadIdentity)();
	typedef void (*_glRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
	typedef void (*_glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	typedef void (*_glTranslated)(GLdouble x, GLdouble y, GLdouble z);
	typedef void (*_glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
	typedef void (*_glScaled)(GLdouble x, GLdouble y, GLdouble z);
	typedef void (*_glScalef)(GLfloat x, GLfloat y, GLfloat z);
	typedef void (*_glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
	typedef void (*_glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
	typedef void (*_glPushMatrix)();
	typedef void (*_glPopMatrix)();
	typedef void (*_glEnable)(GLenum cap);
	typedef void (*_glDisable)(GLenum cap);
	typedef void (*_glTexGend)(GLenum coord, GLenum pname, GLdouble param);
	typedef void (*_glTexGenf)(GLenum coord, GLenum pname, GLfloat param);
	typedef void (*_glTexGeni)(GLenum coord, GLenum pname, GLint param);
	typedef void (*_glTexGendv)(GLenum coord, GLenum pname, const GLdouble *params);
	typedef void (*_glTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
	typedef void (*_glTexGeniv)(GLenum coord, GLenum pname, const GLint *params);
	typedef void (*_glClipPlane)(GLenum plane, const GLdouble *equation);
	typedef void (*_glRasterPos2d)(GLdouble x, GLdouble y);
	typedef void (*_glRasterPos2f)(GLfloat x, GLfloat y);
	typedef void (*_glRasterPos2i)(GLint x, GLint y);
	typedef void (*_glRasterPos2s)(GLshort x, GLshort y);
	typedef void (*_glRasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
	typedef void (*_glRasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
	typedef void (*_glRasterPos3i)(GLint x, GLint y, GLint z);
	typedef void (*_glRasterPos3s)(GLshort x, GLshort y, GLshort z);
	typedef void (*_glRasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	typedef void (*_glRasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	typedef void (*_glRasterPos4i)(GLint x, GLint y, GLint z, GLint w);
	typedef void (*_glRasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
	typedef void (*_glRasterPos2dv)(const GLdouble *v);
	typedef void (*_glRasterPos2fv)(const GLfloat *v);
	typedef void (*_glRasterPos2iv)(const GLint *v);
	typedef void (*_glRasterPos2sv)(const GLshort *v);
	typedef void (*_glRasterPos3dv)(const GLdouble *v);
	typedef void (*_glRasterPos3fv)(const GLfloat *v);
	typedef void (*_glRasterPos3iv)(const GLint *v);
	typedef void (*_glRasterPos3sv)(const GLshort *v);
	typedef void (*_glRasterPos4dv)(const GLdouble *v);
	typedef void (*_glRasterPos4fv)(const GLfloat *v);
	typedef void (*_glRasterPos4iv)(const GLint *v);
	typedef void (*_glRasterPos4sv)(const GLshort *v);
	typedef void (*_glMaterialf)(GLenum face, GLenum pname, GLfloat param);
	typedef void (*_glMateriali)(GLenum face, GLenum pname, GLint param);
	typedef void (*_glMaterialfv)(GLenum face, GLenum pname, const GLfloat *params);
	typedef void (*_glMaterialiv)(GLenum face, GLenum pname, const GLint *params);
	typedef void (*_glFrontFace)(GLenum mode);
	typedef void (*_glLightf)(GLenum light, GLenum pname, GLfloat param);
	typedef void (*_glLighti)(GLenum light, GLenum pname, GLint param);
	typedef void (*_glLightfv)(GLenum light, GLenum pname, const GLfloat *params);
	typedef void (*_glLightiv)(GLenum light, GLenum pname, const GLint *params);
	typedef void (*_glLightModelf)(GLenum pname, GLfloat param);
	typedef void (*_glLightModeli)(GLenum pname, GLint param);
	typedef void (*_glLightModelfv)(GLenum pname, const GLfloat *params);
	typedef void (*_glLightModeliv)(GLenum pname, const GLint *params);
	typedef void (*_glColorMaterial)(GLenum face, GLenum mode);
	typedef void (*_glShadeModel)(GLenum mode);
	typedef void (*_glPointSize)(GLfloat size);
	typedef void (*_glLineWidth)(GLfloat width);
	typedef void (*_glLineStipple)(GLint factor, GLushort pattern);
	typedef void (*_glCullFace)(GLenum mode);
	typedef void (*_glPolygonStipple)(const GLubyte *mask);
	typedef void (*_glPolygonMode)(GLenum face, GLenum mode);
	typedef void (*_glPolygonOffset)(GLfloat factor, GLfloat units);
	typedef void (*_glPixelStoref)(GLenum pname, GLfloat param);
	typedef void (*_glPixelStorei)(GLenum pname, GLint param);
	typedef void (*_glPixelTransferf)(GLenum pname, GLfloat param);
	typedef void (*_glPixelTransferi)(GLenum pname, GLint param);
	typedef void (*_glPixelMapfv)(GLenum map, GLint mapsize, const GLfloat *values);
	typedef void (*_glPixelMapuiv)(GLenum map, GLint mapsize, const GLuint *values);
	typedef void (*_glPixelMapusv)(GLenum map, GLint mapsize, const GLushort *values);
	typedef void (*_glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glPixelZoom)(GLfloat xfactor, GLfloat yfactor);
	typedef void (*_glBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
	typedef void (*_glTexImage1D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glTexImage2D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glCopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
	typedef void (*_glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	typedef void (*_glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	typedef void (*_glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	typedef void (*_glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
	typedef void (*_glTexParameteri)(GLenum target, GLenum pname, GLint param);
	typedef void (*_glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
	typedef void (*_glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
	typedef void (*_glBindTexture)(GLenum target, GLuint texture);
	typedef void (*_glDeleteTextures)(GLsizei n, const GLuint *textures);
	typedef void (*_glGenTextures)(GLsizei n, GLuint *textures);
	typedef GLboolean (*_glAreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences);
	typedef void (*_glPrioritizeTextures)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
	typedef void (*_glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
	typedef void (*_glTexEnvi)(GLenum target, GLenum pname, GLint param);
	typedef void (*_glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
	typedef void (*_glTexEnviv)(GLenum target, GLenum pname, const GLint *params);
	typedef void (*_glFogf)(GLenum pname, GLfloat param);
	typedef void (*_glFogi)(GLenum pname, GLint param);
	typedef void (*_glFogfv)(GLenum pname, const GLfloat *params);
	typedef void (*_glFogiv)(GLenum pname, const GLint *params);
	typedef void (*_glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
	typedef void (*_glAlphaFunc)(GLenum func, GLclampf ref);
	typedef void (*_glStencilFunc)(GLenum func, GLint ref, GLuint mask);
	typedef void (*_glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
	typedef void (*_glDepthFunc)(GLenum func);
	typedef void (*_glBlendFunc)(GLenum sfactor, GLenum dfactor);
	typedef void (*_glLogicOp)(GLenum opcode);
	typedef void (*_glDrawBuffer)(GLenum mode);
	typedef void (*_glIndexMask)(GLuint mask);
	typedef void (*_glDepthMask)(GLboolean flag);
	typedef void (*_glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	typedef void (*_glStencilMask)(GLuint mask);
	typedef void (*_glClear)(GLbitfield mask);
	typedef void (*_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	typedef void (*_glClearIndex)(GLfloat c);
	typedef void (*_glClearDepth)(GLclampd depth);
	typedef void (*_glClearStencil)(GLint s);
	typedef void (*_glClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	typedef void (*_glAccum)(GLenum op, GLfloat value);
	typedef void (*_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
	typedef void (*_glReadBuffer)(GLenum mode);
	typedef void (*_glCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
	typedef void (*_glMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
	typedef void (*_glMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
	typedef void (*_glMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
	typedef void (*_glMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
	typedef void (*_glEvalCoord1d)(GLdouble u);
	typedef void (*_glEvalCoord1f)(GLfloat u);
	typedef void (*_glEvalCoord1dv)(const GLdouble *u);
	typedef void (*_glEvalCoord1fv)(const GLfloat *u);
	typedef void (*_glEvalCoord2d)(GLdouble u, GLdouble v);
	typedef void (*_glEvalCoord2f)(GLfloat u, GLfloat v);
	typedef void (*_glEvalCoord2dv)(const GLdouble *u);
	typedef void (*_glEvalCoord2fv)(const GLfloat *u);
	typedef void (*_glMapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
	typedef void (*_glMapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
	typedef void (*_glMapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
	typedef void (*_glMapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
	typedef void (*_glEvalMesh1)(GLenum mode, GLint i1, GLint i2);
	typedef void (*_glEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
	typedef void (*_glEvalPoint1)(GLint i);
	typedef void (*_glEvalPoint2)(GLint i, GLint j);
	typedef void (*_glInitNames)();
	typedef void (*_glLoadName)(GLuint name);
	typedef void (*_glPushName)(GLuint name);
	typedef void (*_glPopName)();
	typedef GLint (*_glRenderMode)(GLenum mode);
	typedef void (*_glSelectBuffer)(GLsizei size, GLuint *buffer);
	typedef void (*_glFeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer);
	typedef void (*_glPassThrough)(GLfloat token);
	typedef void (*_glNewList)(GLuint list, GLenum mode);
	typedef void (*_glEndList)();
	typedef void (*_glCallList)(GLuint list);
	typedef void (*_glCallLists)(GLsizei n, GLenum type, const GLvoid *lists);
	typedef void (*_glListBase)(GLuint base);
	typedef GLuint (*_glGenLists)(GLsizei range);
	typedef GLboolean (*_glIsList)(GLuint list);
	typedef void (*_glDeleteLists)(GLuint list, GLsizei range);
	typedef void (*_glFlush)();
	typedef void (*_glFinish)();
	typedef void (*_glHint)(GLenum target, GLenum mode);
	typedef void (*_glGetBooleanv)(GLenum pname, GLboolean *params);
	typedef void (*_glGetIntegerv)(GLenum pname, GLint *params);
	typedef void (*_glGetDoublev)(GLenum pname, GLdouble *params);
	typedef void (*_glGetFloatv)(GLenum pname, GLfloat *params);
	typedef GLboolean (*_glIsEnabled)(GLenum cap);
	typedef void (*_glGetClipPlane)(GLenum plane, GLdouble *equation);
	typedef void (*_glGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
	typedef void (*_glGetLightiv)(GLenum light, GLenum pname, GLint *params);
	typedef void (*_glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
	typedef void (*_glGetMaterialiv)(GLenum face, GLenum pname, GLint *params);
	typedef void (*_glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params);
	typedef void (*_glGetTexEnviv)(GLenum target, GLenum pname, GLint *params);
	typedef void (*_glGetTexGendv)(GLenum coord, GLenum pname, GLdouble *params);
	typedef void (*_glGetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params);
	typedef void (*_glGetTexGeniv)(GLenum coord, GLenum pname, GLint *params);
	typedef void (*_glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
	typedef void (*_glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
	typedef void (*_glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
	typedef void (*_glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
	typedef void (*_glGetPixelMapfv)(GLenum map, GLfloat *values);
	typedef void (*_glGetPixelMapuiv)(GLenum map, GLuint *values);
	typedef void (*_glGetPixelMapusv)(GLenum map, GLushort *values);
	typedef void (*_glGetMapdv)(GLenum target, GLenum query, GLdouble *v);
	typedef void (*_glGetMapfv)(GLenum target, GLenum query, GLfloat *v);
	typedef void (*_glGetMapiv)(GLenum target, GLenum query, GLint *v);
	typedef void (*_glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
	typedef GLboolean (*_glIsTexture)(GLuint texture);
	typedef void (*_glGetPolygonStipple)(GLubyte *mask);
	typedef void (*_glGetPointerv)(GLenum pname, GLvoid **params);
	typedef const GLubyte* (*_glGetString)(GLenum name);
	typedef void (*_glPushAttrib)(GLbitfield mask);
	typedef void (*_glPopAttrib)();
	typedef void (*_glPushClientAttrib)(GLbitfield mask);
	typedef void (*_glPopClientAttrib)();

	typedef void (*_glEdgeFlagPointer)(GLsizei stride, const GLvoid *ptr);
	typedef void (*_glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
	typedef void (*_glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
	typedef void (*_glIndexPointer)(GLenum type, GLsizei stride, const GLvoid *ptr);
	typedef void (*_glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *ptr);
	typedef void (*_glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
	typedef void (*_glEnableClientState)(GLenum cap);
	typedef void (*_glDisableClientState)(GLenum cap);
	typedef void (*_glArrayElement)(GLint i);
	typedef void (*_glDrawArrays)(GLenum mode, GLint first, GLsizei count);
	typedef void (*_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
	typedef void (*_glInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer);




	// GL function pointers
	extern _glGetError bo_glGetError;
	extern _glBegin bo_glBegin;
	extern _glEnd bo_glEnd;
	extern _glEdgeFlag bo_glEdgeFlag;
	extern _glEdgeFlagv bo_glEdgeFlagv;
	extern _glVertex2d bo_glVertex2d;
	extern _glVertex2f bo_glVertex2f;
	extern _glVertex2i bo_glVertex2i;
	extern _glVertex2s bo_glVertex2s;
	extern _glVertex3d bo_glVertex3d;
	extern _glVertex3f bo_glVertex3f;
	extern _glVertex3i bo_glVertex3i;
	extern _glVertex3s bo_glVertex3s;
	extern _glVertex4d bo_glVertex4d;
	extern _glVertex4f bo_glVertex4f;
	extern _glVertex4i bo_glVertex4i;
	extern _glVertex4s bo_glVertex4s;
	extern _glVertex2dv bo_glVertex2dv;
	extern _glVertex2fv bo_glVertex2fv;
	extern _glVertex2iv bo_glVertex2iv;
	extern _glVertex2sv bo_glVertex2sv;
	extern _glVertex3dv bo_glVertex3dv;
	extern _glVertex3fv bo_glVertex3fv;
	extern _glVertex3iv bo_glVertex3iv;
	extern _glVertex3sv bo_glVertex3sv;
	extern _glVertex4dv bo_glVertex4dv;
	extern _glVertex4fv bo_glVertex4fv;
	extern _glVertex4iv bo_glVertex4iv;
	extern _glVertex4sv bo_glVertex4sv;
	extern _glTexCoord1d bo_glTexCoord1d;
	extern _glTexCoord1f bo_glTexCoord1f;
	extern _glTexCoord1i bo_glTexCoord1i;
	extern _glTexCoord1s bo_glTexCoord1s;
	extern _glTexCoord2d bo_glTexCoord2d;
	extern _glTexCoord2f bo_glTexCoord2f;
	extern _glTexCoord2i bo_glTexCoord2i;
	extern _glTexCoord2s bo_glTexCoord2s;
	extern _glTexCoord3d bo_glTexCoord3d;
	extern _glTexCoord3f bo_glTexCoord3f;
	extern _glTexCoord3i bo_glTexCoord3i;
	extern _glTexCoord3s bo_glTexCoord3s;
	extern _glTexCoord4d bo_glTexCoord4d;
	extern _glTexCoord4f bo_glTexCoord4f;
	extern _glTexCoord4i bo_glTexCoord4i;
	extern _glTexCoord4s bo_glTexCoord4s;
	extern _glTexCoord1dv bo_glTexCoord1dv;
	extern _glTexCoord1fv bo_glTexCoord1fv;
	extern _glTexCoord1iv bo_glTexCoord1iv;
	extern _glTexCoord1sv bo_glTexCoord1sv;
	extern _glTexCoord2dv bo_glTexCoord2dv;
	extern _glTexCoord2fv bo_glTexCoord2fv;
	extern _glTexCoord2iv bo_glTexCoord2iv;
	extern _glTexCoord2sv bo_glTexCoord2sv;
	extern _glTexCoord3dv bo_glTexCoord3dv;
	extern _glTexCoord3fv bo_glTexCoord3fv;
	extern _glTexCoord3iv bo_glTexCoord3iv;
	extern _glTexCoord3sv bo_glTexCoord3sv;
	extern _glTexCoord4dv bo_glTexCoord4dv;
	extern _glTexCoord4fv bo_glTexCoord4fv;
	extern _glTexCoord4iv bo_glTexCoord4iv;
	extern _glTexCoord4sv bo_glTexCoord4sv;
	extern _glNormal3b bo_glNormal3b;
	extern _glNormal3d bo_glNormal3d;
	extern _glNormal3f bo_glNormal3f;
	extern _glNormal3i bo_glNormal3i;
	extern _glNormal3s bo_glNormal3s;
	extern _glNormal3bv bo_glNormal3bv;
	extern _glNormal3dv bo_glNormal3dv;
	extern _glNormal3fv bo_glNormal3fv;
	extern _glNormal3iv bo_glNormal3iv;
	extern _glNormal3sv bo_glNormal3sv;
	extern _glColor3b bo_glColor3b;
	extern _glColor3d bo_glColor3d;
	extern _glColor3f bo_glColor3f;
	extern _glColor3i bo_glColor3i;
	extern _glColor3s bo_glColor3s;
	extern _glColor3ub bo_glColor3ub;
	extern _glColor3ui bo_glColor3ui;
	extern _glColor3us bo_glColor3us;
	extern _glColor4b bo_glColor4b;
	extern _glColor4d bo_glColor4d;
	extern _glColor4f bo_glColor4f;
	extern _glColor4i bo_glColor4i;
	extern _glColor4s bo_glColor4s;
	extern _glColor4ub bo_glColor4ub;
	extern _glColor4ui bo_glColor4ui;
	extern _glColor4us bo_glColor4us;
	extern _glColor3bv bo_glColor3bv;
	extern _glColor3dv bo_glColor3dv;
	extern _glColor3fv bo_glColor3fv;
	extern _glColor3iv bo_glColor3iv;
	extern _glColor3sv bo_glColor3sv;
	extern _glColor3ubv bo_glColor3ubv;
	extern _glColor3uiv bo_glColor3uiv;
	extern _glColor3usv bo_glColor3usv;
	extern _glColor4bv bo_glColor4bv;
	extern _glColor4dv bo_glColor4dv;
	extern _glColor4fv bo_glColor4fv;
	extern _glColor4iv bo_glColor4iv;
	extern _glColor4sv bo_glColor4sv;
	extern _glColor4ubv bo_glColor4ubv;
	extern _glColor4uiv bo_glColor4uiv;
	extern _glColor4usv bo_glColor4usv;
	extern _glIndexd bo_glIndexd;
	extern _glIndexf bo_glIndexf;
	extern _glIndexi bo_glIndexi;
	extern _glIndexs bo_glIndexs;
	extern _glIndexub bo_glIndexub;
	extern _glIndexdv bo_glIndexdv;
	extern _glIndexfv bo_glIndexfv;
	extern _glIndexiv bo_glIndexiv;
	extern _glIndexsv bo_glIndexsv;
	extern _glIndexubv bo_glIndexubv;
	extern _glRectd bo_glRectd;
	extern _glRectf bo_glRectf;
	extern _glRecti bo_glRecti;
	extern _glRects bo_glRects;
	extern _glRectdv bo_glRectdv;
	extern _glRectfv bo_glRectfv;
	extern _glRectiv bo_glRectiv;
	extern _glRectsv bo_glRectsv;
	extern _glDepthRange bo_glDepthRange;
	extern _glViewport bo_glViewport;
	extern _glMatrixMode bo_glMatrixMode;
	extern _glLoadMatrixd bo_glLoadMatrixd;
	extern _glLoadMatrixf bo_glLoadMatrixf;
	extern _glMultMatrixd bo_glMultMatrixd;
	extern _glMultMatrixf bo_glMultMatrixf;
	extern _glLoadIdentity bo_glLoadIdentity;
	extern _glRotated bo_glRotated;
	extern _glRotatef bo_glRotatef;
	extern _glTranslated bo_glTranslated;
	extern _glTranslatef bo_glTranslatef;
	extern _glScaled bo_glScaled;
	extern _glScalef bo_glScalef;
	extern _glFrustum bo_glFrustum;
	extern _glOrtho bo_glOrtho;
	extern _glPushMatrix bo_glPushMatrix;
	extern _glPopMatrix bo_glPopMatrix;
	extern _glEnable bo_glEnable;
	extern _glDisable bo_glDisable;
	extern _glTexGend bo_glTexGend;
	extern _glTexGenf bo_glTexGenf;
	extern _glTexGeni bo_glTexGeni;
	extern _glTexGendv bo_glTexGendv;
	extern _glTexGenfv bo_glTexGenfv;
	extern _glTexGeniv bo_glTexGeniv;
	extern _glClipPlane bo_glClipPlane;
	extern _glRasterPos2d bo_glRasterPos2d;
	extern _glRasterPos2f bo_glRasterPos2f;
	extern _glRasterPos2i bo_glRasterPos2i;
	extern _glRasterPos2s bo_glRasterPos2s;
	extern _glRasterPos3d bo_glRasterPos3d;
	extern _glRasterPos3f bo_glRasterPos3f;
	extern _glRasterPos3i bo_glRasterPos3i;
	extern _glRasterPos3s bo_glRasterPos3s;
	extern _glRasterPos4d bo_glRasterPos4d;
	extern _glRasterPos4f bo_glRasterPos4f;
	extern _glRasterPos4i bo_glRasterPos4i;
	extern _glRasterPos4s bo_glRasterPos4s;
	extern _glRasterPos2dv bo_glRasterPos2dv;
	extern _glRasterPos2fv bo_glRasterPos2fv;
	extern _glRasterPos2iv bo_glRasterPos2iv;
	extern _glRasterPos2sv bo_glRasterPos2sv;
	extern _glRasterPos3dv bo_glRasterPos3dv;
	extern _glRasterPos3fv bo_glRasterPos3fv;
	extern _glRasterPos3iv bo_glRasterPos3iv;
	extern _glRasterPos3sv bo_glRasterPos3sv;
	extern _glRasterPos4dv bo_glRasterPos4dv;
	extern _glRasterPos4fv bo_glRasterPos4fv;
	extern _glRasterPos4iv bo_glRasterPos4iv;
	extern _glRasterPos4sv bo_glRasterPos4sv;
	extern _glMaterialf bo_glMaterialf;
	extern _glMateriali bo_glMateriali;
	extern _glMaterialfv bo_glMaterialfv;
	extern _glMaterialiv bo_glMaterialiv;
	extern _glFrontFace bo_glFrontFace;
	extern _glLightf bo_glLightf;
	extern _glLighti bo_glLighti;
	extern _glLightfv bo_glLightfv;
	extern _glLightiv bo_glLightiv;
	extern _glLightModelf bo_glLightModelf;
	extern _glLightModeli bo_glLightModeli;
	extern _glLightModelfv bo_glLightModelfv;
	extern _glLightModeliv bo_glLightModeliv;
	extern _glColorMaterial bo_glColorMaterial;
	extern _glShadeModel bo_glShadeModel;
	extern _glPointSize bo_glPointSize;
	extern _glLineWidth bo_glLineWidth;
	extern _glLineStipple bo_glLineStipple;
	extern _glCullFace bo_glCullFace;
	extern _glPolygonStipple bo_glPolygonStipple;
	extern _glPolygonMode bo_glPolygonMode;
	extern _glPolygonOffset bo_glPolygonOffset;
	extern _glPixelStoref bo_glPixelStoref;
	extern _glPixelStorei bo_glPixelStorei;
	extern _glPixelTransferf bo_glPixelTransferf;
	extern _glPixelTransferi bo_glPixelTransferi;
	extern _glPixelMapfv bo_glPixelMapfv;
	extern _glPixelMapuiv bo_glPixelMapuiv;
	extern _glPixelMapusv bo_glPixelMapusv;
	extern _glDrawPixels bo_glDrawPixels;
	extern _glPixelZoom bo_glPixelZoom;
	extern _glBitmap bo_glBitmap;
	extern _glTexImage1D bo_glTexImage1D;
	extern _glTexImage2D bo_glTexImage2D;
	extern _glCopyTexImage1D bo_glCopyTexImage1D;
	extern _glCopyTexImage2D bo_glCopyTexImage2D;
	extern _glTexSubImage1D bo_glTexSubImage1D;
	extern _glTexSubImage2D bo_glTexSubImage2D;
	extern _glCopyTexSubImage1D bo_glCopyTexSubImage1D;
	extern _glCopyTexSubImage2D bo_glCopyTexSubImage2D;
	extern _glTexParameterf bo_glTexParameterf;
	extern _glTexParameteri bo_glTexParameteri;
	extern _glTexParameterfv bo_glTexParameterfv;
	extern _glTexParameteriv bo_glTexParameteriv;
	extern _glBindTexture bo_glBindTexture;
	extern _glDeleteTextures bo_glDeleteTextures;
	extern _glGenTextures bo_glGenTextures;
	extern _glAreTexturesResident bo_glAreTexturesResident;
	extern _glPrioritizeTextures bo_glPrioritizeTextures;
	extern _glTexEnvf bo_glTexEnvf;
	extern _glTexEnvi bo_glTexEnvi;
	extern _glTexEnvfv bo_glTexEnvfv;
	extern _glTexEnviv bo_glTexEnviv;
	extern _glFogf bo_glFogf;
	extern _glFogi bo_glFogi;
	extern _glFogfv bo_glFogfv;
	extern _glFogiv bo_glFogiv;
	extern _glScissor bo_glScissor;
	extern _glAlphaFunc bo_glAlphaFunc;
	extern _glStencilFunc bo_glStencilFunc;
	extern _glStencilOp bo_glStencilOp;
	extern _glDepthFunc bo_glDepthFunc;
	extern _glBlendFunc bo_glBlendFunc;
	extern _glLogicOp bo_glLogicOp;
	extern _glDrawBuffer bo_glDrawBuffer;
	extern _glIndexMask bo_glIndexMask;
	extern _glColorMask bo_glColorMask;
	extern _glDepthMask bo_glDepthMask;
	extern _glStencilMask bo_glStencilMask;
	extern _glClear bo_glClear;
	extern _glClearColor bo_glClearColor;
	extern _glClearIndex bo_glClearIndex;
	extern _glClearDepth bo_glClearDepth;
	extern _glClearStencil bo_glClearStencil;
	extern _glClearAccum bo_glClearAccum;
	extern _glAccum bo_glAccum;
	extern _glReadPixels bo_glReadPixels;
	extern _glReadBuffer bo_glReadBuffer;
	extern _glCopyPixels bo_glCopyPixels;
	extern _glMap2d bo_glMap2d;
	extern _glMap2f bo_glMap2f;
	extern _glMap1d bo_glMap1d;
	extern _glMap1f bo_glMap1f;
	extern _glEvalCoord1d bo_glEvalCoord1d;
	extern _glEvalCoord1f bo_glEvalCoord1f;
	extern _glEvalCoord1dv bo_glEvalCoord1dv;
	extern _glEvalCoord1fv bo_glEvalCoord1fv;
	extern _glEvalCoord2d bo_glEvalCoord2d;
	extern _glEvalCoord2f bo_glEvalCoord2f;
	extern _glEvalCoord2dv bo_glEvalCoord2dv;
	extern _glEvalCoord2fv bo_glEvalCoord2fv;
	extern _glMapGrid1d bo_glMapGrid1d;
	extern _glMapGrid1f bo_glMapGrid1f;
	extern _glMapGrid2d bo_glMapGrid2d;
	extern _glMapGrid2f bo_glMapGrid2f;
	extern _glEvalMesh1 bo_glEvalMesh1;
	extern _glEvalMesh2 bo_glEvalMesh2;
	extern _glEvalPoint1 bo_glEvalPoint1;
	extern _glEvalPoint2 bo_glEvalPoint2;
	extern _glInitNames bo_glInitNames;
	extern _glLoadName bo_glLoadName;
	extern _glPushName bo_glPushName;
	extern _glPopName bo_glPopName;
	extern _glRenderMode bo_glRenderMode;
	extern _glSelectBuffer bo_glSelectBuffer;
	extern _glFeedbackBuffer bo_glFeedbackBuffer;
	extern _glPassThrough bo_glPassThrough;
	extern _glNewList bo_glNewList;
	extern _glEndList bo_glEndList;
	extern _glCallList bo_glCallList;
	extern _glCallLists bo_glCallLists;
	extern _glListBase bo_glListBase;
	extern _glGenLists bo_glGenLists;
	extern _glIsList bo_glIsList;
	extern _glDeleteLists bo_glDeleteLists;
	extern _glFlush bo_glFlush;
	extern _glFinish bo_glFinish;
	extern _glHint bo_glHint;
	extern _glGetBooleanv bo_glGetBooleanv;
	extern _glGetIntegerv bo_glGetIntegerv;
	extern _glGetDoublev bo_glGetDoublev;
	extern _glGetFloatv bo_glGetFloatv;
	extern _glIsEnabled bo_glIsEnabled;
	extern _glGetClipPlane bo_glGetClipPlane;
	extern _glGetLightfv bo_glGetLightfv;
	extern _glGetLightiv bo_glGetLightiv;
	extern _glGetMaterialfv bo_glGetMaterialfv;
	extern _glGetMaterialiv bo_glGetMaterialiv;
	extern _glGetTexEnvfv bo_glGetTexEnvfv;
	extern _glGetTexEnviv bo_glGetTexEnviv;
	extern _glGetTexGenfv bo_glGetTexGenfv;
	extern _glGetTexGeniv bo_glGetTexGeniv;
	extern _glGetTexGendv bo_glGetTexGendv; // AB: the d version is not in the 1.1 spec. an error in the spec?
	extern _glGetTexParameterfv bo_glGetTexParameterfv;
	extern _glGetTexParameteriv bo_glGetTexParameteriv;
	extern _glGetTexLevelParameterfv bo_glGetTexLevelParameterfv;
	extern _glGetTexLevelParameteriv bo_glGetTexLevelParameteriv;
	extern _glGetPixelMapfv bo_glGetPixelMapfv;
	extern _glGetPixelMapuiv bo_glGetPixelMapuiv;
	extern _glGetPixelMapusv bo_glGetPixelMapusv;
	extern _glGetMapdv bo_glGetMapdv;
	extern _glGetMapfv bo_glGetMapfv;
	extern _glGetMapiv bo_glGetMapiv;
	extern _glGetTexImage bo_glGetTexImage;
	extern _glIsTexture bo_glIsTexture;
	extern _glGetPolygonStipple bo_glGetPolygonStipple;
	extern _glGetPointerv bo_glGetPointerv;
	extern _glGetString bo_glGetString;
	extern _glPushAttrib bo_glPushAttrib;
	extern _glPopAttrib bo_glPopAttrib;
	extern _glPushClientAttrib bo_glPushClientAttrib;
	extern _glPopClientAttrib bo_glPopClientAttrib;

	extern _glEdgeFlagPointer bo_glEdgeFlagPointer;
	extern _glTexCoordPointer bo_glTexCoordPointer;
	extern _glColorPointer bo_glColorPointer;
	extern _glIndexPointer bo_glIndexPointer;
	extern _glNormalPointer bo_glNormalPointer;
	extern _glVertexPointer bo_glVertexPointer;
	extern _glEnableClientState bo_glEnableClientState;
	extern _glDisableClientState bo_glDisableClientState;
	extern _glArrayElement bo_glArrayElement;
	extern _glDrawArrays bo_glDrawArrays;
	extern _glDrawElements bo_glDrawElements;
	extern _glInterleavedArrays bo_glInterleavedArrays;

} // extern "C"





// GL defines
#if BOGL_DO_DLOPEN

#define glGetError bo_glGetError
#define glBegin bo_glBegin
#define glEnd bo_glEnd
#define glEdgeFlag bo_glEdgeFlag
#define glEdgeFlagv bo_glEdgeFlagv
#define glVertex2d bo_glVertex2d
#define glVertex2f bo_glVertex2f
#define glVertex2i bo_glVertex2i
#define glVertex2s bo_glVertex2s
#define glVertex3d bo_glVertex3d
#define glVertex3f bo_glVertex3f
#define glVertex3i bo_glVertex3i
#define glVertex3s bo_glVertex3s
#define glVertex4d bo_glVertex4d
#define glVertex4f bo_glVertex4f
#define glVertex4i bo_glVertex4i
#define glVertex4s bo_glVertex4s
#define glVertex2dv bo_glVertex2dv
#define glVertex2fv bo_glVertex2fv
#define glVertex2iv bo_glVertex2iv
#define glVertex2sv bo_glVertex2sv
#define glVertex3dv bo_glVertex3dv
#define glVertex3fv bo_glVertex3fv
#define glVertex3iv bo_glVertex3iv
#define glVertex3sv bo_glVertex3sv
#define glVertex4dv bo_glVertex4dv
#define glVertex4fv bo_glVertex4fv
#define glVertex4iv bo_glVertex4iv
#define glVertex4sv bo_glVertex4sv
#define glTexCoord1d bo_glTexCoord1d
#define glTexCoord1f bo_glTexCoord1f
#define glTexCoord1i bo_glTexCoord1i
#define glTexCoord1s bo_glTexCoord1s
#define glTexCoord2d bo_glTexCoord2d
#define glTexCoord2f bo_glTexCoord2f
#define glTexCoord2i bo_glTexCoord2i
#define glTexCoord2s bo_glTexCoord2s
#define glTexCoord3d bo_glTexCoord3d
#define glTexCoord3f bo_glTexCoord3f
#define glTexCoord3i bo_glTexCoord3i
#define glTexCoord3s bo_glTexCoord3s
#define glTexCoord4d bo_glTexCoord4d
#define glTexCoord4f bo_glTexCoord4f
#define glTexCoord4i bo_glTexCoord4i
#define glTexCoord4s bo_glTexCoord4s
#define glTexCoord1dv bo_glTexCoord1dv
#define glTexCoord1fv bo_glTexCoord1fv
#define glTexCoord1iv bo_glTexCoord1iv
#define glTexCoord1sv bo_glTexCoord1sv
#define glTexCoord2dv bo_glTexCoord2dv
#define glTexCoord2fv bo_glTexCoord2fv
#define glTexCoord2iv bo_glTexCoord2iv
#define glTexCoord2sv bo_glTexCoord2sv
#define glTexCoord3dv bo_glTexCoord3dv
#define glTexCoord3fv bo_glTexCoord3fv
#define glTexCoord3iv bo_glTexCoord3iv
#define glTexCoord3sv bo_glTexCoord3sv
#define glTexCoord4dv bo_glTexCoord4dv
#define glTexCoord4fv bo_glTexCoord4fv
#define glTexCoord4iv bo_glTexCoord4iv
#define glTexCoord4sv bo_glTexCoord4sv
#define glNormal3b bo_glNormal3b
#define glNormal3d bo_glNormal3d
#define glNormal3f bo_glNormal3f
#define glNormal3i bo_glNormal3i
#define glNormal3s bo_glNormal3s
#define glNormal3bv bo_glNormal3bv
#define glNormal3dv bo_glNormal3dv
#define glNormal3fv bo_glNormal3fv
#define glNormal3iv bo_glNormal3iv
#define glNormal3sv bo_glNormal3sv
#define glColor3b bo_glColor3b
#define glColor3d bo_glColor3d
#define glColor3f bo_glColor3f
#define glColor3i bo_glColor3i
#define glColor3s bo_glColor3s
#define glColor3ub bo_glColor3ub
#define glColor3ui bo_glColor3ui
#define glColor3us bo_glColor3us
#define glColor4b bo_glColor4b
#define glColor4d bo_glColor4d
#define glColor4f bo_glColor4f
#define glColor4i bo_glColor4i
#define glColor4s bo_glColor4s
#define glColor4ub bo_glColor4ub
#define glColor4ui bo_glColor4ui
#define glColor4us bo_glColor4us
#define glColor3bv bo_glColor3bv
#define glColor3dv bo_glColor3dv
#define glColor3fv bo_glColor3fv
#define glColor3iv bo_glColor3iv
#define glColor3sv bo_glColor3sv
#define glColor3ubv bo_glColor3ubv
#define glColor3uiv bo_glColor3uiv
#define glColor3usv bo_glColor3usv
#define glColor4bv bo_glColor4bv
#define glColor4dv bo_glColor4dv
#define glColor4fv bo_glColor4fv
#define glColor4iv bo_glColor4iv
#define glColor4sv bo_glColor4sv
#define glColor4ubv bo_glColor4ubv
#define glColor4uiv bo_glColor4uiv
#define glColor4usv bo_glColor4usv
#define glIndexd bo_glIndexd
#define glIndexf bo_glIndexf
#define glIndexi bo_glIndexi
#define glIndexs bo_glIndexs
#define glIndexub bo_glIndexub
#define glIndexdv bo_glIndexdv
#define glIndexfv bo_glIndexfv
#define glIndexiv bo_glIndexiv
#define glIndexsv bo_glIndexsv
#define glIndexubv bo_glIndexubv
#define glRectd bo_glRectd
#define glRectf bo_glRectf
#define glRecti bo_glRecti
#define glRects bo_glRects
#define glRectdv bo_glRectdv
#define glRectfv bo_glRectfv
#define glRectiv bo_glRectiv
#define glRectsv bo_glRectsv
#define glDepthRange bo_glDepthRange
#define glViewport bo_glViewport
#define glMatrixMode bo_glMatrixMode
#define glLoadMatrixd bo_glLoadMatrixd
#define glLoadMatrixf bo_glLoadMatrixf
#define glMultMatrixd bo_glMultMatrixd
#define glMultMatrixf bo_glMultMatrixf
#define glLoadIdentity bo_glLoadIdentity
#define glRotated bo_glRotated
#define glRotatef bo_glRotatef
#define glTranslated bo_glTranslated
#define glTranslatef bo_glTranslatef
#define glScaled bo_glScaled
#define glScalef bo_glScalef
#define glFrustum bo_glFrustum
#define glOrtho bo_glOrtho
#define glPushMatrix bo_glPushMatrix
#define glPopMatrix bo_glPopMatrix
#define glEnable bo_glEnable
#define glDisable bo_glDisable
#define glTexGend bo_glTexGend
#define glTexGenf bo_glTexGenf
#define glTexGeni bo_glTexGeni
#define glTexGendv bo_glTexGendv
#define glTexGenfv bo_glTexGenfv
#define glTexGeniv bo_glTexGeniv
#define glClipPlane bo_glClipPlane
#define glRasterPos2d bo_glRasterPos2d
#define glRasterPos2f bo_glRasterPos2f
#define glRasterPos2i bo_glRasterPos2i
#define glRasterPos2s bo_glRasterPos2s
#define glRasterPos3d bo_glRasterPos3d
#define glRasterPos3f bo_glRasterPos3f
#define glRasterPos3i bo_glRasterPos3i
#define glRasterPos3s bo_glRasterPos3s
#define glRasterPos4d bo_glRasterPos4d
#define glRasterPos4f bo_glRasterPos4f
#define glRasterPos4i bo_glRasterPos4i
#define glRasterPos4s bo_glRasterPos4s
#define glRasterPos2dv bo_glRasterPos2dv
#define glRasterPos2fv bo_glRasterPos2fv
#define glRasterPos2iv bo_glRasterPos2iv
#define glRasterPos2sv bo_glRasterPos2sv
#define glRasterPos3dv bo_glRasterPos3dv
#define glRasterPos3fv bo_glRasterPos3fv
#define glRasterPos3iv bo_glRasterPos3iv
#define glRasterPos3sv bo_glRasterPos3sv
#define glRasterPos4dv bo_glRasterPos4dv
#define glRasterPos4fv bo_glRasterPos4fv
#define glRasterPos4iv bo_glRasterPos4iv
#define glRasterPos4sv bo_glRasterPos4sv
#define glMaterialf bo_glMaterialf
#define glMateriali bo_glMateriali
#define glMaterialfv bo_glMaterialfv
#define glMaterialiv bo_glMaterialiv
#define glFrontFace bo_glFrontFace
#define glLightf bo_glLightf
#define glLighti bo_glLighti
#define glLightfv bo_glLightfv
#define glLightiv bo_glLightiv
#define glLightModelf bo_glLightModelf
#define glLightModeli bo_glLightModeli
#define glLightModelfv bo_glLightModelfv
#define glLightModeliv bo_glLightModeliv
#define glColorMaterial bo_glColorMaterial
#define glShadeModel bo_glShadeModel
#define glPointSize bo_glPointSize
#define glLineWidth bo_glLineWidth
#define glLineStipple bo_glLineStipple
#define glCullFace bo_glCullFace
#define glPolygonStipple bo_glPolygonStipple
#define glPolygonMode bo_glPolygonMode
#define glPolygonOffset bo_glPolygonOffset
#define glPixelStoref bo_glPixelStoref
#define glPixelStorei bo_glPixelStorei
#define glPixelTransferf bo_glPixelTransferf
#define glPixelTransferi bo_glPixelTransferi
#define glPixelMapfv bo_glPixelMapfv
#define glPixelMapuiv bo_glPixelMapuiv
#define glPixelMapusv bo_glPixelMapusv
#define glDrawPixels bo_glDrawPixels
#define glPixelZoom bo_glPixelZoom
#define glBitmap bo_glBitmap
#define glTexImage1D bo_glTexImage1D
#define glTexImage2D bo_glTexImage2D
#define glCopyTexImage1D bo_glCopyTexImage1D
#define glCopyTexImage2D bo_glCopyTexImage2D
#define glTexSubImage1D bo_glTexSubImage1D
#define glTexSubImage2D bo_glTexSubImage2D
#define glCopyTexSubImage1D bo_glCopyTexSubImage1D
#define glCopyTexSubImage2D bo_glCopyTexSubImage2D
#define glTexParameterf bo_glTexParameterf
#define glTexParameteri bo_glTexParameteri
#define glTexParameterfv bo_glTexParameterfv
#define glTexParameteriv bo_glTexParameteriv
#define glBindTexture bo_glBindTexture
#define glDeleteTextures bo_glDeleteTextures
#define glGenTextures bo_glGenTextures
#define glAreTexturesResident bo_glAreTexturesResident
#define glPrioritizeTextures bo_glPrioritizeTextures
#define glTexEnvf bo_glTexEnvf
#define glTexEnvi bo_glTexEnvi
#define glTexEnvfv bo_glTexEnvfv
#define glTexEnviv bo_glTexEnviv
#define glFogf bo_glFogf
#define glFogi bo_glFogi
#define glFogfv bo_glFogfv
#define glFogiv bo_glFogiv
#define glScissor bo_glScissor
#define glAlphaFunc bo_glAlphaFunc
#define glStencilFunc bo_glStencilFunc
#define glStencilOp bo_glStencilOp
#define glDepthFunc bo_glDepthFunc
#define glBlendFunc bo_glBlendFunc
#define glLogicOp bo_glLogicOp
#define glDrawBuffer bo_glDrawBuffer
#define glIndexMask bo_glIndexMask
#define glColorMask bo_glColorMask
#define glDepthMask bo_glDepthMask
#define glStencilMask bo_glStencilMask
#define glClear bo_glClear
#define glClearColor bo_glClearColor
#define glClearIndex bo_glClearIndex
#define glClearDepth bo_glClearDepth
#define glClearStencil bo_glClearStencil
#define glClearAccum bo_glClearAccum
#define glAccum bo_glAccum
#define glReadPixels bo_glReadPixels
#define glReadBuffer bo_glReadBuffer
#define glCopyPixels bo_glCopyPixels
#define glMap2d bo_glMap2d
#define glMap2f bo_glMap2f
#define glMap1d bo_glMap1d
#define glMap1f bo_glMap1f
#define glEvalCoord1d bo_glEvalCoord1d
#define glEvalCoord1f bo_glEvalCoord1f
#define glEvalCoord1dv bo_glEvalCoord1dv
#define glEvalCoord1fv bo_glEvalCoord1fv
#define glEvalCoord2d bo_glEvalCoord2d
#define glEvalCoord2f bo_glEvalCoord2f
#define glEvalCoord2dv bo_glEvalCoord2dv
#define glEvalCoord2fv bo_glEvalCoord2fv
#define glMapGrid1d bo_glMapGrid1d
#define glMapGrid1f bo_glMapGrid1f
#define glMapGrid2d bo_glMapGrid2d
#define glMapGrid2f bo_glMapGrid2f
#define glEvalMesh1 bo_glEvalMesh1
#define glEvalMesh2 bo_glEvalMesh2
#define glEvalPoint1 bo_glEvalPoint1
#define glEvalPoint2 bo_glEvalPoint2
#define glInitNames bo_glInitNames
#define glLoadName bo_glLoadName
#define glPushName bo_glPushName
#define glPopName bo_glPopName
#define glRenderMode bo_glRenderMode
#define glSelectBuffer bo_glSelectBuffer
#define glFeedbackBuffer bo_glFeedbackBuffer
#define glPassThrough bo_glPassThrough
#define glNewList bo_glNewList
#define glEndList bo_glEndList
#define glCallList bo_glCallList
#define glCallLists bo_glCallLists
#define glListBase bo_glListBase
#define glGenLists bo_glGenLists
#define glIsList bo_glIsList
#define glDeleteLists bo_glDeleteLists
#define glFlush bo_glFlush
#define glFinish bo_glFinish
#define glHint bo_glHint
#define glGetBooleanv bo_glGetBooleanv
#define glGetIntegerv bo_glGetIntegerv
#define glGetDoublev bo_glGetDoublev
#define glGetFloatv bo_glGetFloatv
#define glIsEnabled bo_glIsEnabled
#define glGetClipPlane bo_glGetClipPlane
#define glGetLightfv bo_glGetLightfv
#define glGetLightiv bo_glGetLightiv
#define glGetMaterialfv bo_glGetMaterialfv
#define glGetMaterialiv bo_glGetMaterialiv
#define glGetTexEnvfv bo_glGetTexEnvfv
#define glGetTexEnviv bo_glGetTexEnviv
#define glGetTexGenfv bo_glGetTexGenfv
#define glGetTexGeniv bo_glGetTexGeniv
#define glGetTexGendv bo_glGetTexGendv
#define glGetTexParameterfv bo_glGetTexParameterfv
#define glGetTexParameteriv bo_glGetTexParameteriv
#define glGetTexLevelParameterfv bo_glGetTexLevelParameterfv
#define glGetTexLevelParameteriv bo_glGetTexLevelParameteriv
#define glGetPixelMapfv bo_glGetPixelMapfv
#define glGetPixelMapuiv bo_glGetPixelMapuiv
#define glGetPixelMapusv bo_glGetPixelMapusv
#define glGetMapdv bo_glGetMapdv
#define glGetMapfv bo_glGetMapfv
#define glGetMapiv bo_glGetMapiv
#define glGetTexImage bo_glGetTexImage
#define glIsTexture bo_glIsTexture
#define glGetPolygonStipple bo_glGetPolygonStipple
#define glGetPointerv bo_glGetPointerv
#define glGetString bo_glGetString
#define glPushAttrib bo_glPushAttrib
#define glPopAttrib bo_glPopAttrib
#define glPushClientAttrib bo_glPushClientAttrib
#define glPopClientAttrib bo_glPopClientAttrib

#define glEdgeFlagPointer bo_glEdgeFlagPointer
#define glTexCoordPointer bo_glTexCoordPointer
#define glColorPointer bo_glColorPointer
#define glIndexPointer bo_glIndexPointer
#define glNormalPointer bo_glNormalPointer
#define glVertexPointer bo_glVertexPointer
#define glEnableClientState bo_glEnableClientState
#define glDisableClientState bo_glDisableClientState
#define glArrayElement bo_glArrayElement
#define glDrawArrays bo_glDrawArrays
#define glDrawElements bo_glDrawElements
#define glInterleavedArrays bo_glInterleavedArrays

#endif // BOGL_DO_DLOPEN

#endif // BOGLDECL_P_H
