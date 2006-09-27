/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/text/udocumentfactory.hpp
    begin             : Sat Dec 8 2001
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

#ifndef UDOCUMENTFACTORY_HPP
#define UDOCUMENTFACTORY_HPP

#include "../uobject.hpp"
#include "udocument.hpp"

namespace ufo {

/*
template <class NumT>
class UNumberDocument : public UDefaultDocument {
};
*/

/**A factory for some commonly used documents like "Only-Numbers"-documents, ..
  *@author johannes
  */

class UFO_EXPORT UDocumentFactory : public UObject  {
	UFO_DECLARE_DYNAMIC_CLASS(UDocumentFactory)
public:
	/** Returns a document for the given mime type.
	  * So far, only "text/plain" is supported.
	  * Returns Null for undefined document types.
	  */
	static UDocument * createMimeDocument(const std::string & typeA);

	/** Returns a plain text document.
	  * The returned document is suitable for text fields (\n filtering).
	  */
	static UDocument * createPlainDocument();

	/** Returns a document which allows digits as input.
	  * The returned document is suitable for text fields (\n filtering).
	  */
	static UDocument * createDigitDocument();

	/** Returns a document which allows only numbers as input.
	  * The returned document is suitable for text fields (\n filtering).
	  */
	/*
	template <class NumT>
	static UDocument * createNumberDocument() {
		return new UNumberDocument<NumT>;
	}
	*/

	/** Returns a document which only allows characters which are
	  * within the <code>allowedCharatersA</code> string.
	  * The returned document is suitable for text fields (\n filtering).
	  */
	static UDocument * createSpecialCharDocument(const std::string & allowedCharatersA);

	/** Creates a filter which removes \n charaters. */
	//static UDocumentFilter * createNewLineFilter();

	/** Creates a filter which allows only numbers as input. */
	//static UDocumentFilter * createDigitFilter();
};

} // namespace ufo

#endif // UDOCUMENTFACTORY_HPP
