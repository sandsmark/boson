/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uplugin.hpp
    begin             : Wed Apr 2 2003
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

#ifndef UPLUGIN_HPP
#define UPLUGIN_HPP

#include "uobject.hpp"
#include "font/ufontinfo.hpp"

namespace ufo {

class UFontRenderer;
class ULookAndFeel;
class UVideoDriver;
class USharedLib;

/** @short This is the base class for all plug-ins.
  * @ingroup plugin
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UPluginBase : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UPluginBase)
};

typedef UPluginBase* (*plugin_create_t)(void);
typedef void (*plugin_destroy_t)(UPluginBase*);

#define UFO_PLUGIN(category, feature, plugin) \
  extern "C" { \
    const char* ufo_plugin_category = category; \
    const char* ufo_plugin_feature  = feature; \
    UPluginBase* ufo_plugin_create() \
    { return static_cast<PluginBase*>(new plugin()); } \
    void ufo_plugin_destroy(PluginBase* c) \
    { delete static_cast<plugin*>(c); } \
  }

/** @short The plugin info struct stores create and destroy methods for
  *  dynamic plugin creation.
  * @ingroup plugin
  *
  * @author Johannes Schmidt
  */
struct UFO_EXPORT UPluginInfo {
	/** The shared lib which the plugin is compiled in.
	  * May be NULL for static plugins (like texture font renderer, ...)
	  */
	USharedLib * lib;
	/** The overall category (e.g. font, laf, ...). */
	std::string category;
	/** The feature, e.g. 'native' for the native font renderer. */
	std::string feature;
	plugin_create_t create;
	plugin_destroy_t destroy;
};


inline UFO_EXPORT bool operator==(const UPluginInfo & p1,const UPluginInfo & p2) {
	return (
		p1.lib == p2.lib &&
		p1.category == p2.category &&
		p1.feature == p2.feature &&
		p1.create == p2.create &&
		p1.destroy == p2.destroy
	);
}

/** @short Base class for font plug-ins..
  * @ingroup plugin
  *
  * The destructor should clean up all created font renderers.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UFontPlugin : public UPluginBase {
	UFO_DECLARE_DYNAMIC_CLASS(UFontPlugin)
public:
	/** Creates a new font renderer. The returned font renderer does not
	  * have to match the desired font info.
	  */
	virtual UFontRenderer * createFontRenderer(const UFontInfo & fontInfo) = 0;
	/** Returns a font info which describes what would have been
	  * returned by createFontRenderer.
	  */
	virtual UFontInfo queryFont(const UFontInfo & fontInfo) = 0;
	virtual std::vector<UFontInfo> listFonts(const UFontInfo & fontInfo) = 0;
	virtual std::vector<UFontInfo> listFonts() = 0;
};

/** @short Base class for look and feel plug-ins.
  * @ingroup plugin
  *
  * The destructor should clean up all created 'look and feel's.
  * @author Johannes Schmidt
  */
class UFO_EXPORT ULAFPlugin : public UPluginBase {
	UFO_DECLARE_DYNAMIC_CLASS(ULAFPlugin)
public:
	virtual ULookAndFeel * createLookAndFeel() = 0;
};

/** @short Base class for image io method plugins.
  * @ingroup plugin
  *
  * There is generally not much to do for image io plug ins.
  * Registering and deregistering at UImageIO is one point.
  * @author Johannes Schmidt
  */
class UFO_EXPORT UImageIOPlugin : public UPluginBase {
	UFO_DECLARE_DYNAMIC_CLASS(UImageIOPlugin)
public:
};

/** @short Base class for video driver plugins.
  * @ingroup plugin
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UVideoPlugin : public UPluginBase {
	UFO_DECLARE_DYNAMIC_CLASS(UVideoPlugin)
public:
	virtual bool isAvailable() = 0;
	virtual UVideoDriver * createVideoDriver() = 0;
};

} // namespace ufo

#endif // UPLUGIN_HPP
