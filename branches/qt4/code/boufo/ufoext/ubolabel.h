/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2004-2005 by Andreas Beckermann
    email             : b_mann at gmx.de
                             -------------------

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


#ifndef UBOLABEL_H
#define UBOLABEL_H

#include <ufo/widgets/ulabel.hpp>
#include <ufo/util/ucolor.hpp>

namespace ufo {

class UFO_EXPORT UBoLabel : public ULabel {
	UFO_DECLARE_DYNAMIC_CLASS(UBoLabel)
	UFO_UI_CLASS(UBoLabelUI)
public:
	UBoLabel();

protected:
	virtual UDimension getContentsSize(const UDimension& maxSize) const;
};

} // namespace ufo

#endif

