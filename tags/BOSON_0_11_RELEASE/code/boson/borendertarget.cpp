/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

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

#include "borendertarget.h"

#include "../bomemory/bodummymemory.h"
#include "boglx.h"
#include "bodebug.h"
#include "botexture.h"

#include <string.h>



class BoRenderTarget::PBufferData
{
  public:
    Display* display;

    GLXPbuffer pbuffer;
    GLXContext context;

    GLXPbuffer oldpbuffer;
    GLXContext oldcontext;
};


BoRenderTarget::BoRenderTarget(int width, int height, int flags, BoTexture* tex)
{
  mTexture = 0;
  mPBufferData = 0;
  mValid = false;

  mWidth = width;
  mHeight = height;
  mFlags = flags;

  // TODO: check for FBO
  mType = PBuffer;

  if(mType == PBuffer)
  {
    initPBuffer();
  }

  if(tex)
  {
    setTexture(tex);
  }
}

BoRenderTarget::~BoRenderTarget()
{
  if(mType == PBuffer && mPBufferData)
  {
    if(mPBufferData->context)
    {
      glXDestroyContext(mPBufferData->display, mPBufferData->context);
    }
    if(mPBufferData->pbuffer)
    {
      glXDestroyPbuffer(mPBufferData->display, mPBufferData->pbuffer);
    }
    delete mPBufferData;
  }
}

bool BoRenderTarget::enable()
{
  if(!valid())
  {
    boError() << k_funcinfo << "Can't enable invalid render target!" << endl;
    return false;
  }

  mPBufferData->oldpbuffer = glXGetCurrentDrawable();
  mPBufferData->oldcontext = glXGetCurrentContext();

  if(!glXMakeCurrent(mPBufferData->display, mPBufferData->pbuffer, mPBufferData->context))
  {
    boError() << k_funcinfo << "Couldn't enable render target!" << endl;
    return false;
  }
  boTextureManager->invalidateCache();

  return true;
}

bool BoRenderTarget::disable(bool updatetex)
{
  if(!valid())
  {
    boError() << k_funcinfo << "Can't disable invalid render target!" << endl;
    return false;
  }

  // Copy PBuffer contents to the texture
  if(updatetex)
  {
    updateTexture();
  }

  if(!glXMakeCurrent(mPBufferData->display, mPBufferData->oldpbuffer, mPBufferData->oldcontext))
  {
    boError() << k_funcinfo << "Couldn't disable render target!" << endl;
    return false;
  }
  boTextureManager->invalidateCache();

  return true;
}

void BoRenderTarget::updateTexture(BoTexture* tex)
{
  if(!tex)
  {
    tex = mTexture;
  }
  if(tex)
  {
    tex->bind();
    glCopyTexSubImage2D(tex->type(), 0,  0, 0,  0, 0,  mWidth, mHeight);
  }
}

void BoRenderTarget::setTexture(BoTexture* tex)
{
  mTexture = tex;
}

void BoRenderTarget::initPBuffer()
{
  // How many bits per channel
  int bpc = (mFlags & Float) ? 32 : 8;

  int attrib[40];
  int i = 0;
  attrib[i++] = GLX_RENDER_TYPE;
  attrib[i++] = GLX_RGBA_BIT;
  attrib[i++] = GLX_DRAWABLE_TYPE;
  attrib[i++] = GLX_PBUFFER_BIT;
  if(mFlags & RGB || mFlags & RGBA) {
    attrib[i++] = GLX_RED_SIZE;
    attrib[i++] = bpc;
    attrib[i++] = GLX_GREEN_SIZE;
    attrib[i++] = bpc;
    attrib[i++] = GLX_BLUE_SIZE;
    attrib[i++] = bpc;
    if(mFlags & RGBA) {
      attrib[i++] = GLX_ALPHA_SIZE;
      attrib[i++] = bpc;
    }
  }
  if(mFlags & Depth) {
    attrib[i++] = GLX_DEPTH_SIZE;
    attrib[i++] = 24;
  }
  if(mFlags & Float)
  {
    attrib[i++] = GLX_FLOAT_COMPONENTS_NV;
    attrib[i++] = 1;
  }
  //attrib[i++] = GLX_LEVEL;
  //attrib[i++] = 0;
  attrib[i++] = 0;


  int pattrib[40];
  int pi = 0;

  pattrib[pi++] = GLX_LARGEST_PBUFFER;
  pattrib[pi++] = true;
  pattrib[pi++] = GLX_PRESERVED_CONTENTS;
  pattrib[pi++] = true;

  if(!createContext(attrib, i, pattrib, pi))
  {
    return;
  }

  mValid = true;
}

bool BoRenderTarget::createContext(int* attrib, int& i, int* pattrib, int& pi)
{
  Display *display = glXGetCurrentDisplay();
  int screen = DefaultScreen(display);
  GLXContext oldcontext = glXGetCurrentContext();

  GLXPbuffer pbuffer;
  GLXContext context;

  int count;
  GLXFBConfig *config;

  const char *extensions = glXQueryExtensionsString(display,screen);

  if(strstr(extensions,"GLX_SGIX_pbuffer") && strstr(extensions,"GLX_SGIX_fbconfig"))
  {
    pattrib[pi++] = 0;

    config = glXChooseFBConfigSGIX(display, screen, &attrib[0], &count);
    if(!config)
    {
      boError() << k_funcinfo << "glXChooseFBConfigSGIX() failed" << endl;
      return false;
    }

    pbuffer = glXCreateGLXPbufferSGIX(display, config[0], mWidth, mHeight, &pattrib[0]);
    if(!pbuffer)
    {
      boError() << k_funcinfo << "glXCreateGLXPbufferSGIX() failed" << endl;
      return false;
    }

    context = glXCreateContextWithConfigSGIX(display, config[0], GLX_RGBA_TYPE, oldcontext, true);
    if(!context)
    {
      boError() << k_funcinfo << "glXCreateContextWithConfigSGIX() failed" << endl;
      return false;
    }

  }
  else
  {
    pattrib[pi++] = GLX_PBUFFER_WIDTH;
    pattrib[pi++] = mWidth;
    pattrib[pi++] = GLX_PBUFFER_HEIGHT;
    pattrib[pi++] = mHeight;
    pattrib[pi++] = 0;

    config = glXChooseFBConfig(display, screen, &attrib[0], &count);
    if(!config)
    {
      boError() << k_funcinfo << "glXChooseFBConfig() failed" << endl;
      return false;
    }

    pbuffer = glXCreatePbuffer(display, config[0], &pattrib[0]);
    if(!pbuffer)
    {
      boError() << k_funcinfo << "glXCreatePbuffer() failed" << endl;
      return false;
    }

    /*XVisualInfo *visual = glXGetVisualFromFBConfig(display, config[0]);
    if(!visual)
    {
      boError() << k_funcinfo << "glXGetVisualFromFBConfig() failed" << endl;
      return false;
    }

    context = glXCreateContext(display, visual, oldcontext, true);*/
    context = glXCreateNewContext(display, config[0], GLX_RGBA_TYPE, oldcontext, GL_TRUE);
    if(!context)
    {
      boError() << k_funcinfo << "glXCreateContext() failed" << endl;
      return false;
    }
  }

  mPBufferData = new PBufferData;
  mPBufferData->display = display;

  mPBufferData->pbuffer = pbuffer;
  mPBufferData->context = context;

  mPBufferData->oldpbuffer = glXGetCurrentDrawable();
  mPBufferData->oldcontext = oldcontext;

  return true;
}

