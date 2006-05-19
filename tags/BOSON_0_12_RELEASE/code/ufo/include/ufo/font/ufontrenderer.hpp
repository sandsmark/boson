/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/font/ufontrenderer.hpp
    begin             : Thu Mar 6 2003
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

#ifndef UFONTRENDERER_HPP
#define UFONTRENDERER_HPP

#include "../uobject.hpp"
#include "../uvolatiledata.hpp"
#include "ufontinfo.hpp"

namespace ufo {

class UFontMetrics;
class UGraphics;
class UContextGroup;

/** @short This class is responsible for low level rendering of fonts.
  * @ingroup text
  * @ingroup drawing
  * @ingroup internal
  *
  * Usually this class is not used directly.
  * Use instead the graphics object.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UFontRenderer : public UVolatileData {
	UFO_DECLARE_ABSTRACT_CLASS(UFontRenderer)
public: // text drawing operations
	/** Draws the string at the given position
	  * @param text The character array which should be drawn
	  * @param nChar The length of the character array
	  * @param xA The x offset on the screen
	  * @param yA The y offset on the screen
	  * @return The advance in x direction
	  * 	(must be the same value like the width returned getStringSize)
	  */
	virtual int
	drawString(UGraphics * g, const char * text, unsigned int nChar,
		int xA = 0, int yA = 0) = 0;

	/** If you have to draw several strings after each other, you can optimize
	  * the drawing with using beginDrawing before and endDrawing at the end.
	  * You should not call any other method which changes the drawing state
	  * within one enclosing block.
	  * Example:
	  * <pre>
	  *     beginDrawing();
	  *     drawString(line1, strlen(line1), 0, 0);
	  *     drawString(line1, strlen(line1), lineHeight, 0);
	  *     endDrawing();
	  * </pre>
	  */
	virtual void beginDrawing(UGraphics * g) = 0;

	/** Ends the drawing block started with beginDrawing. */
	virtual void endDrawing(UGraphics * g) = 0;

	/** This method is called whenever the associated OpenGL context group
	  * was destroyed and recreated.
	  * Some font renderer need to recreate some resources (textures, ..)
	  */
	//virtual void refresh() = 0;

public: // renderer attributes
	/** Returns the font metrics object for this font. */
	virtual const UFontMetrics * getFontMetrics() const = 0;

	/** Returns a font info object which describes several font attributes */
	virtual UFontInfo getFontInfo() const = 0;

	/** The system dependent name of the used font. */
	virtual std::string getSystemName() const = 0;
};

} // namespace ufo

#endif // UFONTRENDERER_HPP
