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

#include "texture.h"

#include "debug.h"

#include <qimage.h>
#include <qfileinfo.h>


Texture::Texture(const QString& filename)
{
  mFilename = filename;
  mId = -1;
  mTotalUsedTexArea = -1;
  mTotalUsedFaceArea = -1;
  mUsedMinX = mUsedMinY = mUsedMaxX = mUsedMaxY = 0;
  mImage = 0;
  mHasTransparency = false;

  if(filename.lower().endsWith(".png"))
  {
#warning this leads to wrong results with -noloadtex
    mHasTransparency = true;
  }
}

bool Texture::load()
{
  if(mImage)
  {
    // Texture is already loaded
    return true;
  }
  boDebug() << k_funcinfo << "loading texture " << mFilename << endl;

  // Find out complete file path of the texture file (e.g. /foo/bar.jpg)
  QString filepath;
  for(QStringList::Iterator it = mTexturePaths.begin(); it != mTexturePaths.end(); ++it)
  {
    if(checkTexturePath(*it, filepath))
    {
      break;
    }
  }
  if(filepath.isNull())
  {
    // Try current path
    checkTexturePath("./", filepath);
  }

  if(filepath.isNull())
  {
    boError() << k_funcinfo << "No file '" << mFilename << "' found in any of texture paths" << endl;
    return false;
  }

  mImage = new QImage();
  if(!mImage->load(filepath))
  {
    boError() << k_funcinfo << "Couldn't load texture from file '" << filepath << "'" << endl;
    delete mImage;
    mImage = 0;
    return false;
  }

  // Check for transparency
  mHasTransparency = false;
  if(mImage->depth() != 32)
  {
    boWarning() << k_funcinfo << "Depth of '" << filepath << "' is " << mImage->depth() << endl;
    mHasTransparency = mImage->hasAlphaBuffer();
  }
  else if(mImage->hasAlphaBuffer())
  {
    // The alpha buffer might be unused, we need to check for it
    for(int y = 0; y < mImage->height(); y++)
    {
      uint* p = (uint*)mImage->scanLine(y);
      for(int x = 0; x < mImage->width(); x++)
      {
        if(qAlpha(*p) != 255)
        {
          mHasTransparency = true;
          break;
        }
#warning AB: I believe the next line is required
#if 0
        p++;
#endif
      }
      if(mHasTransparency)
      {
        break;
      }
    }
  }

  boDebug() << k_funcinfo << "'" << filepath << "' " << (mHasTransparency ? "has" : "doesn't have") << " transparency" << endl;

  return true;
}

bool Texture::checkTexturePath(const QString& path, QString& filepath)
{
  // We don't trust filenames found in model files, so we test the original
  //  filename plus upper and lowercase versions of it
  QFileInfo fi;
  fi.setFile(path + mFilename);
  if(fi.exists())
  {
    filepath = path + mFilename;
    return true;
  }
  fi.setFile(path + mFilename.lower());
  if(fi.exists())
  {
    filepath = path + mFilename.lower();
    return true;
  }
  fi.setFile(path + mFilename.upper());
  if(fi.exists())
  {
    filepath = path + mFilename.upper();
    return true;
  }

  return false;
}

int Texture::width() const
{
  if(!mImage)
  {
    return 0;
  }
  else
  {
    return mImage->width();
  }
}

int Texture::height() const
{
  if(!mImage)
  {
    return 0;
  }
  else
  {
    return mImage->height();
  }
}


// Static stuff
QStringList Texture::mTexturePaths;

void Texture::addTexturePath(const QString& path)
{
  mTexturePaths.append(path);
}

const QStringList& Texture::texturePaths()
{
  return mTexturePaths;
}

/*
 * vim: et sw=2
 */
