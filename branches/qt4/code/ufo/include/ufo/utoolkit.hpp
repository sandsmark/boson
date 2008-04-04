/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/utoolkit.hpp
    begin             : Mon Oct 29 2001
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

#ifndef UTOOLKIT_HPP
#define UTOOLKIT_HPP

#include "uobject.hpp"
#include "ucontext.hpp"
#include "uversioninfo.hpp"
#include "uplugin.hpp"

#include "events/ukeysym.hpp"

#include "util/uinsets.hpp"
#include "util/udimension.hpp"

#include <vector>

namespace ufo {

class UContext;
class UPlugin;
class ULookAndFeel;
class UStyleManager;
class UFontRenderer;

/** @short The toolkit encapsulates some important system functions and loads
  *  plugins.
  * @ingroup core
  *
  * An abstract class that represents data shared by all UContext objects
  * and controls the interaction of UContext objects.
  * Furthermore it stores several system properties, can load plugins
  * and encapsulates some important system functions.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UToolkit : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UToolkit)
public:
	/** Returns the registered Toolkit. The auxiliary lib must
	  * ensure that this function does not return NULL.
	  */
	static UToolkit * getToolkit();

	/** This function should not be invoked by client code, but
	  * by the auxiliary lib, which implements this UToolkit.
	  */
	static void setToolkit(UToolkit * toolkitA);

	/**
	  */
	virtual void makeContextCurrent(UContext * contextA) = 0;
	virtual UContext * getCurrentContext() const = 0;

public: // administrative methods
	/** Returns a version info struct with the version numbers of the linked
	  * ufo library.
	  */
	virtual UVersionInfo getVersionInfo() const = 0;

public: // window manager and user informations
	/** @return The screen size in pixels.
	  */
	virtual UDimension getScreenSize() const = 0;
	/** Screen insets are task bar, desktop menu bars and such.
	  * @return The screen insets like task bars.
	  */
	virtual UInsets getScreenInsets() const = 0;
	/** @return The screen depth.
	  */
	virtual int getScreenDepth() const = 0;


	virtual std::string getUserName() const = 0;
	virtual std::string getRealName() const = 0;

	virtual std::string getHomeDir() const = 0;
	virtual std::string getTmpDir() const = 0;

	virtual std::string getPrgName() const = 0;
	virtual void setPrgName(const std::string & prgName) = 0;


public: // Plugin methods
	/** Loads the plugin which is located within the given shared object file.
	  */
	virtual void loadPlugin(const std::string & fileName) = 0;
	/** Manually loads a plugin. */
	virtual void loadPlugin(UPluginInfo plugin) = 0;

	virtual void unloadPlugin(const std::string & pluginName) = 0;
	/** Searches in the plugin cache for a matching plugin and unloads it. */
	virtual void unloadPlugin(UPluginInfo plugin) = 0;

	/** Returns a vector with plugin info structs for all loaded plugins. */
	virtual std::vector<UPluginInfo> getPluginInfos() const = 0;

public: // font and laf methods
	virtual UVideoDriver * createVideoDriver() = 0;
	/** Creates a look and feel using the currently default laf plugin.
	  * The plugin is chosen using the 'look_and_feel' property.
	  * This method is called by UFO backends at UFO context creation .
	  * @see putProperty
	  */
	virtual ULookAndFeel * createLookAndFeel() = 0;
	virtual UStyleManager * getStyleManager() = 0;

	/** Returns a colon separated list of font directories.
	  * This is the search path for font ressource files.
	  */
	virtual std::string getFontDir() const = 0;
	/** Creates a font renderer with the given font familiy,
	  * style and point size.
	  * The toolkit should use the active font plugin.
	  * The font plugin is chosen using the 'font' property.
	  * @see putProperty
	  */
	virtual UFontRenderer * createFontRenderer(const UFontInfo & fontInfo) = 0;
	/** Returns a font info which describes what would have been
	  * returned by createFontRenderer.
	  * This means, if the user requests an
	  * antialiased font, but there exists none, this method returns which
	  * would be returned instead.
	  */
	virtual UFontInfo queryFont(const UFontInfo & fontInfo) = 0;
	/** Lists all available font renderers of the current font plugin
	  * which matches the given font info.
	  */
	virtual std::vector<UFontInfo> listFonts(const UFontInfo & fontInfo) = 0;
	/** Lists all available font renderers of the current font plugin.
	  */
	virtual std::vector<UFontInfo> listFonts() = 0;


public: // some useful and system dependent functions
	/** Waits a specified number of milliseconds before returning.
	  * Delay will wait at least the specified time, but possible longer due
	  * to OS scheduling.
	  *
	  *@param millis
	  *	the minimum delay in milliseconds
	  */
	virtual void sleep(uint32_t millis) = 0;

	/** Returns the time in milliseconds since lib initializing
	  */
	virtual uint32_t getTicks() const = 0;

public: // UFO properties
	/** Returns a UFO property registered at this toolkit.
	  * Properties so far: user_name, real_name, home_dir, tmp_dir, prg_name
	  * look_and_feel
	  */
	virtual std::string getProperty(const std::string & keyA) const = 0;
	/** Sets the property keyA with value valueA.
	  * UFO properties control the behaviour
	  */
	virtual void putProperty(const std::string & keyA, const std::string & valueA) = 0;

protected:  // Protected attributes
	/** the toolkit instance */
	static UToolkit * m_instance;
};


//
// inline implementation
//

inline UToolkit *
UToolkit::getToolkit() {
	return m_instance;
}

inline void
UToolkit::setToolkit(UToolkit * toolkitA) {
	m_instance = toolkitA;
}


} // namespace ufo

#endif // UTOOLKIT_HPP
