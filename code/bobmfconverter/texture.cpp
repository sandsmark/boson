/*
    This file is part of the Boson game
    Copyright (C) 2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
}

bool Texture::load()
{
  if(mImage)
  {
    boWarning() << k_funcinfo << "Texture '" << mFilename << "' already loaded?!" << endl;
    return true;
  }

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

