/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ugraphics.hpp
    begin             : Sat Jul 20 2002
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

#ifndef UGRAPHICS_HPP
#define UGRAPHICS_HPP

#include "uobject.hpp"

#include "util/urectangle.hpp"

namespace ufo {

class UContext;
class UFont;
class UColor;

class UImage;
class UImageIO;

/** Basic Graphics objects.
  * The Graphics object is a basic wrapper for drawing functions and
  *  some higher level drawing functions.
  * This is not an high performance wrapper and should only be used for
  * strict platform and device independent drawing routines.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UGraphics : public UObject  {
	UFO_DECLARE_ABSTRACT_CLASS(UGraphics)
public: // initialization
	/** Returns the context which is used for the drawing operations. */
	virtual UContext * getContext() const = 0;

	/** Inits the context for drawing with this drawing abstraction kit.
	  * This also includes initing context attributes and view matrix.
	  * @see initContextAttributes
	  * @see initViewMatrix
	  */
	//virtual void initDevice() const = 0;
	/** Re-inits all device specific attributes.
	  * This resets any previously set attributes like color, font, ...
	  * This is useful if you did custom drawings (e.g.
	  */
	virtual void resetDeviceAttributes() = 0;

	/** Inits the view matrices of the graphics device.
	  * The coordinates are usually used by the associated context.
	  */
	virtual void resetDeviceViewMatrix() = 0;

	/** Maps the rectangle rect, which is in the coordinate space
	  * of this graphics object, to the underlying graphics device,
	  * i.e. the OpenGL context.
	  */
	virtual URectangle mapToDevice(const URectangle & rect) = 0;

	/** Maps the rectangle rect from the underlying graphics device,
	  * i.e. the OpenGL context to root coordinates! (which means to
	  * coordinates of the root pane)
	  */
	virtual URectangle mapFromDevice(const URectangle & rect) = 0;

public: // extended methods
	/** Begins drawing operations. */
	//virtual void begin() = 0;
	/** Ends drawing operations. */
	//virtual void end() = 0;

	/** Flushes drawing operations. */
	virtual void flush() = 0;
	/** Clears the screen with the clear color. */
	virtual void clear() = 0;
	/** Dumps the screen contence to the new image io object.
	  * May be very slow when using hardware accelereated rendering.
	  */
	virtual UImageIO * dump() = 0;

public: // Public attributes
	/** Sets the color for this graphics object.
	  * Changes internally the color for the underlying graphics context.
	  * @param color The new color for this graphics object
	  */
	virtual void setColor(const UColor & color) = 0;
	/** Returns the color which was previously set by setColor.
	  * May differ from the color of the underlying graphics context,
	  * when the color was directly set (without this graphics object).
	  * @see setColor
	  */
	virtual const UColor & getColor() const = 0;

	virtual void setClearColor(const UColor & clearColor) = 0;
	virtual const UColor & getClearColor() const = 0;

	/** Sets the font for this graphics object. */
	virtual void setFont(const UFont * font) = 0;
	/** Returns the font for this graphics object. */
	virtual const UFont * getFont() const = 0;

public: // text drawing functions
	/** Draws the given character string at the specified position
	  * using the current font.
	  */
	virtual void drawString(const std::string & string, int x = 0, int y = 0) = 0;

	/** Returns the bounding rect for the given character string
	  * using the current font.
	  */
	virtual UDimension getStringSize(const std::string & string) = 0;

public: // ?
	//virtual void pushClipRect() = 0;
	//virtual void popClipRect() = 0;
	/** Sets the clip rect in root coordinates.
	  * If rect is empty, clipping is disabled.
	  */
	virtual void setClipRect(const URectangle & rect) = 0;
	/** Returns the clip rect in root coordinates. */
	virtual URectangle getClipRect() const = 0;

public: //
	/** Translates the coordinate system by the given x and y values. */
	virtual void translate(float x, float y) = 0;
	virtual float getTranslationX() const = 0;
	virtual float getTranslationY() const = 0;

public: // basic drawing operations
	/** draws the outline of the given rectangle
	  * with the current writing color.
	  */
	virtual void drawRect(const URectangle & rect) = 0;
	/** fills the given rectangle with the current writing color. */
	virtual void fillRect(const URectangle & rect) = 0;

	/** Draws a line with the given coordinates and the current color. */
	virtual void drawLine(const UPoint & p1, const UPoint & p2) = 0;
	/** Draws several lines with the given coordinates and the current color.
	  * Each lines is defined by two x values and two y values.
	  * Point 2*n-1 and 2*n define line n. 'nPoints' lines are drawn.
	  */
	virtual void drawLines(int x[], int y[], int nPoints) = 0;
	/** Draws a connected group of lines from the first point to the last.
	  * Points n and n+1 define line n. nPoints - 1 lines are drawn.
	  */
	virtual void drawPolygon(int x[], int y[], int nPoints) = 0;
	/** Fills a polygon specified by the given x,y values and with the current
	  * color.
	  */
	virtual void fillPolygon(int x[], int y[], int nPoints) = 0;

	/** Draws the given image at the specified position.
	  * @param x The x coordinate for drawing
	  * @param y The y coordinate for drawing
	  */
	virtual void drawImage(UImage * image, const UPoint & p) = 0;
	/** Draws the given image at the specified position,
	  * eventually scaled on the fly when width and height differ
	  * from the image size.
	  *
	  * @param x The x coordinate for drawing
	  * @param y The y coordinate for drawing
	  * @param w The width of the drawn image
	  * @param h The height of the drawn image
	  */
	virtual void drawImage(UImage * image, const URectangle & rect) = 0;

	/** Draws the given rectangle of the given image
	  * at the specified position. If srcRect.w and srcRect.h are 0,
	  * the whole image is drawn.
	  *
	  * @param srcRect The source rectangle of the sub image
	  * @param destLocation The location on this graphics object to paint the
	  * 	subimage
	  */
	virtual void drawSubImage(UImage * image,
		const URectangle & srcRect,
		const UPoint & destLocation) = 0;

	/** Draws the given image at the specified position,
	  * eventually scaled on the fly when width and height differ
	  * from the sub image size.
	  * If destRect.w and destRect.h are 0, the image is drawn without scaling
	  *
	  * @param srcRect The source rectangle of the sub image
	  * @param destLocation The location on this graphics object to paint the
	  * 	subimage
	  */
	virtual void drawSubImage(UImage * image,
		const URectangle & srcRect, const URectangle & destRect) = 0;

public: // inline helper methods
	void drawRect(int x, int y, int w, int h) {
		drawRect(URectangle(x, y, w, h));
	}
	void fillRect(int x, int y, int w, int h) {
		fillRect(URectangle(x, y, w, h));
	}

	void drawLine(int x1, int y1, int x2, int y2) {
		drawLine(UPoint(x1, y1), UPoint(x2, y2));
	}
	void drawImage(UImage * image, int x, int y) {
		drawImage(image, UPoint(x, y));
	}
	void drawImage(UImage * image, int x, int y, int w, int h) {
		drawImage(image, URectangle(x, y, w, h));
	}
	void drawSubImage(UImage * image,
			int srcX, int srcY, int srcWidth, int srcHeight,
			int destX, int destY) {
		drawSubImage(image, URectangle(srcX, srcY, srcWidth, srcHeight),
			UPoint(destX, destY));
	}
	void drawSubImage(UImage * image,
			int srcX, int srcY, int srcWidth, int srcHeight,
			int destX, int destY, int destWidth, int destHeight) {
		drawSubImage(image, URectangle(srcX, srcY, srcWidth, srcHeight),
			URectangle(destX, destY, destWidth, destHeight));
	}
};

} // namespace ufo

#endif // UGRAPHICS_HPP
