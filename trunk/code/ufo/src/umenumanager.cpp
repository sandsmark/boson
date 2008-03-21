/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/umenumanager.hpp"

#include "ufo/widgets/umenuitem.hpp"
#include "ufo/widgets/umenu.hpp"

// popup close listener
#include "ufo/ucontext.hpp"
#include "ufo/events/uevent.hpp"
#include "ufo/events/umouseevent.hpp"
#include "ufo/events/ukeyevent.hpp"
#include "ufo/widgets/upopupmenu.hpp"
#include "ufo/widgets/urootpane.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UMenuManager, UObject)

UMenuManager * UMenuManager::sm_menuManager = new UMenuManager();

UMenuManager::UMenuManager()
	: m_menuPath()
	, m_currentItem(NULL)
{}


void
UMenuManager::highlightItem(UMenuItem * item) {
	recalcPathWithLeaf(item);
}

void
UMenuManager::activateItem(UMenuItem * item) {
	if (UMenu * menu = dynamic_cast<UMenu*>(item)) {
		recalcPathWithLeaf(item);
		// toggle popup menu
		if (menu->isPopupMenuVisible()) {
			menu->setPopupMenuVisible(false);
			m_menuPath.pop_back();
			if (!m_menuPath.size()) {
				menu->getContext()->disconnectListener(slot(*this, &UMenuManager::closeMenuPopups));
			}
		} else {
			openMenu(menu);
		}
		m_currentItem = menu;
	} else {
		clearPath();
	}
}

void
UMenuManager::processKeyEvent(UKeyEvent * e) {
	if (e->isConsumed()) {
		return;
	}
	// always operate on the current item
	UWidget * w = m_currentItem;//e->getWidget();
	if (!w) {
		return;
	}
	UMenuItem * item = dynamic_cast<UMenuItem*>(w);
	UMenu * menu = dynamic_cast<UMenu*>(w);
	bool topLevel = false;
	if (menu) {
		topLevel = menu->isTopLevelMenu();
	}

	if (e->getType() == UEvent::KeyPressed) {
		switch (e->getKeyCode()) {
			case UKey::UK_KP_UP:
			case UKey::UK_UP:
				if (topLevel) {
					highlightPreviousSibling(dynamic_cast<UMenuItem*>(menu->getPopupMenu()->getWidget(0)));
				} else {
					highlightPreviousSibling(item);
				}
				e->consume();
			break;
			case UKey::UK_KP_DOWN:
			case UKey::UK_DOWN:
				if (topLevel) {
					highlightNextSibling(dynamic_cast<UMenuItem*>(menu->getPopupMenu()->getWidget(
						menu->getPopupMenu()->getWidgetCount() - 1)));
				} else {
					highlightNextSibling(item);
				}
				e->consume();
			break;
			case UKey::UK_KP_LEFT:
			case UKey::UK_LEFT:
				if (topLevel) {
					highlightPreviousTopLevel(menu);
				} else {
					UMenu * parentMenu = item->getParentMenu();
					if (parentMenu && !parentMenu->isTopLevelMenu()) {
						recalcPathWithLeaf(parentMenu);
						//activateItem(parentMenu);
					} else {
						highlightPreviousTopLevel(item);
					}
				}
				e->consume();
			break;
			case UKey::UK_KP_RIGHT:
			case UKey::UK_RIGHT:
				if (topLevel) {
					highlightNextTopLevel(menu);
				} else if (menu) {
					recalcPathWithLeaf(dynamic_cast<UMenuItem*>(menu->getPopupMenu()->getWidget(0)));
				} else {
					highlightNextTopLevel(item);
				}
				e->consume();
			break;
			case UKey::UK_KP_ENTER:
			case UKey::UK_RETURN:
				if (m_currentItem) {
					m_currentItem->activate();
				}
			break;
			case UKey::UK_ESCAPE:
				clearPath();
			break;
			default:
			break;
		}
	}
}

void
UMenuManager::processMouseEvent(UMouseEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed:
			if (UMenu * menu = dynamic_cast<UMenu*>(e->getSource())) {
				if (menu->contains(e->getLocation())) {
					e->consume();
					menu->requestFocus();
					menu->activate();
				}
			} else if (UMenuItem * item = dynamic_cast<UMenuItem*>(e->getSource())) {
				if (item->contains(e->getLocation())) {
					e->consume();
					// don't do any actual action yet.
				}
			}
		break;
		case UEvent::MouseReleased: {
			UWidget * w_under_mouse = e->getWidget();
			if (!w_under_mouse->contains(e->getLocation()) &&
					e->getWidget()->getRootPane(true)) {
				w_under_mouse = e->getWidget()->
					getRootPane(true)->getVisibleWidgetAt(e->getRootLocation());
			}

			if (dynamic_cast<UMenu*>(w_under_mouse) == NULL) {
				if (UMenuItem * item = dynamic_cast<UMenuItem*>(w_under_mouse)) {
					e->consume();
					item->activate();
				}
			}
		}
		break;
		case UEvent::MouseEntered:
			if (UMenu * menu = dynamic_cast<UMenu*>(e->getWidget())) {
				if (m_menuPath.size()) {
					if (std::find(m_menuPath.begin(), m_menuPath.end(), menu) == m_menuPath.end()) {
						// not yet in menu path, open menu
						menu->activate();
					} else {
						// in menu path, do nothing
						if (!dynamic_cast<UMenu*>(m_currentItem)) {
							m_currentItem->setState(WidgetHighlighted, false);
						}
						m_currentItem = menu;
					}
				} else {
					// open new menu
					//recalcPathWithLeaf(menu);
					//openMenu(menu);
					m_currentItem = menu;
				}
			} else {
				highlightItem(dynamic_cast<UMenuItem*>(e->getWidget()));
			}
			e->consume();
		break;
		default:
		break;
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
	}

	for (std::vector<UMenu*>::reverse_iterator rev_iter = m_menuPath.rbegin();
			(*rev_iter) != (*iter); ++rev_iter) {
		(*rev_iter)->setState(WidgetHighlighted, false);
		(*rev_iter)->setPopupMenuVisible(false);
	}
	(*iter)->setPopupMenuVisible(false);
	m_menuPath.erase(iter, m_menuPath.end());
}

void
UMenuManager::clearPath() {
	//recalcPathWithLeaf(NULL);
	clearPathFrom(m_menuPath.begin());
	if (m_currentItem) {
		m_currentItem->setState(WidgetHighlighted, false);
		m_currentItem = NULL;
	}
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
UMenuManager::openMenu(UMenu * menu) {
	if (!menu->isPopupMenuVisible()) {
		m_menuPath.push_back(menu);
		menu->setPopupMenuVisible(true);
		menu->setState(WidgetHighlighted);
		if (m_menuPath.size() == 1) {
			menu->getContext()->connectListener(slot(*this, &UMenuManager::closeMenuPopups));
		}
	}
}

void
UMenuManager::recalcPathWithLeaf(UMenuItem * item) {
	// select no item
	if (item == NULL) {
		clearPath();
		return;
	}
	// parent menus of item
	std::vector<UMenu*> parents;
	//UMenu * menu = dynamic_cast<UMenu*>(item);

	UMenu * temp = dynamic_cast<UMenu*>(item->getParentMenu());
	while (temp) {
		parents.insert(parents.begin(), temp);
		temp = dynamic_cast<UMenu*>(temp->getParentMenu());
	}
	/*
	// FIXME: or top level item
	if (!parents.size()) {
		// not in a valid hierarchy
		return;
	}*/

	std::vector<UMenu*>::iterator parents_iter = parents.begin();
	std::vector<UMenu*>::iterator path_iter = m_menuPath.begin();

	// this while loop checks for identical parent menus
	while (parents_iter != parents.end() && path_iter != m_menuPath.end()) {
		std::vector<UMenu*>::iterator pos =
			std::find(m_menuPath.begin(), m_menuPath.end(), *(parents_iter));
		if (pos != m_menuPath.end()) {
			++parents_iter;
			++path_iter;
		} else {
			break;
		}
	}
	// special case for deselecting top level menu
	if (!parents.size() && m_menuPath.size() && *(m_menuPath.begin()) == item) {
		++path_iter;
	}

	// clear all non-identical parents
	clearPathFrom(path_iter);

	// open all new parent menus
	for (std::vector<UMenu*>::iterator iter = parents_iter;
			iter != parents.end(); ++iter) {
		openMenu(*iter);
	}

	// deselect old current item, if in the current menu path
	if (m_currentItem) {
		std::vector<UMenu*>::iterator pos = std::find(m_menuPath.begin(), m_menuPath.end(), m_currentItem);
		if (pos == m_menuPath.end()) {
			m_currentItem->setState(WidgetHighlighted, false);
		}
	}

	// select the new current item
	m_currentItem = item;
	m_currentItem->setState(WidgetHighlighted);
}


void
UMenuManager::highlightNextSibling(UMenuItem * item) {
	if (item == NULL) {
		return;
	}
	UWidget * parent = item->getParent();
	if (!parent || !parent->getWidgetCount()) {
		return;
	}
	unsigned int startIndex = parent->getIndexOf(item);
	unsigned int index = startIndex;
	do {
		index++;
		if (index >= parent->getWidgetCount()) {
			index = 0;
		}
	} while ((!parent->getWidget(index)->isEnabled() ||
		!dynamic_cast<UMenuItem*>(parent->getWidget(index))) && index != startIndex);

	recalcPathWithLeaf(dynamic_cast<UMenuItem*>(parent->getWidget(index)));
}

void
UMenuManager::highlightPreviousSibling(UMenuItem * item) {
	if (item == NULL) {
		return;
	}
	UWidget * parent = item->getParent();
	if (!parent || !parent->getWidgetCount()) {
		return;
	}
	int startIndex = parent->getIndexOf(item);
	int index = startIndex;
	do {
		index--;
		if (index < 0) {
			index = parent->getWidgetCount() - 1;
		}
	} while ((!parent->getWidget(index)->isEnabled() ||
		!dynamic_cast<UMenuItem*>(parent->getWidget(index))) && index != startIndex);
	recalcPathWithLeaf(dynamic_cast<UMenuItem*>(parent->getWidget(index)));
}

void
UMenuManager::highlightNextTopLevel(UMenuItem * item) {
	if (m_menuPath.begin() == m_menuPath.end()) {
		return;
	}
	UMenu * menu = *(m_menuPath.begin());
	UWidget * parent = menu->getParent();
	if (!parent || !parent->getWidgetCount()) {
		return;
	}
	unsigned int startIndex = parent->getIndexOf(menu);
	unsigned int index = startIndex;
	do {
		index++;
		if (index >= parent->getWidgetCount()) {
			index = 0;
		}
	} while (!parent->getWidget(index)->isEnabled() && index != startIndex);

	if (parent->getWidget(index)->getPopupMenu() && parent->getWidget(index)->getPopupMenu()->getWidgetCount()) {
		recalcPathWithLeaf(dynamic_cast<UMenuItem*>(parent->getWidget(index)->getPopupMenu()->getWidget(0)));
	} else {
		recalcPathWithLeaf(dynamic_cast<UMenuItem*>(parent->getWidget(index)));
	}
}

void
UMenuManager::highlightPreviousTopLevel(UMenuItem * item) {
	if (m_menuPath.begin() == m_menuPath.end()) {
		return;
	}
	UMenu * menu = *(m_menuPath.begin());
	UWidget * parent = menu->getParent();
	if (!parent || !parent->getWidgetCount()) {
		return;
	}
	int startIndex = parent->getIndexOf(menu);
	int index = startIndex;
	do {
		index--;
		if (index < 0) {
			index = parent->getWidgetCount() - 1;
		}
	} while (!parent->getWidget(index)->isEnabled() && index != startIndex);

	if (parent->getWidget(index)->getPopupMenu() && parent->getWidget(index)->getPopupMenu()->getWidgetCount()) {
		recalcPathWithLeaf(dynamic_cast<UMenuItem*>(parent->getWidget(index)->getPopupMenu()->getWidget(0)));
	} else {
		recalcPathWithLeaf(dynamic_cast<UMenuItem*>(parent->getWidget(index)));
	}
}

void
UMenuManager::setMenuManager(UMenuManager * manager) {
	if (manager) {
		sm_menuManager = manager;
	}
}
