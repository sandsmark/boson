/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/gl/ugl_image.hpp
    begin             : Sat Oct 18 2003
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UGL_IMAGE_HPP
#define UGL_IMAGE_HPP

#include "../image/uimage.hpp"

// we need some opengl specific types
#include "../ufo_gl.hpp"
#include "../image/uimageio.hpp"

#include "../util/udimension.hpp"
#include "../util/urectangle.hpp"

namespace ufo {

/** OpenGL implementation of an image.
  * Loads and paints openGL textures to screen.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UGL_Image : public UImage {
	UFO_DECLARE_DYNAMIC_CLASS(UGL_Image)
public: // constructors
	/** Creates an OpenGL texture from the given file.
	  *
	  * @param fileName The image file
	  * @param autoRefresh When true recreates automatically
	  * 	the OpenGL texture on GL context recreation
	  * @param imageFormat The format used to interpret the image data
	  * @param internalFormat The format used to save the texture in the VRAM
	  */
	UGL_Image(const std::string & fileName, bool autoRefresh = false,
		GLenum imageFormat = 0, GLenum internalFormat = 0);

	/** Creates an OpenGL texture from the given image IO data.
	  *
	  * @param imageIO the image data
	  * @param autoRefresh When true recreates automatically
	  * 	the OpenGL texture on GL context recreation
	  * @param imageFormat The format used to interpret the image data
	  * @param internalFormat The format used to save the texture in the VRAM
	  */
	UGL_Image(UImageIO * imageIO, bool autoRefresh = false,
		GLenum imageFormat = 0, GLenum internalFormat = 0);

	virtual ~UGL_Image();

public: // Implements UImage
	virtual int getImageWidth() const;
	virtual int getImageHeight() const;
	virtual int getImageComponents() const;
	virtual unsigned long handle() const;

public: // Public methods
	/** @return The size of the image as UDimension */
	const UDimension & getSize() const;

	/** paints the texture */
	void paint(UGraphics * g) const;

	/** Paints the texture with an offset of x, y. */
	void paint(UGraphics * g, const UPoint & location) const;
	/** paints the texture and scale it if it is necessary
	  */
	void paint(UGraphics * g, const URectangle & rect) const;


	void paintSubImage(UGraphics * g, const URectangle & rect,
		const UPoint & dest) const;
	void paintSubImage(UGraphics * g, const URectangle & rect,
		const URectangle & dest) const;

	/** Disposes the saved image data which is normally used to do an
	  * auto refresh.
	  */
	void dispose();
	/** Disposes the OpenGL texture. The texture can be recreated using
	  * refresh as long as it was created with autoRefresh enabled.
	  */
	void disposeGL();

	/** Refreshes the OpenGL texture using the saved image data. */
	void refresh();

protected:  // Protected methods
	/** makes a openGL texture */
	virtual bool createGLTexture(GLubyte * dataA, int componentsA,
	                 GLenum imageFormatA, GLenum internalFormatA);

	/** rounds to the next power of two
	  */
	int round2(int n);

private:  // Private attributes
	GLuint m_index;
	GLenum m_imageFormat;
	GLenum m_internalFormat;
	/** the image size in pixels */
	UDimension m_size;
	int m_imageComponents;
	bool m_isValid;

	/** A pointer to the image data. Used to recreate the GL texture. */
	UImageIO * m_imageData;
};

} // namespace ufo

#endif // UGL_IMAGE_HPP
