/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "botexture.h"

#include <qstring.h>
#include <qimage.h>
#include <qgl.h>

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bosonconfig.h"
#include "bosonprofiling.h"
#include "info/boinfo.h"

#define USE_BTF 0
#if USE_BTF
#include "bobtfload.h"
#endif


/*****  BoTexture  *****/

BoTexture::BoTexture(Class texclass)
{
  setOptions(texclass);
  init();
}

BoTexture::BoTexture(int options, Type type)
{
  mType = type;
  mOptions = options;
  mClass = Custom;
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
  mClass = Custom;
  init();
  load(name);
}

BoTexture::BoTexture(unsigned char* data, int width, int height, Class texclass)
{
  setOptions(texclass);
  init();
  load(data, width, height);
}

BoTexture::BoTexture(unsigned char* data, int width, int height, int options, Type type)
{
  mType = type;
  mOptions = options;
  mClass = Custom;
  init();
  load(data, width, height);
}

BoTexture::~BoTexture()
{
  glDeleteTextures(1, &mId);
  // Unregister texture in texture manager
  boTextureManager->unregisterTexture(this);
}

void BoTexture::init()
{
  glGenTextures(1, &mId);
  applyOptions();
  // Reset dimensions
  mWidth = mHeight = mDepth = 0;
  mLoaded = false;
  // Register texture in texture manager
  boTextureManager->registerTexture(this);
}

void BoTexture::bind()
{
  boTextureManager->bindTexture(this);
}

void BoTexture::setOptions(Class c)
{
  mClass = c;
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
  else if(c == NormalMap)
  {
    mType = Texture2D;
    // Normal maps look bad with compression
    mOptions = FilterLinearMipmapLinear | FormatAuto | DontCompress;
  }
}

void BoTexture::applyOptions()
{
  // Bind
  bind();

  // This is the highest filter that can be used
  int maxfilter = boTextureManager->textureFilter();

  // These are the selected filters
  int filtermag = GL_LINEAR;
  int filtermin = GL_LINEAR;
  if(mOptions & FilterNearest)
  {
    filtermag = GL_NEAREST;
    filtermin = GL_NEAREST;
  }
  else if(mOptions & FilterLinear)
  {
    filtermag = GL_LINEAR;
    filtermin = GL_LINEAR;
  }
  else if(mOptions & FilterNearestMipmapNearest)
  {
    filtermag = GL_NEAREST;  // is this ok?
    filtermin = GL_NEAREST_MIPMAP_NEAREST;
  }
  else if(mOptions & FilterNearestMipmapLinear)
  {
    filtermag = GL_NEAREST;  // is this ok?
    filtermin = GL_NEAREST_MIPMAP_LINEAR;
  }
  else if(mOptions & FilterLinearMipmapNearest)
  {
    filtermag = GL_LINEAR;
    filtermin = GL_LINEAR_MIPMAP_NEAREST;
  }
  else if(mOptions & FilterLinearMipmapLinear)
  {
    filtermag = GL_LINEAR;
    filtermin = GL_LINEAR_MIPMAP_LINEAR;
  }

  // Make sure filter is allowed by config settings
  if(filtermag > maxfilter)
  {
    filtermag = maxfilter;
  }
  if(filtermin > maxfilter)
  {
    filtermin = maxfilter;
  }

  // Set correct filter
  glTexParameteri(mType, GL_TEXTURE_MAG_FILTER, filtermag);
  glTexParameteri(mType, GL_TEXTURE_MIN_FILTER, filtermin);

  // Set wrapping mode
  if(mOptions & ClampToEdge)
  {
    glTexParameteri(mType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(mType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  // Set anisotropy
  if(boTextureManager->supportsAnisotropicFiltering())
  {
    glTexParameteri(mType, GL_TEXTURE_MAX_ANISOTROPY_EXT, boTextureManager->textureAnisotropy());
  }
}

void BoTexture::load(const QString& name)
{
  BosonProfiler prof("BoTexture::load()");
  if(mType == TextureCube)
  {
    BosonProfiler prof("BoTexture::load() (TextureCube)");
    if(!boTextureManager->supportsTextureCube())
    {
      boError() << k_funcinfo << "Cubemap textures are not supported (trying to load from file '" <<
          name << "')!" << endl;
      return;
    }
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
        mLoaded = false;
        return;
      }
      img = QGLWidget::convertToGLFormat(img);
      load(img.bits(), img.width(), img.height(), i);
    }
  }
  else
  {
    // Load the image using QImage
#if !USE_BTF
    QImage img(name);
    if(img.isNull())
    {
      boError() << k_funcinfo << "Couldn't load image from file '" << name << "'" << endl;
      return;
    }
    img = QGLWidget::convertToGLFormat(img);
#else
    BoBTFLoad btfLoader(name);
    if(!btfLoader.loadTexture())
    {
      boError() << k_funcinfo << "Couldn't load image from file '" << name << "'" << endl;
      return;
    }
#endif
    // QImage always uses RGBA
    mOptions |= FormatRGBA;
    mOptions &= (~FormatAuto);
#if !USE_BTF
    load(img.bits(), img.width(), img.height());
#else
    bool useMipmaps = !(mOptions & DontGenMipmaps);
    mOptions |= DontGenMipmaps;
    load((unsigned char*)btfLoader.data(0), btfLoader.width(0), btfLoader.height(0));
    if(useMipmaps)
    {
      mOptions &= ~DontGenMipmaps;
      for(unsigned int i = 1; i < btfLoader.mipmapLevels(); i++)
      {
        glTexImage2D(mType, i, determineInternalFormat(),
            btfLoader.width(i), btfLoader.height(i),
            0, determineFormat(), GL_UNSIGNED_BYTE, btfLoader.data(i));
      }
    }
#endif
  }

  mFilePath = name;
}

void BoTexture::load(unsigned char* data, int width, int height, int side)
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
    type = GL_TEXTURE_CUBE_MAP_POSITIVE_X + side;
  }

  // Find out format
  GLenum format = determineFormat();

  // Find out internal format
  GLenum internalFormat = determineInternalFormat();

  // Ensure that texture has correct size
  unsigned char* newdata = ensureCorrectSize(data, width, height);
  if(data && !newdata)
  {
    return;
  }

  // Check if we need to generate mipmaps
  int useMipmaps = FilterNearestMipmapNearest | FilterNearestMipmapLinear |
      FilterLinearMipmapNearest | FilterLinearMipmapLinear;
  if((mOptions & useMipmaps) && !(mOptions & DontGenMipmaps))
  {
    // Generate mipmaps
    BosonProfiler profMipMaps("BoTexture::load() - generate mipmaps");
    if(newdata == 0)
    {
      // Just specify null data for every mipmap level
      int w = width;
      int h = height;
      int level = 0;
      while(true)
      {
        glTexImage2D(type, level, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, 0);
        if(w == 1 && h == 1)
        {
          break;
        }
        w = (w < 2) ? 1 : w / 2;
        h = (h < 2) ? 1 : h / 2;
        level++;
      }
    }
    else if(boTextureManager->useColoredMipmaps())
    {
      if(boTextureManager->supportsGenerateMipmap())
      {
        glTexParameteri(mType, GL_GENERATE_MIPMAP, GL_FALSE);
      }
      // Use colored mipmaps.
      int error = buildColoredMipmaps(newdata, width, height, format, internalFormat, type);
      if(error)
      {
        boWarning() << k_funcinfo << "buildColoredMipmaps() returned error: " << error << endl;
      }
    }
    else
    {
      // Use automatic mipmap generation if it's supported
      if(boTextureManager->supportsGenerateMipmap())
      {
        glTexParameteri(mType, GL_GENERATE_MIPMAP, GL_TRUE);
        // Just specify base level - mipmaps will be automatically created.
        glTexImage2D(type, 0, internalFormat, width, height, 0, format,
            GL_UNSIGNED_BYTE, newdata);
      }
      else
      {
        // Can't generate mipmaps on hardware. Use software fallback.
        int error = gluBuild2DMipmaps(type, internalFormat, width, height,
            format, GL_UNSIGNED_BYTE, newdata);
        if(error != 0)
        {
          boWarning() << k_funcinfo << "gluBuild2DMipmaps() returned error: " << error << endl;
        }
      }
    }
  }
  else
  {
    BosonProfiler prof("BoTexture::load() - glTexImage2D without mipmaps");
    // No mipmaps
    // Load base texture
    glTexImage2D(type, 0, internalFormat, width, height, 0, format,
        GL_UNSIGNED_BYTE, newdata);
  }


  mWidth = width;
  mHeight = height;
  mDepth = 0;

  bool wasloaded = mLoaded;
  mLoaded = true;
  // Reset filepath. If we're loading from a file, it will be set to correct
  //  path later (in another load() method).
  mFilePath = QString();

  if(newdata != data)
  {
    delete[] newdata;
  }

  boTextureManager->textureLoaded(this, !wasloaded);
}

unsigned char* BoTexture::ensureCorrectSize(unsigned char* data, int &width, int &height)
{
  BosonProfiler prof("BoTexture::ensureCorrectSize()");
  // Find out max size of the texture
  int maxSize;
  if(mType == Texture2D)
  {
    maxSize = boTextureManager->maxTextureSize();
  }
  else if(mType == Texture3D)
  {
    maxSize = boTextureManager->max3DTextureSize();
  }
  else if(mType == TextureCube)
  {
    maxSize = boTextureManager->maxCubeTextureSize();
  }
  else
  {
    boError() << k_funcinfo << "invalid type " << mType << endl;
    return data;
  }

  // Ensure correct width and height
  int newW, newH;
  if((mOptions & EnableNPOT) && boTextureManager->supportsNPOTTextures())
  {
    newW = QMIN(width, maxSize);
    newH = QMIN(height, maxSize);
  }
  else
  {
    newW = QMIN(nextPower2(width), maxSize);
    newH = QMIN(nextPower2(height), maxSize);
  }

  if(!data)
  {
    // Update dimensions
    width = newW;
    height = newH;
    // Return NULL data
    return 0;
  }

  if((newW != width) || (newH != height))
  {
    // Resize texture
    // TODO: this might be quite slow, I suppose. Perhaps we could provide our
    //  own specific scaling function?
    GLenum format = (mOptions & FormatRGB) ? GL_RGB : GL_RGBA;
    int bpp = (format == GL_RGB) ? 3 : 4;
    unsigned char* scaledImage = new unsigned char[newW * newH * bpp];
    int error = gluScaleImage(format, width, height, GL_UNSIGNED_BYTE, data,
        newW, newH, GL_UNSIGNED_BYTE, scaledImage);
    if(error)
    {
      boError() << k_funcinfo << "Error while scaling texture: " << error << endl;
      delete[] scaledImage;
      return 0;
    }
    // Update dimensions
    width = newW;
    height = newH;
    // Return scaled image data
    return scaledImage;
  }
  else
  {
    // No resizing is necessary, return original data.
    return data;
  }
}

int BoTexture::nextPower2(int n)
{
  if(n <= 0)
  {
    return 1;
  }
  int i = 1;
  while(n > i)
  {
    i *= 2;
  }
  return i;
}

GLenum BoTexture::determineFormat() const
{
  GLenum format;
  if(mOptions & FormatDepth)
  {
    format = GL_DEPTH_COMPONENT;
  }
  else
  {
    format = (mOptions & FormatRGB) ? GL_RGB : GL_RGBA;
  }
  return format;
}

GLenum BoTexture::determineInternalFormat() const
{
  GLenum internalFormat;
  // Check if we can use texture compression
  if(boTextureManager->supportsTextureCompression() && boTextureManager->useTextureCompression() && !(mOptions & DontCompress) && !(mOptions & FormatDepth))
  {
    internalFormat = (mOptions & FormatRGB) ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
  }
  else
  {
    internalFormat = determineFormat();
  }
  return internalFormat;
}

int BoTexture::memoryUsed() const
{
  if(!mLoaded)
  {
    return 0;
  }

  // Find out number of pixels in this texture.
  int pixels = mWidth * mHeight;
  if(mType == TextureCube)
  {
    pixels *= 6;
  }
  else if(mType == Texture3D)
  {
    pixels *= mDepth;
  }

  // Find out bytes-per-pixel
  // FIXME: at least most NVidia's cards (dunno about others) internally still
  //  use 4 bpp, even if format is RGB.
  int bpp = (mOptions & FormatDepth) ? 3 : ((mOptions & FormatRGB) ? 3 : 4);

  int bytes = pixels * bpp;

  // A bit more tricky stuff
  // Mipmaps
  int useMipmaps = FilterNearestMipmapNearest | FilterNearestMipmapLinear |
      FilterLinearMipmapNearest | FilterLinearMipmapLinear;
  if((mOptions & useMipmaps) && !(mOptions & DontGenMipmaps))
  {
    // Mipmaps are used, which makes the texture about 1/3 bigger.
    bytes = (int)(bytes * 1.333);
  }
  // Compression
  if(boTextureManager->supportsTextureCompression() && boTextureManager->useTextureCompression())
  {
    if((mOptions & DontCompress) || (mOptions & FormatDepth))
    {
      // No compression
    }
    else if(mOptions & FormatRGB)
    {
      // GL_COMPRESSED_RGB_S3TC_DXT1_EXT format uses 64 bits (8 bytes) of image
      //  data for every 4x4 block
      // No compression uses 4*4*3 = 48 bytes for such block, so the difference
      //  is 6 times.
      bytes /= 6;
    }
    else
    {
      // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT format uses 128 bits (16 bytes) of
      //  image data for every 4x4 block
      // No compression uses 4*4*4 = 64 bytes for such block, so the difference
      //  is 4 times.
      bytes /= 4;
    }
  }

  return bytes;
}

int BoTexture::buildColoredMipmaps(unsigned char* data, int width, int height,
         GLenum format, GLenum internalformat, GLenum target)
{
  // This is more tricky as we have to specify each mipmap separately to be
  //  able to color it.
  // This code is based on gluBuild2DMipmaps() function from glu/mipmap.c in Mesa 6.0.

  unsigned char* image;
  unsigned char* newimage;
  GLint unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
  GLint packrowlength, packalignment, packskiprows, packskippixels;
  GLint w, h;
  GLint neww, newh, level, bpp;
  int error;
  bool done;
  GLint retval = 0;

  bpp = (format == GL_RGB) ? 3 : 4;
  w = width;
  h = height;

  /* Get current glPixelStore values */
  glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unpackrowlength);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackalignment);
  glGetIntegerv(GL_UNPACK_SKIP_ROWS, &unpackskiprows);
  glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &unpackskippixels);
  glGetIntegerv(GL_PACK_ROW_LENGTH, &packrowlength);
  glGetIntegerv(GL_PACK_ALIGNMENT, &packalignment);
  glGetIntegerv(GL_PACK_SKIP_ROWS, &packskiprows);
  glGetIntegerv(GL_PACK_SKIP_PIXELS, &packskippixels);

  /* set pixel packing */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

  // Push attributes for glPixelTransfer()
  glPushAttrib(GL_PIXEL_MODE_BIT);

  // Mipmap coloring:
  // We first scale colors by 0.5 and then add mipmap color (which is also
  //  scaled by 0.5) to them. This makes the resulting color be 50-50 blend
  //  between original color and mipmap color.
  glPixelTransferf(GL_RED_SCALE, 0.5);
  glPixelTransferf(GL_GREEN_SCALE, 0.5);
  glPixelTransferf(GL_BLUE_SCALE, 0.5);
  // Mipmap colors:
  int mipmapColorsCount = 10;  // 512x512 texture has base level and 9 mipmaps
  const float mipmapColorsR[] = { 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0 };
  const float mipmapColorsG[] = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.5, 0.5, 0.5 };
  const float mipmapColorsB[] = { 0.0, 0.0, 1.0, 1.0, 0.5, 1.0, 0.5, 0.5, 0.0, 0.0 };


  done = false;
  image = data;

  level = 0;
  while (!done)
  {
    if (image != data)
    {
      /* set pixel unpacking */
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    }

    // Set mipmap color
    int colorindex = level;
    if(colorindex >= mipmapColorsCount)
    {
      colorindex = mipmapColorsCount - 1;
    }
    glPixelTransferf(GL_RED_BIAS,   0.5 * mipmapColorsR[colorindex]);
    glPixelTransferf(GL_GREEN_BIAS, 0.5 * mipmapColorsG[colorindex]);
    glPixelTransferf(GL_BLUE_BIAS,  0.5 * mipmapColorsB[colorindex]);

    glTexImage2D(target, level, internalformat, w, h, 0, format, GL_UNSIGNED_BYTE, image);

    if (w == 1 && h == 1)
    {
      break;
    }

    neww = (w < 2) ? 1 : w / 2;
    newh = (h < 2) ? 1 : h / 2;
    newimage = new unsigned char[(neww + 4) * newh * bpp];
    if (!newimage)
    {
      glPopAttrib();
      return GLU_OUT_OF_MEMORY;
    }

    error = gluScaleImage(format, w, h, GL_UNSIGNED_BYTE, image,
        neww, newh, GL_UNSIGNED_BYTE, newimage);
    if (error)
    {
      retval = error;
      done = true;
    }

    if (image != data)
    {
      delete[] image;
    }
    image = newimage;

    w = neww;
    h = newh;
    level++;
  }

  if (image != data)
  {
    delete[] image;
  }

  // Restore glPixelTransfer() attributes
  glPopAttrib();

  /* Restore original glPixelStore state */
  glPixelStorei(GL_UNPACK_ROW_LENGTH, unpackrowlength);
  glPixelStorei(GL_UNPACK_ALIGNMENT, unpackalignment);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, unpackskiprows);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, unpackskippixels);
  glPixelStorei(GL_PACK_ROW_LENGTH, packrowlength);
  glPixelStorei(GL_PACK_ALIGNMENT, packalignment);
  glPixelStorei(GL_PACK_SKIP_ROWS, packskiprows);
  glPixelStorei(GL_PACK_SKIP_PIXELS, packskippixels);

  return retval;
}

void BoTexture::reload()
{
  if(mFilePath.isNull())
  {
    return;
  }

  boDebug() << k_funcinfo << "Reloading texture from " << mFilePath << endl;

  applyOptions();
  // Reset dimensions
  mWidth = mHeight = mDepth = 0;

  // Load the texture
  QString oldpath = mFilePath;
  load(oldpath);
  mFilePath = oldpath;
}



/*****  BoTextureArray  *****/

BoTextureArray::BoTextureArray(const QStringList& files, BoTexture::Class texclass)
{
  mAutoDelete = true;
  mTextures.reserve(files.count());
  QStringList::ConstIterator it;
  for (it = files.begin(); it != files.end(); ++it)
  {
    BoTexture* tex = new BoTexture(*it, texclass);
    mTextures.append(tex);
  }
}

BoTextureArray::BoTextureArray(const QStringList& files, int options, BoTexture::Type type)
{
  mAutoDelete = true;
  mTextures.reserve(files.count());
  QStringList::ConstIterator it;
  for (it = files.begin(); it != files.end(); ++it)
  {
    BoTexture* tex = new BoTexture(*it, options, type);
    mTextures.append(tex);
  }
}

BoTextureArray::BoTextureArray(const QPtrList<BoTexture>& textures)
{
  mAutoDelete = false;
  mTextures.reserve(textures.count());
  QPtrListIterator<BoTexture> it(textures);
  while(it.current())
  {
    mTextures.append(it.current());
    ++it;
  }
}

BoTextureArray::~BoTextureArray()
{
  if(mAutoDelete)
  {
    // Delete all textures
    for(unsigned int i = 0; i < mTextures.count(); i++)
    {
      delete mTextures[i];
    }
  }
}




/*****  BoTextureManager  *****/

BoTextureManager* BoTextureManager::mManager = 0;

BoTextureManager* BoTextureManager::textureManager()
{
  if(!mManager)
  {
    boError() << k_funcinfo << "initStatic() has not been called yet!" << endl;
    return 0;
  }
  return mManager;
}

BoTextureManager::BoTextureManager()
{
  mOpenGLInited = false;

  mUsedTextureMemory = 0;
  mTextureBinds = 0;

  mActiveTextureUnit = 0;
  mActiveTextureType = 0;
  mActiveTexture = 0;

  mSupportsTexture3D = false;
  mSupportsTextureCube = false;
  mSupportsGenerateMipmap = false;
  mSupportsTextureCompressionS3TC = false;
  mMaxTextureSize = 0;
  mMax3DTextureSize = 0;
  mMaxCubeTextureSize = 0;
  mTextureUnits = 0;
  mMaxAnisotropy = 0;
}

BoTextureManager::~BoTextureManager()
{
  mConstTextures.clear();

  if(mTextures.count() > 0)
  {
    boDebug() << k_funcinfo << "Deleting remaining " << mTextures.count() << " textures" << endl;
  }
  mTextures.setAutoDelete(true);
  mTextures.clear();

  delete[] mActiveTexture;
  delete[] mActiveTextureType;
}

void BoTextureManager::initStatic()
{
  if(mManager)
  {
    return;
  }
  mManager = new BoTextureManager();
  mManager->initOpenGL();
}

void BoTextureManager::deleteStatic()
{
  delete mManager;
  mManager = 0;
}

void BoTextureManager::initOpenGL()
{
  if(mOpenGLInited)
  {
    boDebug() << k_funcinfo << "OpenGL already inited, returning" << endl;
    return;
  }

  // Get list of supported opengl extensions
  const BoInfoGLCache* glInfo = BoInfo::boInfo()->gl();
  QStringList extensions = glInfo->openGLExtensions();
  // Get OpenGL (runtime) version
  unsigned int openglversion = glInfo->openGLVersion();

  // Find out maximal 2d texture size
  mMaxTextureSize = glInfo->maxTextureSize();

  // Find out number of texture units that the card has
  mTextureUnits = glInfo->maxTextureUnits();
  if (mTextureUnits == 1)
  {
    boDebug() << k_funcinfo << "Multitexturing is not supported!" << endl;
  }

  // Check if cube map textures are supported.
  // Cube maps are part of the core since OpenGL 1.3
  mSupportsTextureCube = glInfo->supportsTextureCube();
  mMaxCubeTextureSize = glInfo->maxCubeMapTextureSize();
  if(!mSupportsTextureCube)
  {
    boDebug() << k_funcinfo << "Cube map textures are not supported!" << endl;
  }

  // Check if 3D textures are supported
  mSupportsTexture3D = glInfo->supportsTexture3D();
  mMax3DTextureSize = glInfo->max3DTextureSize();
  if(!mSupportsTexture3D)
  {
    boDebug() << k_funcinfo << "3D textures are not supported!" << endl;
  }

  // Check if automatic mipmap generation is supported
  mSupportsGenerateMipmap = glInfo->supportsGenerateMipmap();
  if(!mSupportsGenerateMipmap)
  {
    boDebug() << k_funcinfo << "Automatic mipmap generation is not supported!" << endl;
  }

  // Check if texture compression is supported
  mSupportsTextureCompressionS3TC = glInfo->supportsTextureCompressionS3TC();
  if(!mSupportsTextureCompressionS3TC)
  {
    boDebug() << k_funcinfo << "S3TC texture compression is not supported!" << endl;
  }

  // Check if NPOT (non-power-of-two) textures are supported
  mSupportsNPOTTextures = glInfo->supportsNPOTTextures();
  if(!mSupportsNPOTTextures)
  {
    boDebug() << k_funcinfo << "non-power-of-two textures are not supported!" << endl;
  }

  // Check if anisotropic texture filtering is supported
  mMaxAnisotropy = glInfo->maxTextureMaxAnisotropy();
  if(boConfig->intValue("TextureAnisotropy") > mMaxAnisotropy)
  {
    boConfig->setIntValue("TextureAnisotropy", mMaxAnisotropy);
  }


  // Init active texture cache
  mActiveTexture = new BoTexture*[mTextureUnits];
  mActiveTextureType = new int[mTextureUnits];
  invalidateCache();

  mOpenGLInited = true;
}

void BoTextureManager::bindTexture(BoTexture* texture, int textureUnit)
{
  activateTextureUnit(textureUnit);
  bindTexture(texture);
}

void BoTextureManager::bindTexture(BoTexture* texture)
{
  if(!mActiveTextureType)
  {
    return;
  }
  if(mActiveTextureType[mActiveTextureUnit] != texture->type())
  {
    // Disable current texture type and enable new one.
    if(mActiveTextureType[mActiveTextureUnit])
    {
      glDisable(mActiveTextureType[mActiveTextureUnit]);
    }
    glEnable(texture->type());
    mActiveTextureType[mActiveTextureUnit] = texture->type();
  }
  if(mActiveTexture[mActiveTextureUnit] != texture)
  {
    // Bind new texture and update cache.
    glBindTexture(texture->type(), texture->id());
    mActiveTexture[mActiveTextureUnit] = texture;
    mTextureBinds++;
  }
}

void BoTextureManager::unbindTexture(int textureUnit)
{
  activateTextureUnit(textureUnit);
  unbindTexture();
}

void BoTextureManager::unbindTexture()
{
  if(!mActiveTextureType)
  {
    return;
  }
  if(mActiveTextureType[mActiveTextureUnit] != 0)
  {
    glBindTexture(mActiveTextureType[mActiveTextureUnit], 0);
    mActiveTexture[mActiveTextureUnit] = 0;
    mTextureBinds++;
  }
}

void BoTextureManager::disableTexturing()
{
  if(!mActiveTextureType)
  {
    return;
  }
  if(mActiveTextureType[mActiveTextureUnit] != 0)
  {
    glDisable(mActiveTextureType[mActiveTextureUnit]);
    mActiveTextureType[mActiveTextureUnit] = 0;
  }
}

void BoTextureManager::activateTextureUnit(int textureUnit)
{
  if(textureUnit < 0 || textureUnit >= mTextureUnits)
  {
    boError() << k_funcinfo << "Invalid texture unit " << textureUnit <<
        " (" << mTextureUnits << " texture units are supported" << endl;
    return;
  }
  if(mActiveTextureUnit != textureUnit)
  {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    mActiveTextureUnit = textureUnit;
  }
}

void BoTextureManager::invalidateCache()
{
  for(int i = 0; i < mTextureUnits; i++)
  {
    // Disable all texturing
    if(mTextureUnits > 1)
    {
      glActiveTexture(GL_TEXTURE0 + i);
    }
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    if(supportsTexture3D())
    {
      glDisable(GL_TEXTURE_3D);
    }
    if(supportsTextureCube())
    {
      glDisable(GL_TEXTURE_CUBE_MAP);
    }
    // TODO: do we need to bind any (0) texture here?
    // Reset cache variables
    mActiveTexture[i] = 0;
    mActiveTextureType[i] = 0;  // 0 means none
  }
  if(mTextureUnits > 1)
  {
    glActiveTexture(GL_TEXTURE0);
  }
  mActiveTextureUnit = 0;
}

void BoTextureManager::registerTexture(BoTexture* tex)
{
  mTextures.append(tex);
  mConstTextures.append(tex);
}

void BoTextureManager::unregisterTexture(BoTexture* tex)
{
  if(tex->loaded())
  {
    mUsedTextureMemory = QMAX(0, mUsedTextureMemory - tex->memoryUsed());
  }

  mTextures.remove(tex);
  mConstTextures.remove(tex);

  if(mActiveTexture[mActiveTextureUnit] == tex)
  {
    invalidateCache();
  }
}

void BoTextureManager::textureLoaded(BoTexture* tex, bool firsttime)
{
  if(firsttime)
  {
    // Increase amount of used texture memory
    mUsedTextureMemory += tex->memoryUsed();
  }
}

void BoTextureManager::clearStatistics()
{
  mTextureBinds = 0;
}

void BoTextureManager::textureFilterChanged()
{
  // Go through all the textures and change their filters.
  QPtrListIterator<BoTexture> it(mTextures);
  while(it.current())
  {
    it.current()->applyOptions();
    ++it;
  }
}

void BoTextureManager::reloadTextures()
{
  // Go through all the textures and change their filters.
  QPtrListIterator<BoTexture> it(mTextures);
  int total = 0;
  int reloaded = 0;
  while(it.current())
  {
    if(!it.current()->filePath().isNull())
    {
      // Texture has been loaded from image file. Reload it.
      it.current()->reload();
      reloaded++;
    }
    total++;
    ++it;
  }
  boDebug() << k_funcinfo << reloaded << " of " << total << " textures reloaded" << endl;
}

bool BoTextureManager::useColoredMipmaps() const
{
  return boConfig->boolValue("TextureColorMipmaps");
}

bool BoTextureManager::useTextureCompression() const
{
  return (boConfig->boolValue("TextureCompression") && !boConfig->boolValue("ForceDisableTextureCompression"));
}

int BoTextureManager::textureFilter() const
{
  return boConfig->intValue("TextureFilter");
}

int BoTextureManager::textureAnisotropy() const
{
  return boConfig->intValue("TextureAnisotropy");
}


/*
 * vim: et sw=2
 */
