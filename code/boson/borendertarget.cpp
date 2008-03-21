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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "borendertarget.h"

#include "../bomemory/bodummymemory.h"
#include "boglx.h"
#include "bodebug.h"
#include "botexture.h"
#include "info/boinfo.h"

#include <qstringlist.h>

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

class BoRenderTarget::FBOData
{
  public:
    GLuint framebuffer;
    GLuint depthbuffer;
    GLuint colorbuffer;
};


BoRenderTarget::BoRenderTarget(int width, int height, int flags, BoTexture* color, BoTexture* depth)
{
  const BoInfoGLCache* glInfo = BoInfo::boInfo()->gl();
  QStringList extensions = glInfo->openGLExtensions();
  // Use FBO if it's supported
  if(extensions.contains("GL_EXT_framebuffer_object") && glFramebufferTexture2DEXT)
  {
    mType = FBO;
  }
  else
  {
    mType = PBuffer;
  }

  // Reset variables
  mTexture = 0;
  mDepthTexture = 0;
  mPBufferData = 0;
  mFBOData = 0;
  mValid = false;

  mWidth = width;
  mHeight = height;
  mFlags = flags;

  mTexture = color;
  mDepthTexture = depth;

  if(mType == FBO)
  {
    initFBO();
  }
  else
  {
    initPBuffer();
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
  else if(mType == FBO && mFBOData)
  {
    // TODO: what if this changes during this object's lifetime?
    if(!mDepthTexture && (mFlags & Depth))
    {
      glDeleteRenderbuffersEXT(1, &mFBOData->depthbuffer);
    }
    if(!mTexture && (mFlags & RGB || mFlags & RGBA))
    {
      glDeleteRenderbuffersEXT(1, &mFBOData->colorbuffer);
    }
    glDeleteFramebuffersEXT(1, &mFBOData->framebuffer);
    delete mFBOData;
  }
}

bool BoRenderTarget::enable()
{
  if(!valid())
  {
    boError() << k_funcinfo << "Can't enable invalid render target!" << endl;
    return false;
  }

  if(mType == FBO)
  {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOData->framebuffer);
  }
  else
  {
    mPBufferData->oldpbuffer = glXGetCurrentDrawable();
    mPBufferData->oldcontext = glXGetCurrentContext();

    if(!glXMakeCurrent(mPBufferData->display, mPBufferData->pbuffer, mPBufferData->context))
    {
      boError() << k_funcinfo << "Couldn't enable render target!" << endl;
      return false;
    }
    boTextureManager->invalidateCache();
  }

  return true;
}

bool BoRenderTarget::disable()
{
  if(!valid())
  {
    boError() << k_funcinfo << "Can't disable invalid render target!" << endl;
    return false;
  }

  if(mType == FBO)
  {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  }
  else
  {
    // Copy PBuffer contents to the texture
    if(mTexture && (mFlags & RGB || mFlags & RGBA))
    {
      mTexture->bind();
      glCopyTexSubImage2D(mTexture->type(), 0,  0, 0,  0, 0,  mWidth, mHeight);
    }
    if(mDepthTexture && (mFlags & Depth))
    {
      mDepthTexture->bind();
      glCopyTexSubImage2D(mDepthTexture->type(), 0,  0, 0,  0, 0,  mWidth, mHeight);
    }

    if(!glXMakeCurrent(mPBufferData->display, mPBufferData->oldpbuffer, mPBufferData->oldcontext))
    {
      boError() << k_funcinfo << "Couldn't disable render target!" << endl;
      return false;
    }
    boTextureManager->invalidateCache();
  }

  return true;
}

void BoRenderTarget::initPBuffer()
{
  // How many bits per channel
  // 1 is minimum bpc of the color buffer, biggest available one is preferred
  int bpc = (mFlags & Float) ? 32 : 1;

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
    attrib[i++] = 1;  // 1 is minimum size of the depth buffer, biggest available one is preferred
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

void BoRenderTarget::initFBO()
{
  mFBOData = new FBOData;
  glGenFramebuffersEXT(1, &mFBOData->framebuffer);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOData->framebuffer);

  if(mFlags & RGB || mFlags & RGBA)
  {
    if(mTexture)
    {
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mTexture->id(), 0);
    }
    else
    {
      glGenRenderbuffersEXT(1, &mFBOData->colorbuffer);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mFBOData->colorbuffer);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, mWidth, mHeight);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, mFBOData->colorbuffer);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    }
  }
  else
  {
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 0, 0, 0);
  }

  if(mFlags & Depth)
  {
    if(mDepthTexture)
    {
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, mDepthTexture->id(), 0);
    }
    else
    {
      glGenRenderbuffersEXT(1, &mFBOData->depthbuffer);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mFBOData->depthbuffer);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, mWidth, mHeight);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mFBOData->depthbuffer);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    }
  }
  else
  {
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, 0);
  }

  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
  {
    boError() << k_funcinfo << "Invalid fb status: " << status << endl;
  }

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  mValid = true;
}

