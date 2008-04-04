/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/utabwidget.hpp
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

#ifndef UTABWIDGET_HPP
#define UTABWIDGET_HPP

#include "uwidget.hpp"

namespace ufo {

class UStackWidget;
class UTabBar;

/** @short A tab widget provides a stack of widgets which order
  *  may be changed via tabs.
  * @ingroup widgets
  *
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UTabWidget : public UWidget {
	UFO_DECLARE_CLASS(UTabWidget)
public:
	UTabWidget();
	virtual ~UTabWidget();

public: // Public methods
	void addTab(UWidget * child, const std::string & label);
	void removeTab(int index);

	/** Returns the label of the tab bar tab with the given index
	  * or "" if the index is out of range.
	  */
	std::string getTabText(int index) const;
	/** Returns the tab page with the given index
	  * or NULL if the index is out of range.
	  */
	UWidget * getTabWidget(int index) const;

	/** Returns the index of the tab bar tab with the given label
	  * or -1 if no tab has this label.
	  */
	int getTabIndex(const std::string & label) const;
	/** Returns the tab index for the given child (or -1). */
	int getTabIndex(UWidget * child) const;

	void setSelectedIndex(int index);
	int getSelectedIndex() const;

	int getTabCount() const;

protected: // Protected methods
	virtual void slotTabSelected(UTabBar * bar);

public: // Public signals
	USignal1<UTabWidget*> & sigSelectionChanged();

private: // Private signals
	USignal1<UTabWidget*> m_sigSelectionChanged;

private:
	UStackWidget * m_stackWidget;
	UTabBar * m_tabBar;
};

//
// inline implementation
//

inline USignal1<UTabWidget*> &
UTabWidget::sigSelectionChanged() {
	return m_sigSelectionChanged;
}

} // namespace ufo

#endif // UTABWIDGET_HPP
