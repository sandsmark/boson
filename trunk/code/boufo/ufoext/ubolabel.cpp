/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2004 by Andreas Beckermann
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ubolabel.h"
#include "ubolabelui.h"

#include <bogl.h>

#include <ufo/util/udimension.hpp>

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UBoLabel, ULabel)

UBoLabel::UBoLabel()
{
}

// AB: note: we use the default ULabel rendering, which gets reimplemented by us
// in UBosonStyle.
// we need to implement size calculation here, because we do not have access to
// * class type (label/button)
// * maxSize parameter
// in UStyle::getCompoundPreferredSize().
UDimension UBoLabel::getContentsSize(const UDimension& maxSize) const
{
 UBoLabelUI ui;
 return ui.getPreferredSize(this, maxSize);
}

