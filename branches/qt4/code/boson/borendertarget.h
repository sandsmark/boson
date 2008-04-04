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

#ifndef BORENDERTARGET_H
#define BORENDERTARGET_H


class BoTexture;


/**
 * @short Render target object
 *
 * Render target object enables you to render onto a texture. This texture can
 *  later be used to e.g. do post-processing of the scene.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoRenderTarget
{
  public:
    enum Type { PBuffer = 1, FBO };
    enum { RGB = 1, RGBA = 2, Depth = 16, Float = 128 };


    /**
     * Constructs a BoRenderTarget
     * @param width width of the rendertarget
     * @param height height of the rendertarget
     * @param flags format flags for the rendertarget
     * @param color texture where the scene will be rendered onto
     * @param depth if specified, depth will be rendered onto this depth-texture
     **/
    BoRenderTarget(int width, int height, int flags = RGBA | Depth, BoTexture* color = 0, BoTexture* depth = 0);
    ~BoRenderTarget();

    /**
     * Enables this render target.
     * All OpenGL commands from now on affect this render target until the
     *  @ref disable method is called
     **/
    bool enable();
    /**
     * Disables this render target, activating whichever target was active
     *  when @ref enable was called.
     **/
    bool disable();

    bool valid() const  { return mValid; }
    Type type() const  { return mType; }
    int flags() const  { return mFlags; }
    int width() const  { return mWidth; }
    int height() const  { return mHeight; }


  protected:
    void initPBuffer();
    void initFBO();
    bool createContext(int* attrib, int& i, int* pattrib, int& pi);


  private:
    int mWidth;
    int mHeight;
    Type mType;
    int mFlags;
    BoTexture* mTexture;
    BoTexture* mDepthTexture;
    bool mValid;

    class PBufferData;
    PBufferData* mPBufferData;
    class FBOData;
    FBOData* mFBOData;
};

#endif //BORENDERTARGET_H
