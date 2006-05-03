/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann <b_mann@gmx.de>

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

#ifndef BOGLDECL_P_H
#define BOGLDECL_P_H

#ifndef BOGL_H
#error Never include this file directly! Include bogl.h instead!
#endif

#include "bogl_do_dlopen.h"


// This file contains declarations from GL/gl.h and GL/glx.h ONLY!
// Do NOT add any other functions here!

// AB: we do this quite similar to Qt: define a typedef starting with an
// underscore, declare a variable and finally redefine the GL function to use
// our variable instead.
extern "C" {
// GL typedefs
typedef void (*_glClearIndex)(GLfloat c);
typedef void (*_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (*_glClear)(GLbitfield mask);
typedef void (*_glIndexMask)(GLuint mask);
typedef void (*_glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (*_glAlphaFunc)(GLenum func, GLclampf ref);
typedef void (*_glBlendFunc)(GLenum sfactor, GLenum dfactor);
typedef void (*_glLogicOp)(GLenum opcode);
typedef void (*_glCullFace)(GLenum mode);
typedef void (*_glFrontFace)(GLenum mode);
typedef void (*_glPointSize)(GLfloat size);
typedef void (*_glLineWidth)(GLfloat width);
typedef void (*_glLineStipple)(GLint factor, GLushort pattern);
typedef void (*_glPolygonMode)(GLenum face, GLenum mode);
typedef void (*_glPolygonOffset)(GLfloat factor, GLfloat units);
typedef void (*_glPolygonStipple)(const GLubyte *mask);
typedef void (*_glGetPolygonStipple)(GLubyte *mask);
typedef void (*_glEdgeFlag)(GLboolean flag);
typedef void (*_glEdgeFlagv)(const GLboolean *flag);
typedef void (*_glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (*_glClipPlane)(GLenum plane, const GLdouble *equation);
typedef void (*_glGetClipPlane)(GLenum plane, GLdouble *equation);
typedef void (*_glDrawBuffer)(GLenum mode);
typedef void (*_glReadBuffer)(GLenum mode);
typedef void (*_glEnable)(GLenum cap);
typedef void (*_glDisable)(GLenum cap);
typedef GLboolean (*_glIsEnabled)(GLenum cap);
typedef void (*_glEnableClientState)(GLenum cap);
typedef void (*_glDisableClientState)(GLenum cap);
typedef void (*_glGetBooleanv)(GLenum pname, GLboolean *params);
typedef void (*_glGetDoublev)(GLenum pname, GLdouble *params);
typedef void (*_glGetFloatv)(GLenum pname, GLfloat *params);
typedef void (*_glGetIntegerv)(GLenum pname, GLint *params);
typedef void (*_glPushAttrib)(GLbitfield mask);
typedef void (*_glPopAttrib)();
typedef void (*_glPushClientAttrib)(GLbitfield mask);
typedef void (*_glPopClientAttrib)();
typedef GLint (*_glRenderMode)(GLenum mode);
typedef GLenum (*_glGetError)();
typedef const GLubyte* (*_glGetString)(GLenum name);
typedef void (*_glFinish)();
typedef void (*_glFlush)();
typedef void (*_glHint)(GLenum target, GLenum mode);
typedef void (*_glClearDepth)(GLclampd depth);
typedef void (*_glDepthFunc)(GLenum func);
typedef void (*_glDepthMask)(GLboolean flag);
typedef void (*_glDepthRange)(GLclampd near_val, GLclampd far_val);
typedef void (*_glClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (*_glAccum)(GLenum op, GLfloat value);
typedef void (*_glMatrixMode)(GLenum mode);
typedef void (*_glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
typedef void (*_glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
typedef void (*_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (*_glPushMatrix)();
typedef void (*_glPopMatrix)();
typedef void (*_glLoadIdentity)();
typedef void (*_glLoadMatrixd)(const GLdouble *m);
typedef void (*_glLoadMatrixf)(const GLfloat *m);
typedef void (*_glMultMatrixd)(const GLdouble *m);
typedef void (*_glMultMatrixf)(const GLfloat *m);
typedef void (*_glRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
typedef void (*_glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void (*_glScaled)(GLdouble x, GLdouble y, GLdouble z);
typedef void (*_glScalef)(GLfloat x, GLfloat y, GLfloat z);
typedef void (*_glTranslated)(GLdouble x, GLdouble y, GLdouble z);
typedef void (*_glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
typedef GLboolean (*_glIsList)(GLuint list);
typedef void (*_glDeleteLists)(GLuint list, GLsizei range);
typedef GLuint (*_glGenLists)(GLsizei range);
typedef void (*_glNewList)(GLuint list, GLenum mode);
typedef void (*_glEndList)();
typedef void (*_glCallList)(GLuint list);
typedef void (*_glCallLists)(GLsizei n, GLenum type, const GLvoid *lists);
typedef void (*_glListBase)(GLuint base);
typedef void (*_glBegin)(GLenum mode);
typedef void (*_glEnd)();
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
typedef void (*_glRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
typedef void (*_glRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
typedef void (*_glRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
typedef void (*_glRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
typedef void (*_glRectdv)(const GLdouble *v1, const GLdouble *v2);
typedef void (*_glRectfv)(const GLfloat *v1, const GLfloat *v2);
typedef void (*_glRectiv)(const GLint *v1, const GLint *v2);
typedef void (*_glRectsv)(const GLshort *v1, const GLshort *v2);
typedef void (*_glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (*_glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (*_glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (*_glIndexPointer)(GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (*_glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
typedef void (*_glEdgeFlagPointer)(GLsizei stride, const GLvoid *ptr);
typedef void (*_glGetPointerv)(GLenum pname, GLvoid **params);
typedef void (*_glArrayElement)(GLint i);
typedef void (*_glDrawArrays)(GLenum mode, GLint first, GLsizei count);
typedef void (*_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (*_glInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer);
typedef void (*_glShadeModel)(GLenum mode);
typedef void (*_glLightf)(GLenum light, GLenum pname, GLfloat param);
typedef void (*_glLighti)(GLenum light, GLenum pname, GLint param);
typedef void (*_glLightfv)(GLenum light, GLenum pname, const GLfloat *params);
typedef void (*_glLightiv)(GLenum light, GLenum pname, const GLint *params);
typedef void (*_glGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
typedef void (*_glGetLightiv)(GLenum light, GLenum pname, GLint *params);
typedef void (*_glLightModelf)(GLenum pname, GLfloat param);
typedef void (*_glLightModeli)(GLenum pname, GLint param);
typedef void (*_glLightModelfv)(GLenum pname, const GLfloat *params);
typedef void (*_glLightModeliv)(GLenum pname, const GLint *params);
typedef void (*_glMaterialf)(GLenum face, GLenum pname, GLfloat param);
typedef void (*_glMateriali)(GLenum face, GLenum pname, GLint param);
typedef void (*_glMaterialfv)(GLenum face, GLenum pname, const GLfloat *params);
typedef void (*_glMaterialiv)(GLenum face, GLenum pname, const GLint *params);
typedef void (*_glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
typedef void (*_glGetMaterialiv)(GLenum face, GLenum pname, GLint *params);
typedef void (*_glColorMaterial)(GLenum face, GLenum mode);
typedef void (*_glPixelZoom)(GLfloat xfactor, GLfloat yfactor);
typedef void (*_glPixelStoref)(GLenum pname, GLfloat param);
typedef void (*_glPixelStorei)(GLenum pname, GLint param);
typedef void (*_glPixelTransferf)(GLenum pname, GLfloat param);
typedef void (*_glPixelTransferi)(GLenum pname, GLint param);
typedef void (*_glPixelMapfv)(GLenum map, GLint mapsize, const GLfloat *values);
typedef void (*_glPixelMapuiv)(GLenum map, GLint mapsize, const GLuint *values);
typedef void (*_glPixelMapusv)(GLenum map, GLint mapsize, const GLushort *values);
typedef void (*_glGetPixelMapfv)(GLenum map, GLfloat *values);
typedef void (*_glGetPixelMapuiv)(GLenum map, GLuint *values);
typedef void (*_glGetPixelMapusv)(GLenum map, GLushort *values);
typedef void (*_glBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
typedef void (*_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
typedef void (*_glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
typedef void (*_glStencilFunc)(GLenum func, GLint ref, GLuint mask);
typedef void (*_glStencilMask)(GLuint mask);
typedef void (*_glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
typedef void (*_glClearStencil)(GLint s);
typedef void (*_glTexGend)(GLenum coord, GLenum pname, GLdouble param);
typedef void (*_glTexGenf)(GLenum coord, GLenum pname, GLfloat param);
typedef void (*_glTexGeni)(GLenum coord, GLenum pname, GLint param);
typedef void (*_glTexGendv)(GLenum coord, GLenum pname, const GLdouble *params);
typedef void (*_glTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
typedef void (*_glTexGeniv)(GLenum coord, GLenum pname, const GLint *params);
typedef void (*_glGetTexGendv)(GLenum coord, GLenum pname, GLdouble *params);
typedef void (*_glGetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params);
typedef void (*_glGetTexGeniv)(GLenum coord, GLenum pname, GLint *params);
typedef void (*_glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
typedef void (*_glTexEnvi)(GLenum target, GLenum pname, GLint param);
typedef void (*_glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
typedef void (*_glTexEnviv)(GLenum target, GLenum pname, const GLint *params);
typedef void (*_glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params);
typedef void (*_glGetTexEnviv)(GLenum target, GLenum pname, GLint *params);
typedef void (*_glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
typedef void (*_glTexParameteri)(GLenum target, GLenum pname, GLint param);
typedef void (*_glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
typedef void (*_glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
typedef void (*_glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
typedef void (*_glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
typedef void (*_glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (*_glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (*_glTexImage1D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glTexImage2D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
typedef void (*_glGenTextures)(GLsizei n, GLuint *textures);
typedef void (*_glDeleteTextures)(GLsizei n, const GLuint *textures);
typedef void (*_glBindTexture)(GLenum target, GLuint texture);
typedef void (*_glPrioritizeTextures)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef GLboolean (*_glAreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences);
typedef GLboolean (*_glIsTexture)(GLuint texture);
typedef void (*_glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glCopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (*_glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (*_glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (*_glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (*_glMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
typedef void (*_glMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
typedef void (*_glMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
typedef void (*_glMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
typedef void (*_glGetMapdv)(GLenum target, GLenum query, GLdouble *v);
typedef void (*_glGetMapfv)(GLenum target, GLenum query, GLfloat *v);
typedef void (*_glGetMapiv)(GLenum target, GLenum query, GLint *v);
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
typedef void (*_glEvalPoint1)(GLint i);
typedef void (*_glEvalPoint2)(GLint i, GLint j);
typedef void (*_glEvalMesh1)(GLenum mode, GLint i1, GLint i2);
typedef void (*_glEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
typedef void (*_glFogf)(GLenum pname, GLfloat param);
typedef void (*_glFogi)(GLenum pname, GLint param);
typedef void (*_glFogfv)(GLenum pname, const GLfloat *params);
typedef void (*_glFogiv)(GLenum pname, const GLint *params);
typedef void (*_glFeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer);
typedef void (*_glPassThrough)(GLfloat token);
typedef void (*_glSelectBuffer)(GLsizei size, GLuint *buffer);
typedef void (*_glInitNames)();
typedef void (*_glLoadName)(GLuint name);
typedef void (*_glPushName)(GLuint name);
typedef void (*_glPopName)();
typedef void (*_glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (*_glTexImage3D)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (*_glColorTable)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (*_glColorSubTable)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (*_glColorTableParameteriv)(GLenum target, GLenum pname, const GLint *params);
typedef void (*_glColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
typedef void (*_glCopyColorSubTable)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (*_glCopyColorTable)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (*_glGetColorTable)(GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (*_glGetColorTableParameterfv)(GLenum target, GLenum pname, GLfloat *params);
typedef void (*_glGetColorTableParameteriv)(GLenum target, GLenum pname, GLint *params);
typedef void (*_glBlendEquation)(GLenum mode);
typedef void (*_glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (*_glHistogram)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (*_glResetHistogram)(GLenum target);
typedef void (*_glGetHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (*_glGetHistogramParameterfv)(GLenum target, GLenum pname, GLfloat *params);
typedef void (*_glGetHistogramParameteriv)(GLenum target, GLenum pname, GLint *params);
typedef void (*_glMinmax)(GLenum target, GLenum internalformat, GLboolean sink);
typedef void (*_glResetMinmax)(GLenum target);
typedef void (*_glGetMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values);
typedef void (*_glGetMinmaxParameterfv)(GLenum target, GLenum pname, GLfloat *params);
typedef void (*_glGetMinmaxParameteriv)(GLenum target, GLenum pname, GLint *params);
typedef void (*_glConvolutionFilter1D)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (*_glConvolutionFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (*_glConvolutionParameterf)(GLenum target, GLenum pname, GLfloat params);
typedef void (*_glConvolutionParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
typedef void (*_glConvolutionParameteri)(GLenum target, GLenum pname, GLint params);
typedef void (*_glConvolutionParameteriv)(GLenum target, GLenum pname, const GLint *params);
typedef void (*_glCopyConvolutionFilter1D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (*_glCopyConvolutionFilter2D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (*_glGetConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (*_glGetConvolutionParameterfv)(GLenum target, GLenum pname, GLfloat *params);
typedef void (*_glGetConvolutionParameteriv)(GLenum target, GLenum pname, GLint *params);
typedef void (*_glSeparableFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (*_glGetSeparableFilter)(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef void (*_glActiveTexture)(GLenum texture);
typedef void (*_glClientActiveTexture)(GLenum texture);
typedef void (*_glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (*_glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (*_glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (*_glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (*_glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (*_glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (*_glGetCompressedTexImage)(GLenum target, GLint lod, GLvoid *img);
typedef void (*_glMultiTexCoord1d)(GLenum target, GLdouble s);
typedef void (*_glMultiTexCoord1dv)(GLenum target, const GLdouble *v);
typedef void (*_glMultiTexCoord1f)(GLenum target, GLfloat s);
typedef void (*_glMultiTexCoord1fv)(GLenum target, const GLfloat *v);
typedef void (*_glMultiTexCoord1i)(GLenum target, GLint s);
typedef void (*_glMultiTexCoord1iv)(GLenum target, const GLint *v);
typedef void (*_glMultiTexCoord1s)(GLenum target, GLshort s);
typedef void (*_glMultiTexCoord1sv)(GLenum target, const GLshort *v);
typedef void (*_glMultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t);
typedef void (*_glMultiTexCoord2dv)(GLenum target, const GLdouble *v);
typedef void (*_glMultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t);
typedef void (*_glMultiTexCoord2fv)(GLenum target, const GLfloat *v);
typedef void (*_glMultiTexCoord2i)(GLenum target, GLint s, GLint t);
typedef void (*_glMultiTexCoord2iv)(GLenum target, const GLint *v);
typedef void (*_glMultiTexCoord2s)(GLenum target, GLshort s, GLshort t);
typedef void (*_glMultiTexCoord2sv)(GLenum target, const GLshort *v);
typedef void (*_glMultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (*_glMultiTexCoord3dv)(GLenum target, const GLdouble *v);
typedef void (*_glMultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (*_glMultiTexCoord3fv)(GLenum target, const GLfloat *v);
typedef void (*_glMultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r);
typedef void (*_glMultiTexCoord3iv)(GLenum target, const GLint *v);
typedef void (*_glMultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (*_glMultiTexCoord3sv)(GLenum target, const GLshort *v);
typedef void (*_glMultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (*_glMultiTexCoord4dv)(GLenum target, const GLdouble *v);
typedef void (*_glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (*_glMultiTexCoord4fv)(GLenum target, const GLfloat *v);
typedef void (*_glMultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (*_glMultiTexCoord4iv)(GLenum target, const GLint *v);
typedef void (*_glMultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (*_glMultiTexCoord4sv)(GLenum target, const GLshort *v);
typedef void (*_glLoadTransposeMatrixd)(const GLdouble m[16]);
typedef void (*_glLoadTransposeMatrixf)(const GLfloat m[16]);
typedef void (*_glMultTransposeMatrixd)(const GLdouble m[16]);
typedef void (*_glMultTransposeMatrixf)(const GLfloat m[16]);
typedef void (*_glSampleCoverage)(GLclampf value, GLboolean invert);
typedef void (*_glActiveTextureARB)(GLenum texture);
typedef void (*_glClientActiveTextureARB)(GLenum texture);
typedef void (*_glMultiTexCoord1dARB)(GLenum target, GLdouble s);
typedef void (*_glMultiTexCoord1dvARB)(GLenum target, const GLdouble *v);
typedef void (*_glMultiTexCoord1fARB)(GLenum target, GLfloat s);
typedef void (*_glMultiTexCoord1fvARB)(GLenum target, const GLfloat *v);
typedef void (*_glMultiTexCoord1iARB)(GLenum target, GLint s);
typedef void (*_glMultiTexCoord1ivARB)(GLenum target, const GLint *v);
typedef void (*_glMultiTexCoord1sARB)(GLenum target, GLshort s);
typedef void (*_glMultiTexCoord1svARB)(GLenum target, const GLshort *v);
typedef void (*_glMultiTexCoord2dARB)(GLenum target, GLdouble s, GLdouble t);
typedef void (*_glMultiTexCoord2dvARB)(GLenum target, const GLdouble *v);
typedef void (*_glMultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t);
typedef void (*_glMultiTexCoord2fvARB)(GLenum target, const GLfloat *v);
typedef void (*_glMultiTexCoord2iARB)(GLenum target, GLint s, GLint t);
typedef void (*_glMultiTexCoord2ivARB)(GLenum target, const GLint *v);
typedef void (*_glMultiTexCoord2sARB)(GLenum target, GLshort s, GLshort t);
typedef void (*_glMultiTexCoord2svARB)(GLenum target, const GLshort *v);
typedef void (*_glMultiTexCoord3dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (*_glMultiTexCoord3dvARB)(GLenum target, const GLdouble *v);
typedef void (*_glMultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (*_glMultiTexCoord3fvARB)(GLenum target, const GLfloat *v);
typedef void (*_glMultiTexCoord3iARB)(GLenum target, GLint s, GLint t, GLint r);
typedef void (*_glMultiTexCoord3ivARB)(GLenum target, const GLint *v);
typedef void (*_glMultiTexCoord3sARB)(GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (*_glMultiTexCoord3svARB)(GLenum target, const GLshort *v);
typedef void (*_glMultiTexCoord4dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (*_glMultiTexCoord4dvARB)(GLenum target, const GLdouble *v);
typedef void (*_glMultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (*_glMultiTexCoord4fvARB)(GLenum target, const GLfloat *v);
typedef void (*_glMultiTexCoord4iARB)(GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (*_glMultiTexCoord4ivARB)(GLenum target, const GLint *v);
typedef void (*_glMultiTexCoord4sARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (*_glMultiTexCoord4svARB)(GLenum target, const GLshort *v);
typedef void (*_glBlendColorEXT)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (*_glPolygonOffsetEXT)(GLfloat factor, GLfloat bias);
typedef void (*_glTexImage3DEXT)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glTexSubImage3DEXT)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (*_glCopyTexSubImage3DEXT)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (*_glGenTexturesEXT)(GLsizei n, GLuint *textures);
typedef void (*_glDeleteTexturesEXT)(GLsizei n, const GLuint *textures);
typedef void (*_glBindTextureEXT)(GLenum target, GLuint texture);
typedef void (*_glPrioritizeTexturesEXT)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef GLboolean (*_glAreTexturesResidentEXT)(GLsizei n, const GLuint *textures, GLboolean *residences);
typedef GLboolean (*_glIsTextureEXT)(GLuint texture);
typedef void (*_glVertexPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
typedef void (*_glNormalPointerEXT)(GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
typedef void (*_glColorPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
typedef void (*_glIndexPointerEXT)(GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
typedef void (*_glTexCoordPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
typedef void (*_glEdgeFlagPointerEXT)(GLsizei stride, GLsizei count, const GLboolean *ptr);
typedef void (*_glGetPointervEXT)(GLenum pname, GLvoid **params);
typedef void (*_glArrayElementEXT)(GLint i);
typedef void (*_glDrawArraysEXT)(GLenum mode, GLint first, GLsizei count);
typedef void (*_glBlendEquationEXT)(GLenum mode);
typedef void (*_glPointParameterfEXT)(GLenum pname, GLfloat param);
typedef void (*_glPointParameterfvEXT)(GLenum pname, const GLfloat *params);
typedef void (*_glPointParameterfSGIS)(GLenum pname, GLfloat param);
typedef void (*_glPointParameterfvSGIS)(GLenum pname, const GLfloat *params);
typedef void (*_glColorTableEXT)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (*_glColorSubTableEXT)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (*_glGetColorTableEXT)(GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (*_glGetColorTableParameterfvEXT)(GLenum target, GLenum pname, GLfloat *params);
typedef void (*_glGetColorTableParameterivEXT)(GLenum target, GLenum pname, GLint *params);
typedef void (*_glLockArraysEXT)(GLint first, GLsizei count);
typedef void (*_glUnlockArraysEXT)();
typedef void (*_glWindowPos2iMESA)(GLint x, GLint y);
typedef void (*_glWindowPos2sMESA)(GLshort x, GLshort y);
typedef void (*_glWindowPos2fMESA)(GLfloat x, GLfloat y);
typedef void (*_glWindowPos2dMESA)(GLdouble x, GLdouble y);
typedef void (*_glWindowPos2ivMESA)(const GLint *p);
typedef void (*_glWindowPos2svMESA)(const GLshort *p);
typedef void (*_glWindowPos2fvMESA)(const GLfloat *p);
typedef void (*_glWindowPos2dvMESA)(const GLdouble *p);
typedef void (*_glWindowPos3iMESA)(GLint x, GLint y, GLint z);
typedef void (*_glWindowPos3sMESA)(GLshort x, GLshort y, GLshort z);
typedef void (*_glWindowPos3fMESA)(GLfloat x, GLfloat y, GLfloat z);
typedef void (*_glWindowPos3dMESA)(GLdouble x, GLdouble y, GLdouble z);
typedef void (*_glWindowPos3ivMESA)(const GLint *p);
typedef void (*_glWindowPos3svMESA)(const GLshort *p);
typedef void (*_glWindowPos3fvMESA)(const GLfloat *p);
typedef void (*_glWindowPos3dvMESA)(const GLdouble *p);
typedef void (*_glWindowPos4iMESA)(GLint x, GLint y, GLint z, GLint w);
typedef void (*_glWindowPos4sMESA)(GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (*_glWindowPos4fMESA)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (*_glWindowPos4dMESA)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (*_glWindowPos4ivMESA)(const GLint *p);
typedef void (*_glWindowPos4svMESA)(const GLshort *p);
typedef void (*_glWindowPos4fvMESA)(const GLfloat *p);
typedef void (*_glWindowPos4dvMESA)(const GLdouble *p);
typedef void (*_glResizeBuffersMESA)();
typedef void (*_glEnableTraceMESA)(GLbitfield mask);
typedef void (*_glDisableTraceMESA)(GLbitfield mask);
typedef void (*_glNewTraceMESA)(GLbitfield mask, const GLubyte* traceName);
typedef void (*_glEndTraceMESA)();
typedef void (*_glTraceAssertAttribMESA)(GLbitfield attribMask);
typedef void (*_glTraceCommentMESA)(const GLubyte* comment);
typedef void (*_glTraceTextureMESA)(GLuint name, const GLubyte* comment);
typedef void (*_glTraceListMESA)(GLuint name, const GLubyte* comment);
typedef void (*_glTracePointerMESA)(GLvoid* pointer, const GLubyte* comment);
typedef void (*_glTracePointerRangeMESA)(const GLvoid* first, const GLvoid* last, const GLubyte* comment);

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

// GL function pointers
extern _glClearIndex bo_glClearIndex;
extern _glClearColor bo_glClearColor;
extern _glClear bo_glClear;
extern _glIndexMask bo_glIndexMask;
extern _glColorMask bo_glColorMask;
extern _glAlphaFunc bo_glAlphaFunc;
extern _glBlendFunc bo_glBlendFunc;
extern _glLogicOp bo_glLogicOp;
extern _glCullFace bo_glCullFace;
extern _glFrontFace bo_glFrontFace;
extern _glPointSize bo_glPointSize;
extern _glLineWidth bo_glLineWidth;
extern _glLineStipple bo_glLineStipple;
extern _glPolygonMode bo_glPolygonMode;
extern _glPolygonOffset bo_glPolygonOffset;
extern _glPolygonStipple bo_glPolygonStipple;
extern _glGetPolygonStipple bo_glGetPolygonStipple;
extern _glEdgeFlag bo_glEdgeFlag;
extern _glEdgeFlagv bo_glEdgeFlagv;
extern _glScissor bo_glScissor;
extern _glClipPlane bo_glClipPlane;
extern _glGetClipPlane bo_glGetClipPlane;
extern _glDrawBuffer bo_glDrawBuffer;
extern _glReadBuffer bo_glReadBuffer;
extern _glEnable bo_glEnable;
extern _glDisable bo_glDisable;
extern _glIsEnabled bo_glIsEnabled;
extern _glEnableClientState bo_glEnableClientState;
extern _glDisableClientState bo_glDisableClientState;
extern _glGetBooleanv bo_glGetBooleanv;
extern _glGetDoublev bo_glGetDoublev;
extern _glGetFloatv bo_glGetFloatv;
extern _glGetIntegerv bo_glGetIntegerv;
extern _glPushAttrib bo_glPushAttrib;
extern _glPopAttrib bo_glPopAttrib;
extern _glPushClientAttrib bo_glPushClientAttrib;
extern _glPopClientAttrib bo_glPopClientAttrib;
extern _glRenderMode bo_glRenderMode;
extern _glGetError bo_glGetError;
extern _glGetString bo_glGetString;
extern _glFinish bo_glFinish;
extern _glFlush bo_glFlush;
extern _glHint bo_glHint;
extern _glClearDepth bo_glClearDepth;
extern _glDepthFunc bo_glDepthFunc;
extern _glDepthMask bo_glDepthMask;
extern _glDepthRange bo_glDepthRange;
extern _glClearAccum bo_glClearAccum;
extern _glAccum bo_glAccum;
extern _glMatrixMode bo_glMatrixMode;
extern _glOrtho bo_glOrtho;
extern _glFrustum bo_glFrustum;
extern _glViewport bo_glViewport;
extern _glPushMatrix bo_glPushMatrix;
extern _glPopMatrix bo_glPopMatrix;
extern _glLoadIdentity bo_glLoadIdentity;
extern _glLoadMatrixd bo_glLoadMatrixd;
extern _glLoadMatrixf bo_glLoadMatrixf;
extern _glMultMatrixd bo_glMultMatrixd;
extern _glMultMatrixf bo_glMultMatrixf;
extern _glRotated bo_glRotated;
extern _glRotatef bo_glRotatef;
extern _glScaled bo_glScaled;
extern _glScalef bo_glScalef;
extern _glTranslated bo_glTranslated;
extern _glTranslatef bo_glTranslatef;
extern _glIsList bo_glIsList;
extern _glDeleteLists bo_glDeleteLists;
extern _glGenLists bo_glGenLists;
extern _glNewList bo_glNewList;
extern _glEndList bo_glEndList;
extern _glCallList bo_glCallList;
extern _glCallLists bo_glCallLists;
extern _glListBase bo_glListBase;
extern _glBegin bo_glBegin;
extern _glEnd bo_glEnd;
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
extern _glRectd bo_glRectd;
extern _glRectf bo_glRectf;
extern _glRecti bo_glRecti;
extern _glRects bo_glRects;
extern _glRectdv bo_glRectdv;
extern _glRectfv bo_glRectfv;
extern _glRectiv bo_glRectiv;
extern _glRectsv bo_glRectsv;
extern _glVertexPointer bo_glVertexPointer;
extern _glNormalPointer bo_glNormalPointer;
extern _glColorPointer bo_glColorPointer;
extern _glIndexPointer bo_glIndexPointer;
extern _glTexCoordPointer bo_glTexCoordPointer;
extern _glEdgeFlagPointer bo_glEdgeFlagPointer;
extern _glGetPointerv bo_glGetPointerv;
extern _glArrayElement bo_glArrayElement;
extern _glDrawArrays bo_glDrawArrays;
extern _glDrawElements bo_glDrawElements;
extern _glInterleavedArrays bo_glInterleavedArrays;
extern _glShadeModel bo_glShadeModel;
extern _glLightf bo_glLightf;
extern _glLighti bo_glLighti;
extern _glLightfv bo_glLightfv;
extern _glLightiv bo_glLightiv;
extern _glGetLightfv bo_glGetLightfv;
extern _glGetLightiv bo_glGetLightiv;
extern _glLightModelf bo_glLightModelf;
extern _glLightModeli bo_glLightModeli;
extern _glLightModelfv bo_glLightModelfv;
extern _glLightModeliv bo_glLightModeliv;
extern _glMaterialf bo_glMaterialf;
extern _glMateriali bo_glMateriali;
extern _glMaterialfv bo_glMaterialfv;
extern _glMaterialiv bo_glMaterialiv;
extern _glGetMaterialfv bo_glGetMaterialfv;
extern _glGetMaterialiv bo_glGetMaterialiv;
extern _glColorMaterial bo_glColorMaterial;
extern _glPixelZoom bo_glPixelZoom;
extern _glPixelStoref bo_glPixelStoref;
extern _glPixelStorei bo_glPixelStorei;
extern _glPixelTransferf bo_glPixelTransferf;
extern _glPixelTransferi bo_glPixelTransferi;
extern _glPixelMapfv bo_glPixelMapfv;
extern _glPixelMapuiv bo_glPixelMapuiv;
extern _glPixelMapusv bo_glPixelMapusv;
extern _glGetPixelMapfv bo_glGetPixelMapfv;
extern _glGetPixelMapuiv bo_glGetPixelMapuiv;
extern _glGetPixelMapusv bo_glGetPixelMapusv;
extern _glBitmap bo_glBitmap;
extern _glReadPixels bo_glReadPixels;
extern _glDrawPixels bo_glDrawPixels;
extern _glCopyPixels bo_glCopyPixels;
extern _glStencilFunc bo_glStencilFunc;
extern _glStencilMask bo_glStencilMask;
extern _glStencilOp bo_glStencilOp;
extern _glClearStencil bo_glClearStencil;
extern _glTexGend bo_glTexGend;
extern _glTexGenf bo_glTexGenf;
extern _glTexGeni bo_glTexGeni;
extern _glTexGendv bo_glTexGendv;
extern _glTexGenfv bo_glTexGenfv;
extern _glTexGeniv bo_glTexGeniv;
extern _glGetTexGendv bo_glGetTexGendv;
extern _glGetTexGenfv bo_glGetTexGenfv;
extern _glGetTexGeniv bo_glGetTexGeniv;
extern _glTexEnvf bo_glTexEnvf;
extern _glTexEnvi bo_glTexEnvi;
extern _glTexEnvfv bo_glTexEnvfv;
extern _glTexEnviv bo_glTexEnviv;
extern _glGetTexEnvfv bo_glGetTexEnvfv;
extern _glGetTexEnviv bo_glGetTexEnviv;
extern _glTexParameterf bo_glTexParameterf;
extern _glTexParameteri bo_glTexParameteri;
extern _glTexParameterfv bo_glTexParameterfv;
extern _glTexParameteriv bo_glTexParameteriv;
extern _glGetTexParameterfv bo_glGetTexParameterfv;
extern _glGetTexParameteriv bo_glGetTexParameteriv;
extern _glGetTexLevelParameterfv bo_glGetTexLevelParameterfv;
extern _glGetTexLevelParameteriv bo_glGetTexLevelParameteriv;
extern _glTexImage1D bo_glTexImage1D;
extern _glTexImage2D bo_glTexImage2D;
extern _glGetTexImage bo_glGetTexImage;
extern _glGenTextures bo_glGenTextures;
extern _glDeleteTextures bo_glDeleteTextures;
extern _glBindTexture bo_glBindTexture;
extern _glPrioritizeTextures bo_glPrioritizeTextures;
extern _glAreTexturesResident bo_glAreTexturesResident;
extern _glIsTexture bo_glIsTexture;
extern _glTexSubImage1D bo_glTexSubImage1D;
extern _glTexSubImage2D bo_glTexSubImage2D;
extern _glCopyTexImage1D bo_glCopyTexImage1D;
extern _glCopyTexImage2D bo_glCopyTexImage2D;
extern _glCopyTexSubImage1D bo_glCopyTexSubImage1D;
extern _glCopyTexSubImage2D bo_glCopyTexSubImage2D;
extern _glMap1d bo_glMap1d;
extern _glMap1f bo_glMap1f;
extern _glMap2d bo_glMap2d;
extern _glMap2f bo_glMap2f;
extern _glGetMapdv bo_glGetMapdv;
extern _glGetMapfv bo_glGetMapfv;
extern _glGetMapiv bo_glGetMapiv;
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
extern _glEvalPoint1 bo_glEvalPoint1;
extern _glEvalPoint2 bo_glEvalPoint2;
extern _glEvalMesh1 bo_glEvalMesh1;
extern _glEvalMesh2 bo_glEvalMesh2;
extern _glFogf bo_glFogf;
extern _glFogi bo_glFogi;
extern _glFogfv bo_glFogfv;
extern _glFogiv bo_glFogiv;
extern _glFeedbackBuffer bo_glFeedbackBuffer;
extern _glPassThrough bo_glPassThrough;
extern _glSelectBuffer bo_glSelectBuffer;
extern _glInitNames bo_glInitNames;
extern _glLoadName bo_glLoadName;
extern _glPushName bo_glPushName;
extern _glPopName bo_glPopName;
extern _glDrawRangeElements bo_glDrawRangeElements;
extern _glTexImage3D bo_glTexImage3D;
extern _glTexSubImage3D bo_glTexSubImage3D;
extern _glCopyTexSubImage3D bo_glCopyTexSubImage3D;
extern _glColorTable bo_glColorTable;
extern _glColorSubTable bo_glColorSubTable;
extern _glColorTableParameteriv bo_glColorTableParameteriv;
extern _glColorTableParameterfv bo_glColorTableParameterfv;
extern _glCopyColorSubTable bo_glCopyColorSubTable;
extern _glCopyColorTable bo_glCopyColorTable;
extern _glGetColorTable bo_glGetColorTable;
extern _glGetColorTableParameterfv bo_glGetColorTableParameterfv;
extern _glGetColorTableParameteriv bo_glGetColorTableParameteriv;
extern _glBlendEquation bo_glBlendEquation;
extern _glBlendColor bo_glBlendColor;
extern _glHistogram bo_glHistogram;
extern _glResetHistogram bo_glResetHistogram;
extern _glGetHistogram bo_glGetHistogram;
extern _glGetHistogramParameterfv bo_glGetHistogramParameterfv;
extern _glGetHistogramParameteriv bo_glGetHistogramParameteriv;
extern _glMinmax bo_glMinmax;
extern _glResetMinmax bo_glResetMinmax;
extern _glGetMinmax bo_glGetMinmax;
extern _glGetMinmaxParameterfv bo_glGetMinmaxParameterfv;
extern _glGetMinmaxParameteriv bo_glGetMinmaxParameteriv;
extern _glConvolutionFilter1D bo_glConvolutionFilter1D;
extern _glConvolutionFilter2D bo_glConvolutionFilter2D;
extern _glConvolutionParameterf bo_glConvolutionParameterf;
extern _glConvolutionParameterfv bo_glConvolutionParameterfv;
extern _glConvolutionParameteri bo_glConvolutionParameteri;
extern _glConvolutionParameteriv bo_glConvolutionParameteriv;
extern _glCopyConvolutionFilter1D bo_glCopyConvolutionFilter1D;
extern _glCopyConvolutionFilter2D bo_glCopyConvolutionFilter2D;
extern _glGetConvolutionFilter bo_glGetConvolutionFilter;
extern _glGetConvolutionParameterfv bo_glGetConvolutionParameterfv;
extern _glGetConvolutionParameteriv bo_glGetConvolutionParameteriv;
extern _glSeparableFilter2D bo_glSeparableFilter2D;
extern _glGetSeparableFilter bo_glGetSeparableFilter;
extern _glActiveTexture bo_glActiveTexture;
extern _glClientActiveTexture bo_glClientActiveTexture;
extern _glCompressedTexImage1D bo_glCompressedTexImage1D;
extern _glCompressedTexImage2D bo_glCompressedTexImage2D;
extern _glCompressedTexImage3D bo_glCompressedTexImage3D;
extern _glCompressedTexSubImage1D bo_glCompressedTexSubImage1D;
extern _glCompressedTexSubImage2D bo_glCompressedTexSubImage2D;
extern _glCompressedTexSubImage3D bo_glCompressedTexSubImage3D;
extern _glGetCompressedTexImage bo_glGetCompressedTexImage;
extern _glMultiTexCoord1d bo_glMultiTexCoord1d;
extern _glMultiTexCoord1dv bo_glMultiTexCoord1dv;
extern _glMultiTexCoord1f bo_glMultiTexCoord1f;
extern _glMultiTexCoord1fv bo_glMultiTexCoord1fv;
extern _glMultiTexCoord1i bo_glMultiTexCoord1i;
extern _glMultiTexCoord1iv bo_glMultiTexCoord1iv;
extern _glMultiTexCoord1s bo_glMultiTexCoord1s;
extern _glMultiTexCoord1sv bo_glMultiTexCoord1sv;
extern _glMultiTexCoord2d bo_glMultiTexCoord2d;
extern _glMultiTexCoord2dv bo_glMultiTexCoord2dv;
extern _glMultiTexCoord2f bo_glMultiTexCoord2f;
extern _glMultiTexCoord2fv bo_glMultiTexCoord2fv;
extern _glMultiTexCoord2i bo_glMultiTexCoord2i;
extern _glMultiTexCoord2iv bo_glMultiTexCoord2iv;
extern _glMultiTexCoord2s bo_glMultiTexCoord2s;
extern _glMultiTexCoord2sv bo_glMultiTexCoord2sv;
extern _glMultiTexCoord3d bo_glMultiTexCoord3d;
extern _glMultiTexCoord3dv bo_glMultiTexCoord3dv;
extern _glMultiTexCoord3f bo_glMultiTexCoord3f;
extern _glMultiTexCoord3fv bo_glMultiTexCoord3fv;
extern _glMultiTexCoord3i bo_glMultiTexCoord3i;
extern _glMultiTexCoord3iv bo_glMultiTexCoord3iv;
extern _glMultiTexCoord3s bo_glMultiTexCoord3s;
extern _glMultiTexCoord3sv bo_glMultiTexCoord3sv;
extern _glMultiTexCoord4d bo_glMultiTexCoord4d;
extern _glMultiTexCoord4dv bo_glMultiTexCoord4dv;
extern _glMultiTexCoord4f bo_glMultiTexCoord4f;
extern _glMultiTexCoord4fv bo_glMultiTexCoord4fv;
extern _glMultiTexCoord4i bo_glMultiTexCoord4i;
extern _glMultiTexCoord4iv bo_glMultiTexCoord4iv;
extern _glMultiTexCoord4s bo_glMultiTexCoord4s;
extern _glMultiTexCoord4sv bo_glMultiTexCoord4sv;
extern _glLoadTransposeMatrixd bo_glLoadTransposeMatrixd;
extern _glLoadTransposeMatrixf bo_glLoadTransposeMatrixf;
extern _glMultTransposeMatrixd bo_glMultTransposeMatrixd;
extern _glMultTransposeMatrixf bo_glMultTransposeMatrixf;
extern _glSampleCoverage bo_glSampleCoverage;
extern _glActiveTextureARB bo_glActiveTextureARB;
extern _glClientActiveTextureARB bo_glClientActiveTextureARB;
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
extern _glBlendColorEXT bo_glBlendColorEXT;
extern _glPolygonOffsetEXT bo_glPolygonOffsetEXT;
extern _glTexImage3DEXT bo_glTexImage3DEXT;
extern _glTexSubImage3DEXT bo_glTexSubImage3DEXT;
extern _glCopyTexSubImage3DEXT bo_glCopyTexSubImage3DEXT;
extern _glGenTexturesEXT bo_glGenTexturesEXT;
extern _glDeleteTexturesEXT bo_glDeleteTexturesEXT;
extern _glBindTextureEXT bo_glBindTextureEXT;
extern _glPrioritizeTexturesEXT bo_glPrioritizeTexturesEXT;
extern _glAreTexturesResidentEXT bo_glAreTexturesResidentEXT;
extern _glIsTextureEXT bo_glIsTextureEXT;
extern _glVertexPointerEXT bo_glVertexPointerEXT;
extern _glNormalPointerEXT bo_glNormalPointerEXT;
extern _glColorPointerEXT bo_glColorPointerEXT;
extern _glIndexPointerEXT bo_glIndexPointerEXT;
extern _glTexCoordPointerEXT bo_glTexCoordPointerEXT;
extern _glEdgeFlagPointerEXT bo_glEdgeFlagPointerEXT;
extern _glGetPointervEXT bo_glGetPointervEXT;
extern _glArrayElementEXT bo_glArrayElementEXT;
extern _glDrawArraysEXT bo_glDrawArraysEXT;
extern _glBlendEquationEXT bo_glBlendEquationEXT;
extern _glPointParameterfEXT bo_glPointParameterfEXT;
extern _glPointParameterfvEXT bo_glPointParameterfvEXT;
extern _glPointParameterfSGIS bo_glPointParameterfSGIS;
extern _glPointParameterfvSGIS bo_glPointParameterfvSGIS;
extern _glColorTableEXT bo_glColorTableEXT;
extern _glColorSubTableEXT bo_glColorSubTableEXT;
extern _glGetColorTableEXT bo_glGetColorTableEXT;
extern _glGetColorTableParameterfvEXT bo_glGetColorTableParameterfvEXT;
extern _glGetColorTableParameterivEXT bo_glGetColorTableParameterivEXT;
extern _glLockArraysEXT bo_glLockArraysEXT;
extern _glUnlockArraysEXT bo_glUnlockArraysEXT;
extern _glWindowPos2iMESA bo_glWindowPos2iMESA;
extern _glWindowPos2sMESA bo_glWindowPos2sMESA;
extern _glWindowPos2fMESA bo_glWindowPos2fMESA;
extern _glWindowPos2dMESA bo_glWindowPos2dMESA;
extern _glWindowPos2ivMESA bo_glWindowPos2ivMESA;
extern _glWindowPos2svMESA bo_glWindowPos2svMESA;
extern _glWindowPos2fvMESA bo_glWindowPos2fvMESA;
extern _glWindowPos2dvMESA bo_glWindowPos2dvMESA;
extern _glWindowPos3iMESA bo_glWindowPos3iMESA;
extern _glWindowPos3sMESA bo_glWindowPos3sMESA;
extern _glWindowPos3fMESA bo_glWindowPos3fMESA;
extern _glWindowPos3dMESA bo_glWindowPos3dMESA;
extern _glWindowPos3ivMESA bo_glWindowPos3ivMESA;
extern _glWindowPos3svMESA bo_glWindowPos3svMESA;
extern _glWindowPos3fvMESA bo_glWindowPos3fvMESA;
extern _glWindowPos3dvMESA bo_glWindowPos3dvMESA;
extern _glWindowPos4iMESA bo_glWindowPos4iMESA;
extern _glWindowPos4sMESA bo_glWindowPos4sMESA;
extern _glWindowPos4fMESA bo_glWindowPos4fMESA;
extern _glWindowPos4dMESA bo_glWindowPos4dMESA;
extern _glWindowPos4ivMESA bo_glWindowPos4ivMESA;
extern _glWindowPos4svMESA bo_glWindowPos4svMESA;
extern _glWindowPos4fvMESA bo_glWindowPos4fvMESA;
extern _glWindowPos4dvMESA bo_glWindowPos4dvMESA;
extern _glResizeBuffersMESA bo_glResizeBuffersMESA;
extern _glEnableTraceMESA bo_glEnableTraceMESA;
extern _glDisableTraceMESA bo_glDisableTraceMESA;
extern _glNewTraceMESA bo_glNewTraceMESA;
extern _glEndTraceMESA bo_glEndTraceMESA;
extern _glTraceAssertAttribMESA bo_glTraceAssertAttribMESA;
extern _glTraceCommentMESA bo_glTraceCommentMESA;
extern _glTraceTextureMESA bo_glTraceTextureMESA;
extern _glTraceListMESA bo_glTraceListMESA;
extern _glTracePointerMESA bo_glTracePointerMESA;
extern _glTracePointerRangeMESA bo_glTracePointerRangeMESA;

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


// GL defines
#if BOGL_DO_DLOPEN
#define glClearIndex bo_glClearIndex
#define glClearColor bo_glClearColor
#define glClear bo_glClear
#define glIndexMask bo_glIndexMask
#define glColorMask bo_glColorMask
#define glAlphaFunc bo_glAlphaFunc
#define glBlendFunc bo_glBlendFunc
#define glLogicOp bo_glLogicOp
#define glCullFace bo_glCullFace
#define glFrontFace bo_glFrontFace
#define glPointSize bo_glPointSize
#define glLineWidth bo_glLineWidth
#define glLineStipple bo_glLineStipple
#define glPolygonMode bo_glPolygonMode
#define glPolygonOffset bo_glPolygonOffset
#define glPolygonStipple bo_glPolygonStipple
#define glGetPolygonStipple bo_glGetPolygonStipple
#define glEdgeFlag bo_glEdgeFlag
#define glEdgeFlagv bo_glEdgeFlagv
#define glScissor bo_glScissor
#define glClipPlane bo_glClipPlane
#define glGetClipPlane bo_glGetClipPlane
#define glDrawBuffer bo_glDrawBuffer
#define glReadBuffer bo_glReadBuffer
#define glEnable bo_glEnable
#define glDisable bo_glDisable
#define glIsEnabled bo_glIsEnabled
#define glEnableClientState bo_glEnableClientState
#define glDisableClientState bo_glDisableClientState
#define glGetBooleanv bo_glGetBooleanv
#define glGetDoublev bo_glGetDoublev
#define glGetFloatv bo_glGetFloatv
#define glGetIntegerv bo_glGetIntegerv
#define glPushAttrib bo_glPushAttrib
#define glPopAttrib bo_glPopAttrib
#define glPushClientAttrib bo_glPushClientAttrib
#define glPopClientAttrib bo_glPopClientAttrib
#define glRenderMode bo_glRenderMode
#define glGetError bo_glGetError
#define glGetString bo_glGetString
#define glFinish bo_glFinish
#define glFlush bo_glFlush
#define glHint bo_glHint
#define glClearDepth bo_glClearDepth
#define glDepthFunc bo_glDepthFunc
#define glDepthMask bo_glDepthMask
#define glDepthRange bo_glDepthRange
#define glClearAccum bo_glClearAccum
#define glAccum bo_glAccum
#define glMatrixMode bo_glMatrixMode
#define glOrtho bo_glOrtho
#define glFrustum bo_glFrustum
#define glViewport bo_glViewport
#define glPushMatrix bo_glPushMatrix
#define glPopMatrix bo_glPopMatrix
#define glLoadIdentity bo_glLoadIdentity
#define glLoadMatrixd bo_glLoadMatrixd
#define glLoadMatrixf bo_glLoadMatrixf
#define glMultMatrixd bo_glMultMatrixd
#define glMultMatrixf bo_glMultMatrixf
#define glRotated bo_glRotated
#define glRotatef bo_glRotatef
#define glScaled bo_glScaled
#define glScalef bo_glScalef
#define glTranslated bo_glTranslated
#define glTranslatef bo_glTranslatef
#define glIsList bo_glIsList
#define glDeleteLists bo_glDeleteLists
#define glGenLists bo_glGenLists
#define glNewList bo_glNewList
#define glEndList bo_glEndList
#define glCallList bo_glCallList
#define glCallLists bo_glCallLists
#define glListBase bo_glListBase
#define glBegin bo_glBegin
#define glEnd bo_glEnd
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
#define glRectd bo_glRectd
#define glRectf bo_glRectf
#define glRecti bo_glRecti
#define glRects bo_glRects
#define glRectdv bo_glRectdv
#define glRectfv bo_glRectfv
#define glRectiv bo_glRectiv
#define glRectsv bo_glRectsv
#define glVertexPointer bo_glVertexPointer
#define glNormalPointer bo_glNormalPointer
#define glColorPointer bo_glColorPointer
#define glIndexPointer bo_glIndexPointer
#define glTexCoordPointer bo_glTexCoordPointer
#define glEdgeFlagPointer bo_glEdgeFlagPointer
#define glGetPointerv bo_glGetPointerv
#define glArrayElement bo_glArrayElement
#define glDrawArrays bo_glDrawArrays
#define glDrawElements bo_glDrawElements
#define glInterleavedArrays bo_glInterleavedArrays
#define glShadeModel bo_glShadeModel
#define glLightf bo_glLightf
#define glLighti bo_glLighti
#define glLightfv bo_glLightfv
#define glLightiv bo_glLightiv
#define glGetLightfv bo_glGetLightfv
#define glGetLightiv bo_glGetLightiv
#define glLightModelf bo_glLightModelf
#define glLightModeli bo_glLightModeli
#define glLightModelfv bo_glLightModelfv
#define glLightModeliv bo_glLightModeliv
#define glMaterialf bo_glMaterialf
#define glMateriali bo_glMateriali
#define glMaterialfv bo_glMaterialfv
#define glMaterialiv bo_glMaterialiv
#define glGetMaterialfv bo_glGetMaterialfv
#define glGetMaterialiv bo_glGetMaterialiv
#define glColorMaterial bo_glColorMaterial
#define glPixelZoom bo_glPixelZoom
#define glPixelStoref bo_glPixelStoref
#define glPixelStorei bo_glPixelStorei
#define glPixelTransferf bo_glPixelTransferf
#define glPixelTransferi bo_glPixelTransferi
#define glPixelMapfv bo_glPixelMapfv
#define glPixelMapuiv bo_glPixelMapuiv
#define glPixelMapusv bo_glPixelMapusv
#define glGetPixelMapfv bo_glGetPixelMapfv
#define glGetPixelMapuiv bo_glGetPixelMapuiv
#define glGetPixelMapusv bo_glGetPixelMapusv
#define glBitmap bo_glBitmap
#define glReadPixels bo_glReadPixels
#define glDrawPixels bo_glDrawPixels
#define glCopyPixels bo_glCopyPixels
#define glStencilFunc bo_glStencilFunc
#define glStencilMask bo_glStencilMask
#define glStencilOp bo_glStencilOp
#define glClearStencil bo_glClearStencil
#define glTexGend bo_glTexGend
#define glTexGenf bo_glTexGenf
#define glTexGeni bo_glTexGeni
#define glTexGendv bo_glTexGendv
#define glTexGenfv bo_glTexGenfv
#define glTexGeniv bo_glTexGeniv
#define glGetTexGendv bo_glGetTexGendv
#define glGetTexGenfv bo_glGetTexGenfv
#define glGetTexGeniv bo_glGetTexGeniv
#define glTexEnvf bo_glTexEnvf
#define glTexEnvi bo_glTexEnvi
#define glTexEnvfv bo_glTexEnvfv
#define glTexEnviv bo_glTexEnviv
#define glGetTexEnvfv bo_glGetTexEnvfv
#define glGetTexEnviv bo_glGetTexEnviv
#define glTexParameterf bo_glTexParameterf
#define glTexParameteri bo_glTexParameteri
#define glTexParameterfv bo_glTexParameterfv
#define glTexParameteriv bo_glTexParameteriv
#define glGetTexParameterfv bo_glGetTexParameterfv
#define glGetTexParameteriv bo_glGetTexParameteriv
#define glGetTexLevelParameterfv bo_glGetTexLevelParameterfv
#define glGetTexLevelParameteriv bo_glGetTexLevelParameteriv
#define glTexImage1D bo_glTexImage1D
#define glTexImage2D bo_glTexImage2D
#define glGetTexImage bo_glGetTexImage
#define glGenTextures bo_glGenTextures
#define glDeleteTextures bo_glDeleteTextures
#define glBindTexture bo_glBindTexture
#define glPrioritizeTextures bo_glPrioritizeTextures
#define glAreTexturesResident bo_glAreTexturesResident
#define glIsTexture bo_glIsTexture
#define glTexSubImage1D bo_glTexSubImage1D
#define glTexSubImage2D bo_glTexSubImage2D
#define glCopyTexImage1D bo_glCopyTexImage1D
#define glCopyTexImage2D bo_glCopyTexImage2D
#define glCopyTexSubImage1D bo_glCopyTexSubImage1D
#define glCopyTexSubImage2D bo_glCopyTexSubImage2D
#define glMap1d bo_glMap1d
#define glMap1f bo_glMap1f
#define glMap2d bo_glMap2d
#define glMap2f bo_glMap2f
#define glGetMapdv bo_glGetMapdv
#define glGetMapfv bo_glGetMapfv
#define glGetMapiv bo_glGetMapiv
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
#define glEvalPoint1 bo_glEvalPoint1
#define glEvalPoint2 bo_glEvalPoint2
#define glEvalMesh1 bo_glEvalMesh1
#define glEvalMesh2 bo_glEvalMesh2
#define glFogf bo_glFogf
#define glFogi bo_glFogi
#define glFogfv bo_glFogfv
#define glFogiv bo_glFogiv
#define glFeedbackBuffer bo_glFeedbackBuffer
#define glPassThrough bo_glPassThrough
#define glSelectBuffer bo_glSelectBuffer
#define glInitNames bo_glInitNames
#define glLoadName bo_glLoadName
#define glPushName bo_glPushName
#define glPopName bo_glPopName
#define glDrawRangeElements bo_glDrawRangeElements
#define glTexImage3D bo_glTexImage3D
#define glTexSubImage3D bo_glTexSubImage3D
#define glCopyTexSubImage3D bo_glCopyTexSubImage3D
#define glColorTable bo_glColorTable
#define glColorSubTable bo_glColorSubTable
#define glColorTableParameteriv bo_glColorTableParameteriv
#define glColorTableParameterfv bo_glColorTableParameterfv
#define glCopyColorSubTable bo_glCopyColorSubTable
#define glCopyColorTable bo_glCopyColorTable
#define glGetColorTable bo_glGetColorTable
#define glGetColorTableParameterfv bo_glGetColorTableParameterfv
#define glGetColorTableParameteriv bo_glGetColorTableParameteriv
#define glBlendEquation bo_glBlendEquation
#define glBlendColor bo_glBlendColor
#define glHistogram bo_glHistogram
#define glResetHistogram bo_glResetHistogram
#define glGetHistogram bo_glGetHistogram
#define glGetHistogramParameterfv bo_glGetHistogramParameterfv
#define glGetHistogramParameteriv bo_glGetHistogramParameteriv
#define glMinmax bo_glMinmax
#define glResetMinmax bo_glResetMinmax
#define glGetMinmax bo_glGetMinmax
#define glGetMinmaxParameterfv bo_glGetMinmaxParameterfv
#define glGetMinmaxParameteriv bo_glGetMinmaxParameteriv
#define glConvolutionFilter1D bo_glConvolutionFilter1D
#define glConvolutionFilter2D bo_glConvolutionFilter2D
#define glConvolutionParameterf bo_glConvolutionParameterf
#define glConvolutionParameterfv bo_glConvolutionParameterfv
#define glConvolutionParameteri bo_glConvolutionParameteri
#define glConvolutionParameteriv bo_glConvolutionParameteriv
#define glCopyConvolutionFilter1D bo_glCopyConvolutionFilter1D
#define glCopyConvolutionFilter2D bo_glCopyConvolutionFilter2D
#define glGetConvolutionFilter bo_glGetConvolutionFilter
#define glGetConvolutionParameterfv bo_glGetConvolutionParameterfv
#define glGetConvolutionParameteriv bo_glGetConvolutionParameteriv
#define glSeparableFilter2D bo_glSeparableFilter2D
#define glGetSeparableFilter bo_glGetSeparableFilter
#define glActiveTexture bo_glActiveTexture
#define glClientActiveTexture bo_glClientActiveTexture
#define glCompressedTexImage1D bo_glCompressedTexImage1D
#define glCompressedTexImage2D bo_glCompressedTexImage2D
#define glCompressedTexImage3D bo_glCompressedTexImage3D
#define glCompressedTexSubImage1D bo_glCompressedTexSubImage1D
#define glCompressedTexSubImage2D bo_glCompressedTexSubImage2D
#define glCompressedTexSubImage3D bo_glCompressedTexSubImage3D
#define glGetCompressedTexImage bo_glGetCompressedTexImage
#define glMultiTexCoord1d bo_glMultiTexCoord1d
#define glMultiTexCoord1dv bo_glMultiTexCoord1dv
#define glMultiTexCoord1f bo_glMultiTexCoord1f
#define glMultiTexCoord1fv bo_glMultiTexCoord1fv
#define glMultiTexCoord1i bo_glMultiTexCoord1i
#define glMultiTexCoord1iv bo_glMultiTexCoord1iv
#define glMultiTexCoord1s bo_glMultiTexCoord1s
#define glMultiTexCoord1sv bo_glMultiTexCoord1sv
#define glMultiTexCoord2d bo_glMultiTexCoord2d
#define glMultiTexCoord2dv bo_glMultiTexCoord2dv
#define glMultiTexCoord2f bo_glMultiTexCoord2f
#define glMultiTexCoord2fv bo_glMultiTexCoord2fv
#define glMultiTexCoord2i bo_glMultiTexCoord2i
#define glMultiTexCoord2iv bo_glMultiTexCoord2iv
#define glMultiTexCoord2s bo_glMultiTexCoord2s
#define glMultiTexCoord2sv bo_glMultiTexCoord2sv
#define glMultiTexCoord3d bo_glMultiTexCoord3d
#define glMultiTexCoord3dv bo_glMultiTexCoord3dv
#define glMultiTexCoord3f bo_glMultiTexCoord3f
#define glMultiTexCoord3fv bo_glMultiTexCoord3fv
#define glMultiTexCoord3i bo_glMultiTexCoord3i
#define glMultiTexCoord3iv bo_glMultiTexCoord3iv
#define glMultiTexCoord3s bo_glMultiTexCoord3s
#define glMultiTexCoord3sv bo_glMultiTexCoord3sv
#define glMultiTexCoord4d bo_glMultiTexCoord4d
#define glMultiTexCoord4dv bo_glMultiTexCoord4dv
#define glMultiTexCoord4f bo_glMultiTexCoord4f
#define glMultiTexCoord4fv bo_glMultiTexCoord4fv
#define glMultiTexCoord4i bo_glMultiTexCoord4i
#define glMultiTexCoord4iv bo_glMultiTexCoord4iv
#define glMultiTexCoord4s bo_glMultiTexCoord4s
#define glMultiTexCoord4sv bo_glMultiTexCoord4sv
#define glLoadTransposeMatrixd bo_glLoadTransposeMatrixd
#define glLoadTransposeMatrixf bo_glLoadTransposeMatrixf
#define glMultTransposeMatrixd bo_glMultTransposeMatrixd
#define glMultTransposeMatrixf bo_glMultTransposeMatrixf
#define glSampleCoverage bo_glSampleCoverage
#define glActiveTextureARB bo_glActiveTextureARB
#define glClientActiveTextureARB bo_glClientActiveTextureARB
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
#define glBlendColorEXT bo_glBlendColorEXT
#define glPolygonOffsetEXT bo_glPolygonOffsetEXT
#define glTexImage3DEXT bo_glTexImage3DEXT
#define glTexSubImage3DEXT bo_glTexSubImage3DEXT
#define glCopyTexSubImage3DEXT bo_glCopyTexSubImage3DEXT
#define glGenTexturesEXT bo_glGenTexturesEXT
#define glDeleteTexturesEXT bo_glDeleteTexturesEXT
#define glBindTextureEXT bo_glBindTextureEXT
#define glPrioritizeTexturesEXT bo_glPrioritizeTexturesEXT
#define glAreTexturesResidentEXT bo_glAreTexturesResidentEXT
#define glIsTextureEXT bo_glIsTextureEXT
#define glVertexPointerEXT bo_glVertexPointerEXT
#define glNormalPointerEXT bo_glNormalPointerEXT
#define glColorPointerEXT bo_glColorPointerEXT
#define glIndexPointerEXT bo_glIndexPointerEXT
#define glTexCoordPointerEXT bo_glTexCoordPointerEXT
#define glEdgeFlagPointerEXT bo_glEdgeFlagPointerEXT
#define glGetPointervEXT bo_glGetPointervEXT
#define glArrayElementEXT bo_glArrayElementEXT
#define glDrawArraysEXT bo_glDrawArraysEXT
#define glBlendEquationEXT bo_glBlendEquationEXT
#define glPointParameterfEXT bo_glPointParameterfEXT
#define glPointParameterfvEXT bo_glPointParameterfvEXT
#define glPointParameterfSGIS bo_glPointParameterfSGIS
#define glPointParameterfvSGIS bo_glPointParameterfvSGIS
#define glColorTableEXT bo_glColorTableEXT
#define glColorSubTableEXT bo_glColorSubTableEXT
#define glGetColorTableEXT bo_glGetColorTableEXT
#define glGetColorTableParameterfvEXT bo_glGetColorTableParameterfvEXT
#define glGetColorTableParameterivEXT bo_glGetColorTableParameterivEXT
#define glLockArraysEXT bo_glLockArraysEXT
#define glUnlockArraysEXT bo_glUnlockArraysEXT
#define glWindowPos2iMESA bo_glWindowPos2iMESA
#define glWindowPos2sMESA bo_glWindowPos2sMESA
#define glWindowPos2fMESA bo_glWindowPos2fMESA
#define glWindowPos2dMESA bo_glWindowPos2dMESA
#define glWindowPos2ivMESA bo_glWindowPos2ivMESA
#define glWindowPos2svMESA bo_glWindowPos2svMESA
#define glWindowPos2fvMESA bo_glWindowPos2fvMESA
#define glWindowPos2dvMESA bo_glWindowPos2dvMESA
#define glWindowPos3iMESA bo_glWindowPos3iMESA
#define glWindowPos3sMESA bo_glWindowPos3sMESA
#define glWindowPos3fMESA bo_glWindowPos3fMESA
#define glWindowPos3dMESA bo_glWindowPos3dMESA
#define glWindowPos3ivMESA bo_glWindowPos3ivMESA
#define glWindowPos3svMESA bo_glWindowPos3svMESA
#define glWindowPos3fvMESA bo_glWindowPos3fvMESA
#define glWindowPos3dvMESA bo_glWindowPos3dvMESA
#define glWindowPos4iMESA bo_glWindowPos4iMESA
#define glWindowPos4sMESA bo_glWindowPos4sMESA
#define glWindowPos4fMESA bo_glWindowPos4fMESA
#define glWindowPos4dMESA bo_glWindowPos4dMESA
#define glWindowPos4ivMESA bo_glWindowPos4ivMESA
#define glWindowPos4svMESA bo_glWindowPos4svMESA
#define glWindowPos4fvMESA bo_glWindowPos4fvMESA
#define glWindowPos4dvMESA bo_glWindowPos4dvMESA
#define glResizeBuffersMESA bo_glResizeBuffersMESA
#define glEnableTraceMESA bo_glEnableTraceMESA
#define glDisableTraceMESA bo_glDisableTraceMESA
#define glNewTraceMESA bo_glNewTraceMESA
#define glEndTraceMESA bo_glEndTraceMESA
#define glTraceAssertAttribMESA bo_glTraceAssertAttribMESA
#define glTraceCommentMESA bo_glTraceCommentMESA
#define glTraceTextureMESA bo_glTraceTextureMESA
#define glTraceListMESA bo_glTraceListMESA
#define glTracePointerMESA bo_glTracePointerMESA
#define glTracePointerRangeMESA bo_glTracePointerRangeMESA


// GLU defines
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
#endif // BOGL_DO_DLOPEN

#endif // BOGLDECL_P_H
