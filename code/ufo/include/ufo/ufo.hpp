/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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
//#include "usysteminfo.hpp"
#include "utoolkit.hpp"
#include "uversioninfo.hpp"
#include "uvideodevice.hpp"
#include "uvideodriver.hpp"

// deleted
//#include "utexture.hpp"


// headers in util
#include "util/ucolor.hpp"
#include "util/ucolorgroup.hpp"
#include "util/ufilearchive.hpp"
#include "util/ugeom.hpp"
#include "util/uinsets.hpp"
#include "util/uinteger.hpp"
#include "util/upalette.hpp"
#include "util/uproperties.hpp"
#include "util/ustring.hpp"

// deleted
//#include "util/uhashmap.hpp"
//#include "util/uvector.hpp"
//#include "util/ulist.hpp"

// headers in events
#include "events/uevents.hpp"


// headers in widgets

#include "widgets/uwidget.hpp"
#include "widgets/ubutton.hpp"
#include "widgets/ucheckbox.hpp"
#include "widgets/ucheckboxmenuitem.hpp"
#include "widgets/ucombobox.hpp"
#include "widgets/ucompound.hpp"
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
#include "widgets/uradiobutton.hpp"
#include "widgets/urootpane.hpp"
#include "widgets/uscrollablewidget.hpp"
#include "widgets/uscrollbar.hpp"
#include "widgets/uscrollpane.hpp"
#include "widgets/useparator.hpp"
#include "widgets/uslider.hpp"
#include "widgets/utextedit.hpp"
#include "widgets/uviewport.hpp"

/*
// headers in borders
// deleted
#include "borders/uborder.hpp"
#include "borders/uborderfactory.hpp"
#include "borders/ubevelborder.hpp"
#include "borders/ucompoundborder.hpp"
#include "borders/uemptyborder.hpp"
#include "borders/ulineborder.hpp"
#include "borders/utitledborder.hpp"
*/
// headers in layouts

#include "layouts/ulayoutmanager.hpp"
#include "layouts/uflowlayout.hpp"
#include "layouts/uboxlayout.hpp"
#include "layouts/uborderlayout.hpp"

// headers in ui
#include "ui/ubuttonui.hpp"
#include "ui/ucomboboxui.hpp"
#include "ui/uinternalframeui.hpp"
#include "ui/ulabelui.hpp"
#include "ui/ulistboxui.hpp"
#include "ui/umenubarui.hpp"
#include "ui/umenuitemui.hpp"
#include "ui/upopupmenuui.hpp"
#include "ui/uscrollbarui.hpp"
#include "ui/useparatorui.hpp"
#include "ui/usliderui.hpp"
#include "ui/utexteditui.hpp"
#include "ui/uwidgetui.hpp"

#include "ui/uuimanager.hpp"
#include "ui/ustyle.hpp"
#include "ui/ulookandfeel.hpp"
#include "ui/uthemelookandfeel.hpp"

// deprecated

// deleted
//#include "ui/umenuui.hpp"
//#include "ui/uuiutilities.hpp"

// headers in ui/basic
// FIXME
// should they be included in main include file?
//#include "ui/basic/ubasicbuttonui.hpp"
//#include "ui/basic/ubasiccheckboxui.hpp"
//#include "ui/basic/ubasicinternalframeui.hpp"
//#include "ui/basic/ubasiclabelui.hpp"
//#include "ubasiclistui.hpp"
//#include "ui/basic/ubasiclookandfeel.hpp"
//#include "ui/basic/ubasicmenubarui.hpp"
//#include "ui/basic/ubasicmenuitemui.hpp"
//#include "ui/basic/ubasicmenuui.hpp"
//#include "ui/basic/ubasicpopupmenuui.hpp"
//#include "ui/basic/ubasicseparatorui.hpp"
//#include "ui/basic/ubasictextfieldui.hpp"
//#include "ui/basic/ubasictextpaneui.hpp"
//#include "ui/basic/ubasicborderfactory.hpp"
//#include "ui/basic/ubasiclistboxui.hpp"

// headers in text
#include "text/udocument.hpp"
#include "text/udocumentfactory.hpp"
#include "text/ubasicdocument.hpp"
#include "text/udocumentfilter.hpp"

#include "text/udocumentrenderer.hpp"
#include "text/udefaultdocumentrenderer.hpp"

// deleted
//#include "text/utextwidget.hpp"
//#include "text/uattribute.hpp"

// headers in font
#include "font/ufont.hpp"
#include "font/ufontrenderer.hpp"
#include "font/ufontmetrics.hpp"
//#include "font/unativefontrenderer.hpp"
#include "font/utexturefont.hpp"

// deleted
//#include "font/uglyph.hpp"

// headers in image
#include "image/uimage.hpp"
#include "image/uimagefilter.hpp"
#include "image/uimageicon.hpp"
#include "image/uimageio.hpp"

// deprecated
#include "image/uxbmicon.hpp"

#endif // UFO_HPP
