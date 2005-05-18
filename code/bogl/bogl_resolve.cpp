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

#include "bogl.h"
#include "boglx.h"

#include "bodebug.h"

#include <qlibrary.h>

static bool boglResolveOpenGLSymbols(QLibrary& gl);
static bool boglResolveGLUSymbols(QLibrary& glu);
static bool boglResolveGLXSymbols(QLibrary& gl);

extern "C" {
 // GL function pointers
 _glClearIndex bo_glClearIndex;
 _glClearColor bo_glClearColor;
 _glClear bo_glClear;
 _glIndexMask bo_glIndexMask;
 _glColorMask bo_glColorMask;
 _glAlphaFunc bo_glAlphaFunc;
 _glBlendFunc bo_glBlendFunc;
 _glLogicOp bo_glLogicOp;
 _glCullFace bo_glCullFace;
 _glFrontFace bo_glFrontFace;
 _glPointSize bo_glPointSize;
 _glLineWidth bo_glLineWidth;
 _glLineStipple bo_glLineStipple;
 _glPolygonMode bo_glPolygonMode;
 _glPolygonOffset bo_glPolygonOffset;
 _glPolygonStipple bo_glPolygonStipple;
 _glGetPolygonStipple bo_glGetPolygonStipple;
 _glEdgeFlag bo_glEdgeFlag;
 _glEdgeFlagv bo_glEdgeFlagv;
 _glScissor bo_glScissor;
 _glClipPlane bo_glClipPlane;
 _glGetClipPlane bo_glGetClipPlane;
 _glDrawBuffer bo_glDrawBuffer;
 _glReadBuffer bo_glReadBuffer;
 _glEnable bo_glEnable;
 _glDisable bo_glDisable;
 _glIsEnabled bo_glIsEnabled;
 _glEnableClientState bo_glEnableClientState;
 _glDisableClientState bo_glDisableClientState;
 _glGetBooleanv bo_glGetBooleanv;
 _glGetDoublev bo_glGetDoublev;
 _glGetFloatv bo_glGetFloatv;
 _glGetIntegerv bo_glGetIntegerv;
 _glPushAttrib bo_glPushAttrib;
 _glPopAttrib bo_glPopAttrib;
 _glPushClientAttrib bo_glPushClientAttrib;
 _glPopClientAttrib bo_glPopClientAttrib;
 _glRenderMode bo_glRenderMode;
 _glGetError bo_glGetError;
 _glGetString bo_glGetString;
 _glFinish bo_glFinish;
 _glFlush bo_glFlush;
 _glHint bo_glHint;
 _glClearDepth bo_glClearDepth;
 _glDepthFunc bo_glDepthFunc;
 _glDepthMask bo_glDepthMask;
 _glDepthRange bo_glDepthRange;
 _glClearAccum bo_glClearAccum;
 _glAccum bo_glAccum;
 _glMatrixMode bo_glMatrixMode;
 _glOrtho bo_glOrtho;
 _glFrustum bo_glFrustum;
 _glViewport bo_glViewport;
 _glPushMatrix bo_glPushMatrix;
 _glPopMatrix bo_glPopMatrix;
 _glLoadIdentity bo_glLoadIdentity;
 _glLoadMatrixd bo_glLoadMatrixd;
 _glLoadMatrixf bo_glLoadMatrixf;
 _glMultMatrixd bo_glMultMatrixd;
 _glMultMatrixf bo_glMultMatrixf;
 _glRotated bo_glRotated;
 _glRotatef bo_glRotatef;
 _glScaled bo_glScaled;
 _glScalef bo_glScalef;
 _glTranslated bo_glTranslated;
 _glTranslatef bo_glTranslatef;
 _glIsList bo_glIsList;
 _glDeleteLists bo_glDeleteLists;
 _glGenLists bo_glGenLists;
 _glNewList bo_glNewList;
 _glEndList bo_glEndList;
 _glCallList bo_glCallList;
 _glCallLists bo_glCallLists;
 _glListBase bo_glListBase;
 _glBegin bo_glBegin;
 _glEnd bo_glEnd;
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
 _glRectd bo_glRectd;
 _glRectf bo_glRectf;
 _glRecti bo_glRecti;
 _glRects bo_glRects;
 _glRectdv bo_glRectdv;
 _glRectfv bo_glRectfv;
 _glRectiv bo_glRectiv;
 _glRectsv bo_glRectsv;
 _glVertexPointer bo_glVertexPointer;
 _glNormalPointer bo_glNormalPointer;
 _glColorPointer bo_glColorPointer;
 _glIndexPointer bo_glIndexPointer;
 _glTexCoordPointer bo_glTexCoordPointer;
 _glEdgeFlagPointer bo_glEdgeFlagPointer;
 _glGetPointerv bo_glGetPointerv;
 _glArrayElement bo_glArrayElement;
 _glDrawArrays bo_glDrawArrays;
 _glDrawElements bo_glDrawElements;
 _glInterleavedArrays bo_glInterleavedArrays;
 _glShadeModel bo_glShadeModel;
 _glLightf bo_glLightf;
 _glLighti bo_glLighti;
 _glLightfv bo_glLightfv;
 _glLightiv bo_glLightiv;
 _glGetLightfv bo_glGetLightfv;
 _glGetLightiv bo_glGetLightiv;
 _glLightModelf bo_glLightModelf;
 _glLightModeli bo_glLightModeli;
 _glLightModelfv bo_glLightModelfv;
 _glLightModeliv bo_glLightModeliv;
 _glMaterialf bo_glMaterialf;
 _glMateriali bo_glMateriali;
 _glMaterialfv bo_glMaterialfv;
 _glMaterialiv bo_glMaterialiv;
 _glGetMaterialfv bo_glGetMaterialfv;
 _glGetMaterialiv bo_glGetMaterialiv;
 _glColorMaterial bo_glColorMaterial;
 _glPixelZoom bo_glPixelZoom;
 _glPixelStoref bo_glPixelStoref;
 _glPixelStorei bo_glPixelStorei;
 _glPixelTransferf bo_glPixelTransferf;
 _glPixelTransferi bo_glPixelTransferi;
 _glPixelMapfv bo_glPixelMapfv;
 _glPixelMapuiv bo_glPixelMapuiv;
 _glPixelMapusv bo_glPixelMapusv;
 _glGetPixelMapfv bo_glGetPixelMapfv;
 _glGetPixelMapuiv bo_glGetPixelMapuiv;
 _glGetPixelMapusv bo_glGetPixelMapusv;
 _glBitmap bo_glBitmap;
 _glReadPixels bo_glReadPixels;
 _glDrawPixels bo_glDrawPixels;
 _glCopyPixels bo_glCopyPixels;
 _glStencilFunc bo_glStencilFunc;
 _glStencilMask bo_glStencilMask;
 _glStencilOp bo_glStencilOp;
 _glClearStencil bo_glClearStencil;
 _glTexGend bo_glTexGend;
 _glTexGenf bo_glTexGenf;
 _glTexGeni bo_glTexGeni;
 _glTexGendv bo_glTexGendv;
 _glTexGenfv bo_glTexGenfv;
 _glTexGeniv bo_glTexGeniv;
 _glGetTexGendv bo_glGetTexGendv;
 _glGetTexGenfv bo_glGetTexGenfv;
 _glGetTexGeniv bo_glGetTexGeniv;
 _glTexEnvf bo_glTexEnvf;
 _glTexEnvi bo_glTexEnvi;
 _glTexEnvfv bo_glTexEnvfv;
 _glTexEnviv bo_glTexEnviv;
 _glGetTexEnvfv bo_glGetTexEnvfv;
 _glGetTexEnviv bo_glGetTexEnviv;
 _glTexParameterf bo_glTexParameterf;
 _glTexParameteri bo_glTexParameteri;
 _glTexParameterfv bo_glTexParameterfv;
 _glTexParameteriv bo_glTexParameteriv;
 _glGetTexParameterfv bo_glGetTexParameterfv;
 _glGetTexParameteriv bo_glGetTexParameteriv;
 _glGetTexLevelParameterfv bo_glGetTexLevelParameterfv;
 _glGetTexLevelParameteriv bo_glGetTexLevelParameteriv;
 _glTexImage1D bo_glTexImage1D;
 _glTexImage2D bo_glTexImage2D;
 _glGetTexImage bo_glGetTexImage;
 _glGenTextures bo_glGenTextures;
 _glDeleteTextures bo_glDeleteTextures;
 _glBindTexture bo_glBindTexture;
 _glPrioritizeTextures bo_glPrioritizeTextures;
 _glAreTexturesResident bo_glAreTexturesResident;
 _glIsTexture bo_glIsTexture;
 _glTexSubImage1D bo_glTexSubImage1D;
 _glTexSubImage2D bo_glTexSubImage2D;
 _glCopyTexImage1D bo_glCopyTexImage1D;
 _glCopyTexImage2D bo_glCopyTexImage2D;
 _glCopyTexSubImage1D bo_glCopyTexSubImage1D;
 _glCopyTexSubImage2D bo_glCopyTexSubImage2D;
 _glMap1d bo_glMap1d;
 _glMap1f bo_glMap1f;
 _glMap2d bo_glMap2d;
 _glMap2f bo_glMap2f;
 _glGetMapdv bo_glGetMapdv;
 _glGetMapfv bo_glGetMapfv;
 _glGetMapiv bo_glGetMapiv;
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
 _glEvalPoint1 bo_glEvalPoint1;
 _glEvalPoint2 bo_glEvalPoint2;
 _glEvalMesh1 bo_glEvalMesh1;
 _glEvalMesh2 bo_glEvalMesh2;
 _glFogf bo_glFogf;
 _glFogi bo_glFogi;
 _glFogfv bo_glFogfv;
 _glFogiv bo_glFogiv;
 _glFeedbackBuffer bo_glFeedbackBuffer;
 _glPassThrough bo_glPassThrough;
 _glSelectBuffer bo_glSelectBuffer;
 _glInitNames bo_glInitNames;
 _glLoadName bo_glLoadName;
 _glPushName bo_glPushName;
 _glPopName bo_glPopName;
 _glDrawRangeElements bo_glDrawRangeElements;
 _glTexImage3D bo_glTexImage3D;
 _glTexSubImage3D bo_glTexSubImage3D;
 _glCopyTexSubImage3D bo_glCopyTexSubImage3D;
 _glColorTable bo_glColorTable;
 _glColorSubTable bo_glColorSubTable;
 _glColorTableParameteriv bo_glColorTableParameteriv;
 _glColorTableParameterfv bo_glColorTableParameterfv;
 _glCopyColorSubTable bo_glCopyColorSubTable;
 _glCopyColorTable bo_glCopyColorTable;
 _glGetColorTable bo_glGetColorTable;
 _glGetColorTableParameterfv bo_glGetColorTableParameterfv;
 _glGetColorTableParameteriv bo_glGetColorTableParameteriv;
 _glBlendEquation bo_glBlendEquation;
 _glBlendColor bo_glBlendColor;
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
 _glConvolutionFilter1D bo_glConvolutionFilter1D;
 _glConvolutionFilter2D bo_glConvolutionFilter2D;
 _glConvolutionParameterf bo_glConvolutionParameterf;
 _glConvolutionParameterfv bo_glConvolutionParameterfv;
 _glConvolutionParameteri bo_glConvolutionParameteri;
 _glConvolutionParameteriv bo_glConvolutionParameteriv;
 _glCopyConvolutionFilter1D bo_glCopyConvolutionFilter1D;
 _glCopyConvolutionFilter2D bo_glCopyConvolutionFilter2D;
 _glGetConvolutionFilter bo_glGetConvolutionFilter;
 _glGetConvolutionParameterfv bo_glGetConvolutionParameterfv;
 _glGetConvolutionParameteriv bo_glGetConvolutionParameteriv;
 _glSeparableFilter2D bo_glSeparableFilter2D;
 _glGetSeparableFilter bo_glGetSeparableFilter;
 _glActiveTexture bo_glActiveTexture;
 _glClientActiveTexture bo_glClientActiveTexture;
 _glCompressedTexImage1D bo_glCompressedTexImage1D;
 _glCompressedTexImage2D bo_glCompressedTexImage2D;
 _glCompressedTexImage3D bo_glCompressedTexImage3D;
 _glCompressedTexSubImage1D bo_glCompressedTexSubImage1D;
 _glCompressedTexSubImage2D bo_glCompressedTexSubImage2D;
 _glCompressedTexSubImage3D bo_glCompressedTexSubImage3D;
 _glGetCompressedTexImage bo_glGetCompressedTexImage;
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
 _glLoadTransposeMatrixd bo_glLoadTransposeMatrixd;
 _glLoadTransposeMatrixf bo_glLoadTransposeMatrixf;
 _glMultTransposeMatrixd bo_glMultTransposeMatrixd;
 _glMultTransposeMatrixf bo_glMultTransposeMatrixf;
 _glSampleCoverage bo_glSampleCoverage;
 _glActiveTextureARB bo_glActiveTextureARB;
 _glClientActiveTextureARB bo_glClientActiveTextureARB;
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
 _glBlendColorEXT bo_glBlendColorEXT;
 _glPolygonOffsetEXT bo_glPolygonOffsetEXT;
 _glTexImage3DEXT bo_glTexImage3DEXT;
 _glTexSubImage3DEXT bo_glTexSubImage3DEXT;
 _glCopyTexSubImage3DEXT bo_glCopyTexSubImage3DEXT;
 _glGenTexturesEXT bo_glGenTexturesEXT;
 _glDeleteTexturesEXT bo_glDeleteTexturesEXT;
 _glBindTextureEXT bo_glBindTextureEXT;
 _glPrioritizeTexturesEXT bo_glPrioritizeTexturesEXT;
 _glAreTexturesResidentEXT bo_glAreTexturesResidentEXT;
 _glIsTextureEXT bo_glIsTextureEXT;
 _glVertexPointerEXT bo_glVertexPointerEXT;
 _glNormalPointerEXT bo_glNormalPointerEXT;
 _glColorPointerEXT bo_glColorPointerEXT;
 _glIndexPointerEXT bo_glIndexPointerEXT;
 _glTexCoordPointerEXT bo_glTexCoordPointerEXT;
 _glEdgeFlagPointerEXT bo_glEdgeFlagPointerEXT;
 _glGetPointervEXT bo_glGetPointervEXT;
 _glArrayElementEXT bo_glArrayElementEXT;
 _glDrawArraysEXT bo_glDrawArraysEXT;
 _glBlendEquationEXT bo_glBlendEquationEXT;
 _glPointParameterfEXT bo_glPointParameterfEXT;
 _glPointParameterfvEXT bo_glPointParameterfvEXT;
 _glPointParameterfSGIS bo_glPointParameterfSGIS;
 _glPointParameterfvSGIS bo_glPointParameterfvSGIS;
 _glColorTableEXT bo_glColorTableEXT;
 _glColorSubTableEXT bo_glColorSubTableEXT;
 _glGetColorTableEXT bo_glGetColorTableEXT;
 _glGetColorTableParameterfvEXT bo_glGetColorTableParameterfvEXT;
 _glGetColorTableParameterivEXT bo_glGetColorTableParameterivEXT;
 _glLockArraysEXT bo_glLockArraysEXT;
 _glUnlockArraysEXT bo_glUnlockArraysEXT;
 _glWindowPos2iMESA bo_glWindowPos2iMESA;
 _glWindowPos2sMESA bo_glWindowPos2sMESA;
 _glWindowPos2fMESA bo_glWindowPos2fMESA;
 _glWindowPos2dMESA bo_glWindowPos2dMESA;
 _glWindowPos2ivMESA bo_glWindowPos2ivMESA;
 _glWindowPos2svMESA bo_glWindowPos2svMESA;
 _glWindowPos2fvMESA bo_glWindowPos2fvMESA;
 _glWindowPos2dvMESA bo_glWindowPos2dvMESA;
 _glWindowPos3iMESA bo_glWindowPos3iMESA;
 _glWindowPos3sMESA bo_glWindowPos3sMESA;
 _glWindowPos3fMESA bo_glWindowPos3fMESA;
 _glWindowPos3dMESA bo_glWindowPos3dMESA;
 _glWindowPos3ivMESA bo_glWindowPos3ivMESA;
 _glWindowPos3svMESA bo_glWindowPos3svMESA;
 _glWindowPos3fvMESA bo_glWindowPos3fvMESA;
 _glWindowPos3dvMESA bo_glWindowPos3dvMESA;
 _glWindowPos4iMESA bo_glWindowPos4iMESA;
 _glWindowPos4sMESA bo_glWindowPos4sMESA;
 _glWindowPos4fMESA bo_glWindowPos4fMESA;
 _glWindowPos4dMESA bo_glWindowPos4dMESA;
 _glWindowPos4ivMESA bo_glWindowPos4ivMESA;
 _glWindowPos4svMESA bo_glWindowPos4svMESA;
 _glWindowPos4fvMESA bo_glWindowPos4fvMESA;
 _glWindowPos4dvMESA bo_glWindowPos4dvMESA;
 _glResizeBuffersMESA bo_glResizeBuffersMESA;
 _glEnableTraceMESA bo_glEnableTraceMESA;
 _glDisableTraceMESA bo_glDisableTraceMESA;
 _glNewTraceMESA bo_glNewTraceMESA;
 _glEndTraceMESA bo_glEndTraceMESA;
 _glTraceAssertAttribMESA bo_glTraceAssertAttribMESA;
 _glTraceCommentMESA bo_glTraceCommentMESA;
 _glTraceTextureMESA bo_glTraceTextureMESA;
 _glTraceListMESA bo_glTraceListMESA;
 _glTracePointerMESA bo_glTracePointerMESA;
 _glTracePointerRangeMESA bo_glTracePointerRangeMESA;

 // GLU function pointers
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


 // GLX function pointers
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

bool boglResolveGLSymbols()
{
 static bool symbolsResolved = false;
 if (symbolsResolved) {
	return true;
 }
 QLibrary gl("GL");
 gl.setAutoUnload(false);

 if (!boglResolveOpenGLSymbols(gl)) {
	return false;
 }
 if (!boglResolveGLXSymbols(gl)) {
	return false;
 }

 // TODO: this should load from the application, as GLU is linked in statically.
 // can we actually do so safely? is some flag like -export-dynamic required for
 // the linker?
 // TODO: maybe provide a copy of libGLU.so from mesa in our package?
 QLibrary glu("GLU");
 glu.setAutoUnload(false);

 if (!boglResolveGLUSymbols(glu)) {
	return false;
 }
 return true;
}

static bool boglResolveOpenGLSymbols(QLibrary& gl)
{
 bo_glClearIndex = (_glClearIndex)gl.resolve("glClearIndex");
 if (bo_glClearIndex == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glClearIndex" << endl;
	return false;
 }
 bo_glClearColor = (_glClearColor)gl.resolve("glClearColor");
 if (bo_glClearColor == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glClearColor" << endl;
	return false;
 }
 bo_glClear = (_glClear)gl.resolve("glClear");
 if (bo_glClear == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glClear" << endl;
	return false;
 }
 bo_glIndexMask = (_glIndexMask)gl.resolve("glIndexMask");
 if (bo_glIndexMask == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glIndexMask" << endl;
	return false;
 }
 bo_glColorMask = (_glColorMask)gl.resolve("glColorMask");
 if (bo_glColorMask == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glColorMask" << endl;
	return false;
 }
 bo_glAlphaFunc = (_glAlphaFunc)gl.resolve("glAlphaFunc");
 if (bo_glAlphaFunc == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glAlphaFunc" << endl;
	return false;
 }
 bo_glBlendFunc = (_glBlendFunc)gl.resolve("glBlendFunc");
 if (bo_glBlendFunc == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glBlendFunc" << endl;
	return false;
 }
 bo_glLogicOp = (_glLogicOp)gl.resolve("glLogicOp");
 if (bo_glLogicOp == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glLogicOp" << endl;
	return false;
 }
 bo_glCullFace = (_glCullFace)gl.resolve("glCullFace");
 if (bo_glCullFace == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glCullFace" << endl;
	return false;
 }
 bo_glFrontFace = (_glFrontFace)gl.resolve("glFrontFace");
 if (bo_glFrontFace == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glFrontFace" << endl;
	return false;
 }
 bo_glPointSize = (_glPointSize)gl.resolve("glPointSize");
 if (bo_glPointSize == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glPointSize" << endl;
	return false;
 }
 bo_glLineWidth = (_glLineWidth)gl.resolve("glLineWidth");
 if (bo_glLineWidth == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glLineWidth" << endl;
	return false;
 }
 bo_glLineStipple = (_glLineStipple)gl.resolve("glLineStipple");
 if (bo_glLineStipple == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glLineStipple" << endl;
	return false;
 }
 bo_glPolygonMode = (_glPolygonMode)gl.resolve("glPolygonMode");
 if (bo_glPolygonMode == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glPolygonMode" << endl;
	return false;
 }
 bo_glPolygonOffset = (_glPolygonOffset)gl.resolve("glPolygonOffset");
 if (bo_glPolygonOffset == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glPolygonOffset" << endl;
	return false;
 }
 bo_glPolygonStipple = (_glPolygonStipple)gl.resolve("glPolygonStipple");
 if (bo_glPolygonStipple == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glPolygonStipple" << endl;
	return false;
 }
 bo_glGetPolygonStipple = (_glGetPolygonStipple)gl.resolve("glGetPolygonStipple");
 if (bo_glGetPolygonStipple == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glGetPolygonStipple" << endl;
	return false;
 }
 bo_glEdgeFlag = (_glEdgeFlag)gl.resolve("glEdgeFlag");
 if (bo_glEdgeFlag == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glEdgeFlag" << endl;
	return false;
 }
 bo_glEdgeFlagv = (_glEdgeFlagv)gl.resolve("glEdgeFlagv");
 if (bo_glEdgeFlagv == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glEdgeFlagv" << endl;
	return false;
 }
 bo_glScissor = (_glScissor)gl.resolve("glScissor");
 if (bo_glScissor == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glScissor" << endl;
	return false;
 }
 bo_glClipPlane = (_glClipPlane)gl.resolve("glClipPlane");
 if (bo_glClipPlane == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glClipPlane" << endl;
	return false;
 }
 bo_glGetClipPlane = (_glGetClipPlane)gl.resolve("glGetClipPlane");
 if (bo_glGetClipPlane == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glGetClipPlane" << endl;
	return false;
 }
 bo_glDrawBuffer = (_glDrawBuffer)gl.resolve("glDrawBuffer");
 if (bo_glDrawBuffer == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glDrawBuffer" << endl;
	return false;
 }
 bo_glReadBuffer = (_glReadBuffer)gl.resolve("glReadBuffer");
 if (bo_glReadBuffer == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glReadBuffer" << endl;
	return false;
 }
 bo_glEnable = (_glEnable)gl.resolve("glEnable");
 if (bo_glEnable == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glEnable" << endl;
	return false;
 }
 bo_glDisable = (_glDisable)gl.resolve("glDisable");
 if (bo_glDisable == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glDisable" << endl;
	return false;
 }
 bo_glIsEnabled = (_glIsEnabled)gl.resolve("glIsEnabled");
 if (bo_glIsEnabled == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glIsEnabled" << endl;
	return false;
 }
 bo_glEnableClientState = (_glEnableClientState)gl.resolve("glEnableClientState");
 if (bo_glEnableClientState == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glEnableClientState" << endl;
	return false;
 }
 bo_glDisableClientState = (_glDisableClientState)gl.resolve("glDisableClientState");
 if (bo_glDisableClientState == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glDisableClientState" << endl;
	return false;
 }
 bo_glGetBooleanv = (_glGetBooleanv)gl.resolve("glGetBooleanv");
 if (bo_glGetBooleanv == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glGetBooleanv" << endl;
	return false;
 }
 bo_glGetDoublev = (_glGetDoublev)gl.resolve("glGetDoublev");
 if (bo_glGetDoublev == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glGetDoublev" << endl;
	return false;
 }
 bo_glGetFloatv = (_glGetFloatv)gl.resolve("glGetFloatv");
 if (bo_glGetFloatv == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glGetFloatv" << endl;
	return false;
 }
 bo_glGetIntegerv = (_glGetIntegerv)gl.resolve("glGetIntegerv");
 if (bo_glGetIntegerv == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glGetIntegerv" << endl;
	return false;
 }
 bo_glPushAttrib = (_glPushAttrib)gl.resolve("glPushAttrib");
 if (bo_glPushAttrib == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glPushAttrib" << endl;
	return false;
 }
 bo_glPopAttrib = (_glPopAttrib)gl.resolve("glPopAttrib");
 if (bo_glPopAttrib == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glPopAttrib" << endl;
	return false;
 }
 bo_glPushClientAttrib = (_glPushClientAttrib)gl.resolve("glPushClientAttrib");
 if (bo_glPushClientAttrib == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glPushClientAttrib" << endl;
	return false;
 }
 bo_glPopClientAttrib = (_glPopClientAttrib)gl.resolve("glPopClientAttrib");
 if (bo_glPopClientAttrib == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glPopClientAttrib" << endl;
	return false;
 }
 bo_glRenderMode = (_glRenderMode)gl.resolve("glRenderMode");
 if (bo_glRenderMode == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glRenderMode" << endl;
	return false;
 }
 bo_glGetError = (_glGetError)gl.resolve("glGetError");
 if (bo_glGetError == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glGetError" << endl;
	return false;
 }
 bo_glGetString = (_glGetString)gl.resolve("glGetString");
 if (bo_glGetString == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glGetString" << endl;
	return false;
 }
 bo_glFinish = (_glFinish)gl.resolve("glFinish");
 if (bo_glFinish == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glFinish" << endl;
	return false;
 }
 bo_glFlush = (_glFlush)gl.resolve("glFlush");
 if (bo_glFlush == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glFlush" << endl;
	return false;
 }
 bo_glHint = (_glHint)gl.resolve("glHint");
 if (bo_glHint == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glHint" << endl;
	return false;
 }
 bo_glClearDepth = (_glClearDepth)gl.resolve("glClearDepth");
 if (bo_glClearDepth == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glClearDepth" << endl;
	return false;
 }
 // AB: from now on we assume that resolving succeeds. there is not need to
 // check _all_ symbols.
 // -> speed reasons
 //    (note that this point is absolutely random)
 bo_glDepthFunc = (_glDepthFunc)gl.resolve("glDepthFunc");
 bo_glDepthMask = (_glDepthMask)gl.resolve("glDepthMask");
 bo_glDepthRange = (_glDepthRange)gl.resolve("glDepthRange");
 bo_glClearAccum = (_glClearAccum)gl.resolve("glClearAccum");
 bo_glAccum = (_glAccum)gl.resolve("glAccum");
 bo_glMatrixMode = (_glMatrixMode)gl.resolve("glMatrixMode");
 bo_glOrtho = (_glOrtho)gl.resolve("glOrtho");
 bo_glFrustum = (_glFrustum)gl.resolve("glFrustum");
 bo_glViewport = (_glViewport)gl.resolve("glViewport");
 bo_glPushMatrix = (_glPushMatrix)gl.resolve("glPushMatrix");
 bo_glPopMatrix = (_glPopMatrix)gl.resolve("glPopMatrix");
 bo_glLoadIdentity = (_glLoadIdentity)gl.resolve("glLoadIdentity");
 bo_glLoadMatrixd = (_glLoadMatrixd)gl.resolve("glLoadMatrixd");
 bo_glLoadMatrixf = (_glLoadMatrixf)gl.resolve("glLoadMatrixf");
 bo_glMultMatrixd = (_glMultMatrixd)gl.resolve("glMultMatrixd");
 bo_glMultMatrixf = (_glMultMatrixf)gl.resolve("glMultMatrixf");
 bo_glRotated = (_glRotated)gl.resolve("glRotated");
 bo_glRotatef = (_glRotatef)gl.resolve("glRotatef");
 bo_glScaled = (_glScaled)gl.resolve("glScaled");
 bo_glScalef = (_glScalef)gl.resolve("glScalef");
 bo_glTranslated = (_glTranslated)gl.resolve("glTranslated");
 bo_glTranslatef = (_glTranslatef)gl.resolve("glTranslatef");
 bo_glIsList = (_glIsList)gl.resolve("glIsList");
 bo_glDeleteLists = (_glDeleteLists)gl.resolve("glDeleteLists");
 bo_glGenLists = (_glGenLists)gl.resolve("glGenLists");
 bo_glNewList = (_glNewList)gl.resolve("glNewList");
 bo_glEndList = (_glEndList)gl.resolve("glEndList");
 bo_glCallList = (_glCallList)gl.resolve("glCallList");
 bo_glCallLists = (_glCallLists)gl.resolve("glCallLists");
 bo_glListBase = (_glListBase)gl.resolve("glListBase");
 bo_glBegin = (_glBegin)gl.resolve("glBegin");
 bo_glEnd = (_glEnd)gl.resolve("glEnd");
 bo_glVertex2d = (_glVertex2d)gl.resolve("glVertex2d");
 bo_glVertex2f = (_glVertex2f)gl.resolve("glVertex2f");
 bo_glVertex2i = (_glVertex2i)gl.resolve("glVertex2i");
 bo_glVertex2s = (_glVertex2s)gl.resolve("glVertex2s");
 bo_glVertex3d = (_glVertex3d)gl.resolve("glVertex3d");
 bo_glVertex3f = (_glVertex3f)gl.resolve("glVertex3f");
 bo_glVertex3i = (_glVertex3i)gl.resolve("glVertex3i");
 bo_glVertex3s = (_glVertex3s)gl.resolve("glVertex3s");
 bo_glVertex4d = (_glVertex4d)gl.resolve("glVertex4d");
 bo_glVertex4f = (_glVertex4f)gl.resolve("glVertex4f");
 bo_glVertex4i = (_glVertex4i)gl.resolve("glVertex4i");
 bo_glVertex4s = (_glVertex4s)gl.resolve("glVertex4s");
 bo_glVertex2dv = (_glVertex2dv)gl.resolve("glVertex2dv");
 bo_glVertex2fv = (_glVertex2fv)gl.resolve("glVertex2fv");
 bo_glVertex2iv = (_glVertex2iv)gl.resolve("glVertex2iv");
 bo_glVertex2sv = (_glVertex2sv)gl.resolve("glVertex2sv");
 bo_glVertex3dv = (_glVertex3dv)gl.resolve("glVertex3dv");
 bo_glVertex3fv = (_glVertex3fv)gl.resolve("glVertex3fv");
 bo_glVertex3iv = (_glVertex3iv)gl.resolve("glVertex3iv");
 bo_glVertex3sv = (_glVertex3sv)gl.resolve("glVertex3sv");
 bo_glVertex4dv = (_glVertex4dv)gl.resolve("glVertex4dv");
 bo_glVertex4fv = (_glVertex4fv)gl.resolve("glVertex4fv");
 bo_glVertex4iv = (_glVertex4iv)gl.resolve("glVertex4iv");
 bo_glVertex4sv = (_glVertex4sv)gl.resolve("glVertex4sv");
 bo_glNormal3b = (_glNormal3b)gl.resolve("glNormal3b");
 bo_glNormal3d = (_glNormal3d)gl.resolve("glNormal3d");
 bo_glNormal3f = (_glNormal3f)gl.resolve("glNormal3f");
 bo_glNormal3i = (_glNormal3i)gl.resolve("glNormal3i");
 bo_glNormal3s = (_glNormal3s)gl.resolve("glNormal3s");
 bo_glNormal3bv = (_glNormal3bv)gl.resolve("glNormal3bv");
 bo_glNormal3dv = (_glNormal3dv)gl.resolve("glNormal3dv");
 bo_glNormal3fv = (_glNormal3fv)gl.resolve("glNormal3fv");
 bo_glNormal3iv = (_glNormal3iv)gl.resolve("glNormal3iv");
 bo_glNormal3sv = (_glNormal3sv)gl.resolve("glNormal3sv");
 bo_glIndexd = (_glIndexd)gl.resolve("glIndexd");
 bo_glIndexf = (_glIndexf)gl.resolve("glIndexf");
 bo_glIndexi = (_glIndexi)gl.resolve("glIndexi");
 bo_glIndexs = (_glIndexs)gl.resolve("glIndexs");
 bo_glIndexub = (_glIndexub)gl.resolve("glIndexub");
 bo_glIndexdv = (_glIndexdv)gl.resolve("glIndexdv");
 bo_glIndexfv = (_glIndexfv)gl.resolve("glIndexfv");
 bo_glIndexiv = (_glIndexiv)gl.resolve("glIndexiv");
 bo_glIndexsv = (_glIndexsv)gl.resolve("glIndexsv");
 bo_glIndexubv = (_glIndexubv)gl.resolve("glIndexubv");
 bo_glColor3b = (_glColor3b)gl.resolve("glColor3b");
 bo_glColor3d = (_glColor3d)gl.resolve("glColor3d");
 bo_glColor3f = (_glColor3f)gl.resolve("glColor3f");
 bo_glColor3i = (_glColor3i)gl.resolve("glColor3i");
 bo_glColor3s = (_glColor3s)gl.resolve("glColor3s");
 bo_glColor3ub = (_glColor3ub)gl.resolve("glColor3ub");
 bo_glColor3ui = (_glColor3ui)gl.resolve("glColor3ui");
 bo_glColor3us = (_glColor3us)gl.resolve("glColor3us");
 bo_glColor4b = (_glColor4b)gl.resolve("glColor4b");
 bo_glColor4d = (_glColor4d)gl.resolve("glColor4d");
 bo_glColor4f = (_glColor4f)gl.resolve("glColor4f");
 bo_glColor4i = (_glColor4i)gl.resolve("glColor4i");
 bo_glColor4s = (_glColor4s)gl.resolve("glColor4s");
 bo_glColor4ub = (_glColor4ub)gl.resolve("glColor4ub");
 bo_glColor4ui = (_glColor4ui)gl.resolve("glColor4ui");
 bo_glColor4us = (_glColor4us)gl.resolve("glColor4us");
 bo_glColor3bv = (_glColor3bv)gl.resolve("glColor3bv");
 bo_glColor3dv = (_glColor3dv)gl.resolve("glColor3dv");
 bo_glColor3fv = (_glColor3fv)gl.resolve("glColor3fv");
 bo_glColor3iv = (_glColor3iv)gl.resolve("glColor3iv");
 bo_glColor3sv = (_glColor3sv)gl.resolve("glColor3sv");
 bo_glColor3ubv = (_glColor3ubv)gl.resolve("glColor3ubv");
 bo_glColor3uiv = (_glColor3uiv)gl.resolve("glColor3uiv");
 bo_glColor3usv = (_glColor3usv)gl.resolve("glColor3usv");
 bo_glColor4bv = (_glColor4bv)gl.resolve("glColor4bv");
 bo_glColor4dv = (_glColor4dv)gl.resolve("glColor4dv");
 bo_glColor4fv = (_glColor4fv)gl.resolve("glColor4fv");
 bo_glColor4iv = (_glColor4iv)gl.resolve("glColor4iv");
 bo_glColor4sv = (_glColor4sv)gl.resolve("glColor4sv");
 bo_glColor4ubv = (_glColor4ubv)gl.resolve("glColor4ubv");
 bo_glColor4uiv = (_glColor4uiv)gl.resolve("glColor4uiv");
 bo_glColor4usv = (_glColor4usv)gl.resolve("glColor4usv");
 bo_glTexCoord1d = (_glTexCoord1d)gl.resolve("glTexCoord1d");
 bo_glTexCoord1f = (_glTexCoord1f)gl.resolve("glTexCoord1f");
 bo_glTexCoord1i = (_glTexCoord1i)gl.resolve("glTexCoord1i");
 bo_glTexCoord1s = (_glTexCoord1s)gl.resolve("glTexCoord1s");
 bo_glTexCoord2d = (_glTexCoord2d)gl.resolve("glTexCoord2d");
 bo_glTexCoord2f = (_glTexCoord2f)gl.resolve("glTexCoord2f");
 bo_glTexCoord2i = (_glTexCoord2i)gl.resolve("glTexCoord2i");
 bo_glTexCoord2s = (_glTexCoord2s)gl.resolve("glTexCoord2s");
 bo_glTexCoord3d = (_glTexCoord3d)gl.resolve("glTexCoord3d");
 bo_glTexCoord3f = (_glTexCoord3f)gl.resolve("glTexCoord3f");
 bo_glTexCoord3i = (_glTexCoord3i)gl.resolve("glTexCoord3i");
 bo_glTexCoord3s = (_glTexCoord3s)gl.resolve("glTexCoord3s");
 bo_glTexCoord4d = (_glTexCoord4d)gl.resolve("glTexCoord4d");
 bo_glTexCoord4f = (_glTexCoord4f)gl.resolve("glTexCoord4f");
 bo_glTexCoord4i = (_glTexCoord4i)gl.resolve("glTexCoord4i");
 bo_glTexCoord4s = (_glTexCoord4s)gl.resolve("glTexCoord4s");
 bo_glTexCoord1dv = (_glTexCoord1dv)gl.resolve("glTexCoord1dv");
 bo_glTexCoord1fv = (_glTexCoord1fv)gl.resolve("glTexCoord1fv");
 bo_glTexCoord1iv = (_glTexCoord1iv)gl.resolve("glTexCoord1iv");
 bo_glTexCoord1sv = (_glTexCoord1sv)gl.resolve("glTexCoord1sv");
 bo_glTexCoord2dv = (_glTexCoord2dv)gl.resolve("glTexCoord2dv");
 bo_glTexCoord2fv = (_glTexCoord2fv)gl.resolve("glTexCoord2fv");
 bo_glTexCoord2iv = (_glTexCoord2iv)gl.resolve("glTexCoord2iv");
 bo_glTexCoord2sv = (_glTexCoord2sv)gl.resolve("glTexCoord2sv");
 bo_glTexCoord3dv = (_glTexCoord3dv)gl.resolve("glTexCoord3dv");
 bo_glTexCoord3fv = (_glTexCoord3fv)gl.resolve("glTexCoord3fv");
 bo_glTexCoord3iv = (_glTexCoord3iv)gl.resolve("glTexCoord3iv");
 bo_glTexCoord3sv = (_glTexCoord3sv)gl.resolve("glTexCoord3sv");
 bo_glTexCoord4dv = (_glTexCoord4dv)gl.resolve("glTexCoord4dv");
 bo_glTexCoord4fv = (_glTexCoord4fv)gl.resolve("glTexCoord4fv");
 bo_glTexCoord4iv = (_glTexCoord4iv)gl.resolve("glTexCoord4iv");
 bo_glTexCoord4sv = (_glTexCoord4sv)gl.resolve("glTexCoord4sv");
 bo_glRasterPos2d = (_glRasterPos2d)gl.resolve("glRasterPos2d");
 bo_glRasterPos2f = (_glRasterPos2f)gl.resolve("glRasterPos2f");
 bo_glRasterPos2i = (_glRasterPos2i)gl.resolve("glRasterPos2i");
 bo_glRasterPos2s = (_glRasterPos2s)gl.resolve("glRasterPos2s");
 bo_glRasterPos3d = (_glRasterPos3d)gl.resolve("glRasterPos3d");
 bo_glRasterPos3f = (_glRasterPos3f)gl.resolve("glRasterPos3f");
 bo_glRasterPos3i = (_glRasterPos3i)gl.resolve("glRasterPos3i");
 bo_glRasterPos3s = (_glRasterPos3s)gl.resolve("glRasterPos3s");
 bo_glRasterPos4d = (_glRasterPos4d)gl.resolve("glRasterPos4d");
 bo_glRasterPos4f = (_glRasterPos4f)gl.resolve("glRasterPos4f");
 bo_glRasterPos4i = (_glRasterPos4i)gl.resolve("glRasterPos4i");
 bo_glRasterPos4s = (_glRasterPos4s)gl.resolve("glRasterPos4s");
 bo_glRasterPos2dv = (_glRasterPos2dv)gl.resolve("glRasterPos2dv");
 bo_glRasterPos2fv = (_glRasterPos2fv)gl.resolve("glRasterPos2fv");
 bo_glRasterPos2iv = (_glRasterPos2iv)gl.resolve("glRasterPos2iv");
 bo_glRasterPos2sv = (_glRasterPos2sv)gl.resolve("glRasterPos2sv");
 bo_glRasterPos3dv = (_glRasterPos3dv)gl.resolve("glRasterPos3dv");
 bo_glRasterPos3fv = (_glRasterPos3fv)gl.resolve("glRasterPos3fv");
 bo_glRasterPos3iv = (_glRasterPos3iv)gl.resolve("glRasterPos3iv");
 bo_glRasterPos3sv = (_glRasterPos3sv)gl.resolve("glRasterPos3sv");
 bo_glRasterPos4dv = (_glRasterPos4dv)gl.resolve("glRasterPos4dv");
 bo_glRasterPos4fv = (_glRasterPos4fv)gl.resolve("glRasterPos4fv");
 bo_glRasterPos4iv = (_glRasterPos4iv)gl.resolve("glRasterPos4iv");
 bo_glRasterPos4sv = (_glRasterPos4sv)gl.resolve("glRasterPos4sv");
 bo_glRectd = (_glRectd)gl.resolve("glRectd");
 bo_glRectf = (_glRectf)gl.resolve("glRectf");
 bo_glRecti = (_glRecti)gl.resolve("glRecti");
 bo_glRects = (_glRects)gl.resolve("glRects");
 bo_glRectdv = (_glRectdv)gl.resolve("glRectdv");
 bo_glRectfv = (_glRectfv)gl.resolve("glRectfv");
 bo_glRectiv = (_glRectiv)gl.resolve("glRectiv");
 bo_glRectsv = (_glRectsv)gl.resolve("glRectsv");
 bo_glVertexPointer = (_glVertexPointer)gl.resolve("glVertexPointer");
 bo_glNormalPointer = (_glNormalPointer)gl.resolve("glNormalPointer");
 bo_glColorPointer = (_glColorPointer)gl.resolve("glColorPointer");
 bo_glIndexPointer = (_glIndexPointer)gl.resolve("glIndexPointer");
 bo_glTexCoordPointer = (_glTexCoordPointer)gl.resolve("glTexCoordPointer");
 bo_glEdgeFlagPointer = (_glEdgeFlagPointer)gl.resolve("glEdgeFlagPointer");
 bo_glGetPointerv = (_glGetPointerv)gl.resolve("glGetPointerv");
 bo_glArrayElement = (_glArrayElement)gl.resolve("glArrayElement");
 bo_glDrawArrays = (_glDrawArrays)gl.resolve("glDrawArrays");
 bo_glDrawElements = (_glDrawElements)gl.resolve("glDrawElements");
 bo_glInterleavedArrays = (_glInterleavedArrays)gl.resolve("glInterleavedArrays");
 bo_glShadeModel = (_glShadeModel)gl.resolve("glShadeModel");
 bo_glLightf = (_glLightf)gl.resolve("glLightf");
 bo_glLighti = (_glLighti)gl.resolve("glLighti");
 bo_glLightfv = (_glLightfv)gl.resolve("glLightfv");
 bo_glLightiv = (_glLightiv)gl.resolve("glLightiv");
 bo_glGetLightfv = (_glGetLightfv)gl.resolve("glGetLightfv");
 bo_glGetLightiv = (_glGetLightiv)gl.resolve("glGetLightiv");
 bo_glLightModelf = (_glLightModelf)gl.resolve("glLightModelf");
 bo_glLightModeli = (_glLightModeli)gl.resolve("glLightModeli");
 bo_glLightModelfv = (_glLightModelfv)gl.resolve("glLightModelfv");
 bo_glLightModeliv = (_glLightModeliv)gl.resolve("glLightModeliv");
 bo_glMaterialf = (_glMaterialf)gl.resolve("glMaterialf");
 bo_glMateriali = (_glMateriali)gl.resolve("glMateriali");
 bo_glMaterialfv = (_glMaterialfv)gl.resolve("glMaterialfv");
 bo_glMaterialiv = (_glMaterialiv)gl.resolve("glMaterialiv");
 bo_glGetMaterialfv = (_glGetMaterialfv)gl.resolve("glGetMaterialfv");
 bo_glGetMaterialiv = (_glGetMaterialiv)gl.resolve("glGetMaterialiv");
 bo_glColorMaterial = (_glColorMaterial)gl.resolve("glColorMaterial");
 bo_glPixelZoom = (_glPixelZoom)gl.resolve("glPixelZoom");
 bo_glPixelStoref = (_glPixelStoref)gl.resolve("glPixelStoref");
 bo_glPixelStorei = (_glPixelStorei)gl.resolve("glPixelStorei");
 bo_glPixelTransferf = (_glPixelTransferf)gl.resolve("glPixelTransferf");
 bo_glPixelTransferi = (_glPixelTransferi)gl.resolve("glPixelTransferi");
 bo_glPixelMapfv = (_glPixelMapfv)gl.resolve("glPixelMapfv");
 bo_glPixelMapuiv = (_glPixelMapuiv)gl.resolve("glPixelMapuiv");
 bo_glPixelMapusv = (_glPixelMapusv)gl.resolve("glPixelMapusv");
 bo_glGetPixelMapfv = (_glGetPixelMapfv)gl.resolve("glGetPixelMapfv");
 bo_glGetPixelMapuiv = (_glGetPixelMapuiv)gl.resolve("glGetPixelMapuiv");
 bo_glGetPixelMapusv = (_glGetPixelMapusv)gl.resolve("glGetPixelMapusv");
 bo_glBitmap = (_glBitmap)gl.resolve("glBitmap");
 bo_glReadPixels = (_glReadPixels)gl.resolve("glReadPixels");
 bo_glDrawPixels = (_glDrawPixels)gl.resolve("glDrawPixels");
 bo_glCopyPixels = (_glCopyPixels)gl.resolve("glCopyPixels");
 bo_glStencilFunc = (_glStencilFunc)gl.resolve("glStencilFunc");
 bo_glStencilMask = (_glStencilMask)gl.resolve("glStencilMask");
 bo_glStencilOp = (_glStencilOp)gl.resolve("glStencilOp");
 bo_glClearStencil = (_glClearStencil)gl.resolve("glClearStencil");
 bo_glTexGend = (_glTexGend)gl.resolve("glTexGend");
 bo_glTexGenf = (_glTexGenf)gl.resolve("glTexGenf");
 bo_glTexGeni = (_glTexGeni)gl.resolve("glTexGeni");
 bo_glTexGendv = (_glTexGendv)gl.resolve("glTexGendv");
 bo_glTexGenfv = (_glTexGenfv)gl.resolve("glTexGenfv");
 bo_glTexGeniv = (_glTexGeniv)gl.resolve("glTexGeniv");
 bo_glGetTexGendv = (_glGetTexGendv)gl.resolve("glGetTexGendv");
 bo_glGetTexGenfv = (_glGetTexGenfv)gl.resolve("glGetTexGenfv");
 bo_glGetTexGeniv = (_glGetTexGeniv)gl.resolve("glGetTexGeniv");
 bo_glTexEnvf = (_glTexEnvf)gl.resolve("glTexEnvf");
 bo_glTexEnvi = (_glTexEnvi)gl.resolve("glTexEnvi");
 bo_glTexEnvfv = (_glTexEnvfv)gl.resolve("glTexEnvfv");
 bo_glTexEnviv = (_glTexEnviv)gl.resolve("glTexEnviv");
 bo_glGetTexEnvfv = (_glGetTexEnvfv)gl.resolve("glGetTexEnvfv");
 bo_glGetTexEnviv = (_glGetTexEnviv)gl.resolve("glGetTexEnviv");
 bo_glTexParameterf = (_glTexParameterf)gl.resolve("glTexParameterf");
 bo_glTexParameteri = (_glTexParameteri)gl.resolve("glTexParameteri");
 bo_glTexParameterfv = (_glTexParameterfv)gl.resolve("glTexParameterfv");
 bo_glTexParameteriv = (_glTexParameteriv)gl.resolve("glTexParameteriv");
 bo_glGetTexParameterfv = (_glGetTexParameterfv)gl.resolve("glGetTexParameterfv");
 bo_glGetTexParameteriv = (_glGetTexParameteriv)gl.resolve("glGetTexParameteriv");
 bo_glGetTexLevelParameterfv = (_glGetTexLevelParameterfv)gl.resolve("glGetTexLevelParameterfv");
 bo_glGetTexLevelParameteriv = (_glGetTexLevelParameteriv)gl.resolve("glGetTexLevelParameteriv");
 bo_glTexImage1D = (_glTexImage1D)gl.resolve("glTexImage1D");
 bo_glTexImage2D = (_glTexImage2D)gl.resolve("glTexImage2D");
 bo_glGetTexImage = (_glGetTexImage)gl.resolve("glGetTexImage");
 bo_glGenTextures = (_glGenTextures)gl.resolve("glGenTextures");
 bo_glDeleteTextures = (_glDeleteTextures)gl.resolve("glDeleteTextures");
 bo_glBindTexture = (_glBindTexture)gl.resolve("glBindTexture");
 bo_glPrioritizeTextures = (_glPrioritizeTextures)gl.resolve("glPrioritizeTextures");
 bo_glAreTexturesResident = (_glAreTexturesResident)gl.resolve("glAreTexturesResident");
 bo_glIsTexture = (_glIsTexture)gl.resolve("glIsTexture");
 bo_glTexSubImage1D = (_glTexSubImage1D)gl.resolve("glTexSubImage1D");
 bo_glTexSubImage2D = (_glTexSubImage2D)gl.resolve("glTexSubImage2D");
 bo_glCopyTexImage1D = (_glCopyTexImage1D)gl.resolve("glCopyTexImage1D");
 bo_glCopyTexImage2D = (_glCopyTexImage2D)gl.resolve("glCopyTexImage2D");
 bo_glCopyTexSubImage1D = (_glCopyTexSubImage1D)gl.resolve("glCopyTexSubImage1D");
 bo_glCopyTexSubImage2D = (_glCopyTexSubImage2D)gl.resolve("glCopyTexSubImage2D");
 bo_glMap1d = (_glMap1d)gl.resolve("glMap1d");
 bo_glMap1f = (_glMap1f)gl.resolve("glMap1f");
 bo_glMap2d = (_glMap2d)gl.resolve("glMap2d");
 bo_glMap2f = (_glMap2f)gl.resolve("glMap2f");
 bo_glGetMapdv = (_glGetMapdv)gl.resolve("glGetMapdv");
 bo_glGetMapfv = (_glGetMapfv)gl.resolve("glGetMapfv");
 bo_glGetMapiv = (_glGetMapiv)gl.resolve("glGetMapiv");
 bo_glEvalCoord1d = (_glEvalCoord1d)gl.resolve("glEvalCoord1d");
 bo_glEvalCoord1f = (_glEvalCoord1f)gl.resolve("glEvalCoord1f");
 bo_glEvalCoord1dv = (_glEvalCoord1dv)gl.resolve("glEvalCoord1dv");
 bo_glEvalCoord1fv = (_glEvalCoord1fv)gl.resolve("glEvalCoord1fv");
 bo_glEvalCoord2d = (_glEvalCoord2d)gl.resolve("glEvalCoord2d");
 bo_glEvalCoord2f = (_glEvalCoord2f)gl.resolve("glEvalCoord2f");
 bo_glEvalCoord2dv = (_glEvalCoord2dv)gl.resolve("glEvalCoord2dv");
 bo_glEvalCoord2fv = (_glEvalCoord2fv)gl.resolve("glEvalCoord2fv");
 bo_glMapGrid1d = (_glMapGrid1d)gl.resolve("glMapGrid1d");
 bo_glMapGrid1f = (_glMapGrid1f)gl.resolve("glMapGrid1f");
 bo_glMapGrid2d = (_glMapGrid2d)gl.resolve("glMapGrid2d");
 bo_glMapGrid2f = (_glMapGrid2f)gl.resolve("glMapGrid2f");
 bo_glEvalPoint1 = (_glEvalPoint1)gl.resolve("glEvalPoint1");
 bo_glEvalPoint2 = (_glEvalPoint2)gl.resolve("glEvalPoint2");
 bo_glEvalMesh1 = (_glEvalMesh1)gl.resolve("glEvalMesh1");
 bo_glEvalMesh2 = (_glEvalMesh2)gl.resolve("glEvalMesh2");
 bo_glFogf = (_glFogf)gl.resolve("glFogf");
 bo_glFogi = (_glFogi)gl.resolve("glFogi");
 bo_glFogfv = (_glFogfv)gl.resolve("glFogfv");
 bo_glFogiv = (_glFogiv)gl.resolve("glFogiv");
 bo_glFeedbackBuffer = (_glFeedbackBuffer)gl.resolve("glFeedbackBuffer");
 bo_glPassThrough = (_glPassThrough)gl.resolve("glPassThrough");
 bo_glSelectBuffer = (_glSelectBuffer)gl.resolve("glSelectBuffer");
 bo_glInitNames = (_glInitNames)gl.resolve("glInitNames");
 bo_glLoadName = (_glLoadName)gl.resolve("glLoadName");
 bo_glPushName = (_glPushName)gl.resolve("glPushName");
 bo_glPopName = (_glPopName)gl.resolve("glPopName");
 bo_glDrawRangeElements = (_glDrawRangeElements)gl.resolve("glDrawRangeElements");
 bo_glTexImage3D = (_glTexImage3D)gl.resolve("glTexImage3D");
 bo_glTexSubImage3D = (_glTexSubImage3D)gl.resolve("glTexSubImage3D");
 bo_glCopyTexSubImage3D = (_glCopyTexSubImage3D)gl.resolve("glCopyTexSubImage3D");
 bo_glColorTable = (_glColorTable)gl.resolve("glColorTable");
 bo_glColorSubTable = (_glColorSubTable)gl.resolve("glColorSubTable");
 bo_glColorTableParameteriv = (_glColorTableParameteriv)gl.resolve("glColorTableParameteriv");
 bo_glColorTableParameterfv = (_glColorTableParameterfv)gl.resolve("glColorTableParameterfv");
 bo_glCopyColorSubTable = (_glCopyColorSubTable)gl.resolve("glCopyColorSubTable");
 bo_glCopyColorTable = (_glCopyColorTable)gl.resolve("glCopyColorTable");
 bo_glGetColorTable = (_glGetColorTable)gl.resolve("glGetColorTable");
 bo_glGetColorTableParameterfv = (_glGetColorTableParameterfv)gl.resolve("glGetColorTableParameterfv");
 bo_glGetColorTableParameteriv = (_glGetColorTableParameteriv)gl.resolve("glGetColorTableParameteriv");
 bo_glBlendEquation = (_glBlendEquation)gl.resolve("glBlendEquation");
 bo_glBlendColor = (_glBlendColor)gl.resolve("glBlendColor");
 bo_glHistogram = (_glHistogram)gl.resolve("glHistogram");
 bo_glResetHistogram = (_glResetHistogram)gl.resolve("glResetHistogram");
 bo_glGetHistogram = (_glGetHistogram)gl.resolve("glGetHistogram");
 bo_glGetHistogramParameterfv = (_glGetHistogramParameterfv)gl.resolve("glGetHistogramParameterfv");
 bo_glGetHistogramParameteriv = (_glGetHistogramParameteriv)gl.resolve("glGetHistogramParameteriv");
 bo_glMinmax = (_glMinmax)gl.resolve("glMinmax");
 bo_glResetMinmax = (_glResetMinmax)gl.resolve("glResetMinmax");
 bo_glGetMinmax = (_glGetMinmax)gl.resolve("glGetMinmax");
 bo_glGetMinmaxParameterfv = (_glGetMinmaxParameterfv)gl.resolve("glGetMinmaxParameterfv");
 bo_glGetMinmaxParameteriv = (_glGetMinmaxParameteriv)gl.resolve("glGetMinmaxParameteriv");
 bo_glConvolutionFilter1D = (_glConvolutionFilter1D)gl.resolve("glConvolutionFilter1D");
 bo_glConvolutionFilter2D = (_glConvolutionFilter2D)gl.resolve("glConvolutionFilter2D");
 bo_glConvolutionParameterf = (_glConvolutionParameterf)gl.resolve("glConvolutionParameterf");
 bo_glConvolutionParameterfv = (_glConvolutionParameterfv)gl.resolve("glConvolutionParameterfv");
 bo_glConvolutionParameteri = (_glConvolutionParameteri)gl.resolve("glConvolutionParameteri");
 bo_glConvolutionParameteriv = (_glConvolutionParameteriv)gl.resolve("glConvolutionParameteriv");
 bo_glCopyConvolutionFilter1D = (_glCopyConvolutionFilter1D)gl.resolve("glCopyConvolutionFilter1D");
 bo_glCopyConvolutionFilter2D = (_glCopyConvolutionFilter2D)gl.resolve("glCopyConvolutionFilter2D");
 bo_glGetConvolutionFilter = (_glGetConvolutionFilter)gl.resolve("glGetConvolutionFilter");
 bo_glGetConvolutionParameterfv = (_glGetConvolutionParameterfv)gl.resolve("glGetConvolutionParameterfv");
 bo_glGetConvolutionParameteriv = (_glGetConvolutionParameteriv)gl.resolve("glGetConvolutionParameteriv");
 bo_glSeparableFilter2D = (_glSeparableFilter2D)gl.resolve("glSeparableFilter2D");
 bo_glGetSeparableFilter = (_glGetSeparableFilter)gl.resolve("glGetSeparableFilter");
 bo_glActiveTexture = (_glActiveTexture)gl.resolve("glActiveTexture");
 bo_glClientActiveTexture = (_glClientActiveTexture)gl.resolve("glClientActiveTexture");
 bo_glCompressedTexImage1D = (_glCompressedTexImage1D)gl.resolve("glCompressedTexImage1D");
 bo_glCompressedTexImage2D = (_glCompressedTexImage2D)gl.resolve("glCompressedTexImage2D");
 bo_glCompressedTexImage3D = (_glCompressedTexImage3D)gl.resolve("glCompressedTexImage3D");
 bo_glCompressedTexSubImage1D = (_glCompressedTexSubImage1D)gl.resolve("glCompressedTexSubImage1D");
 bo_glCompressedTexSubImage2D = (_glCompressedTexSubImage2D)gl.resolve("glCompressedTexSubImage2D");
 bo_glCompressedTexSubImage3D = (_glCompressedTexSubImage3D)gl.resolve("glCompressedTexSubImage3D");
 bo_glGetCompressedTexImage = (_glGetCompressedTexImage)gl.resolve("glGetCompressedTexImage");
 bo_glMultiTexCoord1d = (_glMultiTexCoord1d)gl.resolve("glMultiTexCoord1d");
 bo_glMultiTexCoord1dv = (_glMultiTexCoord1dv)gl.resolve("glMultiTexCoord1dv");
 bo_glMultiTexCoord1f = (_glMultiTexCoord1f)gl.resolve("glMultiTexCoord1f");
 bo_glMultiTexCoord1fv = (_glMultiTexCoord1fv)gl.resolve("glMultiTexCoord1fv");
 bo_glMultiTexCoord1i = (_glMultiTexCoord1i)gl.resolve("glMultiTexCoord1i");
 bo_glMultiTexCoord1iv = (_glMultiTexCoord1iv)gl.resolve("glMultiTexCoord1iv");
 bo_glMultiTexCoord1s = (_glMultiTexCoord1s)gl.resolve("glMultiTexCoord1s");
 bo_glMultiTexCoord1sv = (_glMultiTexCoord1sv)gl.resolve("glMultiTexCoord1sv");
 bo_glMultiTexCoord2d = (_glMultiTexCoord2d)gl.resolve("glMultiTexCoord2d");
 bo_glMultiTexCoord2dv = (_glMultiTexCoord2dv)gl.resolve("glMultiTexCoord2dv");
 bo_glMultiTexCoord2f = (_glMultiTexCoord2f)gl.resolve("glMultiTexCoord2f");
 bo_glMultiTexCoord2fv = (_glMultiTexCoord2fv)gl.resolve("glMultiTexCoord2fv");
 bo_glMultiTexCoord2i = (_glMultiTexCoord2i)gl.resolve("glMultiTexCoord2i");
 bo_glMultiTexCoord2iv = (_glMultiTexCoord2iv)gl.resolve("glMultiTexCoord2iv");
 bo_glMultiTexCoord2s = (_glMultiTexCoord2s)gl.resolve("glMultiTexCoord2s");
 bo_glMultiTexCoord2sv = (_glMultiTexCoord2sv)gl.resolve("glMultiTexCoord2sv");
 bo_glMultiTexCoord3d = (_glMultiTexCoord3d)gl.resolve("glMultiTexCoord3d");
 bo_glMultiTexCoord3dv = (_glMultiTexCoord3dv)gl.resolve("glMultiTexCoord3dv");
 bo_glMultiTexCoord3f = (_glMultiTexCoord3f)gl.resolve("glMultiTexCoord3f");
 bo_glMultiTexCoord3fv = (_glMultiTexCoord3fv)gl.resolve("glMultiTexCoord3fv");
 bo_glMultiTexCoord3i = (_glMultiTexCoord3i)gl.resolve("glMultiTexCoord3i");
 bo_glMultiTexCoord3iv = (_glMultiTexCoord3iv)gl.resolve("glMultiTexCoord3iv");
 bo_glMultiTexCoord3s = (_glMultiTexCoord3s)gl.resolve("glMultiTexCoord3s");
 bo_glMultiTexCoord3sv = (_glMultiTexCoord3sv)gl.resolve("glMultiTexCoord3sv");
 bo_glMultiTexCoord4d = (_glMultiTexCoord4d)gl.resolve("glMultiTexCoord4d");
 bo_glMultiTexCoord4dv = (_glMultiTexCoord4dv)gl.resolve("glMultiTexCoord4dv");
 bo_glMultiTexCoord4f = (_glMultiTexCoord4f)gl.resolve("glMultiTexCoord4f");
 bo_glMultiTexCoord4fv = (_glMultiTexCoord4fv)gl.resolve("glMultiTexCoord4fv");
 bo_glMultiTexCoord4i = (_glMultiTexCoord4i)gl.resolve("glMultiTexCoord4i");
 bo_glMultiTexCoord4iv = (_glMultiTexCoord4iv)gl.resolve("glMultiTexCoord4iv");
 bo_glMultiTexCoord4s = (_glMultiTexCoord4s)gl.resolve("glMultiTexCoord4s");
 bo_glMultiTexCoord4sv = (_glMultiTexCoord4sv)gl.resolve("glMultiTexCoord4sv");
 bo_glLoadTransposeMatrixd = (_glLoadTransposeMatrixd)gl.resolve("glLoadTransposeMatrixd");
 bo_glLoadTransposeMatrixf = (_glLoadTransposeMatrixf)gl.resolve("glLoadTransposeMatrixf");
 bo_glMultTransposeMatrixd = (_glMultTransposeMatrixd)gl.resolve("glMultTransposeMatrixd");
 bo_glMultTransposeMatrixf = (_glMultTransposeMatrixf)gl.resolve("glMultTransposeMatrixf");
 bo_glSampleCoverage = (_glSampleCoverage)gl.resolve("glSampleCoverage");
 bo_glActiveTextureARB = (_glActiveTextureARB)gl.resolve("glActiveTextureARB");
 bo_glClientActiveTextureARB = (_glClientActiveTextureARB)gl.resolve("glClientActiveTextureARB");
 bo_glMultiTexCoord1dARB = (_glMultiTexCoord1dARB)gl.resolve("glMultiTexCoord1dARB");
 bo_glMultiTexCoord1dvARB = (_glMultiTexCoord1dvARB)gl.resolve("glMultiTexCoord1dvARB");
 bo_glMultiTexCoord1fARB = (_glMultiTexCoord1fARB)gl.resolve("glMultiTexCoord1fARB");
 bo_glMultiTexCoord1fvARB = (_glMultiTexCoord1fvARB)gl.resolve("glMultiTexCoord1fvARB");
 bo_glMultiTexCoord1iARB = (_glMultiTexCoord1iARB)gl.resolve("glMultiTexCoord1iARB");
 bo_glMultiTexCoord1ivARB = (_glMultiTexCoord1ivARB)gl.resolve("glMultiTexCoord1ivARB");
 bo_glMultiTexCoord1sARB = (_glMultiTexCoord1sARB)gl.resolve("glMultiTexCoord1sARB");
 bo_glMultiTexCoord1svARB = (_glMultiTexCoord1svARB)gl.resolve("glMultiTexCoord1svARB");
 bo_glMultiTexCoord2dARB = (_glMultiTexCoord2dARB)gl.resolve("glMultiTexCoord2dARB");
 bo_glMultiTexCoord2dvARB = (_glMultiTexCoord2dvARB)gl.resolve("glMultiTexCoord2dvARB");
 bo_glMultiTexCoord2fARB = (_glMultiTexCoord2fARB)gl.resolve("glMultiTexCoord2fARB");
 bo_glMultiTexCoord2fvARB = (_glMultiTexCoord2fvARB)gl.resolve("glMultiTexCoord2fvARB");
 bo_glMultiTexCoord2iARB = (_glMultiTexCoord2iARB)gl.resolve("glMultiTexCoord2iARB");
 bo_glMultiTexCoord2ivARB = (_glMultiTexCoord2ivARB)gl.resolve("glMultiTexCoord2ivARB");
 bo_glMultiTexCoord2sARB = (_glMultiTexCoord2sARB)gl.resolve("glMultiTexCoord2sARB");
 bo_glMultiTexCoord2svARB = (_glMultiTexCoord2svARB)gl.resolve("glMultiTexCoord2svARB");
 bo_glMultiTexCoord3dARB = (_glMultiTexCoord3dARB)gl.resolve("glMultiTexCoord3dARB");
 bo_glMultiTexCoord3dvARB = (_glMultiTexCoord3dvARB)gl.resolve("glMultiTexCoord3dvARB");
 bo_glMultiTexCoord3fARB = (_glMultiTexCoord3fARB)gl.resolve("glMultiTexCoord3fARB");
 bo_glMultiTexCoord3fvARB = (_glMultiTexCoord3fvARB)gl.resolve("glMultiTexCoord3fvARB");
 bo_glMultiTexCoord3iARB = (_glMultiTexCoord3iARB)gl.resolve("glMultiTexCoord3iARB");
 bo_glMultiTexCoord3ivARB = (_glMultiTexCoord3ivARB)gl.resolve("glMultiTexCoord3ivARB");
 bo_glMultiTexCoord3sARB = (_glMultiTexCoord3sARB)gl.resolve("glMultiTexCoord3sARB");
 bo_glMultiTexCoord3svARB = (_glMultiTexCoord3svARB)gl.resolve("glMultiTexCoord3svARB");
 bo_glMultiTexCoord4dARB = (_glMultiTexCoord4dARB)gl.resolve("glMultiTexCoord4dARB");
 bo_glMultiTexCoord4dvARB = (_glMultiTexCoord4dvARB)gl.resolve("glMultiTexCoord4dvARB");
 bo_glMultiTexCoord4fARB = (_glMultiTexCoord4fARB)gl.resolve("glMultiTexCoord4fARB");
 bo_glMultiTexCoord4fvARB = (_glMultiTexCoord4fvARB)gl.resolve("glMultiTexCoord4fvARB");
 bo_glMultiTexCoord4iARB = (_glMultiTexCoord4iARB)gl.resolve("glMultiTexCoord4iARB");
 bo_glMultiTexCoord4ivARB = (_glMultiTexCoord4ivARB)gl.resolve("glMultiTexCoord4ivARB");
 bo_glMultiTexCoord4sARB = (_glMultiTexCoord4sARB)gl.resolve("glMultiTexCoord4sARB");
 bo_glMultiTexCoord4svARB = (_glMultiTexCoord4svARB)gl.resolve("glMultiTexCoord4svARB");
 bo_glBlendColorEXT = (_glBlendColorEXT)gl.resolve("glBlendColorEXT");
 bo_glPolygonOffsetEXT = (_glPolygonOffsetEXT)gl.resolve("glPolygonOffsetEXT");
 bo_glTexImage3DEXT = (_glTexImage3DEXT)gl.resolve("glTexImage3DEXT");
 bo_glTexSubImage3DEXT = (_glTexSubImage3DEXT)gl.resolve("glTexSubImage3DEXT");
 bo_glCopyTexSubImage3DEXT = (_glCopyTexSubImage3DEXT)gl.resolve("glCopyTexSubImage3DEXT");
 bo_glGenTexturesEXT = (_glGenTexturesEXT)gl.resolve("glGenTexturesEXT");
 bo_glDeleteTexturesEXT = (_glDeleteTexturesEXT)gl.resolve("glDeleteTexturesEXT");
 bo_glBindTextureEXT = (_glBindTextureEXT)gl.resolve("glBindTextureEXT");
 bo_glPrioritizeTexturesEXT = (_glPrioritizeTexturesEXT)gl.resolve("glPrioritizeTexturesEXT");
 bo_glAreTexturesResidentEXT = (_glAreTexturesResidentEXT)gl.resolve("glAreTexturesResidentEXT");
 bo_glIsTextureEXT = (_glIsTextureEXT)gl.resolve("glIsTextureEXT");
 bo_glVertexPointerEXT = (_glVertexPointerEXT)gl.resolve("glVertexPointerEXT");
 bo_glNormalPointerEXT = (_glNormalPointerEXT)gl.resolve("glNormalPointerEXT");
 bo_glColorPointerEXT = (_glColorPointerEXT)gl.resolve("glColorPointerEXT");
 bo_glIndexPointerEXT = (_glIndexPointerEXT)gl.resolve("glIndexPointerEXT");
 bo_glTexCoordPointerEXT = (_glTexCoordPointerEXT)gl.resolve("glTexCoordPointerEXT");
 bo_glEdgeFlagPointerEXT = (_glEdgeFlagPointerEXT)gl.resolve("glEdgeFlagPointerEXT");
 bo_glGetPointervEXT = (_glGetPointervEXT)gl.resolve("glGetPointervEXT");
 bo_glArrayElementEXT = (_glArrayElementEXT)gl.resolve("glArrayElementEXT");
 bo_glDrawArraysEXT = (_glDrawArraysEXT)gl.resolve("glDrawArraysEXT");
 bo_glBlendEquationEXT = (_glBlendEquationEXT)gl.resolve("glBlendEquationEXT");
 bo_glPointParameterfEXT = (_glPointParameterfEXT)gl.resolve("glPointParameterfEXT");
 bo_glPointParameterfvEXT = (_glPointParameterfvEXT)gl.resolve("glPointParameterfvEXT");
 bo_glPointParameterfSGIS = (_glPointParameterfSGIS)gl.resolve("glPointParameterfSGIS");
 bo_glPointParameterfvSGIS = (_glPointParameterfvSGIS)gl.resolve("glPointParameterfvSGIS");
 bo_glColorTableEXT = (_glColorTableEXT)gl.resolve("glColorTableEXT");
 bo_glColorSubTableEXT = (_glColorSubTableEXT)gl.resolve("glColorSubTableEXT");
 bo_glGetColorTableEXT = (_glGetColorTableEXT)gl.resolve("glGetColorTableEXT");
 bo_glGetColorTableParameterfvEXT = (_glGetColorTableParameterfvEXT)gl.resolve("glGetColorTableParameterfvEXT");
 bo_glGetColorTableParameterivEXT = (_glGetColorTableParameterivEXT)gl.resolve("glGetColorTableParameterivEXT");
 bo_glLockArraysEXT = (_glLockArraysEXT)gl.resolve("glLockArraysEXT");
 bo_glUnlockArraysEXT = (_glUnlockArraysEXT)gl.resolve("glUnlockArraysEXT");
 bo_glWindowPos2iMESA = (_glWindowPos2iMESA)gl.resolve("glWindowPos2iMESA");
 bo_glWindowPos2sMESA = (_glWindowPos2sMESA)gl.resolve("glWindowPos2sMESA");
 bo_glWindowPos2fMESA = (_glWindowPos2fMESA)gl.resolve("glWindowPos2fMESA");
 bo_glWindowPos2dMESA = (_glWindowPos2dMESA)gl.resolve("glWindowPos2dMESA");
 bo_glWindowPos2ivMESA = (_glWindowPos2ivMESA)gl.resolve("glWindowPos2ivMESA");
 bo_glWindowPos2svMESA = (_glWindowPos2svMESA)gl.resolve("glWindowPos2svMESA");
 bo_glWindowPos2fvMESA = (_glWindowPos2fvMESA)gl.resolve("glWindowPos2fvMESA");
 bo_glWindowPos2dvMESA = (_glWindowPos2dvMESA)gl.resolve("glWindowPos2dvMESA");
 bo_glWindowPos3iMESA = (_glWindowPos3iMESA)gl.resolve("glWindowPos3iMESA");
 bo_glWindowPos3sMESA = (_glWindowPos3sMESA)gl.resolve("glWindowPos3sMESA");
 bo_glWindowPos3fMESA = (_glWindowPos3fMESA)gl.resolve("glWindowPos3fMESA");
 bo_glWindowPos3dMESA = (_glWindowPos3dMESA)gl.resolve("glWindowPos3dMESA");
 bo_glWindowPos3ivMESA = (_glWindowPos3ivMESA)gl.resolve("glWindowPos3ivMESA");
 bo_glWindowPos3svMESA = (_glWindowPos3svMESA)gl.resolve("glWindowPos3svMESA");
 bo_glWindowPos3fvMESA = (_glWindowPos3fvMESA)gl.resolve("glWindowPos3fvMESA");
 bo_glWindowPos3dvMESA = (_glWindowPos3dvMESA)gl.resolve("glWindowPos3dvMESA");
 bo_glWindowPos4iMESA = (_glWindowPos4iMESA)gl.resolve("glWindowPos4iMESA");
 bo_glWindowPos4sMESA = (_glWindowPos4sMESA)gl.resolve("glWindowPos4sMESA");
 bo_glWindowPos4fMESA = (_glWindowPos4fMESA)gl.resolve("glWindowPos4fMESA");
 bo_glWindowPos4dMESA = (_glWindowPos4dMESA)gl.resolve("glWindowPos4dMESA");
 bo_glWindowPos4ivMESA = (_glWindowPos4ivMESA)gl.resolve("glWindowPos4ivMESA");
 bo_glWindowPos4svMESA = (_glWindowPos4svMESA)gl.resolve("glWindowPos4svMESA");
 bo_glWindowPos4fvMESA = (_glWindowPos4fvMESA)gl.resolve("glWindowPos4fvMESA");
 bo_glWindowPos4dvMESA = (_glWindowPos4dvMESA)gl.resolve("glWindowPos4dvMESA");
 bo_glResizeBuffersMESA = (_glResizeBuffersMESA)gl.resolve("glResizeBuffersMESA");
 bo_glEnableTraceMESA = (_glEnableTraceMESA)gl.resolve("glEnableTraceMESA");
 bo_glDisableTraceMESA = (_glDisableTraceMESA)gl.resolve("glDisableTraceMESA");
 bo_glNewTraceMESA = (_glNewTraceMESA)gl.resolve("glNewTraceMESA");
 bo_glEndTraceMESA = (_glEndTraceMESA)gl.resolve("glEndTraceMESA");
 bo_glTraceAssertAttribMESA = (_glTraceAssertAttribMESA)gl.resolve("glTraceAssertAttribMESA");
 bo_glTraceCommentMESA = (_glTraceCommentMESA)gl.resolve("glTraceCommentMESA");
 bo_glTraceTextureMESA = (_glTraceTextureMESA)gl.resolve("glTraceTextureMESA");
 bo_glTraceListMESA = (_glTraceListMESA)gl.resolve("glTraceListMESA");
 bo_glTracePointerMESA = (_glTracePointerMESA)gl.resolve("glTracePointerMESA");
 bo_glTracePointerRangeMESA = (_glTracePointerRangeMESA) gl.resolve("glTracePointerRangeMESA");

 return true;
}

static bool boglResolveGLUSymbols(QLibrary& glu)
{
 bo_gluBeginCurve = (_gluBeginCurve)glu.resolve("gluBeginCurve");
 if (bo_gluBeginCurve == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluBeginCurve" << endl;
	return false;
 }
 bo_gluBeginPolygon = (_gluBeginPolygon)glu.resolve("gluBeginPolygon");
 if (bo_gluBeginPolygon == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "uBeginPolygon" << endl;
	return false;
 }
 bo_gluBeginSurface = (_gluBeginSurface)glu.resolve("gluBeginSurface");
 if (bo_gluBeginSurface == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "uBeginSurface" << endl;
	return false;
 }
 bo_gluBeginTrim = (_gluBeginTrim)glu.resolve("gluBeginTrim");
 if (bo_gluBeginTrim == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluBeginTrim" << endl;
	return false;
 }
 bo_gluBuild1DMipmapLevels = (_gluBuild1DMipmapLevels)glu.resolve("gluBuild1DMipmapLevels");
 if (bo_gluBuild1DMipmapLevels == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluBuild1DMipmapLevels" << endl;
	return false;
 }
 bo_gluBuild1DMipmaps = (_gluBuild1DMipmaps)glu.resolve("gluBuild1DMipmaps");
 if (bo_gluBuild1DMipmaps == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluBuild1DMipmaps" << endl;
	return false;
 }
 bo_gluBuild2DMipmapLevels = (_gluBuild2DMipmapLevels)glu.resolve("gluBuild2DMipmapLevels");
 if (bo_gluBuild2DMipmapLevels == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluBuild2DMipmapLevels" << endl;
	return false;
 }
 bo_gluBuild2DMipmaps = (_gluBuild2DMipmaps)glu.resolve("gluBuild2DMipmaps");
 if (bo_gluBuild2DMipmaps == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluBuild2DMipmaps" << endl;
	return false;
 }
 bo_gluBuild3DMipmapLevels = (_gluBuild3DMipmapLevels)glu.resolve("gluBuild3DMipmapLevels");
 if (bo_gluBuild3DMipmapLevels == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluBuild3DMipmapLevels" << endl;
	return false;
 }
 bo_gluBuild3DMipmaps = (_gluBuild3DMipmaps)glu.resolve("gluBuild3DMipmaps");
 if (bo_gluBuild3DMipmaps == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluBuild3DMipmaps" << endl;
	return false;
 }
 bo_gluCheckExtension = (_gluCheckExtension)glu.resolve("gluCheckExtension");
 if (bo_gluCheckExtension == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluCheckExtension" << endl;
	return false;
 }
 bo_gluCylinder = (_gluCylinder)glu.resolve("gluCylinder");
 if (bo_gluCylinder == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluCylinder" << endl;
	return false;
 }
 bo_gluDeleteNurbsRenderer = (_gluDeleteNurbsRenderer)glu.resolve("gluDeleteNurbsRenderer");
 if (bo_gluDeleteNurbsRenderer == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluDeleteNurbsRenderer" << endl;
	return false;
 }
 bo_gluDeleteQuadric = (_gluDeleteQuadric)glu.resolve("gluDeleteQuadric");
 if (bo_gluDeleteQuadric == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluDeleteQuadric" << endl;
	return false;
 }
 bo_gluDeleteTess = (_gluDeleteTess)glu.resolve("gluDeleteTess");
 if (bo_gluDeleteTess == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluDeleteTess" << endl;
	return false;
 }
 bo_gluDisk = (_gluDisk)glu.resolve("gluDisk");
 if (bo_gluDisk == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluDisk" << endl;
	return false;
 }
 bo_gluEndCurve = (_gluEndCurve)glu.resolve("gluEndCurve");
 if (bo_gluEndCurve == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluEndCurve" << endl;
	return false;
 }
 bo_gluEndPolygon = (_gluEndPolygon)glu.resolve("gluEndPolygon");
 if (bo_gluEndPolygon == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluEndPolygon" << endl;
	return false;
 }
 bo_gluEndSurface = (_gluEndSurface)glu.resolve("gluEndSurface");
 if (bo_gluEndSurface == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluEndSurface" << endl;
	return false;
 }
 bo_gluEndTrim = (_gluEndTrim)glu.resolve("gluEndTrim");
 if (bo_gluEndTrim == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluEndTrim" << endl;
	return false;
 }
 bo_gluErrorString = (_gluErrorString)glu.resolve("gluErrorString");
 if (bo_gluErrorString == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluErrorString" << endl;
	return false;
 }
 bo_gluGetNurbsProperty = (_gluGetNurbsProperty)glu.resolve("gluGetNurbsProperty");
 if (bo_gluGetNurbsProperty == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluGetNurbsProperty" << endl;
	return false;
 }
 bo_gluGetString = (_gluGetString)glu.resolve("gluGetString");
 if (bo_gluGetString == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluGetString" << endl;
	return false;
 }
 bo_gluGetTessProperty = (_gluGetTessProperty)glu.resolve("gluGetTessProperty");
 if (bo_gluGetTessProperty == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluGetTessProperty" << endl;
	return false;
 }
 bo_gluLoadSamplingMatrices = (_gluLoadSamplingMatrices)glu.resolve("gluLoadSamplingMatrices");
 if (bo_gluLoadSamplingMatrices == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluLoadSamplingMatrices" << endl;
	return false;
 }
 bo_gluLookAt = (_gluLookAt)glu.resolve("gluLookAt");
 if (bo_gluLookAt == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluLookAt" << endl;
	return false;
 }
 bo_gluNewNurbsRenderer = (_gluNewNurbsRenderer)glu.resolve("gluNewNurbsRenderer");
 if (bo_gluNewNurbsRenderer == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNewNurbsRenderer" << endl;
	return false;
 }
 bo_gluNewQuadric = (_gluNewQuadric)glu.resolve("gluNewQuadric");
 if (bo_gluNewQuadric == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNewQuadric" << endl;
	return false;
 }
 bo_gluNewTess = (_gluNewTess)glu.resolve("gluNewTess");
 if (bo_gluNewTess == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNewTess" << endl;
	return false;
 }
 bo_gluNextContour = (_gluNextContour)glu.resolve("gluNextContour");
 if (bo_gluNextContour == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNextContour" << endl;
	return false;
 }
 bo_gluNurbsCallback = (_gluNurbsCallback)glu.resolve("gluNurbsCallback");
 if (bo_gluNurbsCallback == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNurbsCallback" << endl;
	return false;
 }
 bo_gluNurbsCallbackData = (_gluNurbsCallbackData)glu.resolve("gluNurbsCallbackData");
 if (bo_gluNurbsCallbackData == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNurbsCallbackData" << endl;
	return false;
 }
 bo_gluNurbsCallbackDataEXT = (_gluNurbsCallbackDataEXT)glu.resolve("gluNurbsCallbackDataEXT");
 if (bo_gluNurbsCallbackDataEXT == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNurbsCallbackDataEXT" << endl;
	return false;
 }
 bo_gluNurbsCurve = (_gluNurbsCurve)glu.resolve("gluNurbsCurve");
 if (bo_gluNurbsCurve == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNurbsCurve" << endl;
	return false;
 }
 bo_gluNurbsProperty = (_gluNurbsProperty)glu.resolve("gluNurbsProperty");
 if (bo_gluNurbsProperty == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNurbsProperty" << endl;
	return false;
 }
 bo_gluNurbsSurface = (_gluNurbsSurface)glu.resolve("gluNurbsSurface");
 if (bo_gluNurbsSurface == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluNurbsSurface" << endl;
	return false;
 }
 bo_gluOrtho2D = (_gluOrtho2D)glu.resolve("gluOrtho2D");
 if (bo_gluOrtho2D == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluOrtho2D" << endl;
	return false;
 }
 bo_gluPartialDisk = (_gluPartialDisk)glu.resolve("gluPartialDisk");
 if (bo_gluPartialDisk == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluPartialDisk" << endl;
	return false;
 }
 bo_gluPerspective = (_gluPerspective)glu.resolve("gluPerspective");
 if (bo_gluPerspective == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluPerspective" << endl;
	return false;
 }
 bo_gluPickMatrix = (_gluPickMatrix)glu.resolve("gluPickMatrix");
 if (bo_gluPickMatrix == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluPickMatrix" << endl;
	return false;
 }
 bo_gluProject = (_gluProject)glu.resolve("gluProject");
 if (bo_gluProject == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluProject" << endl;
	return false;
 }
 bo_gluPwlCurve = (_gluPwlCurve)glu.resolve("gluPwlCurve");
 if (bo_gluPwlCurve == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluPwlCurve" << endl;
	return false;
 }
 bo_gluQuadricCallback = (_gluQuadricCallback)glu.resolve("gluQuadricCallback");
 if (bo_gluQuadricCallback == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluQuadricCallback" << endl;
	return false;
 }
 bo_gluQuadricDrawStyle = (_gluQuadricDrawStyle)glu.resolve("gluQuadricDrawStyle");
 if (bo_gluQuadricDrawStyle == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluQuadricDrawStyle" << endl;
	return false;
 }
 bo_gluQuadricNormals = (_gluQuadricNormals)glu.resolve("gluQuadricNormals");
 if (bo_gluQuadricNormals == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluQuadricNormals" << endl;
	return false;
 }
 bo_gluQuadricOrientation = (_gluQuadricOrientation)glu.resolve("gluQuadricOrientation");
 if (bo_gluQuadricOrientation == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluQuadricOrientation" << endl;
	return false;
 }
 bo_gluQuadricTexture = (_gluQuadricTexture)glu.resolve("gluQuadricTexture");
 if (bo_gluQuadricTexture == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluQuadricTexture" << endl;
	return false;
 }
 bo_gluScaleImage = (_gluScaleImage)glu.resolve("gluScaleImage");
 if (bo_gluScaleImage == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluScaleImage" << endl;
	return false;
 }
 bo_gluSphere = (_gluSphere)glu.resolve("gluSphere");
 if (bo_gluSphere == 0) {
	boError() << k_funcinfo << "could not resolve symbol "" << gluSphere" << endl;
	return false;
 }
 bo_gluTessBeginContour = (_gluTessBeginContour)glu.resolve("gluTessBeginContour");
 if (bo_gluTessBeginContour == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluTessBeginContour" << endl;
	return false;
 }
 bo_gluTessBeginPolygon = (_gluTessBeginPolygon)glu.resolve("gluTessBeginPolygon");
 if (bo_gluTessBeginPolygon == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluTessBeginPolygon" << endl;
	return false;
 }
 bo_gluTessCallback = (_gluTessCallback)glu.resolve("gluTessCallback");
 if (bo_gluTessCallback == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluTessCallback" << endl;
	return false;
 }
 bo_gluTessEndContour = (_gluTessEndContour)glu.resolve("gluTessEndContour");
 if (bo_gluTessEndContour == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluTessEndContour" << endl;
	return false;
 }
 bo_gluTessEndPolygon = (_gluTessEndPolygon)glu.resolve("gluTessEndPolygon");
 if (bo_gluTessEndPolygon == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluTessEndPolygon" << endl;
	return false;
 }
 bo_gluTessNormal = (_gluTessNormal)glu.resolve("gluTessNormal");
 if (bo_gluTessNormal == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluTessNormal" << endl;
	return false;
 }
 bo_gluTessProperty = (_gluTessProperty)glu.resolve("gluTessProperty");
 if (bo_gluTessProperty == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluTessProperty" << endl;
	return false;
 }
 bo_gluTessVertex = (_gluTessVertex)glu.resolve("gluTessVertex");
 if (bo_gluTessVertex == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluTessVertex" << endl;
	return false;
 }
 bo_gluUnProject = (_gluUnProject)glu.resolve("gluUnProject");
 if (bo_gluUnProject == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluUnProject" << endl;
	return false;
 }
 bo_gluUnProject4 = (_gluUnProject4)glu.resolve("gluUnProject4");
 if (bo_gluUnProject4 == 0) {
	boError() << k_funcinfo << "could not resolve symbol " << "gluUnProject4" << endl;
	return false;
 }
 return true;
}

static bool boglResolveGLXSymbols(QLibrary& gl)
{
 bo_glXChooseVisual = (_glXChooseVisual)gl.resolve("glXChooseVisual");
 if (bo_glXChooseVisual == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXChooseVisual" << endl;
	return false;
 }
 bo_glXCopyContext = (_glXCopyContext)gl.resolve("glXCopyContext");
 if (bo_glXCopyContext == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXCopyContext" << endl;
	return false;
 }
 bo_glXCreateContext = (_glXCreateContext)gl.resolve("glXCreateContext");
 if (bo_glXCreateContext == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXCreateContext" << endl;
	return false;
 }
 bo_glXCreateGLXPixmap = (_glXCreateGLXPixmap)gl.resolve("glXCreateGLXPixmap");
 if (bo_glXCreateGLXPixmap == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXCreateGLXPixmap" << endl;
	return false;
 }
 bo_glXDestroyContext = (_glXDestroyContext)gl.resolve("glXDestroyContext");
 if (bo_glXDestroyContext == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXDestroyContext" << endl;
	return false;
 }
 bo_glXDestroyGLXPixmap = (_glXDestroyGLXPixmap)gl.resolve("glXDestroyGLXPixmap");
 if (bo_glXDestroyGLXPixmap == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXDestroyGLXPixmap" << endl;
	return false;
 }
 bo_glXGetConfig = (_glXGetConfig)gl.resolve("glXGetConfig");
 if (bo_glXGetConfig == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetConfig" << endl;
	return false;
 }
 bo_glXGetCurrentContext = (_glXGetCurrentContext)gl.resolve("glXGetCurrentContext");
 if (bo_glXGetCurrentContext == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetCurrentContext" << endl;
	return false;
 }
 bo_glXGetCurrentDrawable = (_glXGetCurrentDrawable)gl.resolve("glXGetCurrentDrawable");
 if (bo_glXGetCurrentDrawable == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetCurrentDrawable" << endl;
	return false;
 }
 bo_glXIsDirect = (_glXIsDirect)gl.resolve("glXIsDirect");
 if (bo_glXIsDirect == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXIsDirect" << endl;
	return false;
 }
 bo_glXMakeCurrent = (_glXMakeCurrent)gl.resolve("glXMakeCurrent");
 if (bo_glXMakeCurrent == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXMakeCurrent" << endl;
	return false;
 }
 bo_glXQueryExtension = (_glXQueryExtension)gl.resolve("glXQueryExtension");
 if (bo_glXQueryExtension == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXQueryExtension" << endl;
	return false;
 }
 bo_glXQueryVersion = (_glXQueryVersion)gl.resolve("glXQueryVersion");
 if (bo_glXQueryVersion == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXQueryVersion" << endl;
	return false;
 }
 bo_glXSwapBuffers = (_glXSwapBuffers)gl.resolve("glXSwapBuffers");
 if (bo_glXSwapBuffers == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXSwapBuffers" << endl;
	return false;
 }
 bo_glXUseXFont = (_glXUseXFont)gl.resolve("glXUseXFont");
 if (bo_glXUseXFont == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXUseXFont" << endl;
	return false;
 }
 bo_glXWaitGL = (_glXWaitGL)gl.resolve("glXWaitGL");
 if (bo_glXWaitGL == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXWaitGL" << endl;
	return false;
 }
 bo_glXWaitX = (_glXWaitX)gl.resolve("glXWaitX");
 if (bo_glXWaitX == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXWaitX" << endl;
	return false;
 }
 bo_glXGetClientString = (_glXGetClientString)gl.resolve("glXGetClientString");
 if (bo_glXGetClientString == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetClientString" << endl;
	return false;
 }
 bo_glXQueryServerString = (_glXQueryServerString)gl.resolve("glXQueryServerString");
 if (bo_glXQueryServerString == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXQueryServerString" << endl;
	return false;
 }
 bo_glXQueryExtensionsString = (_glXQueryExtensionsString)gl.resolve("glXQueryExtensionsString");
 if (bo_glXQueryExtensionsString == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXQueryExtensionsString" << endl;
	return false;
 }
 bo_glXGetFBConfigs = (_glXGetFBConfigs)gl.resolve("glXGetFBConfigs");
 if (bo_glXGetFBConfigs == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetFBConfigs" << endl;
	return false;
 }
 bo_glXChooseFBConfig = (_glXChooseFBConfig)gl.resolve("glXChooseFBConfig");
 if (bo_glXChooseFBConfig == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXChooseFBConfig" << endl;
	return false;
 }
 bo_glXGetFBConfigAttrib = (_glXGetFBConfigAttrib)gl.resolve("glXGetFBConfigAttrib");
 if (bo_glXGetFBConfigAttrib == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetFBConfigAttrib" << endl;
	return false;
 }
 bo_glXGetVisualFromFBConfig = (_glXGetVisualFromFBConfig)gl.resolve("glXGetVisualFromFBConfig");
 if (bo_glXGetVisualFromFBConfig == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetVisualFromFBConfig" << endl;
	return false;
 }
 bo_glXCreateWindow = (_glXCreateWindow)gl.resolve("glXCreateWindow");
 if (bo_glXCreateWindow == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXCreateWindow" << endl;
	return false;
 }
 bo_glXDestroyWindow = (_glXDestroyWindow)gl.resolve("glXDestroyWindow");
 if (bo_glXDestroyWindow == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXDestroyWindow" << endl;
	return false;
 }
 bo_glXCreatePixmap = (_glXCreatePixmap)gl.resolve("glXCreatePixmap");
 if (bo_glXCreatePixmap == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXCreatePixmap" << endl;
	return false;
 }
 bo_glXDestroyPixmap = (_glXDestroyPixmap)gl.resolve("glXDestroyPixmap");
 if (bo_glXDestroyPixmap == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXDestroyPixmap" << endl;
	return false;
 }
 bo_glXCreatePbuffer = (_glXCreatePbuffer)gl.resolve("glXCreatePbuffer");
 if (bo_glXCreatePbuffer == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXCreatePbuffer" << endl;
	return false;
 }
 bo_glXDestroyPbuffer = (_glXDestroyPbuffer)gl.resolve("glXDestroyPbuffer");
 if (bo_glXDestroyPbuffer == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXDestroyPbuffer" << endl;
	return false;
 }
 bo_glXQueryDrawable = (_glXQueryDrawable)gl.resolve("glXQueryDrawable");
 if (bo_glXQueryDrawable == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXQueryDrawable" << endl;
	return false;
 }
 bo_glXCreateNewContext = (_glXCreateNewContext)gl.resolve("glXCreateNewContext");
 if (bo_glXCreateNewContext == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXCreateNewContext" << endl;
	return false;
 }
 bo_glXMakeContextCurrent = (_glXMakeContextCurrent)gl.resolve("glXMakeContextCurrent");
 if (bo_glXMakeContextCurrent == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXMakeContextCurrent" << endl;
	return false;
 }
 bo_glXGetCurrentReadDrawable = (_glXGetCurrentReadDrawable)gl.resolve("glXGetCurrentReadDrawable");
 if (bo_glXGetCurrentReadDrawable == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetCurrentReadDrawable" << endl;
	return false;
 }
 bo_glXGetCurrentDisplay = (_glXGetCurrentDisplay)gl.resolve("glXGetCurrentDisplay");
 if (bo_glXGetCurrentDisplay == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetCurrentDisplay" << endl;
	return false;
 }
 bo_glXQueryContext = (_glXQueryContext)gl.resolve("glXQueryContext");
 if (bo_glXQueryContext == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXQueryContext" << endl;
	return false;
 }
 bo_glXSelectEvent = (_glXSelectEvent)gl.resolve("glXSelectEvent");
 if (bo_glXSelectEvent == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXSelectEvent" << endl;
	return false;
 }
 bo_glXGetSelectedEvent = (_glXGetSelectedEvent)gl.resolve("glXGetSelectedEvent");
 if (bo_glXGetSelectedEvent == 0) {
	boError() << k_funcinfo << "unable to resolve symbol " << "glXGetSelectedEvent" << endl;
	return false;
 }

 // AB: it seems glXGetProcAddress() is not available on all systems (Ben's
 // gentoo?). probably this is _not_ critical, as glXGetProcAddressARB() is the
 // one we actually need!
 bo_glXGetProcAddress = (_glXGetProcAddress)gl.resolve("glXGetProcAddress");
 bo_glXGetContextIDEXT = (_glXGetContextIDEXT)gl.resolve("glXGetContextIDEXT");
 bo_glXImportContextEXT = (_glXImportContextEXT)gl.resolve("glXImportContextEXT");
 bo_glXFreeContextEXT = (_glXFreeContextEXT)gl.resolve("glXFreeContextEXT");
 bo_glXQueryContextInfoEXT = (_glXQueryContextInfoEXT)gl.resolve("glXQueryContextInfoEXT");
 bo_glXGetCurrentDisplayEXT = (_glXGetCurrentDisplayEXT)gl.resolve("glXGetCurrentDisplayEXT");
 bo_glXGetProcAddressARB = (_glXGetProcAddressARB)gl.resolve("glXGetProcAddressARB");
 bo_glXAllocateMemoryNV = (_glXAllocateMemoryNV)gl.resolve("glXAllocateMemoryNV");
 bo_glXFreeMemoryNV = (_glXFreeMemoryNV)gl.resolve("glXFreeMemoryNV");
 bo_glXGetAGPOffsetMESA = (_glXGetAGPOffsetMESA)gl.resolve("glXGetAGPOffsetMESA");
 // GLX_SGIX_fbconfig
 bo_glXGetFBConfigAttribSGIX = (_glXGetFBConfigAttribSGIX)gl.resolve("glXGetFBConfigAttribSGIX");
 bo_glXChooseFBConfigSGIX = (_glXChooseFBConfigSGIX)gl.resolve("glXChooseFBConfigSGIX");
 bo_glXCreateGLXPixmapWithConfigSGIX = (_glXCreateGLXPixmapWithConfigSGIX)gl.resolve("glXCreateGLXPixmapWithConfigSGIX");
 bo_glXCreateContextWithConfigSGIX = (_glXCreateContextWithConfigSGIX)gl.resolve("glXCreateContextWithConfigSGIX");
 bo_glXGetVisualFromFBConfigSGIX = (_glXGetVisualFromFBConfigSGIX)gl.resolve("glXGetVisualFromFBConfigSGIX");
 bo_glXGetFBConfigFromVisualSGIX = (_glXGetFBConfigFromVisualSGIX)gl.resolve("glXGetFBConfigFromVisualSGIX");
 //GLX_SGIX_pbuffer
 bo_glXCreateGLXPbufferSGIX = (_glXCreateGLXPbufferSGIX)gl.resolve("glXCreateGLXPbufferSGIX");
 bo_glXDestroyGLXPbufferSGIX = (_glXDestroyGLXPbufferSGIX)gl.resolve("glXDestroyGLXPbufferSGIX");
 bo_glXQueryGLXPbufferSGIX = (_glXQueryGLXPbufferSGIX)gl.resolve("glXQueryGLXPbufferSGIX");
 bo_glXSelectEventSGIX = (_glXSelectEventSGIX)gl.resolve("glXSelectEventSGIX");
 bo_glXGetSelectedEventSGIX = (_glXGetSelectedEventSGIX)gl.resolve("glXGetSelectedEventSGIX");
 return true;
}

