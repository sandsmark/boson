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

#ifndef BOGLXDECL_P_H
#define BOGLXDECL_P_H

#ifndef BOGL_H
#error Never include this file directly! Include boglx.h instead!
#endif

#include "bogl_do_dlopen.h"


// This file contains declarations of GL/glx.h ONLY!
// Do NOT add any other functions here!

// AB: we do this quite similar to Qt: define a typedef starting with an
// underscore, declare a variable and finally redefine the GL function to use
// our variable instead.
extern "C" {
// GLX typedefs
typedef XVisualInfo* (*_glXChooseVisual)(Display *dpy, int screen, int *attribList);
typedef void (*_glXCopyContext)(Display *dpy, GLXContext src, GLXContext dst, unsigned long mask);
typedef GLXContext (*_glXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
typedef GLXPixmap (*_glXCreateGLXPixmap)(Display *dpy, XVisualInfo *vis, Pixmap pixmap);
typedef void (*_glXDestroyContext)(Display *dpy, GLXContext ctx);
typedef void (*_glXDestroyGLXPixmap)(Display *dpy, GLXPixmap pix);
typedef int (*_glXGetConfig)(Display *dpy, XVisualInfo *vis, int attrib, int *value);
typedef GLXContext (*_glXGetCurrentContext)();
typedef GLXDrawable (*_glXGetCurrentDrawable)();
typedef Bool (*_glXIsDirect)(Display *dpy, GLXContext ctx);
typedef Bool (*_glXMakeCurrent)(Display *dpy, GLXDrawable drawable, GLXContext ctx);
typedef Bool (*_glXQueryExtension)(Display *dpy, int *errorBase, int *eventBase);
typedef Bool (*_glXQueryVersion)(Display *dpy, int *major, int *minor);
typedef void (*_glXSwapBuffers)(Display *dpy, GLXDrawable drawable);
typedef void (*_glXUseXFont)(Font font, int first, int count, int listBase);
typedef void (*_glXWaitGL)();
typedef void (*_glXWaitX)();
typedef const char* (*_glXGetClientString)(Display *dpy, int name);
typedef const char* (*_glXQueryServerString)(Display *dpy, int screen, int name);
typedef const char* (*_glXQueryExtensionsString)(Display *dpy, int screen);
typedef GLXFBConfig* (*_glXGetFBConfigs)(Display *dpy, int screen, int *nelements);
typedef GLXFBConfig* (*_glXChooseFBConfig)(Display *dpy, int screen, const int *attrib_list, int *nelements);
typedef int (*_glXGetFBConfigAttrib)(Display *dpy, GLXFBConfig config, int attribute, int *value);
typedef XVisualInfo* (*_glXGetVisualFromFBConfig)(Display *dpy, GLXFBConfig config);
typedef GLXWindow (*_glXCreateWindow)(Display *dpy, GLXFBConfig config, Window win, const int *attrib_list);
typedef void (*_glXDestroyWindow)(Display *dpy, GLXWindow win);
typedef GLXPixmap (*_glXCreatePixmap)(Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attrib_list);
typedef void (*_glXDestroyPixmap)(Display *dpy, GLXPixmap pixmap);
typedef GLXPbuffer (*_glXCreatePbuffer)(Display *dpy, GLXFBConfig config, const int *attrib_list);
typedef void (*_glXDestroyPbuffer)(Display *dpy, GLXPbuffer pbuf);
typedef void (*_glXQueryDrawable)(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
typedef GLXContext (*_glXCreateNewContext)(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
typedef Bool (*_glXMakeContextCurrent)(Display *display, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
typedef GLXDrawable (*_glXGetCurrentReadDrawable)();
typedef Display* (*_glXGetCurrentDisplay)();
typedef int (*_glXQueryContext)(Display *dpy, GLXContext ctx, int attribute, int *value);
typedef void (*_glXSelectEvent)(Display *dpy, GLXDrawable draw, unsigned long event_mask);
typedef void (*_glXGetSelectedEvent)(Display *dpy, GLXDrawable draw, unsigned long *event_mask);
typedef void* (*_glXGetProcAddress)(const GLubyte *procname);
typedef GLXContextID (*_glXGetContextIDEXT)(const GLXContext ctx);
typedef GLXContext (*_glXImportContextEXT)(Display *dpy, GLXContextID contextID);
typedef void (*_glXFreeContextEXT)(Display *dpy, GLXContext ctx);
typedef int (*_glXQueryContextInfoEXT)(Display *dpy, GLXContext ctx, int attribute, int *value);
typedef Display* (*_glXGetCurrentDisplayEXT)();
typedef void* (*_glXGetProcAddressARB)(const GLubyte *procName);
typedef void* (*_glXAllocateMemoryNV)(GLsizei size, GLfloat readfreq, GLfloat writefreq, GLfloat priority);
typedef void* (*_glXFreeMemoryNV)(GLvoid *pointer);
typedef GLuint (*_glXGetAGPOffsetMESA)(const GLvoid *pointer);
// GLX_SGIX_fbconfig
typedef int (*_glXGetFBConfigAttribSGIX)(Display *dpy, GLXFBConfigSGIX config,
    int attribute, int *value_return);
typedef GLXFBConfigSGIX* (*_glXChooseFBConfigSGIX)(Display *dpy, int screen,
    const int *attrib_list, int *nelements);
typedef GLXPixmap (*_glXCreateGLXPixmapWithConfigSGIX)(Display *dpy,
    GLXFBConfigSGIX config, Pixmap pixmap);
typedef GLXContext (*_glXCreateContextWithConfigSGIX)(Display *dpy,
    GLXFBConfigSGIX config, int render_type, GLXContext share_list, Bool direct);
typedef XVisualInfo* (*_glXGetVisualFromFBConfigSGIX)(Display *dpy, GLXFBConfigSGIX config);
typedef GLXFBConfigSGIX (*_glXGetFBConfigFromVisualSGIX)(Display *dpy, XVisualInfo *vis);
//GLX_SGIX_pbuffer
typedef GLXPbuffer (*_glXCreateGLXPbufferSGIX)(Display *dpy, GLXFBConfig config,
    unsigned int width, unsigned int height, const int *attrib_list);
typedef void (*_glXDestroyGLXPbufferSGIX)(Display *dpy, GLXPbuffer pbuf);
typedef void (*_glXQueryGLXPbufferSGIX)(Display *dpy, GLXPbuffer pbuf,
    int attribute, unsigned int *value);
typedef void (*_glXSelectEventSGIX)(Display *dpy, GLXDrawable drawable, unsigned long mask);
typedef void (*_glXGetSelectedEventSGIX)(Display *dpy, GLXDrawable drawable, unsigned long *mask);


// GLX function pointers
extern _glXChooseVisual bo_glXChooseVisual;
extern _glXCopyContext bo_glXCopyContext;
extern _glXCreateContext bo_glXCreateContext;
extern _glXCreateGLXPixmap bo_glXCreateGLXPixmap;
extern _glXDestroyContext bo_glXDestroyContext;
extern _glXDestroyGLXPixmap bo_glXDestroyGLXPixmap;
extern _glXGetConfig bo_glXGetConfig;
extern _glXGetCurrentContext bo_glXGetCurrentContext;
extern _glXGetCurrentDrawable bo_glXGetCurrentDrawable;
extern _glXIsDirect bo_glXIsDirect;
extern _glXMakeCurrent bo_glXMakeCurrent;
extern _glXQueryExtension bo_glXQueryExtension;
extern _glXQueryVersion bo_glXQueryVersion;
extern _glXSwapBuffers bo_glXSwapBuffers;
extern _glXUseXFont bo_glXUseXFont;
extern _glXWaitGL bo_glXWaitGL;
extern _glXWaitX bo_glXWaitX;
extern _glXGetClientString bo_glXGetClientString;
extern _glXQueryServerString bo_glXQueryServerString;
extern _glXQueryExtensionsString bo_glXQueryExtensionsString;
extern _glXGetFBConfigs bo_glXGetFBConfigs;
extern _glXChooseFBConfig bo_glXChooseFBConfig;
extern _glXGetFBConfigAttrib bo_glXGetFBConfigAttrib;
extern _glXGetVisualFromFBConfig bo_glXGetVisualFromFBConfig;
extern _glXCreateWindow bo_glXCreateWindow;
extern _glXDestroyWindow bo_glXDestroyWindow;
extern _glXCreatePixmap bo_glXCreatePixmap;
extern _glXDestroyPixmap bo_glXDestroyPixmap;
extern _glXCreatePbuffer bo_glXCreatePbuffer;
extern _glXDestroyPbuffer bo_glXDestroyPbuffer;
extern _glXQueryDrawable bo_glXQueryDrawable;
extern _glXCreateNewContext bo_glXCreateNewContext;
extern _glXMakeContextCurrent bo_glXMakeContextCurrent;
extern _glXGetCurrentReadDrawable bo_glXGetCurrentReadDrawable;
extern _glXGetCurrentDisplay bo_glXGetCurrentDisplay;
extern _glXQueryContext bo_glXQueryContext;
extern _glXSelectEvent bo_glXSelectEvent;
extern _glXGetSelectedEvent bo_glXGetSelectedEvent;
extern _glXGetProcAddress bo_glXGetProcAddress;
extern _glXGetContextIDEXT bo_glXGetContextIDEXT;
extern _glXImportContextEXT bo_glXImportContextEXT;
extern _glXFreeContextEXT bo_glXFreeContextEXT;
extern _glXQueryContextInfoEXT bo_glXQueryContextInfoEXT;
extern _glXGetCurrentDisplayEXT bo_glXGetCurrentDisplayEXT;
extern _glXGetProcAddressARB bo_glXGetProcAddressARB;
extern _glXAllocateMemoryNV bo_glXAllocateMemoryNV;
extern _glXFreeMemoryNV bo_glXFreeMemoryNV;
extern _glXGetAGPOffsetMESA bo_glXGetAGPOffsetMESA;
// GLX_SGIX_fbconfig
extern _glXGetFBConfigAttribSGIX bo_glXGetFBConfigAttribSGIX;
extern _glXChooseFBConfigSGIX bo_glXChooseFBConfigSGIX;
extern _glXCreateGLXPixmapWithConfigSGIX bo_glXCreateGLXPixmapWithConfigSGIX;
extern _glXCreateContextWithConfigSGIX bo_glXCreateContextWithConfigSGIX;
extern _glXGetVisualFromFBConfigSGIX bo_glXGetVisualFromFBConfigSGIX;
extern _glXGetFBConfigFromVisualSGIX bo_glXGetFBConfigFromVisualSGIX;
//GLX_SGIX_pbuffer
extern _glXCreateGLXPbufferSGIX bo_glXCreateGLXPbufferSGIX;
extern _glXDestroyGLXPbufferSGIX bo_glXDestroyGLXPbufferSGIX;
extern _glXQueryGLXPbufferSGIX bo_glXQueryGLXPbufferSGIX;
extern _glXSelectEventSGIX bo_glXSelectEventSGIX;
extern _glXGetSelectedEventSGIX bo_glXGetSelectedEventSGIX;
}; // extern "C"

// GLX defines
#if BOGL_DO_DLOPEN
#define glXChooseVisual bo_glXChooseVisual
#define glXCopyContext bo_glXCopyContext
#define glXCreateContext bo_glXCreateContext
#define glXCreateGLXPixmap bo_glXCreateGLXPixmap
#define glXDestroyContext bo_glXDestroyContext
#define glXDestroyGLXPixmap bo_glXDestroyGLXPixmap
#define glXGetConfig bo_glXGetConfig
#define glXGetCurrentContext bo_glXGetCurrentContext
#define glXGetCurrentDrawable bo_glXGetCurrentDrawable
#define glXIsDirect bo_glXIsDirect
#define glXMakeCurrent bo_glXMakeCurrent
#define glXQueryExtension bo_glXQueryExtension
#define glXQueryVersion bo_glXQueryVersion
#define glXSwapBuffers bo_glXSwapBuffers
#define glXUseXFont bo_glXUseXFont
#define glXWaitGL bo_glXWaitGL
#define glXWaitX bo_glXWaitX
#define glXGetClientString bo_glXGetClientString
#define glXQueryServerString bo_glXQueryServerString
#define glXQueryExtensionsString bo_glXQueryExtensionsString
#define glXGetFBConfigs bo_glXGetFBConfigs
#define glXChooseFBConfig bo_glXChooseFBConfig
#define glXGetFBConfigAttrib bo_glXGetFBConfigAttrib
#define glXGetVisualFromFBConfig bo_glXGetVisualFromFBConfig
#define glXCreateWindow bo_glXCreateWindow
#define glXDestroyWindow bo_glXDestroyWindow
#define glXCreatePixmap bo_glXCreatePixmap
#define glXDestroyPixmap bo_glXDestroyPixmap
#define glXCreatePbuffer bo_glXCreatePbuffer
#define glXDestroyPbuffer bo_glXDestroyPbuffer
#define glXQueryDrawable bo_glXQueryDrawable
#define glXCreateNewContext bo_glXCreateNewContext
#define glXMakeContextCurrent bo_glXMakeContextCurrent
#define glXGetCurrentReadDrawable bo_glXGetCurrentReadDrawable
#define glXGetCurrentDisplay bo_glXGetCurrentDisplay
#define glXQueryContext bo_glXQueryContext
#define glXSelectEvent bo_glXSelectEvent
#define glXGetSelectedEvent bo_glXGetSelectedEvent
#define glXGetProcAddress bo_glXGetProcAddress
#define glXGetContextIDEXT bo_glXGetContextIDEXT
#define glXImportContextEXT bo_glXImportContextEXT
#define glXFreeContextEXT bo_glXFreeContextEXT
#define glXQueryContextInfoEXT bo_glXQueryContextInfoEXT
#define glXGetCurrentDisplayEXT bo_glXGetCurrentDisplayEXT
#define glXGetProcAddressARB bo_glXGetProcAddressARB
#define glXAllocateMemoryNV bo_glXAllocateMemoryNV
#define glXFreeMemoryNV bo_glXFreeMemoryNV
#define glXGetAGPOffsetMESA bo_glXGetAGPOffsetMESA
// GLX_SGIX_fbconfig
#define glXGetFBConfigAttribSGIX bo_glXGetFBConfigAttribSGIX
#define glXChooseFBConfigSGIX bo_glXChooseFBConfigSGIX
#define glXCreateGLXPixmapWithConfigSGIX bo_glXCreateGLXPixmapWithConfigSGIX
#define glXCreateContextWithConfigSGIX bo_glXCreateContextWithConfigSGIX
#define glXGetVisualFromFBConfigSGIX bo_glXGetVisualFromFBConfigSGIX
#define glXGetFBConfigFromVisualSGIX bo_glXGetFBConfigFromVisualSGIX
//GLX_SGIX_pbuffer
#define glXCreateGLXPbufferSGIX bo_glXCreateGLXPbufferSGIX
#define glXDestroyGLXPbufferSGIX bo_glXDestroyGLXPbufferSGIX
#define glXQueryGLXPbufferSGIX bo_glXQueryGLXPbufferSGIX
#define glXSelectEventSGIX bo_glXSelectEventSGIX
#define glXGetSelectedEventSGIX bo_glXGetSelectedEventSGIX
#endif // BOGL_DO_DLOPEN


#endif // BOGLXDECL_P_H
