/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks <rivolaks@hot.ee>

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

#define GL_GLEXT_LEGACY
#define GLX_GLXEXT_PROTOTYPES

#define QT_CLEAN_NAMESPACE

#include "bogl.h"

#include "bodebug.h"

#include <qstringlist.h>

#include <GL/glx.h>


// bogl variables
bool bogl_inited = false;

// Function pointers for extensions
// Blendcolor
_boglBlendColor boglBlendColor = 0;
// VBO
_boglDeleteBuffers boglDeleteBuffers = 0;
_boglGenBuffers boglGenBuffers = 0;
_boglBindBuffer boglBindBuffer = 0;
_boglBufferData boglBufferData = 0;
_boglMapBuffer boglMapBuffer = 0;
_boglUnmapBuffer boglUnmapBuffer = 0;
// Textures
_boglActiveTexture boglActiveTexture = 0;


void boglInit()
{
  if(bogl_inited)
  {
    boDebug() << k_funcinfo << "OpenGL already inited, returning" << endl;
    return;
  }

#ifdef GLX_ARB_get_proc_address
  // Get pointers to supported opengl functions
  boDebug() << k_funcinfo << "Checking for OpenGL extensions..." << endl;
  QStringList extensions = boglGetOpenGLExtensions();
  unsigned int openglversion = boglGetOpenGLVersion();

  // Blendcolor
  if(extensions.contains("GL_ARB_imaging"))
  {
    boglBlendColor = (_boglBlendColor)glXGetProcAddressARB((const GLubyte*)"glBlendColor");
  }
  else if(extensions.contains("GL_EXT_blend_color"))
  {
    boglBlendColor = (_boglBlendColor)glXGetProcAddressARB((const GLubyte*)"glBlendColorEXT");
  }

  // VBO
  if(extensions.contains("GL_ARB_vertex_buffer_object"))
  {
    boglDeleteBuffers = (_boglDeleteBuffers)glXGetProcAddressARB((const GLubyte*)"glDeleteBuffersARB");
    boglGenBuffers = (_boglGenBuffers)glXGetProcAddressARB((const GLubyte*)"glGenBuffersARB");
    boglBindBuffer = (_boglBindBuffer)glXGetProcAddressARB((const GLubyte*)"glBindBufferARB");
    boglBufferData = (_boglBufferData)glXGetProcAddressARB((const GLubyte*)"glBufferDataARB");
    boglMapBuffer = (_boglMapBuffer)glXGetProcAddressARB((const GLubyte*)"glMapBufferARB");
    boglUnmapBuffer = (_boglUnmapBuffer)glXGetProcAddressARB((const GLubyte*)"glUnmapBufferARB");
  }
  // TODO: check for OpenGL 2.0

  // Textures
  if(openglversion >= MAKE_VERSION_BOGL(1,3,0))
  {
    boglActiveTexture = (_boglActiveTexture)glXGetProcAddressARB((const GLubyte*)"glActiveTexture");
  }
  else if(extensions.contains("GL_ARB_multitexture"))
  {
    boglActiveTexture = (_boglActiveTexture)glXGetProcAddressARB((const GLubyte*)"glActiveTextureARB");
  }
#else //GLX_ARB_get_proc_address
  // TODO: maybe make this an error?
#warning cannot find GLX_ARB_get_proc_address - cant use extensions
  boError() << k_funcinfo << "GLX_ARB_get_proc_address not available. please report this with information about your system!" << endl;
#endif //GLX_ARB_get_proc_address

  // Done
  bogl_inited = true;
}

QStringList boglGetOpenGLExtensions()
{
 QString extensions = (const char*)glGetString(GL_EXTENSIONS);
 return QStringList::split(" ", extensions);
}

QStringList boglGetGLUExtensions()
{
 QString extensions = (const char*)gluGetString(GLU_EXTENSIONS);
 return QStringList::split(" ", extensions);
}

unsigned int boglGetOpenGLVersion()
{
 // Find out OpenGL version
 QString oglversionstring = QString((const char*)glGetString(GL_VERSION));
 unsigned int oglversionmajor = 0, oglversionminor = 0, oglversionrelease = 0;
 int oglversionlength = oglversionstring.find(' ');
 if (oglversionlength == -1) {
	oglversionlength = oglversionstring.length();
 }

 QString versionstr = oglversionstring.left(oglversionlength);
 QStringList versioninfo = QStringList::split(QChar('.'), oglversionstring.left(oglversionlength));
 if (versioninfo.count() < 2 || versioninfo.count() > 3) {
	boError() << k_funcinfo << "versioninfo has " << versioninfo.count() <<
			" entries (version string: '" << oglversionstring << "')" << endl;
 } else {
	oglversionmajor = versioninfo[0].toUInt();
	oglversionminor = versioninfo[1].toUInt();
	if (versioninfo.count() == 3) {
		oglversionrelease = versioninfo[2].toUInt();
	}
 }
 return MAKE_VERSION_BOGL(oglversionmajor, oglversionminor, oglversionrelease);
}

QString boglGetOpenGLVersionString()
{
 return QString::fromLatin1((const char*)glGetString(GL_VERSION));
}

QString boglGetOpenGLVendorString()
{
 return QString::fromLatin1((const char*)glGetString(GL_VENDOR));
}

QString boglGetOpenGLRendererString()
{
 return QString::fromLatin1((const char*)glGetString(GL_RENDERER));
}

QString boglGetGLUVersionString()
{
 return QString::fromLatin1((const char*)gluGetString(GLU_VERSION));
}

