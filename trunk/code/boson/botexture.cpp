/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "botexture.h"

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#include <qstring.h>
#include <qimage.h>
#include <qgl.h>

#include "info/boinfo.h"
#include "bodebug.h"


BoTexture::BoTexture(Class texclass)
{
  setOptions(texclass);
  init();
}

BoTexture::BoTexture(int options, Type type)
{
  mType = type;
  mOptions = options;
  init();
}

BoTexture::BoTexture(const QString& name, Class texclass)
{
  setOptions(texclass);
  init();
  load(name);
}

BoTexture::BoTexture(const QString& name, int options, Type type)
{
  mType = type;
  mOptions = options;
  init();
  load(name);
}

BoTexture::BoTexture(const unsigned char* data, int width, int height, Class texclass)
{
  setOptions(texclass);
  init();
  load(data, width, height);
}

BoTexture::BoTexture(const unsigned char* data, int width, int height, int options, Type type)
{
  mType = type;
  mOptions = options;
  init();
  load(data, width, height);
}

BoTexture::~BoTexture()
{
  glDeleteTextures(1, &mId);
}

void BoTexture::init()
{
  glGenTextures(1, &mId);
  applyOptions();
}

void BoTexture::bind()
{
  glBindTexture(mType, mId);
}

void BoTexture::enable()
{
  glEnable(mType);
}

void BoTexture::disable()
{
  glDisable(mType);
}

void BoTexture::setOptions(Class c)
{
  if(c == Model)
  {
    mType = Texture2D;
    mOptions = FilterLinearMipmapLinear | FormatAuto;
  }
  else if(c == Terrain)
  {
    mType = Texture2D;
    mOptions = FilterLinearMipmapLinear | FormatAuto;
  }
  else if(c == Particle)
  {
    mType = Texture2D;
    mOptions = FilterLinearMipmapLinear | FormatAuto;
  }
  else if(c == UI)
  {
    mType = Texture2D;
    mOptions = FilterLinear | FormatAuto;
  }
}

void BoTexture::applyOptions()
{
  // Bind
  bind();

  if(mOptions & FilterNearest)
  {
    glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  else if(mOptions & FilterLinear)
  {
    glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  else if(mOptions & FilterNearestMipmapNearest)
  {
    glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // is this ok?
    glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  }
  else if(mOptions & FilterNearestMipmapLinear)
  {
    glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // is this ok?
    glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  }
  else if(mOptions & FilterLinearMipmapNearest)
  {
    glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  }
  else if(mOptions & FilterLinearMipmapLinear)
  {
    glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }
}

void BoTexture::load(const QString& name)
{
  if(mType == TextureCube)
  {
    const char* sides[6] = { "px", "nx", "py", "ny", "pz", "nz" };
    // QImage always uses RGBA
    mOptions |= FormatRGBA;
    mOptions &= (~FormatAuto);
    for(int i = 0; i < 6; i++)
    {
      // Load the image using QImage
      QString sidename = name.arg(sides[i]);
      QImage img(sidename);
      if(img.isNull())
      {
        boError() << k_funcinfo << "Couldn't load image for side " << i << " from file '" <<
            sidename << "'" << endl;
        return;
      }
      QImage tex = QGLWidget::convertToGLFormat(img);
      load(tex.bits(), tex.width(), tex.height(), i);
    }
  }
  else
  {
    // Load the image using QImage
    QImage img(name);
    if(img.isNull())
    {
      boError() << k_funcinfo << "Couldn't load image from file '" << name << "'" << endl;
      return;
    }
    QImage tex = QGLWidget::convertToGLFormat(img);
    // QImage always uses RGBA
    mOptions |= FormatRGBA;
    mOptions &= (~FormatAuto);
    load(tex.bits(), tex.width(), tex.height());
  }
}

void BoTexture::load(const unsigned char* data, int width, int height, int side)
{
  // Bind the texture
  bind();

  // Find out type
  GLenum type = mType;
  if(mType == TextureCube)
  {
    if(side < 0 || side > 6)
    {
      boError() << k_funcinfo << "Invalid side " << side << endl;
      side = 0;
    }
    // Special boson logic: swap y and z sides
    /*if(side == 2 || side == 3)  // *y side
    {
      // Turn *y side into *z side
      side += 2;
    }
    else if(side == 4 || side == 5)
    {
      // Turn *z side into *y side
      side -= 2;
    }*/
    type = GL_TEXTURE_CUBE_MAP_POSITIVE_X + side;
  }

  // Find out format
  GLenum format = (mOptions & FormatRGB) ? GL_RGB : GL_RGBA;

  // Find out internal format
  GLenum internalFormat;
  // Check if we can use texture compression
#ifdef GL_EXT_TEXTURE_COMPRESSION_S3TC
  if(BoInfo::boInfo()->openGLExtensions().contains("GL_EXT_texture_compression_s3tc"))
  {
    // Texture compression is in core since 1.3 and was earlier available via
    //  GL_ARB_texture_compression extension.
    bool compressionAvailable = false;
#ifdef GL_VERSION_1_3
    if(BoInfo::boInfo()->hasOpenGLVersion(1,3,0))
    {
      compressionAvailable = true;
    }
    else
#endif
#ifdef GL_EXT_TEXTURE_COMPRESSION_S3TC
    if(BoInfo::boInfo()->openGLExtensions().contains("GL_ARB_texture_compression"))
    {
      compressionAvailable = true;
    }
#endif
    if(compressionAvailable)
    {
      internalFormat = (mOptions & FormatRGB) ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
    else
    {
      internalFormat = format;
    }
  }
  else
#endif // GL_EXT_TEXTURE_COMPRESSION_S3TC
  {
    internalFormat = format;
  }

  // Check if we need to generate mipmaps
  int useMipmaps = FilterNearestMipmapNearest | FilterNearestMipmapLinear |
      FilterLinearMipmapNearest | FilterLinearMipmapLinear;
  if((mOptions & useMipmaps) && !(mOptions & DontGenMipmaps))
  {
    // Generate mipmaps
#ifdef GL_VERSION_1_4
    // Since version 1.4, OpenGL has automatic mipmap generation in core.
    if(BoInfo::boInfo()->hasOpenGLVersion(1,4,0))
    {
      glTexParameteri(mType, GL_GENERATE_MIPMAP, GL_TRUE);
    }
    else
#endif
#ifdef GL_SGIS_GENERATE_MIPMAP
    // If we don't have OGL 1.4, we may still have GL_SGIS_generate_mipmap
    //  extension.
    if(BoInfo::boInfo()->openGLExtensions().contains("GL_SGIS_generate_mipmap"))
    {
      glTexParameteri(mType, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    }
    else
#endif
    {
      // Can't generate mipmaps on hardware. Use software fallback.
      int error = gluBuild2DMipmaps(type, internalFormat, width, height,
          format, GL_UNSIGNED_BYTE, data);
      if (error) {
        boWarning() << k_funcinfo << "gluBuild2DMipmaps returned error: " << error << endl;
      }
    }
  }

  // Load base texture
  glTexImage2D(type, 0, internalFormat, width, height, 0, format,
      GL_UNSIGNED_BYTE, data);

  mWidth = width;
  mHeight = height;
  mDepth = 0;
}

