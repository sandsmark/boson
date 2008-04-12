/*
    This file is part of the Boson game
    Copyright (C) 2005-2006 Rivo Laks (rivolaks@hot.ee)

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

#include "boshader.h"

#include "../bomemory/bodummymemory.h"
#include "bogl.h"
#include "bodebug.h"
#include "bolight.h"
#include "bosonconfig.h"
#include "bosonprofiling.h"

#include <qstring.h>
#include <q3dict.h>
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
  Q3PtrListIterator<BoShader> it(mShaders);
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
      return QString();
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
    mUniformLocations = 0;
  }

  if(mProgram)
  {
    if(mCurrentShader == this)
    {
      unbind();
    }
    glDeleteProgram(mProgram);
  }
}

bool BoShader::load(const QString& name)
{
  BosonProfiler profiler("BoShader::load(name)");
  QString filename = boShaderManager->getFullFilename(name);
  QFile f(filename);
  if(!f.open(QIODevice::ReadOnly))
  {
    boError(130) << k_funcinfo << "Couldn't open " << filename << " for reading!" << endl;
    return false;
  }

  mSources.clear();
  mSources.append(filename);

  QString contents(f.readAll());

  BosonProfiler prepProfiler("Preprocessing of sources");
  QString vertexsrc = preprocessSource(contents, VertexOnly, 0);
  QString fragmentsrc = preprocessSource(contents, FragmentOnly, 0);
  prepProfiler.pop();


  //boDebug() << k_funcinfo << "Processed vertex shader source : \n'" << vertexsrc << "'" << endl;
  //boDebug() << k_funcinfo << "Processed fragment shader source : \n'" << fragmentsrc << "'" << endl;
  if(!load(vertexsrc, fragmentsrc))
  {
    boError() << k_funcinfo << "Couldn't load shader '" << name << "' from '" << filename << "'" << endl;
    return false;
  }
  return true;
}

bool BoShader::load(const QString& vertexsrc, const QString& fragmentsrc)
{
  BosonProfiler loadProfiler("BoShader::load(vertexsrc, fragmentsrc)");
  if(fragmentsrc.isEmpty() && vertexsrc.isEmpty())
  {
    boError(130) << k_funcinfo << "No shader sources given!" << endl;
    return false;
  }
  if(!glCreateProgram)
  {
    boError(130) << k_funcinfo << "Shaders aren't supported!" << endl;
    return false;
  }

  // Create program object
  mProgram = glCreateProgram();

  GLuint vertexshader;
  GLuint fragmentshader;

  GLsizei logsize;
  GLsizei logarraysize;
  char* log = 0;

  // Load vertex shader, if it was given
  if(!vertexsrc.isEmpty())
  {
    BosonProfiler compileProfiler("vertex shader compilation&attaching");
    // Create shader object
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    // Load it
    const char* src = vertexsrc.latin1();
    glShaderSource(vertexshader, 1, &src, NULL);
    // Compile the shader
    glCompileShader(vertexshader);
    // Make sure it compiled correctly
    int compiled;
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &compiled);
    // Get info log
    glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &logarraysize);
    log = new char[logarraysize];
    glGetShaderInfoLog(vertexshader, logarraysize, &logsize, log);
    if(!compiled)
    {
      boError(130) << k_funcinfo << "Couldn't compile vertex shader!" << endl << log << endl;
      printUsedSources();
      delete[] log;
      return false;
    }
    else if(logsize > 0)
    {
      boDebug(130) << "Vertex shader compilation log:" << endl << log << endl;
      printUsedSources();
    }
    // Attach the shader to the program
    glAttachShader(mProgram, vertexshader);
    // Delete shader
    glDeleteShader(vertexshader);
    delete[] log;
  }

  // Load fragment shader, if it was given
  if(!fragmentsrc.isEmpty())
  {
    BosonProfiler compileProfiler("fragment shader compilation&attaching");
    // Create shader object
    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    // Load it
    const char* src = fragmentsrc.latin1();
    glShaderSource(fragmentshader, 1, &src, NULL);
    //glShaderSource(fragmentshader, 1, &fragmentsrc.latin1(), NULL);
    // Compile the shader
    glCompileShader(fragmentshader);
    // Make sure it compiled correctly
    int compiled;
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &compiled);
    // Get info log
    glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &logarraysize);
    log = new char[logarraysize];
    glGetShaderInfoLog(fragmentshader, logarraysize, &logsize, log);
    if(!compiled)
    {
      boError(130) << k_funcinfo << "Couldn't compile fragment shader!" << endl << log << endl;
      printUsedSources();
      delete[] log;
      return false;
    }
    else if(logsize > 0)
    {
      boDebug(130) << "Fragment shader compilation log:" << endl << log << endl;
      printUsedSources();
    }
    // Attach the shader to the program
    glAttachShader(mProgram, fragmentshader);
    // Delete shader
    glDeleteShader(fragmentshader);
    delete[] log;
  }

  // Link the program
  BosonProfiler linkProfiler("shader linking");
  glLinkProgram(mProgram);
  linkProfiler.pop();
  // Make sure it linked correctly
  int linked;
  glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
  // Get info log
  glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logarraysize);
  log = new char[logarraysize];
  glGetProgramInfoLog(mProgram, logarraysize, &logsize, log);

  if(!linked)
  {
    boError(130) << k_funcinfo << "Couldn't link the program!" << endl << log << endl;
    printUsedSources();
    delete[] log;
    return false;
  }
  else if(logsize > 0)
  {
    boDebug(130) << "Shader linking log:" << endl << log << endl;
    printUsedSources();
  }
  delete[] log;

  delete mUniformLocations;
  mUniformLocations = new Q3Dict<int>(17);
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

QString BoShader::preprocessSource(const QString& source, FilterType filter, int sourceid)
{
  // Whether to ignore what is being read (until next <...> marker)
  bool ignore = false;

  // Read through the string, line by line
  QStringList lines = QStringList::split(QChar('\n'), source, true);
  QString result;
  result += QString("#line 0 %1\n").arg(sourceid);

  int linenum = 1;
  for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it, linenum++, result += '\n')
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
      if(!f.open(QIODevice::ReadOnly))
      {
        boError(130) << k_funcinfo << "Couldn't open " << filename << " for reading!" << endl;
        // Keep going
        continue;
      }
      // Include the file
      int newsourceid = mSources.findIndex(filename);
      if(newsourceid == -1)
      {
        newsourceid = mSources.count();
        mSources.append(filename);
      }
      result += preprocessSource(QString(f.readAll()), filter, newsourceid);
      result += QString("#line %1 %2").arg(linenum).arg(sourceid);
    }
    else
    {
      result += line;
    }
  }

  return result;
}

void BoShader::printUsedSources()
{
  if(mSources.count() == 0)
  {
    return;
  }

  boError(130) << k_funcinfo << "Used sources:" << endl;
  for(unsigned int i = 0; i < mSources.count(); i++)
  {
    boError(130) << i << " : " << mSources[i] << endl;
  }
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
    mUniformLocations = 0;
  }
  if(mProgram)
  {
    if(mCurrentShader == this)
    {
      unbind();
    }
    glDeleteProgram(mProgram);
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
    *location = glGetUniformLocation(mProgram, name.latin1());
    mUniformLocations->insert(name, location);
  }
  return *location;
}

bool BoShader::setUniform(const QString& name, float value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    glUniform1f(location, value);
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, int value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    glUniform1i(location, value);
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, bool value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    glUniform1i(location, value ? 1 : 0);
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, const BoVector2Float& value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    glUniform2fv(location, 1, value.data());
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, const BoVector3Float& value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    glUniform3fv(location, 1, value.data());
  }
  return (location >= 0);
}

bool BoShader::setUniform(const QString& name, const BoVector4Float& value)
{
  int location = uniformLocation(name);
  if(location >= 0)
  {
    glUniform4fv(location, 1, value.data());
  }
  return (location >= 0);
}

void BoShader::bind()
{
  if(mCurrentShader != this)
  {
    glUseProgram(mProgram);
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
    glUseProgram(0);
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
