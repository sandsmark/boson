/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uitem.hpp
    begin             : Sat May 24 2003
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

#ifndef UITEM_HPP
#define UITEM_HPP

#include "../uobject.hpp"

#include "../util/udimension.hpp"

namespace ufo {

class UGraphics;
class UColor;
class UWidget;

/** This is an abstract base class for items. This includes list box items,
  * combo box items, tab items, ...
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UItem : public virtual UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UItem)
public:
	/** Paints the item at the specified position with the specified values.
	  */
	virtual void paintItem(UGraphics * g, UWidget * parent,
		int x, int y,
		bool isSelectedA, bool hasFocusA,
		const UColor & foreground, const UColor & background) = 0;
	/** Returns the desired item size. */
	virtual UDimension getItemSize(const UWidget * parent) const = 0;

	/** A string representation of the current value.
	  * Most items show up only a string.
	  */
	virtual std::string itemToString() const = 0;

	virtual void install(UWidget * parent) = 0;
	virtual void uninstall(UWidget * parent) = 0;
};

class UIcon;
/** A common representation of a string (+ icon) icon.
  * @author Johannes Schmidt
  */
class UFO_EXPORT UStringItem : public UItem {
	UFO_DECLARE_DYNAMIC_CLASS(UStringItem)
public:
	UStringItem();
	UStringItem(const std::string & str);
	UStringItem(UIcon * i);
	UStringItem(const std::string & str, UIcon * icon);

public: // Implements UListBoxItem
	virtual void paintItem(UGraphics * g, UWidget * parent,
		int x, int y,
		bool isSelectedA, bool hasFocusA,
		const UColor & foreground, const UColor & background);

	virtual UDimension getItemSize(const UWidget * parent) const;

	virtual std::string itemToString() const;

	virtual void install(UWidget * parent);
	virtual void uninstall(UWidget * parent);

protected: // Overrides UObject
	virtual std::ostream & paramString(std::ostream & os) const;
private: // Private attributes
	UIcon * m_icon;
	std::string m_text;
};

} // namespace ufo

#endif // UITEM_HPP
