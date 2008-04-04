/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ustackwidget.hpp
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

#ifndef USTACKWIDGET_HPP
#define USTACKWIDGET_HPP

#include "uwidget.hpp"

namespace ufo {

/** @short A stack widget orders its child widgets as stack.
  * @ingroup widgets
  *
  * There are two types of stacks. If exclusive mode is selected, only
  * the current selected widget is shown. Otherwise you get the whole stack
  * shown.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UStackWidget : public UWidget {
	UFO_DECLARE_CLASS(UStackWidget)
public:
	/** Creates a stack widget which exclusively shows the selected widget. */
	UStackWidget();
	/** Creates a stack widget with the given selection mode.
	  * FIXME: what is a selection mode, define selection model etc.
	  */
	UStackWidget(int selectionMode);

public: // Public methods
	void setSelectedIndex(int index);
	int getSelectedIndex() const;

protected: // Overrides UWidget
	virtual void addImpl(UWidget * w, UObject * constraints, int index);
	virtual bool removeImpl(int index);
	virtual UDimension getContentsSize(const UDimension & maxSize) const;

private:
	int m_selectedIndex;
	int m_selectionMode;
};

} // namespace ufo

#endif // USTACKEDWIDGET_HPP
