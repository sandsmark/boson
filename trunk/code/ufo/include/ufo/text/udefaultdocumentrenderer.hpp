/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/text/udefaultdocumentrenderer.hpp
    begin             : Wed Sep 5 2001
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

#ifndef UDEFAULTDOCUMENTRENDERER_HPP
#define UDEFAULTDOCUMENTRENDERER_HPP

#include "udocumentrenderer.hpp"

namespace ufo {

class UDocument;
class UFont;

/**
  *@author Johannes Schmidt
  */

class UFO_EXPORT UDefaultDocumentRenderer : public UDocumentRenderer {
	UFO_DECLARE_DYNAMIC_CLASS(UDefaultDocumentRenderer)
public:
	UDefaultDocumentRenderer();
	virtual ~UDefaultDocumentRenderer();

	/**
	  * @return
	  *	The rectangle of the letter at the given offset in the coordinate
	  *	system of the rendered text.
	  */
	virtual URectangle 
	modelToView(const UDocument * docA, int offsA,
		const UFont * fontA) const;

	/**
	  * @return
	  *	The offset of the letter at the given position in the coordinate
	  *	system of the rendered text.
	  */
	virtual int
	viewToModel(const UDocument * docA, const UPoint & posA,
		const UFont * fontA) const;

	UDimension getPreferredSize(const UDocument * docA,
		const UFont * fontA) const;

	/** The document text will be rendered in a rectangle of the given size
	  * @param docA The document which should be rendered
	  * @param sizeA
	  */
	void render(UGraphics * g, const UDocument * docA,
		const UDimension & sizeA, const UFont * fontA);

	void setTabSize(int tabSizeA);
	int getTabSize();
private:
	int m_tabSize;
};

} // namespace ufo

#endif // UDEFAULTDOCUMENTRENDERER_HPP
