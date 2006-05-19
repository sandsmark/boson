/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ufo.hpp
    begin             : Thu Jun 28 2001
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

// the main include header

#ifndef UFO_HPP
#define UFO_HPP

#include "ufo_global.hpp"


// headers in core
#include "ubuttongroup.hpp"
#include "ucollectable.hpp"
#include "ucontext.hpp"
#include "udisplay.hpp"
#include "udrawable.hpp"

#include "ufocusmanager.hpp"
#include "ugraphics.hpp"
#include "uicon.hpp"
#include "uinputmap.hpp"
#include "ukeystroke.hpp"
#include "umenumanager.hpp"
#include "uobject.hpp"
#include "uplugin.hpp"
#include "upopup.hpp"
#include "upopupmanager.hpp"
#include "urepaintmanager.hpp"
#include "usharedlib.hpp"
#include "usharedptr.hpp"
// this would includes native system headers
//#include "usysteminfo.hpp"
#include "utoolkit.hpp"
#include "uversioninfo.hpp"
#include "uvideodevice.hpp"
#include "uvideodriver.hpp"
#include "uvalidator.hpp"


// headers in events
#include "events/uevents.hpp"


// headers in font
#include "font/ufont.hpp"
#include "font/ufontrenderer.hpp"
#include "font/ufontmetrics.hpp"
//#include "font/unativefontrenderer.hpp"
#include "font/utexturefont.hpp"


// headers in image
#include "image/uimage.hpp"
#include "image/uimagefilter.hpp"
#include "image/uimageicon.hpp"
#include "image/uimageio.hpp"

// deprecated
#include "image/uxbmicon.hpp"


// headers in layouts
#include "layouts/ulayoutmanager.hpp"
#include "layouts/uflowlayout.hpp"
#include "layouts/uboxlayout.hpp"
#include "layouts/uborderlayout.hpp"


// headers in text
#include "text/udocument.hpp"
#include "text/udocumentfactory.hpp"
#include "text/ubasicdocument.hpp"
#include "text/utextlayout.hpp"


// headers in ui
#include "ui/ustyle.hpp"
#include "ui/ustylehints.hpp"
#include "ui/ustylemanager.hpp"
#include "ui/ucss.hpp"


// headers in util
#include "util/ucolor.hpp"
#include "util/ufilearchive.hpp"
#include "util/ugeom.hpp"
#include "util/uinsets.hpp"
#include "util/uinteger.hpp"
#include "util/upalette.hpp"
#include "util/uproperties.hpp"
#include "util/ustring.hpp"


// headers in widgets
#include "widgets/uwidget.hpp"
#include "widgets/ubutton.hpp"
#include "widgets/ucheckbox.hpp"
#include "widgets/ucheckboxmenuitem.hpp"
#include "widgets/ucombobox.hpp"
#include "widgets/ucompound.hpp"
#include "widgets/udesktoppane.hpp"
#include "widgets/udockwidget.hpp"
#include "widgets/ugroupbox.hpp"
#include "widgets/uinternalframe.hpp"
#include "widgets/uitem.hpp"
#include "widgets/ulabel.hpp"
#include "widgets/ulayeredpane.hpp"
#include "widgets/ulineedit.hpp"
#include "widgets/ulistbox.hpp"
#include "widgets/umenu.hpp"
#include "widgets/umenubar.hpp"
#include "widgets/umenuitem.hpp"
#include "widgets/upopupmenu.hpp"
#include "widgets/uprogressbar.hpp"
#include "widgets/uradiobutton.hpp"
#include "widgets/urootpane.hpp"
#include "widgets/uscrollablewidget.hpp"
#include "widgets/uscrollbar.hpp"
#include "widgets/uscrollpane.hpp"
#include "widgets/useparator.hpp"
#include "widgets/uslider.hpp"
#include "widgets/uspinbox.hpp"
#include "widgets/ustackwidget.hpp"
#include "widgets/utabbar.hpp"
#include "widgets/utabwidget.hpp"
#include "widgets/utextedit.hpp"
#include "widgets/utextwidget.hpp"
#include "widgets/uviewport.hpp"


// headers in xml
#include "xml/uxul.hpp"

#endif // UFO_HPP
