/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/util/upalette.hpp
    begin             : Fri Jun 11 2004
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

#ifndef UPALETTE_HPP
#define UPALETTE_HPP

#include "../uobject.hpp"

#include "ucolor.hpp"
#include "ucolorgroup.hpp"

namespace ufo {

/** A palettes represents the color set used to paint a widget. 
  * @author Johannes Schmidt
  */
class UFO_EXPORT UPalette : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UPalette)
public: // Public types
	enum ColorGroupType {
		Active,
		Inactive,
		Disabled,
		NColorGroupType
	};
public: // Static members
	static const UPalette nullPalette;

public:
	UPalette(const UColorGroup & active, const UColorGroup & disabled, 
		const UColorGroup & inactive);
	virtual ~UPalette();

	const UColor & getColor(ColorGroupType group, 
		UColorGroup::ColorRole role) const;
	void setColor(ColorGroupType group, 
		UColorGroup::ColorRole role, const UColor & color);

	const UColorGroup & getColorGroup(ColorGroupType groupType) const;
	void setColorGroup(ColorGroupType groupType, const UColorGroup & group);

	const UColorGroup & getActive() const;
	void setActive(const UColorGroup & active);

	const UColorGroup & getInactive() const;
	void setInactive(const UColorGroup & inactive);

	const UColorGroup & getDisabled() const;
	void setDisabled(const UColorGroup & disabled);

public: // Public operators
	bool operator==(const UPalette & pal) const;
	bool operator!=(const UPalette & pal) const;

protected: // Overrides UObject
	virtual std::ostream & paramString(std::ostream & os) const;

private:
	void detach();

private:
	USharedPtr<UColorGroup> m_active;
	USharedPtr<UColorGroup> m_inactive;
	USharedPtr<UColorGroup> m_disabled;
};

//
// inline implementation
//


inline const UColorGroup & 
UPalette::getColorGroup(ColorGroupType groupType) const {
	switch (groupType) {
		case Active:
			return *(m_active);
			break;
		case Inactive:
			return *(m_inactive);
			break;
		case Disabled:
			return *(m_disabled);
			break;
		default:
			return *(m_active);
			break;
	}
}

inline const UColorGroup & 
UPalette::getActive() const {
	return *(m_active);
}
inline const UColorGroup & 
UPalette::getInactive() const {
	return *(m_inactive);
}
inline const UColorGroup & 
UPalette::getDisabled() const {
	return *(m_disabled);
}

inline bool
UPalette::operator!=(const UPalette & pal) const {
	return !(operator==(pal));
}

} // namespace ufo

#endif // UPALETTE_HPP
