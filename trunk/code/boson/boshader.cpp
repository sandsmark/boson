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

#include "boshader.h"

#include "../bomemory/bodummymemory.h"
#include "bogl.h"
#include "bodebug.h"
#include "bolight.h"
#include "bosonconfig.h"

#include <qstring.h>
#include <qdict.h>
#include <qfile.h>
#include <qregexp.h>

#include <kglobal.h>
#include <kstandarddirs.h>



BoShaderManager* BoShaderManager::mShaderManager = 0;

BoShaderManager* BoShaderManager::shaderManager()
{
  if(!mShaderManager)
  {
    mShaderManager = new BoShaderManager();
  }

  return mShaderManager;
}

BoShaderManager::BoShaderManager()
{
  mKnownShaderFiles.setAutoDelete(true);
  shaderSuffixesChanged();
}

BoShaderManager::~BoShaderManager()
{
  mKnownShaderFiles.clear();
}

void BoShaderManager::reloadShaders()
{
  shaderSuffixesChanged();
  // Reload all shaders
  QPtrListIterator<BoShader> it(mShaders);
  while(it.current())
  {
    it.current()->reload();
    ++it;
  }
}

const QString& BoShaderManager::getFullFilename(const QString& shadername)
{
  // Check if we have a cached filename
  QString* filename = mKnownShaderFiles.find(shadername);
  if(!filename)
  {
    // Find the shader file
    filename = new QString;
    // Try to use the filename directly at first
    QString path = KGlobal::dirs()->findResourceDir("data", "boson/themes/shaders/" + shadername);
    if(!path.isNull())
    {
      // Use this file
      *filename = path + "boson/themes/shaders/" + shadername;
      // FIXME: code duplication
      // Add the filename to the cache
      mKnownShaderFiles.insert(shadername, filename);
      return *filename;
    }
    // If direct filename didn't work, try with suffixes
    for(QStringList::Iterator it = mSuffixList.begin(); it != mSuffixList.end(); ++it)
    {
      QString path = KGlobal::dirs()->findResourceDir("data", "boson/themes/shaders/" + shadername + *it + ".shader");
      if(!path.isNull())
      {
        // Use this file
        *filename = path + "boson/themes/shaders/" + shadername + *it + ".shader";
        break;
      }
    }
    // Make sure something was found
    if(filename->isNull())
    {
      boError(130) << k_funcinfo << "Couldn't find any matching files for shader '" << shadername << "'" << endl;
      delete filename;
      return QString::null;
    }
    // Add the filename to the cache
    mKnownShaderFiles.insert(shadername, filename);
  }
  return *filename;
}

void BoShaderManager::shaderSuffixesChanged()
{
  // Clear old cached filenames
  mKnownShaderFiles.clear();
  // Create new suffixes list
  boDebug(130) << k_funcinfo << "suffixes: " << boConfig->stringValue("ShaderSuffixes") << endl;
  mSuffixList = QStringList::split(QChar(','), boConfig->stringValue("ShaderSuffixes"));
  mSuffixList.append("");
}

void BoShaderManager::registerShader(BoShader* shader)
{
  mShaders.append(shader);
}

void BoShaderManager::unregisterShader(BoShader* shader)
{
  mShaders.remove(shader);
}



BoShader* BoShader::mCurrentShader = 0;
BoVector3Float BoShader::mCameraPos;
BoLight* BoShader::mSun = 0;
float BoShader::mTime = 0.0f;
bool BoShader::mFogEnabled = false;
int BoShader::mActiveLights = 0;


BoShader::BoShader(const QString& vertex, const QString& fragment)
{
  mUniformLocations = 0;
  mProgram = 0;
  mValid = false;

  if(load(vertex, fragment))
  {
    boShaderManager->registerShader(this);
  }
}

BoShader::BoShader(const QString& name)
{
  mUniformLocations = 0;
  mProgram = 0;
  mValid = false;
  mName = name;

  if(load(name))
  {
    boShaderManager->registerShader(this);
  }
}

BoShader::~BoShader()
{
  if(mUniformLocations)
  {
    mUniformLocations->clear();
    delete mUniformLocations;
  }

  if(mProgram)
  {
    if(mCurrentShader == this)
    {
      unbind();
    }
    boglDeleteProgram(mProgram);
  }
}

bool BoShader::load(const QString& name)
{
  QString filename = boShaderManager->getFullFilename(name);
  QFile f(filename);
  if(!f.open(IO_ReadOnly))
  {
    boError(130) << k_funcinfo << "Couldn't open " << filename << " for reading!" << endl;
    return false;
  }

  QString contents(f.readAll());

  QString vertexsrc = preprocessSource(contents, VertexOnly);
  QString fragmentsrc = preprocessSource(contents, FragmentOnly);


  if(!load(vertexsrc, fragmentsrc))
  {
    boError() << k_funcinfo << "Couldn't load shader '" << name << "' from '" << filename << "'" << endl;
    return false;
  }
  return true;
}

bool BoShader::load(const QString& vertexsrc, const QString& fragmentsrc)
{
  if(fragmentsrc.isEmpty() && vertexsrc.isEmpty())
  {
    boError(130) << k_funcinfo << "No shader sources given!" << endl;
    return false;
  }
  if(!boglCreateProgram)
  {
    boError(130) << k_funcinfo << "Shaders aren't supported!" << endl;
    return false;
  }

  // Create program object
  mProgram = boglCreateProgram();

  GLuint vertexshader;
  GLuint fragmentshader;

  char log[8192];
  GLsizei logsize;

  // Load vertex shader, if it was given
  if(!vertexsrc.isEmpty())
  {
    // Create shader object
    vertexshader = boglCreateShader(GL_VERTEX_SHADER);
    // Load it
    const char* src = vertexsrc.latin1();
    boglShaderSource(vertexshader, 1, &src, NULL);
    // Compile the shader
    boglCompileShader(vertexshader);
    // Make sure it compiled correctly
    int compiled;
    boglGetShaderiv(vertexshader, GL_COMPILE_STATUS, &compiled);
    boglGetShaderInfoLog(vertexshader, 8192, &logsize, log);
    if(!compiled)
    {
      boError(130) << k_funcinfo << "Couldn't compile vertex shader!" << endl << log;
      return false;
    }
    else if(logsize > 0)
    {
      boDebug(130) << "Vertex shader compilation log:" << endl << log;
    }
    // Attach the shader to the program
    boglAttachShader(mProgram, vertexshader);
    // Delete shader
    boglDeleteShader(vertexshader);
  }

  // Load fragment shader, if it was given
  if(!fragmentsrc.isEmpty())
  {
    // Create shader object
    fragmentshader = boglCreateShader(GL_FRAGMENT_SHADER);
    // Load it
    const char* src = fragmentsrc.latin1();
    boglShaderSource(fragmentshader, 1, &src, NULL);
    //boglShaderSource(fragmentshader, 1, &fragmentsrc.latin1(), NULL);
    // Compile the shader
    boglCompileShader(fragmentshader);
    // Make sure it compiled correctly
    int compiled;
    boglGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &compiled);
    boglGetShaderInfoLog(fragmentshader, 8192, &logsize, log);
    if(!compiled)
    {
      boError(130) << k_funcinfo << "Couldn't compile fragment shader!" << endl << log;
      return false;
    }
    else if(logsize > 0)
    {
      boDebug(130) << "Fragment shader compilation log:" << endl << log;
    }
    // Attach the shader to the program
    boglAttachShader(mProgram, fragmentshader);
    // Delete shader
    boglDeleteShader(fragmentshader);
  }

  // Link the program
  boglLinkProgram(mProgram);
  // Make sure it linked correctly
  int linked;
  boglGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
  boglGetProgramInfoLog(mProgram, 8192, &logsize, log);

  if(!linked)
  {
    boError(130) << k_funcinfo << "Couldn't link the program!" << endl << log;
    return false;
  }
  else if(logsize > 0)
  {
    boDebug(130) << "Shader linking log:" << endl << log << endl;
  }

  // Create uniform locations dict
  mUniformLocations = new QDict<int>(17);
  mUniformLocations->setAutoDelete(true);

  // Init texture samplers for the shader
  bind();
  for(int i = 0; i < 4; i++)
  {
    setUniform(QString("texture_%1").arg(i), i);
  }
  unbind();

  mValid = true;

  return true;
}

QString BoShader::preprocessSource(const QString& source, FilterType filter)
{
  // Whether to ignore what is being read (until next <...> marker)
  bool ignore = false;

  // Read through the string, line by line
  QStringList lines = QStringList::split(QChar('\n'), source, true);
  QString result;

  for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
  {
    QString line = *it;
    // Check for source markers
    if(line.startsWith("<vertex>"))
    {
      ignore = (filter == FragmentOnly);
      continue;
    }
    else if(line.startsWith("<fragment>"))
    {
      ignore = (filter == VertexOnly);
      continue;
    }
    else if(line.startsWith("<all>"))
    {
      ignore = false;
      continue;
    }
    // Throw away ignored lines
    if(ignore)
    {
      continue;
    }

    // Check for #include directive
    if(line.startsWith("#include"))
    {
      // Find out the name of the #included shader
      QRegExp namefinder("#include [<\"]([a-zA-Z0-9\\-_\\.]*)[>\"]");
      namefinder.search(line);
      QString includedname = namefinder.cap(1);
      boDebug(130) << k_funcinfo << "Including " << includedname << endl;
      // Get the full filename
      QString filename = boShaderManager->getFullFilename(includedname);
      QFile f(filename);
      if(!f.open(IO_ReadOnly))
      {
        boError(130) << k_funcinfo << "Couldn't open " << filename << " for reading!" << endl;
        // Keep going
        continue;
      }
      // Include the file
      result += preprocessSource(QString(f.readAll()), filter);
    }
    else
    {
      result += line;
    }
    result += '\n';
  }

  //boDebug() << k_funcinfo << "Loaded vertex shader source : \n'" << vertexsrc << "'" << endl;
  //boDebug() << k_funcinfo << "Loaded fragment shader source : \n'" << fragmentsrc << "'" << endl;
  return result;
}

void BoShader::reload()
{
  if(mName.isEmpty())
  {
    // Custom shaders can't be reloaded
    return;
  }

  // Cleanup
  if(mUniformLocations)
  {
    mUniformLocations->clear();
    delete mUniformLocations;
  }
  if(mProgram)
  {
    if(mCurrentShader == this)
    {
      unbind();
    }
    boglDeleteProgram(mProgram);
  }

  // Reload
  load(mName);
}

int BoShader::uniformLocation(const QString& name)
{
  if(!mUniformLocations)
  {
    return -1;
  }
  int* location = mUniformLocations->find(name);
  if(!location)
  {
    location = new int;
    *location = boglGetUniformLocation(mProgram, name.latin1());
    mUniformLocations->insert(name, location);
  }
  return *location;
}

bool BoShader::setUniform(const QString& name, float value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    boglUniform1f(location, value);
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, int value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    boglUniform1i(location, value);
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, bool value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    boglUniform1i(location, value ? 1 : 0);
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, const BoVector2Float& value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    boglUniform2fv(location, 1, value.data());
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, const BoVector3Float& value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    boglUniform3fv(location, 1, value.data());
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, const BoVector4Float& value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    boglUniform4fv(location, 1, value.data());
  }
  return (location >= 0);
}

void BoShader::bind()
{
  if(mCurrentShader != this)
  {
    boglUseProgram(mProgram);
    mCurrentShader = this;
    setUniform("cameraPos", mCameraPos);
    setUniform("lightPos", mSun->position3());
    setUniform("time", mTime);
    setUniform("fogEnabled", mFogEnabled);
    setUniform("activeLights", mActiveLights);
  }
}

void BoShader::unbind()
{
  if(mCurrentShader)
  {
    boglUseProgram(0);
    mCurrentShader = 0;
  }
}

void BoShader::setCameraPos(const BoVector3Float& pos)
{
  mCameraPos = pos;
}

void BoShader::setSun(BoLight* sun)
{
  mSun = sun;
}

void BoShader::setTime(float time)
{
  mTime = time;
}

void BoShader::setFogEnabled(float enabled)
{
  mFogEnabled = enabled;
}

void BoShader::setActiveLights(int active)
{
  mActiveLights = active;
}

/*
 * vim: et sw=2
 */
