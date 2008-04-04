/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uvideodriver.hpp
    begin             : Sun Aug 8 2004
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

#ifndef UVIDEODRIVER_HPP
#define UVIDEODRIVER_HPP

#include "uobject.hpp"

namespace ufo {

class UVideoDevice;

/** @short The video driver loads all necessary multimedia libraries and
  *  initializes the system to create video devices.
  * @ingroup windowing
  * @ingroup native
  *
  * @see UVideoDevice
  * @author Johannes Schmidt
  */
class UFO_EXPORT UVideoDriver : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UVideoDriver)
public:
	/** Initializes the video driver, loads necessary native drivers.
	  * @return True on sucess
	  */
	virtual bool init() = 0;
	/** @return True if initialization was succesful.
	  */
	virtual bool isInitialized() = 0;
	/** Purges all loaded system resources and deinits the video driver.
	  */
	virtual void quit() = 0;
	/** @return The name describing this video driver. Should be the same
	  *  used via env var UFO_VIDEO_DRIVER.
	  */
	virtual std::string getName() = 0;

	/** Gathers all system events and pumps them to the UFO event queue
	  * (UXDisplay).
	  */
	virtual void pumpEvents() = 0;

	/** Creates a video device (window).
	  * @return A video device which is used by UXFrame or Null on failure.
	  */
	virtual UVideoDevice * createVideoDevice() = 0;
};

} // namespace ufo

#endif // UVIDEODRIVER_HPP
