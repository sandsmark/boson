/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef BOINFO_H
#define BOINFO_H

#include <qstring.h>

class QStringList;
class QWidget;
class QVariant;
template<class T1, class T2> class QMap;
class QDomElement;

class BoInfoPrivate;

#define MAKE_VERSION(a,b,c) ( ((a) << 16) | ((b) << 8) | (c) )

/**
 * There has always been one problem in boson: speed. But not because boson is
 * so slow (yes it is - but less than 1 FPS is <em>not</em> boson's fault) but
 * rather because many people out there have a broken installation.
 *
 * After all these broken (or at least strange) installations make it very
 * difficult to help them ("have you installed your diver correctly?" - "yes i
 * have, i am sure" - "can you check that lib foo is in dir bar" - "oops i
 * missed that"). So I decided that I need information about these system for
 * various reasons. One of them are those people above, so that we can give them
 * useful help. It is not enogh that they tell us which library they use - it is
 * important for us to know which they <em>actually</em> use. Another reason is
 * the fact that it makes profiling easier if you know which graphics card,
 * which version, which cpu and so on is involved.
 *
 * Another problem is that we don't want to do certain things on slow computers.
 * So we need access to e.g. the @ref cpuSpeed().
 *
 * KDE provides some nice KControl modules which provide a lot or even most of
 * the data that we need. Unfortunately it is of no use at all in KControl and
 * these data are not provided in a library. So I wrote our own library...
 * We collect data that is/might be useful for us only, so don't expect data
 * about your printer here ;-) But actually if I found some maybe-useful data
 * then I included them here - even if we will probably never actually use them.
 * Rather more than less.
 *
 * You should use @ref BoInfoDialog to inspect the data further. BoInfo provides
 * functions to @ref load and @ref save data into a stream (note: you should ask
 * the user for permission before storing all these data! there is somthing that
 * is called privacy...) and @ref loadFromFile and @ref saveToFile, which are
 * more convenient to use.
 *
 * Getting the data for this class is really easy: call @ref update and you are
 * done. Everything is stored internally from now on and you can access it. Note
 * that @ref update will need a @ref QWidget , since most X data need at
 * least a widget. Also note that to get useful data for OpenGL/GLU/GLX you need
 * to have a current GLX context (use @ref BosonGLWidget::makeCurrent).
 *
 * You can use @ref getBool, @ref getString, ... to get the data, but most
 * entries have their own functions, such as @ref kdeVersion. You should prefer
 * them, since they often do some error checking and so.
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Central class that collects data about the system
 */
class BoInfo
{
public:
	BoInfo();
	~BoInfo();

	enum InfoEntries {
		// boson specific
		BosonVersion = 0,
		BosonVersionString = 1,

		// KDE specific
		KDEVersion = 2,
		KDEVersionString = 3,

		// Qt specific
		QtVersion = 4,
		QtVersionString = 5,

		// OpenGL specific
		HaveOpenGLData = 10,
		OpenGLVersionString = 20,
		OpenGLVendorString = 21,
		OpenGLRendererString = 22,
		OpenGLExtensionsString = 23,
		OpenGLValuesString = 24,
		OpenGLVersion = 25,
		GLUVersionString = 30,
		GLUExtensionsString = 31,
		GLXClientVersionString = 40,
		GLXClientVendorString = 41,
		GLXClientExtensionsString = 42,
		GLXVersionMajor = 43,
		GLXVersionMinor = 44,
		GLXServerVersionString = 45,
		GLXServerVendorString = 46,
		GLXServerExtensionsString = 47,
		IsDirect = 50,

		// X specific
		HaveXData = 100,
		XDisplayName = 101, // probably useless for us
		XProtocolVersion = 102, // probably useless for us
		XProtocolRevision = 103, // probably useless for us
		XVendorString = 104, // probably always XFree86 for us
		XVendorReleaseNumber = 105, // *very* important!
		XExtensionsString = 106,
		XDefaultScreen = 107,
		XScreenCount = 108,
		XScreen = 109, // widget->x11Screen()
		XScreenWidth = 110, // in pixels
		XScreenHeight = 111,
		XScreenWidthMM = 112, // as width, but in mm
		XScreenHeightMM = 113,

		// OS specific
		CPUString = 300,
		MTRRString = 301,
		OSTypeString = 302, // on linux this is the content of /prov/sys/kernel/ostype
		OSVersionString = 303, // on linux this is the content of /prov/sys/kernel/osrelease
		KernelModuleNVidiaString = 310,
		KernelModuleTDFXString = 311,
		// AB: leave some space for more kernel modules here

		// libs
		HaveXExtLibGLX_a = 500,
		HaveXExtLibGLCore_a = 501,
		HaveXExtLibGLX_so = 503,
		// TODO: also check for libGLU and so on while we are on it!
		HaveLibGL_so = 504,
		LibGL_so_DependsOnLibGLCore = 505,
		HaveLibGLCore_so_1 = 506,
		HaveProprietaryNVidiaXDriver = 550,
		// AB: leave some space for further proprietary drivers

		// devices
		DevNVidiaCTL = 600,
		DevNVidia0 = 601,
		DevNVidia1 = 602,
		DevNVidia2 = 603,
		DevNVidia3 = 604,
		// AB: leave a lot of space for more devices

		// an InfoEntries entry specifies the runtime value. The same
		// entry + CompileOffset specifies the compile time value. note
		// that it is possible for both values to be left out
		CompileOffset = 50000
	};

	enum CharacterDevice {
		CharSuccess = 0x00,
		CharDoesNotExist = 0x01,
		CharCantRead = 0x02,
		CharCantWrite = 0x04,
		CharCantStat = 0x08,
		CharNoCharDevice = 0x16
	};

	/**
	 * @return The global BoInfo object. Will return NULL if @ref initBoInfo
	 * was not yet called.
	 **/
	static BoInfo* boInfo();

	void update(QWidget* w = 0);

	bool loadFromFile(const QString& file);
	bool saveToFile(const QString& file) const;
	bool load(QDomElement& root);
	bool save(QDomElement& root) const;
	bool load(QDataStream& stream);
	bool save(QDataStream& stream) const;
	void copyFrom(const BoInfo&);

	void debug() const;

	/**
	 * @return A i18n'ed name for @p key. If there is no name string for @p
	 * key available i18n("(Unknown)") or something similar will be returned
	 **/
	static QString keyToName(int key);

	/**
	 * @return A the value for @p key in a string.
	 **/
	QString valueToString(int key) const;

	/**
	 * Most commonly used in boson to check which version we are running at.
	 * (AB: I don't think we actually need the major/minor/micro values
	 * anywhere - if we do then feel free to add functions below).
	 *
	 * This will simply return the installed boson version. it is encoded as
	 * major.minor.micro, where micro is usually 0 (e.g. 0.7 is actually
	 * 0.7.0) in releases. Only bugfix releases (such as 0.7.1) have a micro
	 * version != 0.
	 *
	 * All CVS development versions use a very high micro version. E.g. HEAD
	 * after 0.7.0 is 0.7.90
	 * --> so you can still compare versions of release and development
	 *  version. 0.8 will always be greater than a development version
	 *  before 0.8
	 * @return The installed boson version
	 **/
	unsigned int bosonVersion() const
	{
		return getUInt(BosonVersion);
	}

	/**
	 * @return A string displaying the current version. Don't use this for
	 * comparing versions!
	 **/
	QString bosonVersionString() const
	{
		return getString(BosonVersionString);
	}

	/**
	 * @param runtime Will query the running kde version if TRUE, otherwise
	 * the version boson was compiled with.
	 * @return The kde version.
	 **/
	unsigned int kdeVersion(bool runtime = true) const
	{
		return getUInt(KDEVersion + (runtime ? 0 : CompileOffset));
	}

	/**
	 * @param runtime Will query the running kde version if TRUE, otherwise
	 * the version boson was compiled with.
	 * @return A string describing the kde version (@ref KDE::versionString
	 * as in kdeversion.h)
	 **/
	QString kdeVersionString(bool runtime = true) const
	{
		return getString(KDEVersionString + (runtime ? 0 : CompileOffset));
	}

	/**
	 * @return The QT version boson was compiled with.
	 **/
	unsigned int qtVersion() const
	{
		return getUInt(CompileOffset + QtVersion);
	}

	/**
	 * @return A string containing the Qt version.
	 **/
	QString qtVersionString(bool runtime = true) const
	{
		return getString(QtVersionString + (runtime ? 0 : CompileOffset));
	}

	/**
	 * @return TRUE if this class has data about OpenGL/GLU/GLX or FALSE if
	 * these data have not yet been initialized. To get data about them you
	 * have to call @ref update after making a GLX context current (see @ref
	 * BosonGLWidget::makeCurrent)
	 **/
	bool haveOpenGLData() const
	{
		return getBool(HaveOpenGLData);
	}

	/**
	 * Note that we don't have the OpenGL version available boson was
	 * compiled with (at least not yet)!
	 * @return A string containing the OpenGL version (with both, vendor
	 * version such as mesa 3.2 and OpenGL version such as 1.1).
	 **/
	QString openGLVersionString() const
	{
		return getString(OpenGLVersionString);
	}

	/**
	 * @return Runtime OpenGL version which is encoded using MAKE_VERSION macro.
	 **/
	unsigned int openGLVersion() const;

	/**
	 * @return Whether runtime OpenGL version is equal or greater than given
	 *  version.
	 * vesion must be encoded using MAKE_VERSION macro.
	 **/
	bool hasOpenGLVersion(unsigned int version) const;

	/**
	 * @return Whether runtime OpenGL version is equal or greater than given
	 *  version.
	 **/
	bool hasOpenGLVersion(unsigned int major, unsigned int minor, unsigned int release) const
	{
		return hasOpenGLVersion(MAKE_VERSION(major, minor, release));
	}

	/**
	 * @return A string containing the vendor of the installed OpenGL
	 * version (runtime). Usually mesa or nvidia.
	 **/
	QString openGLVendorString() const
	{
		return getString(OpenGLVendorString);
	}

	QString openGLRendererString() const
	{
		return getString(OpenGLRendererString);
	}

	/**
	 * @return A list of available OpenGL extensions. This <em>could</em> be
	 * used to do certain tasks only if the extension is available. But if
	 * we actually want to do that we should cache the list somewhere.
	 **/
	QStringList openGLExtensions() const;

	/**
	 * @return A list of all implementation dependant OpenGL values (those
	 * that can be retrieved using glGetIntegerv() and friends). An example
	 * is GL_MAX_TEXTURE_SIZE.
	 **/
	QStringList openGLValues() const;

	/**
	 * @return A string describing the GLU version.
	 **/
	QString gluVersionString() const
	{
		return getString(GLUVersionString);
	}
	QStringList gluExtensions() const;

	QString glXClientVersionString() const
	{
		return getString(GLXClientVersionString);
	}
	QString glXClientVendorString() const
	{
		return getString(GLXClientVendorString);
	}
	QStringList glXClientExtensions() const;
	void glXVersion(int* major, int* minor) const;
	QString glXServerVersionString() const
	{
		return getString(GLXServerVersionString);
	}
	QString glXServerVendorString() const
	{
		return getString(GLXServerVendorString);
	}
	QStringList glXServerExtensions() const;

	/**
	 * @return TRUE if the rendering context is direct (should be so, as
	 * non-direct is slow)
	 **/
	bool isDirect() const
	{
		return getBool(IsDirect);
	}

	/**
	 * @return TRUE if this object has information about X, otherwise FALSE
	 * (e.g. when you call @ref update with a NULL widget)
	 **/
	bool haveXData() const
	{
		return getBool(BoInfo::HaveXData);
	}

	/**
	 * @return The X display name, such as :0.0 by default.
	 **/
	QString xDisplayName() const
	{
		return getString(BoInfo::XDisplayName);
	}

	/**
	 * @return The version of the X protocol. Probably we don't need this.
	 **/
	int xProtocolVersion() const
	{
		return getInt(BoInfo::XProtocolVersion);
	}

	/**
	 * @return The revision of the X protocol. Probably we don't need this.
	 **/
	int xProtocolRevision() const
	{
		return getInt(BoInfo::XProtocolRevision);
	}

	/**
	 * @return Probably always a string about XFree86 for us.
	 **/
	QString xVendorString() const
	{
		return getString(BoInfo::XVendorString);
	}

	/**
	 * @return The version number of the X installation. See xdpyinfo.c on
	 * how to make this value useful (very confusing way...) WARNING: use
	 * that way of calculating the actual number for XFree86 only!
	 **/
	int xVendorReleaseNumber() const
	{
		return getInt(BoInfo::XVendorReleaseNumber);
	}

	/**
	 * @return The default screen. I am not fully sure how much this may be
	 * usable for us. See also @ref xScreen.
	 **/
	int xDefaultScreen() const
	{
		return getInt(BoInfo::XDefaultScreen);
	}

	/**
	 * @return The number of available screens. Usually 1. See also @ref
	 * xScreen
	 **/
	int xScreenCount() const
	{
		return getInt(BoInfo::XScreenCount);
	}

	/**
	 * @return @ref QWidget::x11Screen. This will be usually 0, except if
	 * you have more than one screen. This might be useful for us, since it
	 * may be possible that a user runs boson on one of his screens (if he
	 * owns several using xinerama) that is not 3d accellerated.
	 **/
	int xScreen() const
	{
		return getInt(BoInfo::XScreen);
	}

	/**
	 * @return 1024 on my system (my screen uses 1024x768). I.e. the width
	 * in pixels of the screen the info is for.
	 **/
	int xScreenWidth() const
	{
		return getInt(BoInfo::XScreenWidth);
	}

	/**
	 * @return 768 on my system (my screen uses 1024x768). I.e. the height
	 * in pixels of the screen the info is for.
	 **/
	int xScreenHeight() const
	{
		return getInt(BoInfo::XScreenHeight);
	}

	/**
	 * Just like @ref xScreenWidth, but in mm. Hardly usable for us.
	 **/
	int xScreenWidthMM() const
	{
		return getInt(BoInfo::XScreenWidthMM);
	}

	/**
	 * Just like @ref xScreenHeight, but in mm. Hardly usable for us.
	 **/
	int xScreenHeightMM() const
	{
		return getInt(BoInfo::XScreenHeightMM);
	}

	/**
	 * @return If the operating system was recognized (working for Linux only
	 * currently) this returns the a string describing the OS. This is
	 * the content of /proc/sys/kernel/ostype on linux (with newlines
	 * removed). Otherwise returns QString::null.
	 **/
	QString osType() const;

	/**
	 * @return If the operating system was recognized (working for Linux only
	 * currently) this returns the a string describing the OS version. This is
	 * the content of /proc/sys/kernel/ostversion on linux (with newlines
	 * removed). Otherwise returns QString::null.
	 **/
	QString osVersion() const;

	/**
	 * @return If the OS was recognized AND the kernel module is loaded this
	 * returns the string that made us believe the kernel module was loaded.
	 * This is a line from /proc/modules on linux.
	 **/
	QString osKernelModuleTDFXString() const
	{
		return getString(BoInfo::KernelModuleTDFXString);
	}

	/**
	 * @return If the OS was recognized AND the kernel module is loaded this
	 * returns the string that made us believe the kernel module was loaded.
	 * This is a line from /proc/modules on linux.
	 **/
	QString osKernelModuleNVidiaString() const
	{
		return getString(BoInfo::KernelModuleNVidiaString);
	}

	/**
	 * This parses the CPU string if available (/proc/cpuinfo on linux) and
	 * returns the speed of the cpu in MHz. -1 for an unknown OS (currently
	 * everything except linux) as well as when an error occurs
	 *
	 * Note that this <em>reall</em> parses the string - the value is not
	 * yet cached, so you should not use this in paintGL() or so! One day
	 * we'll add caching for values like this.
	 **/
	float cpuSpeed() const;

	/**
	 * @return TRUE if the computer has the file /proc/mtrr. This gives
	 * useful results for linux only (other OS's will return FALSE here).
	 **/
	bool haveMtrr() const;

	QMap<int, QVariant> completeData() const;

	/**
	 * This goes through the BoInfo data and tries to ensure that everything
	 * is fine for a user of the proprietary nvidia driver. They have a
	 * strange install precedure (like remove libGL.so and so on) which
	 * usually causes trouble.
	 * @return A list of error messages describing what is missing/wrong for
	 * a correct nvidia installation. non-nvidia users should ignore this.
	 **/
	QStringList checkProprietaryNVidiaDriver() const;

	bool contains(int key) const;
	int getInt(int key) const;
	unsigned int getUInt(int key) const;
	QString getString(int key) const;
	bool getBool(int key) const;

	// TODO: compile time versions of
	// - glx
	// - glu
	// - opengl

	// TODO: (both, string and version number):
	// - nvidia driver
	// - driver in general (*which* and which version)
	// - KGame
	// - lib3ds --> no version
	// - libkdegames?
	// - wml/perl? -> runtime version && compile version

	// misc
	// - resolution (dimension -> ScreenWidth/ScreenHeight)
	// - bpp
	// - xinerama enabled?
	// - if DRI in use -> does user have access? are permissions set in
	// XFree86Config ?
	// - texture RAM
	// - gfx card RAM
	// - RAM in general
	// - is GLX module loaded in XFree86Config ? --> extensions list
	//   --> same about GLcore and so
	//   --> the modules (also drivers) are loaded by "loadmod.c", function
	//   LoadModule(). See xf86Init.c, which parses the X-config and calls
	//   that.
	//   // AB: we need the LoadModule() stuff *only*. if GLX isn't loaded,
	//   then it is not listed in the extensions list.
	//
protected:
	void insert(int key, int value);
	void insert(int key, unsigned int value);
	/**
	 * @param value The value to be stored. Will be stored as a @ref QString
	 **/
	void insert(int key, const char* value);
	void insert(int key, const QString& value);
	void insert(int key, bool value);

	QString findLibrary(const QString& lib) const;

	/**
	 * @param lib Absolute filename (use @ref findLibrary) to the library
	 * that gets checked.
	 * @param dependsOn Check for dependancy of this.
	 * @return TRUE if "ldd lib | grep dependsOn" finds something. otherwise
	 * FALSE.
	 **/
	bool libraryDependsOn(const QString& lib, const QString& dependsOn) const;

	void reset();
	void updateOpenGLInfo(QWidget* w); // note: you must make a GLX context current before calling this!
	void updateXInfo(QWidget* w);
	void updateOSInfo();
	void updateLibraryInfo();
	void updateDevicesInfo();

	/**
	 * @return 0 On success, otherwise from @ref CharacterDevice, OR'ed
	 * together.
	 **/
	int checkCharacterDevice(const QString& file) const;

	QString makeCharacterDeviceErrorString(int error) const;

private:
	void init();

private:
	BoInfoPrivate* d;
};


/**
 * @short Provide current information about the system
 *
 * While @ref BoInfo is meant to provide static information about a system, such
 * as installed libraries, CPU clock, installed RAM and so on, this class is
 * meant to provide information about dynamic data.
 *
 * The data this class operates on can change at any time and therefore it makes
 * little or no sense to store them in the @ref BoInfo database.
 *
 * This class is primarily meant to be used in-game, whereas @ref BoInfo is
 * primarily meant to provide an information log about the system. This class
 * can be used for example to retrieve the current system load. The game may
 * reduce certain effects, depending on that.
 *
 * Note that in addition to dynamic data, this class can also be used to provide
 * static data that may be useful for in-game use. One example may be the CPU
 * speed.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoCurrentInfo
{
public:
	BoCurrentInfo();
	~BoCurrentInfo();

	BoInfo* boInfo() const
	{
		return mInfo;
	}

	/**
	 * Returns the memory that is currently in use in the parameters.
	 *
	 * WARNING: these values can be very confusing, if they are not read
	 * correctly. The most important value is @p vmData, which returns the
	 * size of the data segment of this application. However it does NOT
	 * necessarily return the amount of memory in use! The value depends
	 * heavily on the paging strategy of the operating system.
	 *
	 * If a value is not provided by any reason, the parameter is either
	 * not touched at all, or set to @ref QString::null.
	 *
	 * A NULL pointer parameter is not touched by this method.
	 *
	 * @return TRUE, if the data could be read, FALSE otherwise.
	 **/
	bool memoryInUse(QString* vmSize, QString* vmLck, QString* vmRSS,
			QString* vmData, QString* vmStk, QString* vmExe, QString* vmLib,
			QString* vmPTE) const;

	/**
	 * Returns the number of jiffies that the process has been scheduled.
	 *
	 * Note that this may be very linux dependent - I don't know other
	 * systems good enough to implement an API that might be portable.
	 *
	 * See man 5 proc for the meaning of the parameters.
	 *
	 * @return TRUE if the valeus could be read, otherwise FALSE
	 **/
	bool cpuTime(unsigned long int* utime, unsigned long int* stime, long int* cutime, long int* cstime) const;

	float cpuSpeed() const;


private:
	BoInfo* mInfo;
};


#endif

