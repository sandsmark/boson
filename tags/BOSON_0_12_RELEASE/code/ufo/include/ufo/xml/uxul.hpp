/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/xml/uxul.hpp
    begin             : Sat Feb 27 2005
    $ $
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

#ifndef UXUL_HPP
#define UXUL_HPP

#include "../uobject.hpp"

#include "../util/urectangle.hpp"
#include "../signals/ufo_signals.hpp"

#include <map>

class TiXmlDocument;
class TiXmlElement;

namespace ufo {

class UXFrame;
class URootPane;
class UWidget;

/** @short A XUL parser.
  * @ingroup widgets
  *
  * XUL is the XML User interface Language used by mozila
  * to create its UI.
  * <p>
  * Only one frame and one root pane may be created from a XUL file.
  * Use a new XUL object if you want to create a second root pane/frame.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXul : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UXul)
public:
	UXul();
	UXul(const std::string & guiFile);
	UXul(const std::string & guiFile, std::map<std::string, UActionSlot> map);
	virtual ~UXul();
	void load(const std::string & guiFile);
	void setActionMap(std::map<std::string, UActionSlot> map);
	/** Creates a root pane with widgets created according
	  *  the previous loaded XUL file.
	  */
	URootPane * createRootPane();
	/** Creates a frame with widgets created according
	  *  the previous loaded XUL file.
	  */
	UXFrame * createFrame();
	/** @return The widget with the given id or NULL. */
	UWidget * get(const std::string & id);

private: // Private attributes
	TiXmlDocument * m_doc;
	URootPane * m_root;
	std::string m_title;
	std::map<std::string, UWidget*> m_map;
	std::map<std::string, UActionSlot> m_actionMap;
};

} // namespace ufo

#endif // UXUL_HPP
