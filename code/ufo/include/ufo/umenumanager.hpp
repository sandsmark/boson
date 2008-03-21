/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/umenumanager.hpp
    begin             : Sun Dec 14 2003
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

#ifndef UMENUMANAGER_HPP
#define UMENUMANAGER_HPP

#include "uobject.hpp"

#include <vector>

namespace ufo {

class UMenuItem;
class UMenu;
class UEvent;

class UKeyEvent;
class UMouseEvent;

/** @short Manages menu trees for popup menus.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UMenuManager : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UMenuManager)
public:
	UMenuManager();

	virtual void highlightItem(UMenuItem * item);
	virtual void activateItem(UMenuItem * item);

	virtual void clearPath();

	virtual void processKeyEvent(UKeyEvent * e);
	virtual void processMouseEvent(UMouseEvent * e);

public: // Public static methods
	static void setMenuManager(UMenuManager * manager);
	static UMenuManager * getMenuManager();

protected: // Private slots
	/** @return The parent menu or the given item if it is itsself
	  * an aldready visible menu.
	  */
	std::vector<UMenu*>::iterator getIteratorOfSameHierarchy(UMenuItem * item);
	/** Clears i.e. closes all menus following @p iter. */
	void clearPathFrom(const std::vector<UMenu*>::iterator & iter);
	/** Closes all menu popups. */
	void closeMenuPopups(UEvent * e);

	void openMenu(UMenu * menu);

	void recalcPathWithLeaf(UMenuItem * item);

	void highlightNextSibling(UMenuItem * item);
	void highlightPreviousSibling(UMenuItem * item);
	void highlightNextTopLevel(UMenuItem * item);
	void highlightPreviousTopLevel(UMenuItem * item);

private:
	std::vector<UMenu*> m_menuPath;
	UMenuItem * m_currentItem;

private: // Private static attributes
	static UMenuManager * sm_menuManager;
};

//
// inline implementation
//

inline UMenuManager *
UMenuManager::getMenuManager() {
	return sm_menuManager;
}

} // namespace ufo

#endif // UMENUMANAGER_HPP
