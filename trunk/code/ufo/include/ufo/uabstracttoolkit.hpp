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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
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

/**
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
	UVersionInfo getVersionInfo() const;
	
	std::string getUserName() const;
	std::string getRealName() const;

	std::string getHomeDir() const;
	std::string getTmpDir() const;

	std::string getPrgName() const;
	void setPrgName(const std::string & prgName);

	std::string getFontDir() const;

	std::string getProperty(const std::string & keyA) const;
	void putProperty(const std::string & keyA, const std::string & valueA);

public: // Plugin methods
	void loadPlugin(const std::string & fileName);
	void loadPlugin(UPluginInfo plugin);

	void unloadPlugin(const std::string & pluginName);
	void unloadPlugin(UPluginInfo plugin);

	std::vector<UPluginInfo> getPluginInfos() const;

	virtual UVideoDriver * createVideoDriver();
	/** This method is called internally at UFO context creation .*/
	ULookAndFeel * createLookAndFeel();

	UFontRenderer * createFontRenderer(const UFontInfo & fontInfo);
	UFontInfo queryFont(const UFontInfo & fontInfo);
	std::vector<UFontInfo> listFonts(const UFontInfo & fontInfo);
	std::vector<UFontInfo> listFonts();

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

private: // Private attributes
	UProperties * m_properties;
	typedef std::map<UPluginInfo, UFontPlugin*> FontPluginCache;
	typedef std::map<UPluginInfo, ULAFPlugin*> LAFPluginCache;
	typedef std::map<UPluginInfo, UVideoPlugin*> VideoPluginCache;
	FontPluginCache m_fontPlugins;
	LAFPluginCache m_lafPlugins;
	VideoPluginCache m_videoPlugins;
};

} // namespace ufo

#endif // UABSTRACTTOOLKIT_HPP
