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
     * @param tex texture where the scene will be rendered onto
     **/
    BoRenderTarget(int width, int height, int flags = RGBA | Depth, BoTexture* tex = 0);
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
     * @param updatetex whether to update the texture
     **/
    bool disable(bool updatetex = true);

    /**
     * Updates the texture of this render target.
     * You usually don't need to call this manually.
     **/
    void updateTexture();

    bool valid() const  { return mValid; }
    Type type() const  { return mType; }
    int flags() const  { return mFlags; }

    /**
     * Sets texture where the scene will be rendered for this render target.
     * The texture's size must be at least as big as this render target.
     **/
    void setTexture(BoTexture* tex);


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
    bool mValid;

    class PBufferData;
    PBufferData* mPBufferData;
};

#endif //BORENDERTARGET_H
