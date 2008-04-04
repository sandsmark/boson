/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/utabbar.hpp
    begin             : Fri Sep 23 2005
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

#ifndef UTABBAR_HPP
#define UTABBAR_HPP

#include "uwidget.hpp"

namespace ufo {

/** @short A tab bar provides a box of tabs which may be used to select
  *  tab panels of a tab widgets.
  * @ingroup widgets
  *
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UTabBar : public UWidget {
	UFO_DECLARE_CLASS(UTabBar)
public:
	UTabBar();
	virtual ~UTabBar();

public: // Public methods
	void addTab(const std::string & label);
	/** Removes the tab at the given index and eventually resets
	  * the selected index.
	  */
	void removeTab(int index);

	void setSelectedIndex(int index);
	int getSelectedIndex() const;

	/** Returns the label of the tab bar tab with the given index
	  * or "" if the index is out of range.
	  */
	std::string getTabText(int index) const;
	/** Returns the index of the tab bar tab with the given label
	  * or -1 if no tab has this label.
	  */
	int getTabIndex(const std::string & label) const;

	int getTabCount() const;

private:
	int m_selectedIndex;

public: // Public signals
	USignal1<UTabBar*> & sigSelectionChanged();

private: // Private signals
	USignal1<UTabBar*> m_sigSelectionChanged;
};

//
// inline implementation
//

inline USignal1<UTabBar*> &
UTabBar::sigSelectionChanged() {
	return m_sigSelectionChanged;
}

} // namespace ufo

#endif // UTABBAR_HPP
