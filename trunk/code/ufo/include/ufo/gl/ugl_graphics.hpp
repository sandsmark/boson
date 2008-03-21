/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/gl/ugl_graphics.hpp
    begin             : Fri Oct 17 2003
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#ifndef UGL_GRAPHICS_HPP
#define UGL_GRAPHICS_HPP

#include "../ugraphics.hpp"

#include "../util/urectangle.hpp"
#include "../util/ucolor.hpp"

namespace ufo {

class UImageIO;

/** @short OpenGL implementation of the UGraphics interface.
  * @ingroup opengl
  * @ingroup internal
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UGL_Graphics : public UGraphics  {
	UFO_DECLARE_DYNAMIC_CLASS(UGL_Graphics)
public:
	UGL_Graphics(UContext * context);
	virtual ~UGL_Graphics();

public: // Implements UGraphics
	virtual UContext * getContext() const;

	virtual void resetDeviceAttributes();
	virtual void resetDeviceViewMatrix();

	virtual URectangle mapToDevice(const URectangle & rect);
	virtual URectangle mapFromDevice(const URectangle & rect);

	virtual void begin();
	virtual void end();
	virtual void flush();
	virtual void clear();
	virtual UImageIO * dump();

	virtual void setColor(const UColor & color);
	virtual UColor getColor() const;

	virtual void setClearColor(const UColor & clearColor);
	virtual UColor getClearColor() const;

	virtual void setFont(const UFont & font);
	virtual UFont getFont() const;

	virtual void setEnabled(GCState state, bool b);
	virtual bool isEnabled(GCState state) const;


	virtual void drawString(const std::string & text, int x = 0, int y = 0);
	virtual UDimension getStringSize(const std::string & string);


	//virtual void pushClipRect();
	//virtual void popClipRect();
	virtual void setClipRect(const URectangle & rect);
	virtual URectangle getClipRect() const;

	virtual void setLineWidth(float width);
	virtual float getLineWidth() const;

	virtual void translate(float x, float y);
	virtual float getTranslationX() const;
	virtual float getTranslationY() const;


	virtual void drawRect(const URectangle & rect);
	virtual void fillRect(const URectangle & rect);
	virtual void drawLine(const UPoint & p1, const UPoint & p2);

	virtual void drawVertexArray(VertexType type, UVertexArray * buffer);

	virtual void drawImage(UImage * image, const URectangle & rect);

	virtual void drawSubImage(UImage * image,
		const URectangle & srcRect, const URectangle & destRect);

private:
	UContext * m_context;
	UColor m_color;
	UColor m_clearColor;
	UFont m_font;
	URectangle m_clipRect;
	float m_translationX;
	float m_translationY;
};

} // namespace ufo

#endif // UGL_GRAPHICS_HPP
