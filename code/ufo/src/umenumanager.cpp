/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/umenumanager.cpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/umenumanager.hpp"

#include "ufo/widgets/umenuitem.hpp"
#include "ufo/widgets/umenu.hpp"

// popup close listener
#include "ufo/ucontext.hpp"
#include "ufo/events/uevent.hpp"
#include "ufo/events/umouseevent.hpp"
#include "ufo/widgets/upopupmenu.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UMenuManager, UObject)

UMenuManager * UMenuManager::sm_menuManager = new UMenuManager();

UMenuManager::UMenuManager()
	: m_menuPath()
{}


void
UMenuManager::highlightItem(UMenuItem * item) {
	if (m_menuPath.size()) {
		std::vector<UMenu*>::iterator oldActive = getIteratorOfSameHierarchy(item);
		clearPathFrom(oldActive);
		if (UMenu * menu = dynamic_cast<UMenu*>(item)) {
			// search parent
			//std::vector<UMenu*>::iterator oldActive = getIteratorOfSameHierarchy(menu);
			//if (oldActive != m_menuPath.end()) {
				//clearPathFrom(oldActive);
				menu->setPopupMenuVisible(true);
				m_menuPath.push_back(menu);
				if (m_menuPath.size() == 1) {
					item->getContext()->connectListener(slot(*this, &UMenuManager::closeMenuPopups));
				}
			//}
		}
	}
}

void
UMenuManager::activateItem(UMenuItem * item) {
	if (UMenu * menu = dynamic_cast<UMenu*>(item)) {
		if (menu->isTopLevelMenu()) {
			if (m_menuPath.size() && menu == (*(m_menuPath.begin()))) {
				clearPath();
			} else {
				clearPath();
				menu->setPopupMenuVisible(true);
				m_menuPath.push_back(menu);
				item->getContext()->connectListener(slot(*this, &UMenuManager::closeMenuPopups));
			}
		} else {
			std::vector<UMenu*>::iterator iter = getIteratorOfSameHierarchy(item);
			if (menu->isPopupMenuVisible()) {
				// don't call clearPath before
				// checking for popup menu visibilty
				clearPathFrom(iter);
			} else {
				clearPathFrom(iter);
				menu->setPopupMenuVisible(true);
				m_menuPath.push_back(menu);
			}
		}
	} else {
		//item->doClick();
		item->setRollover(false);
		//item->setActive(false);
		clearPath();
	}
}

std::vector<UMenu*>::iterator
UMenuManager::getIteratorOfSameHierarchy(UMenuItem * item) {
	std::vector<UMenu*>::iterator iter = m_menuPath.begin();
	for (; iter != m_menuPath.end(); ++iter) {
		if ((*iter)->getParent() == item->getParent()) {
			break;
		}
	}
	return iter;
}

void
UMenuManager::clearPathFrom(const std::vector<UMenu*>::iterator & iter) {
	if (iter == m_menuPath.end()) {
		return;
	}
	if (iter == m_menuPath.begin() && iter != m_menuPath.end()) {
		(*iter)->getContext()->disconnectListener(slot(*this, &UMenuManager::closeMenuPopups));
	}/*
	for (std::vector<UMenu*>::iterator iter2 = iter;
			iter2 != m_menuPath.end(); ++iter2) {
		(*iter2)->setPopupMenuVisible(false);
	}*/
	for (std::vector<UMenu*>::reverse_iterator rev_iter = m_menuPath.rbegin();
			(*rev_iter) != (*iter); ++rev_iter) {
		(*rev_iter)->setPopupMenuVisible(false);
	}
	(*iter)->setPopupMenuVisible(false);
	m_menuPath.erase(iter, m_menuPath.end());
}

void
UMenuManager::clearPath() {
	clearPathFrom(m_menuPath.begin());
}

void
UMenuManager::closeMenuPopups(UEvent * e) {
	if (e->getType() == UEvent::MousePressed) {
		UWidget * source = dynamic_cast<UWidget*>(e->getSource());
		// exit when mouse event occured on a menu
		if (dynamic_cast<UMenuItem*>(source)) {
			return;
		}
		// don't care about clicks on popup menus
		bool popup = false;
		for (; !popup && source != NULL; source = source->getParent()) {
			if (dynamic_cast<UPopupMenu*>(source)) {
				popup = true;
			}
		}
		if (!popup) {
			clearPath();
		}
	}
}

void
UMenuManager::setMenuManager(UMenuManager * manager) {
	if (manager) {
		sm_menuManager = manager;
	}
}
