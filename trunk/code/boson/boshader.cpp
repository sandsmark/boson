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

#include <qstring.h>
#include <qdict.h>
#include <qfile.h>


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

  load(vertex, fragment);
}

BoShader::BoShader(const QString& file)
{
  mUniformLocations = 0;
  mProgram = 0;
  mValid = false;

  QFile f(file);
  if(!f.open(IO_ReadOnly))
  {
    boError(130) << k_funcinfo << "Couldn't open " << file << " for reading!" << endl;
    return;
  }

  QString vertexsrc;
  QString fragmentsrc;

#define READ_NOTHING 0
#define READ_VERTEXSRC 1
#define READ_FRAGMENTSRC 2

  // What we're reading atm
  int reading = READ_NOTHING;

  // Read through the file
  while(!f.atEnd())
  {
    // Read next line into buffer
    QString buffer;
    f.readLine(buffer, 4096);
    // Check for source markers
    if(buffer.startsWith("<vertex>"))
    {
      reading = READ_VERTEXSRC;
      continue;
    }
    else if(buffer.startsWith("<fragment>"))
    {
      reading = READ_FRAGMENTSRC;
      continue;
    }

    // Append line to source
    if(reading == READ_VERTEXSRC)
    {
      vertexsrc += buffer;
    }
    else if(reading == READ_FRAGMENTSRC)
    {
      fragmentsrc += buffer;
    }
  }

  //boDebug() << k_funcinfo << "Loaded vertex shader source : \n'" << vertexsrc << "'" << endl;
  //boDebug() << k_funcinfo << "Loaded fragment shader source : \n'" << fragmentsrc << "'" << endl;

#undef READ_NOTHING
#undef READ_VERTEXSRC
#undef READ_FRAGMENTSRC

  load(vertexsrc, fragmentsrc);
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

void BoShader::load(const QString& vertexsrc, const QString& fragmentsrc)
{
  if(fragmentsrc.isEmpty() && vertexsrc.isEmpty())
  {
    boError(130) << k_funcinfo << "No shader sources given!" << endl;
    return;
  }
  if(!boglCreateProgram)
  {
    boError(130) << k_funcinfo << "Shaders aren't supported!" << endl;
    return;
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
      return;
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
      return;
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
    return;
  }
  else if(logsize > 0)
  {
    boDebug(130) << "Shader linking log:" << endl << log;
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
