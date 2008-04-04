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

#ifndef BOTEXTURE_H
#define BOTEXTURE_H

#include <bogl.h>

#include <qptrlist.h>
#include <qvaluevector.h>


class QString;
class QStringList;


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
      Texture2D = 0x0DE1,  // GL_TEXTURE_2D
      Texture3D = 0x806F,  // GL_TEXTURE_3D
      TextureCube = 0x8513  //GL_TEXTURE_CUBE_MAP
    };
    /**
     * Texture class.
     * This can be used to group textures so that options (such as filtering)
     *  can be quickly set for the groups.
     * @li Model model textures
     * @li Terrain terrain textures
     * @li Particle particle (and other effect) textures
     * @li UI user interface textures (e.g. cursor, minimap)
     * @li NormalMap normal maps (bumpmaps), those won't be compressed
     * @li Custom options specified by user
     **/
    enum Class
    {
      Model = 1,
      Terrain,
      Particle,
      UI,
      NormalMap,
      Custom = 1000
    };
    /**
     * Misc options for texture.
     * These include texture filtering and format.
     * @li FormatAuto automatically selects FormatRGB or FormatRGBA, depending
     *  on format of texture image.
     * @li DontGenMipmaps doesn't generate mipmaps for the texture (when using
     *  mipmap filter)
     * @li DontCompress doesn't compress the texture (when texture compression
     *  is available)
     * @li EnableNPOT enables usage of non-power-of-two textures, when
     *  supported, saving some texture memory.
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
      FormatDepth = 512,
      // Misc
      DontGenMipmaps = 1024,
      DontCompress = 2048,
      EnableNPOT = 4096,
      // Wrapping
      ClampToEdge = 8192
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
    BoTexture(unsigned char* data, int width, int height, Class texclass);
    /**
     * Create new texture with specified options, type and size and initialize it with
     *  given data.
     * Type must be either Texture2D or TextureCube.
     * If Type is TextureCube, name must contain %1 which will be replaced with
     *  "[np][xyz]" for corresponding cube sides.
     **/
    BoTexture(unsigned char* data, int width, int height, int options = FilterLinearMipmapLinear | FormatRGB, Type type = Texture2D);


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
    void load(unsigned char* data, int width, int height, int side = 0);

    /**
     * Reloads the texture.
     * Note that it only works if the texture was loaded from a file. If it
     *  was loaded from specified data, this method does nothing.
     **/
    void reload();

    /**
     * Bind this texture.
     * Note that this does not enable the texture.
     **/
    void bind();

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
     * Note that you shouldn't cache this anywhere as some functions, e.g.
     *  @ref reload() can change it.
     **/
    GLuint id() const  { return mId; }

    /**
     * @return Whether texture has been correctly loaded.
     **/
    bool loaded() const  { return mLoaded; }

    /**
     * @return Path to the image file where the texture was downloaded from.
     * Note that it works only if you use load() function which takes filename
     *  as a parameter. It will return null string if texture was loaded from
     *  specified data.
     **/
    const QString& filePath() const  { return mFilePath; }

    /**
     * @return _Approximate_ amount of memory used by this texture, in bytes.
     **/
    int memoryUsed() const;

    static int nextPower2(int n);
    void applyOptions();


  protected:
    void setOptions(Class c);
    void init();

    unsigned char* ensureCorrectSize(unsigned char* data, int &width, int &height);
    int buildColoredMipmaps(unsigned char* data, int width, int height,
        GLenum format, GLenum internalformat, GLenum type);

    GLenum determineFormat() const;
    GLenum determineInternalFormat() const;


    // Dimensions of the texture, in pixels
    int mWidth;
    int mHeight;
    int mDepth;
    // Type of the texture (2d/3d/cube)
    GLenum mType;
    // OpenGL id of texture
    GLuint mId;

    // Whether texture has been correctly loaded
    bool mLoaded;

    // Path to texture image file
    QString mFilePath;

    // Texture class
    Class mClass;
    // Additional options, such as filtering
    int mOptions;
};



class BoTextureArray
{
  public:
    BoTextureArray(const QStringList& files, BoTexture::Class texclass);
    BoTextureArray(const QStringList& files, int options = BoTexture::FilterLinearMipmapLinear | BoTexture::FormatAuto, BoTexture::Type type = BoTexture::Texture2D);
    BoTextureArray(const QPtrList<BoTexture>& textures);
    ~BoTextureArray();

    BoTexture* texture(int i) const  { return mTextures[i]; }
    BoTexture* operator[](int i) const  { return mTextures[i]; }
    unsigned int count() const  { return mTextures.count(); }

  private:
    QValueVector<BoTexture*> mTextures;
    bool mAutoDelete;
};



#define boTextureManager BoTextureManager::textureManager()

class BoTextureManager
{
  public:
    /**
     * @short Small helper class to count texture binds in a method
     *
     * You can create an object of this class in a method to count all texture
     * binds that are made in the method. On destruction of this object, the
     * number of texture binds is automaticall written to the pointer that
     * you've given in the c'tor. Example:
     * <pre>
     * void renderSomething()
     * {
     *  BoTextureManager::BoTextureBindCounter bindCounter(boTextureManager, &mTextureBindsInRenderSomething);
     *  // ... -> render something here
     * }
     * </pre>
     *
     * You can also call @ref stop if you don't want or can wait until the
     * destruction of the object (e.g. because you don't want to count all binds
     * in a method). The destructor won't touch the variable anymore then.
     *
     * Note that if @ref BoTextureManager::textureBinds is reset will there is
     * still an object of this class present, the value of the variable will be
     * undefined.
     * @author Andreas Beckermann <b_mann@gmx.de>
     **/
    class BoTextureBindCounter
    {
    public:
        BoTextureBindCounter(BoTextureManager* manager, int* counter)
          : mManager(manager), mCounter(counter), mWritten(false)
        {
          *mCounter = mManager->textureBinds();
        }
        ~BoTextureBindCounter()
        {
          stop();
        }
        void stop()
        {
          if (!mWritten)
          {
            *mCounter = mManager->textureBinds() - *mCounter;
          }
        }
      private:
        BoTextureManager* mManager;
        int* mCounter;
        bool mWritten;
    };
  public:
    BoTextureManager();
    ~BoTextureManager();

    static void initStatic();
    static void deleteStatic();
    static BoTextureManager* textureManager();


    // Init methods
    void initOpenGL();


    // Texture API methods.
    /**
     * Binds given texture object to active texture unit.
     * It also enables texturing with this texture (e.g.
     *  glEnable(GL_TEXTURE_2D)) and disables texturing with previously binded
     *  texture (e.g. glDisable(GL_TEXTURE_1D)).
     **/
    void bindTexture(BoTexture* texture);
    /**
     * Same as above, but uses given texture unit instead of the active one.
     **/
    void bindTexture(BoTexture* texture, int textureUnit);
    /**
     * Binds 0 (default) texture to active texture unit.
     **/
    void unbindTexture();
    /**
     * Same as above, but uses given texture unit instead of the active one.
     **/
    void unbindTexture(int textureUnit);
    /**
     * Disable texturing for active texture unit.
     **/
    void disableTexturing();
    /**
     * Makes given texture unit active.
     * Note that textureUnit should be an integer (e.g. 1), not OpenGL constant
     *  (e.g. GL_TEXTURE1).
     * If textureUnit is invalid (less than zero or equal to or greater than
     *  @ref textureUnits, then it does nothing).
     **/
    void activateTextureUnit(int textureUnit);
    /**
     * Invalidates cache of active textures/texture units.
     * Call this after executing non-BoTexture code which may change active
     *  texture and/or texture unit.
     **/
    void invalidateCache();


    void clearStatistics();
    int textureBinds() const  { return mTextureBinds; }


    // Config methods.
    bool supportsTexture3D() const  { return mSupportsTexture3D; }
    bool supportsTextureCube() const  { return mSupportsTextureCube; }
    bool supportsGenerateMipmap() const  { return mSupportsGenerateMipmap; }
    bool supportsTextureCompression() const  { return mSupportsTextureCompressionS3TC; }
    bool supportsNPOTTextures() const  { return mSupportsNPOTTextures; }
    bool supportsAnisotropicFiltering() const  { return (mMaxAnisotropy > 1); }

    int maxTextureSize() const  { return mMaxTextureSize; }
    int max3DTextureSize() const  { return mMax3DTextureSize; }
    int maxCubeTextureSize() const  { return mMaxCubeTextureSize; }
    int textureUnits() const  { return mTextureUnits; }
    int maxTextureAnisotropy() const  { return mMaxAnisotropy; }

    bool useColoredMipmaps() const;
    bool useTextureCompression() const;
    int textureFilter() const;
    int textureAnisotropy() const;


    // Those are meant to be used by only BoTexture.
    void registerTexture(BoTexture* tex);
    void unregisterTexture(BoTexture* tex);
    void textureLoaded(BoTexture* tex, bool firsttime);

    void textureFilterChanged();

    /**
     * Reloads all textures that have been loaded from image files.
     * Note that it won't touch textures loaded from custom data.
     * Also note that it might take a few seconds to reload the textures.
     **/
    void reloadTextures();

    /**
     * @return The amount of memory that is used for all textures currently
     * loaded.
     * Note that this is just an <em>approximated</em> value, the actual memory
     * usage depends on the implementation of the graphics card driver. See also
     * @ref BoTexture::memoryUsed
     **/
    int usedTextureMemory() const
    {
      return mUsedTextureMemory;
    }

    /**
     * @return The number of textures currently registered. See also @ref
     * BoTexture
     **/
    unsigned int textureCount() const
    {
      return mTextures.count();
    }

    /**
     * @return A list of all currently registered textures.
     **/
    const QPtrList<const BoTexture>& allTextures() const
    {
      return mConstTextures;
    }


  private:
    static BoTextureManager* mManager;

    QPtrList<BoTexture> mTextures;
    QPtrList<const BoTexture> mConstTextures;
    BoTexture** mActiveTexture;
    int* mActiveTextureType;
    int mActiveTextureUnit;

    bool mOpenGLInited;


    int mUsedTextureMemory;
    int mTextureBinds;


    bool mSupportsTexture3D;
    bool mSupportsTextureCube;
    bool mSupportsGenerateMipmap;
    bool mSupportsTextureCompressionS3TC;
    bool mSupportsNPOTTextures;
    int mMaxTextureSize;
    int mMax3DTextureSize;
    int mMaxCubeTextureSize;
    int mTextureUnits;
    int mMaxAnisotropy;
};

#endif // BOTEXTURE_H

/*
 * vim: et sw=2
 */
