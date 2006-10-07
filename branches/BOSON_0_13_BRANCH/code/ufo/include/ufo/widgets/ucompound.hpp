/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ucompound.hpp
    begin             : Fri Mar 7 2003
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

#ifndef UCOMPOUND_HPP
#define UCOMPOUND_HPP

#include "uwidget.hpp"

#include "../uicon.hpp"

namespace ufo {

class UCompoundModel;

/** @short This class represents a compound widget with a static text and an icon.
  * @ingroup abstractwidgets
  *
  * It is not meant to be used directly. Use instead ULabel, UButton, ...
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UCompound : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UCompound)
public:
	UCompound();
	UCompound(const std::string & text);
	UCompound(UIcon * icon);
	UCompound(const std::string & text, UIcon * icon);

	/** sets the visible text
	  * @see #getText
	  * @param text the static which should be visible on screen
	  */
	virtual void setText(const std::string & text);

	/** gets the visible text
	  * @see #setText
	  * @return the static text visible on screen
	  */
	std::string getText() const;

	//
	//  ICON section
	//

	/** Sets the default icon for painting.
	  */
	void setIcon(UIcon * icon);

	/** @return The icon which was set with setIcon
	  * @see setIcon
	  */
	UIcon * getDefaultIcon() const;

	/** returns the icon that is currently used to paint this button.
	  * @return The icon which would be used to paint this icon at the moment
	  */
	virtual UIcon * getIcon() const;

	/**
	  */
	void setDisabledIcon(UIcon * icon);
	/** returns the icon that is currently used to paint this button.
	  * may be NULL
	  */
	UIcon * getDisabledIcon() const;


	/** Sets the gap between the text and the icon
	  */
	void setIconTextGap(int iconTextGap);
	/** Returns the gap between the text and the icon
	  */
	int getIconTextGap() const;

protected:  // Overrides UWidget
	virtual std::ostream & paramString(std::ostream & os) const;
	virtual void processStateChangeEvent(uint32_t state);
	virtual void processStyleHintChange(uint32_t styleHint);

protected: // Protected methods
	UCompoundModel * getCompoundModel() const;
	void updateMnemonic();

private:  // Protected attributes
	/** the default icon */
	UIcon * m_icon;
	/** the icon which is shown when the icon is disabled */
	UIcon * m_disabledIcon;
	/** The gap between the icon and the text in pixels */
	int m_iconTextGap;
};

//
// inline implementation
//

inline UIcon *
UCompound::getDisabledIcon() const {
	return m_disabledIcon;
}


inline int
UCompound::getIconTextGap() const {
	return m_iconTextGap;
}

} // namespace ufo

#endif // UCOMPOUND_HPP
