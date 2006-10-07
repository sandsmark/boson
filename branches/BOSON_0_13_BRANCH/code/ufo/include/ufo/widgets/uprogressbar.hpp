/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/uprogressbar.hpp
    begin             : Tue Mar 8 2005
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

#ifndef UPROGRESSBAR_HPP
#define UPROGRESSBAR_HPP

#include "uwidget.hpp"

namespace ufo {

class UProgressBarModel;

/** @short A progress bar indicates a progress
  * @ingroup widgets
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UProgressBar : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(UProgressBar)
	UFO_STYLE_TYPE(UStyle::CE_ProgressBar)
public:
	/** Creates a new progress bar with a minimum of 0, a maximum of 100
	  * and a value of 0
	  */
	UProgressBar();
	void setMaximum(int max);
	/** @return The maximum value for value */
	int getMaximum() const;
	void setMinimum(int min);
	/** @return The minimum value for value */
	int getMinimum() const;
	/** Sets minimum and maximum in one call. */
	void setRange(int min, int max);

	/** @return The progress value. */
	int getValue() const;
	void setValue(int value);

	/** @return A formatted text which is used as label.
	  *  Usually the percentage.
	  */
	virtual std::string getText() const;

	/** Sets whether the completed percentage should be displayed. */
	void setTextVisible(bool vis);
	/** @see setTextVisible */
	bool isTextVisible() const;

protected: // Overrides UWidget
	virtual UDimension getContentsSize(const UDimension & maxSize) const;
protected: // protected methods
	UProgressBarModel * getProgressBarModel() const;
	void updateText();
};

} // namespace ufo

#endif // UPROGRESSBAR_HPP
