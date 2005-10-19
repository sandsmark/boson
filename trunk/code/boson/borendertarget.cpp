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
  if(extensions.contains("GL_EXT_framebuffer_object") && boglFramebufferTexture2D)
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
      boglDeleteRenderbuffers(1, &mFBOData->depthbuffer);
    }
    if(!mTexture && (mFlags & RGB || mFlags & RGBA))
    {
      boglDeleteRenderbuffers(1, &mFBOData->colorbuffer);
    }
    boglDeleteFramebuffers(1, &mFBOData->framebuffer);
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
    boglBindFramebuffer(GL_FRAMEBUFFER, mFBOData->framebuffer);
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

#include "bosonprofiling.h"
bool BoRenderTarget::disable()
{
	PROFILE_METHOD;
  if(!valid())
  {
    boError() << k_funcinfo << "Can't disable invalid render target!" << endl;
    return false;
  }

  if(mType == FBO)
  {
	  BosonProfiler prof("FOO1");
    boglBindFramebuffer(GL_FRAMEBUFFER, 0);
    glFinish();
  }
  else
  {
	  BosonProfiler prof("FOOX");
    // Copy PBuffer contents to the texture
    if(mTexture && (mFlags & RGB || mFlags & RGBA))
    {
	  BosonProfiler prof("FOO2");
      mTexture->bind();
    glFinish();
	  BosonProfiler prof2("FOO2.2");
      glCopyTexSubImage2D(mTexture->type(), 0,  0, 0,  0, 0,  mWidth, mHeight);
    glFinish();
    }
    if(mDepthTexture && (mFlags & Depth))
    {
	  BosonProfiler prof("FOO3");
      mDepthTexture->bind();
    glFinish();
	  BosonProfiler prof2("FOO3.2");
      glCopyTexSubImage2D(mDepthTexture->type(), 0,  0, 0,  0, 0,  mWidth, mHeight);
    glFinish();
    }

     BosonProfiler foo4("FOO4");
    if(!glXMakeCurrent(mPBufferData->display, mPBufferData->oldpbuffer, mPBufferData->oldcontext))
    {
      boError() << k_funcinfo << "Couldn't disable render target!" << endl;
      return false;
    }
    glFinish();
    foo4.pop();
    boTextureManager->invalidateCache();
  }
    glFinish();

  return true;
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

void BoRenderTarget::initFBO()
{
  mFBOData = new FBOData;
  boglGenFramebuffers(1, &mFBOData->framebuffer);
  boglBindFramebuffer(GL_FRAMEBUFFER, mFBOData->framebuffer);

  if(mFlags & RGB || mFlags & RGBA)
  {
    if(mTexture)
    {
      boglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture->id(), 0);
    }
    else
    {
      boglGenRenderbuffers(1, &mFBOData->colorbuffer);
      boglBindRenderbuffer(GL_RENDERBUFFER, mFBOData->colorbuffer);
      boglRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, mWidth, mHeight);
      boglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mFBOData->colorbuffer);
      boglBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
  }
  else
  {
    boglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
  }

  if(mFlags & Depth)
  {
    if(mDepthTexture)
    {
      boglFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture->id(), 0);
    }
    else
    {
      boglGenRenderbuffers(1, &mFBOData->depthbuffer);
      boglBindRenderbuffer(GL_RENDERBUFFER, mFBOData->depthbuffer);
      boglRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight);
      boglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mFBOData->depthbuffer);
      boglBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
  }
  else
  {
    boglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
  }

  GLenum status = boglCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE)
  {
    boError() << k_funcinfo << "Invalid fb status: " << status << endl;
  }

  boglBindFramebuffer(GL_FRAMEBUFFER, 0);

  mValid = true;
}

