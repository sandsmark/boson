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

#ifndef BOTEXTURE_H
#define BOTEXTURE_H

#include <GL/gl.h>


class QString;


/**
 * @short General texture class
 *
 * BoTexture class provides generic and easy-to-use interface for dealing with
 *  textures. You can just specify filename and texture will be loaded for you.
 *  But you can also fine-tune what options are used for the texture, e.g.
 *  filtering, format, etc.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoTexture
{
  public:
    /**
     * Type of the texture
     * @li Texture2D normal 2d texture. You'll use this most of the time
     * @li Texture3D 3d texture.
     * @li TextureCube cube map texture
     **/
    enum Type
    {
      // Values are from GL/gl.h. We don't use OpenGL constants because they
      //  may not be available (if user has obsolete gl.h).
      Texture2D = 0x0DE1,
      Texture3D = 0x806F,
      TextureCube = 0x8513
    };
    /**
     * Texture class.
     * This can be used to group textures so that options (such as filtering)
     *  can be quickly set for the groups.
     * @li Model model textures
     * @li Terrain terrain textures
     * @li Particle particle (and other effect) textures
     * @li UI user interface textures (e.g. cursor, minimap)
     **/
    enum Class
    {
      Model = 1,
      Terrain,
      Particle,
      UI
    };
    /**
     * Misc options for texture.
     * These include texture filtering and format.
     * @li FormatAuto automatically selects FormatRGB or FormatRGBA, depending
     *  on format of texture image.
     * @li DontGenMipmaps doesn't generate mipmaps for the texture (when using
     *  mipmap filter)
     **/
    enum Options
    {
      // Filtering
      FilterNearest = 1,
      FilterLinear = 2,
      FilterNearestMipmapNearest = 4,
      FilterNearestMipmapLinear = 8,
      FilterLinearMipmapNearest = 16,
      FilterLinearMipmapLinear = 32,
      // Format
      FormatAuto = 64,
      FormatRGB = 128,
      FormatRGBA = 256,
      // Misc
      DontGenMipmaps = 1024
    };


    /**
     * Create empty 2d texture with specified class.
     * Use one of the @ref load methods to load texture.
     **/
    BoTexture(Class texclass);
    /**
     * Create empty texture with specified type and options.
     * Use one of the @ref load methods to load texture.
     **/
    BoTexture(int options = FilterLinearMipmapLinear | FormatRGB, Type type = Texture2D);

    /**
     * Load 2d texture with specified class from specified file.
     * name must be full path to texture.
     **/
    BoTexture(const QString& name, Class texclass);
    /**
     * Load texture from specified file, using specified type and options.
     * name must be full path to texture.
     * If Type is TextureCube, name must contain %1 which will be replaced with
     *  "[np][xyz]" for corresponding cube sides.
     **/
    BoTexture(const QString& name, int options = FilterLinearMipmapLinear | FormatAuto, Type type = Texture2D);
    /**
     * Create new texture with specified class and size and initialize it with
     *  given data.
     **/
    BoTexture(const unsigned char* data, int width, int height, Class texclass);
    /**
     * Create new texture with specified options, type and size and initialize it with
     *  given data.
     * Type must be either Texture2D or TextureCube.
     * If Type is TextureCube, name must contain %1 which will be replaced with
     *  "[np][xyz]" for corresponding cube sides.
     **/
    BoTexture(const unsigned char* data, int width, int height, int options = FilterLinearMipmapLinear | FormatRGB, Type type = Texture2D);


    /**
     * Releases this texture.
     **/
    ~BoTexture();

    /**
     * Load texture from specified file.
     * name must be full path to texture.
     * Type must be either Texture2D or TextureCube.
     * If Type is TextureCube, name must contain %1 which will be replaced with
     *  "[pn][xyz]" for corresponding cube sides.
     **/
    void load(const QString& name);
    /**
     * Load texture with specified size using specified data.
     * Texture's @ref type must be either Texture2D or TextureCube.
     * If @ref type is TextureCube, side defines which side of the cube is
     *  specified. Side can be in range 0-6, corresponding sides are: px, nx,
     *  py, ny, pz, nz.
     *  If @ref type is not TextureCube, side has no effect.
     **/
//     *  If side is -1, data for all 6 sides of the cube must be in data, in
//     *  order px, nx, py, ny, pz, nz.
    void load(const unsigned char* data, int width, int height, int side = 0);

    /**
     * Bind this texture.
     * Note that this does not enable the texture.
     **/
    void bind();
    /**
     * Enable this texture type.
     * Note that this doesn't bind the texture, so you have to call @ref bind
     *  before using it.
     **/
    void enable();
    /**
     * Disables this texture type.
     **/
    void disable();

    // TODO: do we need set(Width|Height|Depth)() methods?
    /**
     * @return width of this texture.
     **/
    int width() const  { return mWidth; }
    /**
     * @return height of this texture.
     **/
    int height() const  { return mHeight; }
    /**
     * @return depth of this texture.
     * If @ref type of this texture is not Texture3D, it returns 0.
     **/
    int depth() const  { return mDepth; }

    /**
     * @return @ref Type of this texture.
     **/
    Type type() const  { return (Type)mType; }
    /**
     * @return @ref Options of this texture.
     **/
    int options() const  { return mOptions; }
    /**
     * @return Id of this texture.
     **/
    GLuint id() const  { return mId; }

  protected:
    void setOptions(Class c);
    void applyOptions();
    void init();

    int mWidth;
    int mHeight;
    int mDepth;
    GLenum mType;
    int mOptions;
    GLuint mId;
};



class BoTextureManager
{
  public:
    BoTextureManager();
    ~BoTextureManager();

    static BoTextureManager* boTextureManager();

  private:
    static BoTextureManager* mManager;
};

#endif // BOTEXTURE_H
