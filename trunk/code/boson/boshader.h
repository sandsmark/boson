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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef BOSHADER_H
#define BOSHADER_H

#include "bo3dtools.h"

#include <qstringlist.h>
#include <qptrlist.h>
#include <qdict.h>

class BoLight;

class BoShader;



#define boShaderManager BoShaderManager::shaderManager()

class BoShaderManager
{
  public:
    BoShaderManager();
    ~BoShaderManager();

    static BoShaderManager* shaderManager();

    /**
     * Reloads all shaders.
     * Call this e.g. when config changes
     **/
    void reloadShaders();
    void shaderSuffixesChanged();

    /**
     * @return full filename to use for shader with given name.
     * The name might be e.g. "unit" and returned filename could be
     * ".../unit-hi.shader" (when high-quality shaders are enabled).
     **/
    const QString& getFullFilename(const QString& shadername);

    void registerShader(BoShader* shader);
    void unregisterShader(BoShader* shader);


  private:
    static BoShaderManager* mShaderManager;

    QPtrList<BoShader> mShaders;
    QStringList mSuffixList;
    QDict<QString> mKnownShaderFiles;
};

/**
 * @short Shader object
 *
 * BoShader provides you with a way to use shaders on a programmable graphics
 *  hardware.
 * BoShader object incorporates both vertex and fragment shaders, although it
 *  is also possible to use just one of them.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoShader
{
  public:
    /**
     * Creates a shader using parameter strings as sources.
     * If one of the sources is QString::null, corresponding shader will not be
     *  created. You can't specify QString::null for both sources though.
     *
     * @param vertex source of the vertex shader
     * @param fragment source of the fragment shader
     **/
    BoShader(const QString& vertex, const QString& fragment);
    /**
     * Loads shader sources from a file.
     * Filename is automatically created by adding a suffix and ".shader" to
     *  the filename, where suffix might be e.g. "-low", "-hi" (depending on
     *  shader quality level) or nothing.
     * Vertex shader's source must be preceeded by '<vertex>' marker and
     *  fragment shader's source by '<fragment>' marker.
     **/
    BoShader(const QString& name);
    ~BoShader();

    /**
     * Binds the shader
     **/
    void bind();
    /**
     * Unbinds the shader. Fixed-function pipeline will be used instead.
     **/
    static void unbind();

    inline unsigned int id()  { return mProgram; }
    inline bool valid()  { return mValid; }

    void reload();


    bool setUniform(const QString& name, float value);
    bool setUniform(const QString& name, int value);
    bool setUniform(const QString& name, bool value);
    bool setUniform(const QString& name, const BoVector2Float& value);
    bool setUniform(const QString& name, const BoVector3Float& value);
    bool setUniform(const QString& name, const BoVector4Float& value);

    int uniformLocation(const QString& name);


    static void setCameraPos(const BoVector3Float& pos);
    static void setSun(BoLight* sun);
    static void setTime(float time);
    static void setFogEnabled(float enabled);
    static void setActiveLights(int lights);


  protected:
    enum FilterType { All = 0, VertexOnly, FragmentOnly };

    bool load(const QString& vertexsrc, const QString& fragmentsrc);
    bool load(const QString& name);
    QString preprocessSource(const QString& source, FilterType filter, int sourceid = 0);
    void printUsedSources();


  private:
    unsigned int mProgram;
    bool mValid;
    QString mName;
    QStringList mSources;

    QDict<int>* mUniformLocations;

    static BoShader* mCurrentShader;
    static BoVector3Float mCameraPos;
    static BoLight* mSun;
    static float mTime;
    static bool mFogEnabled;
    static int mActiveLights;
};

#endif //BOSHADER_H
/*
 * vim: et sw=2
 */
