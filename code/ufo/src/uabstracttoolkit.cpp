/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/uabstracttoolkit.cpp
    begin             : Mon Feb 10 2003
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

#include "ufo/uabstracttoolkit.hpp"

#include "ufo/uversioninfo.hpp"

#include "ufo/util/uproperties.hpp"

//#include "ufo/ui/ulookandfeel.hpp"
#include "ufo/font/ufontrenderer.hpp"


#include "ufo/usharedlib.hpp"

#include "ufo/util/ufilearchive.hpp"

// pre register font plugins
#include "ufo/font/utexturefont.hpp"

// pre register look and feel plugins
//#include "ufo/ui/basic/ubasiclookandfeel.hpp"
//#include "ufo/ui/uthemelookandfeel.hpp"

// OpenGL specific
#include "ufo/gl/ugl_texturefontrenderer.hpp"
#include "ufo/gl/ugl_builtinfontrenderer.hpp"

#ifdef UFO_USE_SDL
#include "ufo/ux/uxsdldriver.hpp"
#endif
#ifdef UFO_USE_GLX
#include "ufo/ux/uxglxdriver.hpp"
#endif
#ifdef UFO_USE_WGL
#include "ufo/ux/uxwgldriver.hpp"
#endif

#include "ufo/uvideodriver.hpp"

#include <climits>
#include <cstring>

namespace ufo {
bool operator<(const UPluginInfo & p1, const UPluginInfo & p2) {
	return ((p1.category < p2.category) || (p1.feature < p2.feature));
}
}

using namespace ufo;

// FIXME !
// this should be in a file utoolkit.cpp
UToolkit * UToolkit::m_instance = NULL;


UFO_IMPLEMENT_ABSTRACT_CLASS(UAbstractToolkit, UObject)


UAbstractToolkit::UAbstractToolkit(UProperties * prop)
	: m_properties(new UProperties())
{
	if (prop) {
		m_properties->put("user_name", prop->get("user_name"));
		m_properties->put("real_name", prop->get("real_name"));

		m_properties->put("home_dir", prop->get("home_dir"));
		m_properties->put("tmp_dir", prop->get("tmp_dir"));

		m_properties->put("prg_name", prop->get("prg_name"));
		m_properties->put("file_name", prop->get("file_name"));
	}
	// try GLib code to fill missing values
	initMissing();

	// register self as new toolkit instance
	setToolkit(this);

	initUFO();
	initPlugins();
}

UAbstractToolkit::~UAbstractToolkit() {
	deinitUFO();
	if (this == getToolkit()) {
		setToolkit(NULL);
	}
}

UVersionInfo
UAbstractToolkit::getVersionInfo() const {
	UVersionInfo ret = {UFO_MAJOR_VERSION, UFO_MINOR_VERSION, UFO_MICRO_VERSION};
	return ret;
}

std::string
UAbstractToolkit::getUserName() const {
	return m_properties->get("user_name");
}


std::string
UAbstractToolkit::getRealName() const {
	return m_properties->get("real_name");
}

std::string
UAbstractToolkit::getHomeDir() const {
	return m_properties->get("home_dir");
}

std::string
UAbstractToolkit::getTmpDir() const {
	return m_properties->get("tmp_dir");
}


std::string
UAbstractToolkit::getPrgName() const {
	return m_properties->get("prg_name");
}

void
UAbstractToolkit::setPrgName(const std::string & prgName) {
	m_properties->put("prg_name", prgName);
}


std::string
UAbstractToolkit::getFontDir() const {
	return m_properties->get("font_dir");
}

std::string
UAbstractToolkit::getProperty(const std::string & keyA) const {
	return m_properties->get(keyA);
}

void
UAbstractToolkit::putProperty(const std::string & keyA, const std::string & valueA) {
	m_properties->put(keyA, valueA);
}


//
// plugin methods
//

void
UAbstractToolkit::loadPlugin(const std::string & fileName) {
	// FIXME
	UPluginInfo plugin;
	plugin.lib = new USharedLib(fileName);

	const char ** category  = (const char**)(*plugin.lib)["ufo_plugin_category"];
	const char ** feature = (const char**)(*plugin.lib)["ufo_plugin_feature"];

	plugin_create_t create  = (plugin_create_t)(*plugin.lib)["ufo_plugin_create"];
	plugin_destroy_t destroy = (plugin_destroy_t)(*plugin.lib)["ufo_plugin_create"];


	if(!category || !feature || !create || !destroy) {
		delete plugin.lib;
		uError() << "Couldn't load plugin " << fileName << "\n";
		return;
	}

	plugin.category = *category;
	plugin.feature  = *feature;
	plugin.create   = create;
	plugin.destroy  = destroy;

	loadPlugin(plugin);
}

void
UAbstractToolkit::loadPlugin(UPluginInfo plugin) {
	// FIXME
	UPluginBase * pluginBase = plugin.create();
	if (plugin.category == "font") {
		if (UFontPlugin * fontPlugin = dynamic_cast<UFontPlugin*>(pluginBase)) {
			m_fontPlugins.push_back(std::make_pair(plugin, fontPlugin));
		}
	}

	if (plugin.category == "video_driver") {
		if (UVideoPlugin * videoPlugin =
				dynamic_cast<UVideoPlugin*>(pluginBase)) {
			m_videoPlugins.push_back(std::make_pair(plugin, videoPlugin));
		}
	}
}


void
UAbstractToolkit::unloadPlugin(const std::string & pluginName) {
	// FIXME
	std::vector<UPluginInfo> plugins = getPluginInfos();

	for (std::vector<UPluginInfo>::const_iterator iter = plugins.begin();
			iter != plugins.end();
			++iter) {
		if ((*iter).lib) {
			if ((*iter).lib->getFileName() == pluginName) {
				unloadPlugin(*iter);
				break;
			}
		}
	}
}

void
UAbstractToolkit::unloadPlugin(UPluginInfo plugin) {
	// FIXME: only checks for video and font plugins
	UPluginBase * pluginImpl = NULL;

	for (FontPluginCache::iterator font_iter = m_fontPlugins.begin();
			font_iter != m_fontPlugins.end();
			++font_iter) {
		if ((*font_iter).first == plugin) {
			pluginImpl = (*font_iter).second;
			m_fontPlugins.erase(font_iter);
			break;
		}
	}
	if (!pluginImpl)
	for (VideoPluginCache::iterator video_iter = m_videoPlugins.begin();
			video_iter != m_videoPlugins.end();
			++video_iter) {
		if ((*video_iter).first == plugin) {
			pluginImpl = (*video_iter).second;
			m_videoPlugins.erase(video_iter);
			break;
		}
	}

	if (pluginImpl != NULL) {
		plugin.destroy(pluginImpl);
		// Note: pluginImpl is now deleted
		if (plugin.lib != NULL) {
			plugin.lib->unload();
			delete (plugin.lib);
		}
	}
}

std::vector<UPluginInfo>
UAbstractToolkit::getPluginInfos() const {
	std::vector<UPluginInfo> ret;

	for (FontPluginCache::const_iterator font_iter = m_fontPlugins.begin();
			font_iter != m_fontPlugins.end();
			++font_iter) {
		ret.push_back((*font_iter).first);
	}
	for (VideoPluginCache::const_iterator video_iter = m_videoPlugins.begin();
			video_iter != m_videoPlugins.end();
			++video_iter) {
		ret.push_back((*video_iter).first);
	}
	return ret;
}

UVideoDriver *
UAbstractToolkit::createVideoDriver() {
	std::string type = getProperty("video_driver");
	UVideoDriver * ret = NULL;

	VideoPluginCache::iterator iter = m_videoPlugins.begin();
	for (; iter != m_videoPlugins.end(); ++iter) {
		if (type == "" || (*iter).first.feature == type) {
			if (((*iter).second)->isAvailable()) {
				ret = ((*iter).second)->createVideoDriver();
				/*
				if (!((*iter).second)->isInitialized()) {
					((*iter).second)->initVideoDriver();
				}
				ret = ((*iter).second)->createVideoDevice();*/
				if (ret != NULL) {
					break;
				}
			}
		}
	}
	if (ret) {
		return ret;
	} else {
		//throw
		std::cerr << "No available video driver\n";
		return NULL;
		//exit(-1);
	}
}

ULookAndFeel *
UAbstractToolkit::createLookAndFeel() {
	/*//return new UThemeLookAndFeel();

	std::string type = getProperty("look_and_feel");

	LAFPluginCache::iterator iter = m_lafPlugins.begin();
	for (; iter != m_lafPlugins.end(); ++iter) {
		if ((*iter).first.feature == type) {
			return ((*iter).second)->createLookAndFeel();
		}
	}
	// fall back to basic look and feel
	return new UBasicLookAndFeel();*/
	return NULL;
}
#include "ufo/ui/ustylemanager.hpp"
UStyleManager *
UAbstractToolkit::getStyleManager() {
	static UStyleManager * manager = NULL;
	if (!manager) {
		manager = new UStyleManager();
	}
	return manager;
}

UFontRenderer *
UAbstractToolkit::createFontRenderer(const UFontInfo & fontInfo) {
	// override it with given renderer
	//std::string type = getProperty("font");
	UFontRenderer * ret = NULL;

	std::string renderer = getProperty("font");
	UFontInfo info = privateQueryFont(fontInfo, &renderer);

	FontPluginCache::iterator iter = m_fontPlugins.begin();
	for (; iter != m_fontPlugins.end(); ++iter) {
		if ((*iter).first.feature == renderer) {
			ret = ((*iter).second)->createFontRenderer(info);
			break;
		}
	}
	if (ret != NULL) {
		return ret;
	}
	// fall back to texture font renderer
	//return new UTextureFontRenderer(fontInfo);
	return new UGL_BuiltinFontRenderer(fontInfo);
}


UFontInfo
UAbstractToolkit::queryFont(const UFontInfo & fontInfo) {
	return privateQueryFont(fontInfo, NULL);
	/*
	std::string type = getProperty("font");

	FontPluginCache::iterator iter = m_fontPlugins.begin();
	for (; iter != m_fontPlugins.end(); ++iter) {
		if (type != "" && (*iter).first.feature == type) {
			return ((*iter).second)->queryFont(fontInfo);
		} else {
			UFontInfo info = ((*iter).second)->queryFont(fontInfo);
			if (info != UFontInfo()) {
				return info;
			}
		}
	}
	// FIXME
	return UFontInfo();
	*/
}

std::vector<UFontInfo>
UAbstractToolkit::listFonts(const UFontInfo & fontInfo) {
	std::string type = getProperty("font");

	FontPluginCache::iterator iter = m_fontPlugins.begin();
	for (; iter != m_fontPlugins.end(); ++iter) {
		if ((*iter).first.feature == type) {
			return ((*iter).second)->listFonts(fontInfo);
		}
	}
	// FIXME
	std::vector<UFontInfo> ret;
	return ret;
}

std::vector<UFontInfo>
UAbstractToolkit::listFonts() {
	std::string type = getProperty("font");

	FontPluginCache::iterator iter = m_fontPlugins.begin();
	for (; iter != m_fontPlugins.end(); ++iter) {
		if ((*iter).first.feature == type) {
			return ((*iter).second)->listFonts();
		}
	}
	// FIXME
	std::vector<UFontInfo> ret;
	return ret;
}

//
// protected
//

#include "ufo/image/uimageio.hpp"
//#include "ufo/util/ufilearchive.hpp"
#include "ufo/util/ucolor.hpp"
#include "ufo/font/ufont.hpp"
void
UAbstractToolkit::initUFO() {
	UImageIO::init();
	//UFileArchive::init();
	//UColor::init();
}
void
UAbstractToolkit::deinitUFO() {
	UFont::clearCache();
}

void
UAbstractToolkit::initPlugins() {
	// font plugins
	UPluginInfo texture;
	texture.lib = NULL;
	texture.category = "font";
	texture.feature = "texture_font";
	texture.create = &UTextureFontRenderer::createPlugin;
	texture.destroy = &UTextureFontRenderer::destroyPlugin;
	loadPlugin(texture);

	UPluginInfo gl_texture;
	gl_texture.lib = NULL;
	gl_texture.category = "font";
	gl_texture.feature = "gl_texture_font";
	gl_texture.create = &UGL_TextureFontRenderer::createPlugin;
	gl_texture.destroy = &UGL_TextureFontRenderer::destroyPlugin;
	loadPlugin(gl_texture);

	UPluginInfo builtin;
	builtin.lib = NULL;
	builtin.category = "font";
	builtin.feature = "builtin_font";
	builtin.create = &UGL_BuiltinFontRenderer::createPlugin;
	builtin.destroy = &UGL_BuiltinFontRenderer::destroyPlugin;
	loadPlugin(builtin);
/*
	// look and feel plugins
	UPluginInfo basic;
	basic.lib = NULL;
	basic.category = "look_and_feel";
	basic.feature = "basic";
	basic.create = &UBasicLookAndFeel::createPlugin;
	basic.destroy = &UBasicLookAndFeel::destroyPlugin;
	loadPlugin(basic);

	UPluginInfo theme;
	theme.lib = NULL;
	theme.category = "look_and_feel";
	theme.feature = "theme";
	theme.create = &UThemeLookAndFeel::createPlugin;
	theme.destroy = &UThemeLookAndFeel::destroyPlugin;
	loadPlugin(theme);
*/
#ifdef UFO_USE_GLX
	UPluginInfo glxDriver;
	glxDriver.lib = NULL;
	glxDriver.category = "video_driver";
	glxDriver.feature = "GLX";
	glxDriver.create = &UXGLXDriver::createPlugin;
	glxDriver.destroy = &UXGLXDriver::destroyPlugin;
	loadPlugin(glxDriver);
#endif
#ifdef UFO_USE_WGL
	UPluginInfo wglDriver;
	wglDriver.lib = NULL;
	wglDriver.category = "video_driver";
	wglDriver.feature = "WGL";
	wglDriver.create = &UXWGLDriver::createPlugin;
	wglDriver.destroy = &UXWGLDriver::destroyPlugin;
	loadPlugin(wglDriver);
#endif
	// backend plugin
#ifdef UFO_USE_SDL
	UPluginInfo sdlDriver;
	sdlDriver.lib = NULL;
	sdlDriver.category = "video_driver";
	sdlDriver.feature = "SDL";
	sdlDriver.create = &UXSDLDriver::createPlugin;
	sdlDriver.destroy = &UXSDLDriver::destroyPlugin;
	loadPlugin(sdlDriver);
#endif
}


//
// private
//

#ifdef HAVE_PWD_H
#include <pwd.h>
#include <unistd.h>
#endif

#ifdef UFO_OS_WIN32 // G_PLATFORM_WIN32?
#  include <lmcons.h> // For UNLEN
#  include <ctype.h>
#endif // UFO_OS_WIN32

//
// The Unix part is taken from public domain project relocbin
// at autopackage.org
static std::string
ufo_getModuleFileName() {
#ifdef UFO_OS_WIN32
	unsigned int buflen;
	TCHAR buffer[MAX_PATH];
	buflen = GetModuleFileName(NULL, buffer, MAX_PATH);
	return std::string(buffer, buflen);
#else
	#define SIZE PATH_MAX + 100
	FILE *f;
	size_t address_string_len;
	char *address_string, line[SIZE], *found;

	const char * symbol = "";

	f = fopen ("/proc/self/maps", "r");
	if (f == NULL)
		return "";

	address_string_len = 4;
	address_string = (char *) malloc (address_string_len);
	found = (char *) NULL;

	while (!feof (f)) {
		char *start_addr, *end_addr, *end_addr_end, *file;
		void *start_addr_p, *end_addr_p;
		size_t len;

		if (fgets (line, SIZE, f) == NULL)
			break;

		/* Sanity check. */
		if (strstr (line, " r-xp ") == NULL || strchr (line, '/') == NULL)
			continue;

		/* Parse line. */
		start_addr = line;
		end_addr = strchr (line, '-');
		file = strchr (line, '/');

		/* More sanity check. */
		if (!(file > end_addr && end_addr != NULL && end_addr[0] == '-'))
			continue;

		end_addr[0] = '\0';
		end_addr++;
		end_addr_end = strchr (end_addr, ' ');
		if (end_addr_end == NULL)
			continue;

		end_addr_end[0] = '\0';
		len = strlen (file);
		if (len == 0)
			continue;
		if (file[len - 1] == '\n')
			file[len - 1] = '\0';

		/* Get rid of "(deleted)" from the filename. */
		len = strlen (file);
		if (len > 10 && strcmp (file + len - 10, " (deleted)") == 0)
			file[len - 10] = '\0';

		/* I don't know whether this can happen but better safe than sorry. */
		len = strlen (start_addr);
		if (len != strlen (end_addr))
			continue;


		/* Transform the addresses into a string in the form of 0xdeadbeef,
		 * then transform that into a pointer. */
		if (address_string_len < len + 3) {
			address_string_len = len + 3;
			address_string = (char *) realloc (address_string, address_string_len);
		}

		memcpy (address_string, "0x", 2);
		memcpy (address_string + 2, start_addr, len);
		address_string[2 + len] = '\0';
		sscanf (address_string, "%p", &start_addr_p);

		memcpy (address_string, "0x", 2);
		memcpy (address_string + 2, end_addr, len);
		address_string[2 + len] = '\0';
		sscanf (address_string, "%p", &end_addr_p);


		if (symbol >= start_addr_p && symbol < end_addr_p) {
			found = file;
			break;
		}
	}

	free (address_string);
	fclose (f);

	if (found == NULL) {
		return "";
	} else {
		return found;
	}
#endif
}

/** Several parts of this code are taken from the GLib 2.2.1 sources,
  * Thanks guys!
  * Glib sources availabe at http://www.gtk.org, released under the GNU LGPL
  */
void
UAbstractToolkit::initMissing() {
	//
	// video driver
	//
	if (getenv("UFO_VIDEO_DRIVER")) {
		// overwrite with env var
		m_properties->put("video_driver", getenv("UFO_VIDEO_DRIVER"));
	}

	//
	// font
	//
	if (getenv("UFO_FONT")) {
		// overwrite with env var
		m_properties->put("font", getenv("UFO_FONT"));
	} else if (m_properties->get("font").empty()) {
		//m_properties->put("font", "gl_texture_font");
		m_properties->put("font", "");
	}

	if (getenv("UFO_FONT_DIR")) {
		// overwrite with env var
		m_properties->put("font_dir", getenv("UFO_FONT_DIR"));
	} else if (m_properties->get("font_dir").empty()) {
		// set cross-platform font dir
		std::string font_dir = UFO_DATADIR;
		font_dir.append("/font");
		m_properties->put("font_dir", font_dir);
	}

	// the modul file name
	std::string file_name;
	if (getenv("UFO_FILE_NAME")) {
		file_name = getenv("UFO_FILE_NAME");
	} else if (m_properties->get("file_name").empty()) {
		file_name = ufo_getModuleFileName();
	}
	m_properties->put("file_name", file_name);

	//
	// data dir
	//
	if (getenv("UFO_DATA_DIR")) {
		// overwrite with env var
		m_properties->put("data_dir", getenv("UFO_DATA_DIR"));
	} else if (m_properties->get("data_dir").empty()) {
		// set cross-platform font dir
		std::string data_dir;
#ifndef UFO_OS_WIN32
		if (!file_name.empty()) {
			data_dir = UFileArchive::dirName(
				UFileArchive::dirName(m_properties->get("file_name"))
			);
			data_dir.append("/share/ufo");
		} else {
			// FIXME: did not find module filename, try configure data dir
			data_dir = UFO_DATADIR;
		}
#else
		if (!file_name.empty()) {
			data_dir = UFileArchive::dirName(m_properties->get("file_name"));
			data_dir.append("/data");
		} else {
			// FIXME: did not find module filename, try configure data dir
			data_dir = UFO_DATADIR;
		}
#endif
		m_properties->put("data_dir", data_dir);
	}


	// temporary variables to check properties
	std::string tmp_dir;
	std::string home_dir;
	std::string user_name;
	std::string real_name;

	//
	// tmp dir
	//

	if (getenv("UFO_TMP_DIR")) {
		tmp_dir = getenv("UFO_TMP_DIR");
	} else {
		tmp_dir = m_properties->get("tmp_dir");
	}

	if (tmp_dir.empty() && getenv("TMPDIR")) {
		tmp_dir = getenv("TMPDIR");
	}
	if (tmp_dir.empty() && getenv("TMP")) {
		tmp_dir = getenv("TMP");
	}
	if (tmp_dir.empty() && getenv("TEMP")) {
		tmp_dir = getenv("TEMP");
	}

	if (tmp_dir.empty()) {
#ifndef UFO_OS_WIN32
		m_properties->put("tmp_dir", "/tmp");
#else // UFO_OS_WIN32
		m_properties->put("tmp_dir", "C:/windows/temp"); // "C:\\"
#endif // !UFO_OS_WIN32
	} else {
		m_properties->put("tmp_dir", tmp_dir);
	}

	//
	// home dir
	//

	home_dir = m_properties->get("home_dir");

#ifdef UFO_OS_WIN32
	/* We check $HOME first for Win32, though it is a last resort for Unix
	 * where we prefer the results of getpwuid().
	 */
	if (home_dir.empty() && getenv("HOME")) {
		home_dir = getenv("HOME");
	}

	if (home_dir.empty() && getenv("USERPROFILE")) {
		/* USERPROFILE is probably the closest equivalent to $HOME? */
		home_dir = getenv("USERPROFILE");
	}

	if (home_dir.empty()) {
		/* At least at some time, HOMEDRIVE and HOMEPATH were used
		 * to point to the home directory, I think. But on Windows
		 * 2000 HOMEDRIVE seems to be equal to SYSTEMDRIVE, and
		 * HOMEPATH is its root "\"?
		 */
		 if (getenv("HOMEDRIVE") != NULL && getenv("HOMEPATH") != NULL) {
			home_dir = getenv("HOMEDRIVE");
			home_dir.append(getenv("HOMEPATH"));
		}
	}
#endif // UFO_OS_WIN32

#ifdef HAVE_PWD_H
	{
	struct passwd *pw = NULL;

	setpwent();
	pw = getpwuid(getuid());
	endpwent();

	if (pw) {
		user_name = pw->pw_name;
		real_name = pw->pw_gecos;
		if (home_dir.empty()) {
			home_dir = pw->pw_dir;
		}
	}
	}

#else /* !HAVE_PWD_H */

#  ifdef UFO_OS_WIN32
	{
	uint32_t len = UNLEN+1;
	char buffer[UNLEN+1];

	if (GetUserName ((LPTSTR) buffer, (LPDWORD) &len)) {
	    user_name = buffer;
	    real_name = buffer;
	}
    }
#  endif /* UFO_OS_WIN32 */

#endif /* !HAVE_PWD_H */

	// last resort
	if (home_dir.empty() && getenv ("HOME")) {
		home_dir = getenv ("HOME");
	}

	//
	// update values
	//

	if (home_dir.empty()) {
		// oops, shouldn't happen
		m_properties->put("home_dir", m_properties->get("tmp_dir"));
	} else {
		m_properties->put("home_dir", home_dir);
	}

	if (user_name.empty()) {
		m_properties->put("user_name", "somebody");
	} else {
		m_properties->put("user_name", user_name);
	}

	if (real_name.empty()) {
		m_properties->put("real_name", "Unknown");
	} else {
		m_properties->put("real_name", real_name);
	}
}

UFontInfo
UAbstractToolkit::privateQueryFont(const UFontInfo & fontInfo, std::string * renderer) {
	std::string type = getProperty("font");
	UFontInfo ret;

	FontPluginCache::iterator iter = m_fontPlugins.begin();
	if (type != "") {
		for (; iter != m_fontPlugins.end(); ++iter) {
			if ((*iter).first.feature == type) {
				ret = ((*iter).second)->queryFont(fontInfo);
				break;
			}
		}

	}
	// if the specified font could only return the default font info,
	// search for better matching renderer
	if (ret == UFontInfo()) {
		for (; iter != m_fontPlugins.end(); ++iter) {
			UFontInfo info = ((*iter).second)->queryFont(fontInfo);
			if (info != UFontInfo()) {
				ret = info;
				if (renderer) {
					*renderer = (*iter).first.feature;
				}
				break;
			}
		}
	}
	return ret;
}
