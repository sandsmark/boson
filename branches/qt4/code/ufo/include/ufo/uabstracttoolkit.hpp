/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uabstracttoolkit.hpp
    begin             : Sat Jan 18 2003
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

#ifndef UABSTRACTTOOLKIT_HPP
#define UABSTRACTTOOLKIT_HPP

#include "utoolkit.hpp"

#include <map>

namespace ufo {

class UProperties;

// no public API
// used for map sorting
bool operator<(const UPluginInfo & p1, const UPluginInfo & p2);

/** @short Implements some platform independent methods of UToolkit.
  *  Provided for convenience.
  * @ingroup native
  * @ingroup internal
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UAbstractToolkit : public UToolkit {
	UFO_DECLARE_ABSTRACT_CLASS(UAbstractToolkit)
public:

	/** @param prop Use this properties object to initialize UFO properties.
	  */
	UAbstractToolkit(UProperties * prop);
	virtual ~UAbstractToolkit();

public: // Implements UToolkit
	virtual UVersionInfo getVersionInfo() const;

	virtual std::string getUserName() const;
	virtual std::string getRealName() const;

	virtual std::string getHomeDir() const;
	virtual std::string getTmpDir() const;

	virtual std::string getPrgName() const;
	virtual void setPrgName(const std::string & prgName);

	virtual std::string getFontDir() const;

	virtual std::string getProperty(const std::string & keyA) const;
	virtual void putProperty(const std::string & keyA, const std::string & valueA);

public: // Plugin methods
	virtual void loadPlugin(const std::string & fileName);
	virtual void loadPlugin(UPluginInfo plugin);

	virtual void unloadPlugin(const std::string & pluginName);
	virtual void unloadPlugin(UPluginInfo plugin);

	virtual std::vector<UPluginInfo> getPluginInfos() const;

	virtual UVideoDriver * createVideoDriver();
	/** This method is called internally at UFO context creation .*/
	virtual ULookAndFeel * createLookAndFeel();
	virtual UStyleManager * getStyleManager();

	virtual UFontRenderer * createFontRenderer(const UFontInfo & fontInfo);
	virtual UFontInfo queryFont(const UFontInfo & fontInfo);
	virtual std::vector<UFontInfo> listFonts(const UFontInfo & fontInfo);
	virtual std::vector<UFontInfo> listFonts();

protected: // Protected methods
	/** Inits UFO. */
	virtual void initUFO();
	virtual void deinitUFO();
	/** Inits static UFO plugins. */
	virtual void initPlugins();

private: // Private methods
	/** Inits missing properties.
	  * At first, the toolkit tries to init all properties with sane
	  * guessed values.
	  * Next, the toolkit reads environment variables. Env vars are built
	  * by capitalizing the name and prefix it with 'UFO_',
	  * e.g. UFO_LOOK_AND_FEEL
	  */
	void initMissing();
	UFontInfo privateQueryFont(const UFontInfo & fontInfo, std::string * renderer);

private: // Private attributes
	UProperties * m_properties;
	typedef std::list<std::pair<UPluginInfo, UFontPlugin*> > FontPluginCache;
	typedef std::list<std::pair<UPluginInfo, UVideoPlugin*> > VideoPluginCache;
	FontPluginCache m_fontPlugins;
	VideoPluginCache m_videoPlugins;
};

} // namespace ufo

#endif // UABSTRACTTOOLKIT_HPP
