/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann <b_mann@gmx.de>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "boinfo.h"
#include "bodebug.h"

#include "../../bomemory/bodummymemory.h"
#include "../boversion.h"
#include "../boglobal.h"
#include "../defines.h"

#include <kdeversion.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <k3procio.h>
#include <k3staticdeleter.h>

#include <qstringlist.h>
#include <qmap.h>
#include <qvariant.h>
#include <qdom.h>
#include <qfile.h>
#include <qregexp.h>

#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/types.h>


#define BOINFO_VERSION 0x02
#define BOINFO_TAG "BoInfo"

// fix KDE. (this macro didn't exist before 3.1.x)
#ifndef KDE_MAKE_VERSION
#define KDE_MAKE_VERSION(a,b,c) (((a) << 16) | ((b) << 8) | (c))
#endif


// OS dependant stuff
#ifdef __linux__
#include "boinfo_linux.cpp"
#elif defined(__FreeBSD__)
#include "boinfo_freebsd.cpp"
#else
#include "boinfo_generic.cpp"
#endif


#define XEXTLIBGLX_A "/usr/X11R6/lib/modules/extensions/libglx.a"
#define XEXTLIBGLCORE_A "/usr/X11R6/lib/modules/extensions/libGLcore.a"
#define XEXTLIBGLX_SO "/usr/X11R6/lib/modules/extensions/libglx.so" // proprietary
#define NVIDIAXDRIVER "/usr/X11R6/lib/modules/drivers/nvidia_drv.o" // proprietary

static BoGlobalObject<BoInfo> globalInfo(BoGlobalObjectBase::BoGlobalInfo);

class BoInfoPrivate
{
public:
	BoInfoPrivate()
	{
	}
	QMap<int, QVariant> mInfos;
};


BoInfo::BoInfo()
{
 init();
// update();
}

BoInfo::~BoInfo()
{
 reset();
 delete d;
}

void BoInfo::init()
{
 d = new BoInfoPrivate;
 mGLCache = new BoInfoGLCache(this);
}

BoInfo* BoInfo::boInfo()
{
 return BoGlobal::boGlobal()->boInfo();
}

void BoInfo::reset()
{
 d->mInfos.clear();
 mGLCache->setDirty(true);
}

bool BoInfo::save(QDataStream& stream) const
{
 stream << QString(BOINFO_TAG);
 stream << (quint32)BOINFO_VERSION;
 stream << d->mInfos;
 return true;
}

bool BoInfo::load(QDataStream& stream)
{
 QString tag;
 quint32 version;
 stream >> tag;
 if (tag != QString(BOINFO_TAG)) {
	boError() << k_funcinfo << "Expected BoInfo data" << endl;
	return false;
 }
 reset();
 stream >> version;
 stream >> d->mInfos;
 return true;
}

bool BoInfo::save(QDomElement& root) const
{
 // AB: warning: *all* entries in d->mInfos must be able to cast to a string.
 // See QVariant::toString() about this!
 boDebug() << k_funcinfo << endl;
 QDomDocument doc = root.ownerDocument();
 root.setAttribute(QString::fromLatin1("Version"), QString::number((quint32)BOINFO_VERSION));
 QMap<int, QVariant>::Iterator it;
 for (it = d->mInfos.begin(); it != d->mInfos.end(); ++it) {
	if (!it.value().canConvert(QVariant::String)) {
		boError() << k_funcinfo << "Invalid variant type " << it.value().type() << endl;
		continue;
	}
	QDomElement e = doc.createElement(QString::fromLatin1("BoInfo"));
	e.setAttribute(QString::fromLatin1("Key"), it.key());
	e.appendChild(doc.createTextNode(it.value().toString()));
	root.appendChild(e);
 }
 return true;
}

bool BoInfo::load(QDomElement& root)
{
 boDebug() << k_funcinfo << endl;
 QDomDocument doc = root.ownerDocument();
 unsigned int version = root.attribute(QString::fromLatin1("Version")).toUInt();
 if (version < 0x02) {
	boError() << k_funcinfo << "File format version < 0x02 not supported anymore" << endl;
	return false;
 }
 reset();
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("BoInfo"));
 for (int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError() << k_funcinfo << "Could not convert to QDomElement" << endl;
		continue;
	}
	bool ok = true;
	int key = -1;
	key = e.attribute(QString::fromLatin1("Key")).toInt(&ok);
	if (!ok || key < 0) {
		boError() << k_funcinfo << "Invalid key entry" << endl;
		continue;
	}
	QString value = e.text();
	if (value.isNull()) {
		boWarning() << k_funcinfo << "no value for key " << key << endl;
		continue;
	}

	// AB: is this working properly for ALL values? we are using toInt() and
	// so in our get*() methods, so this *should* work...
	insert(key, value);
 }
 return true;
}

bool BoInfo::saveToFile(const QString& fileName) const
{
 QFile file(fileName);
 if (!file.open(QIODevice::WriteOnly)) {
	boError() << k_funcinfo << "Could not open " << fileName << " for writing" << endl;
	return false;
 }
 QDomDocument doc(QString::fromLatin1("BoInfo"));
 QDomElement root = doc.createElement(QString::fromLatin1("BoInfo"));
 doc.appendChild(root);
 bool ret = save(root);
 QString string = doc.toString();
 file.write(string.toAscii());
 file.close();
 return ret;
}

bool BoInfo::loadFromFile(const QString& fileName)
{
 QFile file(fileName);
 if (!file.open(QIODevice::ReadOnly)) {
	boError() << k_funcinfo << "Could not open " << fileName << endl;
	return false;
 }
 QDomDocument doc(QString::fromLatin1("BoInfo"));
 QString errorMsg;
 int lineNo, columnNo;
 if (!doc.setContent(file.readAll(), &errorMsg, &lineNo, &columnNo)) {
	boError() << k_funcinfo << "Parse error in line " << lineNo
			<< ",column " << columnNo
			<< " error message: " << errorMsg << endl;
		return false;
 }
 QDomElement root = doc.documentElement();
 bool ret = load(root);
 file.close();
 return ret;
}

void BoInfo::copyFrom(const BoInfo& b)
{
 reset();
 QByteArray buffer;
 QDataStream wstream(&buffer, QIODevice::WriteOnly);
 b.save(wstream);
 QDataStream rstream(buffer);
 load(rstream);
}

void BoInfo::update(QWidget* widget)
{
 reset();

 // boson
 insert(BosonVersion, (unsigned int)BOSON_VERSION);
 insert(BosonVersionString, BOSON_VERSION_STRING);

 // kde
#if KDE_VERSION >= KDE_MAKE_VERSION (3,2,0)
 // for KDE < 3.2 we don't have a runtime version available
 insert(KDEVersion, (unsigned int)KDE::version());
#endif
 insert(CompileOffset + KDEVersion, (unsigned int)KDE_VERSION);
#if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
 // for KDE < 3.1 we don't have a runtime version string
 insert(KDEVersionString, KDE::versionString());
#endif
 insert(CompileOffset + KDEVersionString, KDE_VERSION_STRING);

 // qt
 insert(CompileOffset + QtVersion, (unsigned int)QT_VERSION); // we don't have runtime version available :(
 insert(QtVersionString, qVersion());
 insert(CompileOffset + QtVersionString, QT_VERSION_STR);

 updateOpenGLInfo(widget);
 updateXInfo(widget);
 updateOSInfo();
 updateLibraryInfo();
 updateDevicesInfo();

 updateCachedValues();
}

void BoInfo::updateLibraryInfo()
{
 // here we search for several installed libraries. mainly OpenGL relevant stuff
 // (see below on nvidia)

 // proprietary nvidia driver use their own libgl and so. these usually conflict
 // with existing free libraries. since this is a major source of problems for
 // nvidia users we check for existance of relevant libraries here.
 // libglx/libGLcore: nvidia users need the .so stuff, .a must not exists. the
 // rest of us (umm.. dunno about ATI) uses the .a files (.so shouldn't exist)
 if (KStandardDirs::exists(QString::fromLatin1(XEXTLIBGLX_A))) {
	insert(HaveXExtLibGLX_a, QString::fromLatin1(XEXTLIBGLX_A));
 } else {
	insert(HaveXExtLibGLX_a, QString::fromLatin1(""));
 }
 if (KStandardDirs::exists(QString::fromLatin1(XEXTLIBGLCORE_A))) {
	insert(HaveXExtLibGLCore_a, QString::fromLatin1(XEXTLIBGLCORE_A));
 } else {
	insert(HaveXExtLibGLCore_a, QString::fromLatin1(""));
 }
 if (KStandardDirs::exists(QString::fromLatin1(XEXTLIBGLX_SO))) {
	insert(HaveXExtLibGLX_so, QString::fromLatin1(XEXTLIBGLX_SO));
 } else {
	insert(HaveXExtLibGLX_so, QString::fromLatin1(""));
 }
 if (KStandardDirs::exists(QString::fromLatin1(NVIDIAXDRIVER))) {
	insert(HaveProprietaryNVidiaXDriver, QString::fromLatin1(NVIDIAXDRIVER));
 } else {
	insert(HaveProprietaryNVidiaXDriver, QString::fromLatin1(""));
 }
 // TODO: whats the name of the proprietary ATI driver?
 // TODO: does ATI provide their own versions of libglx and libGLcore as well?

 QString lib = findLibrary(QString::fromLatin1("libGL.so"));
 if (lib.isEmpty()) {
	// umm.. this can't startup - a bug in findLibrary() probably.
	insert(BoInfo::HaveLibGL_so, QString::fromLatin1(""));
 } else {
	insert(BoInfo::HaveLibGL_so, lib);
	// for nvidia users the libGL.so must depend on libGLcore.so
	insert(BoInfo::LibGL_so_DependsOnLibGLCore, (bool)libraryDependsOn(lib, QString::fromLatin1("libGLcore")));
 }
 lib = findLibrary(QString::fromLatin1("libGLcore.so.1"));
 if (lib.isEmpty()) {
	// only nvidia users have this
	insert(BoInfo::HaveLibGLCore_so_1, QString::fromLatin1(""));
 } else {
	insert(BoInfo::HaveLibGLCore_so_1, lib);
 }

}

void BoInfo::updateDevicesInfo()
{
 insert(BoInfo::DevNVidiaCTL, (int)checkCharacterDevice(QString::fromLatin1("/dev/nvidiactl")));
 insert(BoInfo::DevNVidia0, (int)checkCharacterDevice(QString::fromLatin1("/dev/nvidia0")));
 insert(BoInfo::DevNVidia1, (int)checkCharacterDevice(QString::fromLatin1("/dev/nvidia1")));
 insert(BoInfo::DevNVidia2, (int)checkCharacterDevice(QString::fromLatin1("/dev/nvidia2")));
 insert(BoInfo::DevNVidia3, (int)checkCharacterDevice(QString::fromLatin1("/dev/nvidia3")));
}

void BoInfo::updateCachedValues()
{
 mGLCache->update();
 mGLCache->setDirty(false);
}

void BoInfo::insert(int key, int value)
{
 d->mInfos.insert(key, QVariant((int)value));
}

void BoInfo::insert(int key, unsigned int value)
{
 d->mInfos.insert(key, QVariant((unsigned int)value));
}

void BoInfo::insert(int key, const QString& value)
{
 d->mInfos.insert(key, QVariant(value));
}

void BoInfo::insert(int key, const char* value)
{
 insert(key, QString(value));
}

void BoInfo::insert(int key, bool value)
{
 // we store a boolean as an integer, since it makes loading/saving using
 // QVariant casts easier.
 insert(key, (int)(value ? 1 : 0));
}

bool BoInfo::contains(int key) const
{
 return d->mInfos.contains(key);
}

QString BoInfo::getString(int key) const
{
 if (!d->mInfos.contains(key)) {
	return QString();
 }
 return d->mInfos[key].toString();
}

int BoInfo::getInt(int key) const
{
 if (!d->mInfos.contains(key)) {
	return -1;
 }
 return d->mInfos[key].toInt();
}

unsigned int BoInfo::getUInt(int key) const
{
 if (!d->mInfos.contains(key)) {
	return 0;
 }
 return d->mInfos[key].toUInt();
}

bool BoInfo::getBool(int key) const
{
 return getInt(key);
}

QString BoInfo::keyToName(int key)
{
 if (key - CompileOffset >= CompileOffset || key == CompileOffset) {
	boError() << k_funcinfo << "Invalid key " << key << endl;
	return QString();
 }
 if (key > CompileOffset) {
	return i18n("Compilation %1", keyToName(key - CompileOffset));
 }
 QString string;
 switch (key) {
	case BosonVersion:
		string = i18n("Boson Version");
		break;
	case BosonVersionString:
		string = i18n("Boson Version String");
		break;
	case KDEVersion:
		string = i18n("KDE Version");
		break;
	case KDEVersionString:
		string = i18n("KDE Version String");
		break;
	case QtVersion:
		string = i18n("Qt Version");
		break;
	case QtVersionString:
		string = i18n("Qt Version String");
		break;
	case BoInfo::HaveOpenGLData:
		string = i18n("Have OpenGL information");
		break;
	case OpenGLVersionString:
		string = i18n("OpenGL Version String");
		break;
	case OpenGLVendorString:
		string = i18n("OpenGL Vendor String");
		break;
	case OpenGLRendererString:
		string = i18n("OpenGL Renderer String");
		break;
	case OpenGLExtensionsString:
		string = i18n("OpenGL Extensions String");
		break;
	case OpenGLValuesString:
		string = i18n("OpenGL Values String");
		break;
	case OpenGLVersion:
		string = i18n("OpenGL Version");
		break;
	case GLUVersionString:
		string = i18n("GLU Version String");
		break;
	case GLUExtensionsString:
		string = i18n("GLU Extensions String");
		break;
	case GLXClientVersionString:
		string = i18n("GLX Client Version String");
		break;
	case GLXClientVendorString:
		string = i18n("GLX Client Vendor String");
		break;
	case GLXClientExtensionsString:
		string = i18n("GLX Client Extensions String");
		break;
	case GLXServerVersionString:
		string = i18n("GLX Server Version String");
		break;
	case GLXServerVendorString:
		string = i18n("GLX Server Vendor String");
		break;
	case GLXServerExtensionsString:
		string = i18n("GLX Server Extensions String");
		break;
	case GLXVersionMajor:
		string = i18n("GLX Version (major)");
		break;
	case GLXVersionMinor:
		string = i18n("GLX Version (minor)");
		break;
	case BoInfo::IsDirect:
		string = i18n("Is Direct");
		break;
	case BoInfo::HaveXData:
		string = i18n("Have X information");
		break;
	case BoInfo::XDisplayName:
		string = i18n("X Display name");
		break;
	case BoInfo::XProtocolVersion:
		string = i18n("X protocol version");
		break;
	case BoInfo::XProtocolRevision:
		string = i18n("X protocol revision");
		break;
	case BoInfo::XVendorString:
		string = i18n("X vendor string");
		break;
	case BoInfo::XVendorReleaseNumber:
		string = i18n("X vendor release number");
		break;
	case BoInfo::XExtensionsString:
		string = i18n("X extensions string");
		break;
	case BoInfo::XDefaultScreen:
		string = i18n("X default screen");
		break;
	case BoInfo::XScreenCount:
		string = i18n("X screen count");
		break;
	case BoInfo::XScreen:
		string = i18n("X screen");
		break;
	case BoInfo::XScreenWidth:
		string = i18n("X screen width (pixels)");
		break;
	case BoInfo::XScreenHeight:
		string = i18n("X screen height (pixels)");
		break;
	case BoInfo::XScreenWidthMM:
		string = i18n("X screen width (mm)");
		break;
	case BoInfo::XScreenHeightMM:
		string = i18n("X screen height (mm)");
		break;
	case BoInfo::CPUString:
		string = i18n("CPU string");
		break;
	case BoInfo::MTRRString:
		string = i18n("MTRR string");
		break;
	case BoInfo::OSTypeString:
		string = i18n("OS type string");
		break;
	case BoInfo::OSVersionString:
		string = i18n("OS version string");
		break;
	case BoInfo::KernelModuleNVidiaString:
		string = i18n("Kernel Module (nvidia) string");
		break;
	case BoInfo::KernelModuleTDFXString:
		string = i18n("Kernel Module (tdfx) string");
		break;
	case BoInfo::HaveXExtLibGLX_a:
		string = i18n("Have %1", QString::fromLatin1(XEXTLIBGLX_A));
		break;
	case BoInfo::HaveXExtLibGLX_so:
		string = i18n("Have %1", QString::fromLatin1(XEXTLIBGLX_SO));
		break;
	case BoInfo::HaveXExtLibGLCore_a:
		string = i18n("Have %1", QString::fromLatin1(XEXTLIBGLCORE_A));
		break;
	case BoInfo::HaveLibGL_so:
		string = i18n("Have libGL.so");
		break;
	case BoInfo::LibGL_so_DependsOnLibGLCore:
		string = i18n("libGL.so depends on libGLcore (proprietary libGL.so)");
		break;
	case BoInfo::HaveLibGLCore_so_1:
		string = i18n("Have libGLcore.so.1 (NVidia)");
		break;
	case BoInfo::HaveProprietaryNVidiaXDriver:
		string = i18n("Have the proprietary %1", QString::fromLatin1(NVIDIAXDRIVER));
		break;
	case BoInfo::DevNVidiaCTL:
		string = i18n("/dev/nvidiactl");
		break;
	case BoInfo::DevNVidia0:
		string = i18n("/dev/nvidia0");
		break;
	case BoInfo::DevNVidia1:
		string = i18n("/dev/nvidia1");
		break;
	case BoInfo::DevNVidia2:
		string = i18n("/dev/nvidia2");
		break;
	case BoInfo::DevNVidia3:
		string = i18n("/dev/nvidia3");
		break;

	default:
		string = i18n("(Unknown)");
 }
 return string;
}

QString BoInfo::valueToString(int key) const
{
 QString string;
 QVariant v = d->mInfos[key];
 switch (v.type()) {
	case QVariant::Invalid:
	case QVariant::BitArray:
	case QVariant::ByteArray:
	case QVariant::Bitmap:
	case QVariant::Brush:
	case QVariant::Color:
	case QVariant::Cursor:
	case QVariant::Date:
	case QVariant::DateTime:
	case QVariant::Font:
	case QVariant::Image:
	case QVariant::KeySequence:
	case QVariant::List:
	case QVariant::Map:
	case QVariant::Palette:
#if QT_VERSION >= 0x030100
	case QVariant::Pen:
#endif
	case QVariant::Pixmap:
	case QVariant::Point:
	case QVariant::Rect:
	case QVariant::Region:
	case QVariant::Size:
	case QVariant::SizePolicy:
	case QVariant::StringList:
	case QVariant::Time:
		boWarning() << k_funcinfo << "invalid variant type " << (int)v.type() << " for " << key << endl;
		break;
#ifdef Bool
#undef Bool
#endif
	case QVariant::Bool:
		string = v.toBool() ? i18n("True") : i18n("False");
		break;
	case QVariant::Double:
		string = QString::number(v.toDouble());
		break;
	case QVariant::Int:
		string = QString::number(v.toInt());
		break;
	case QVariant::UInt:
		string = QString::number(v.toUInt());
		break;
	case QVariant::String:
		string = v.toString();
		break;
#if QT_VERSION >= 0x030200
	case QVariant::LongLong:
		string = v.toLongLong();
		break;
	case QVariant::ULongLong:
		string = v.toULongLong();
		break;
#endif
 }
 return string;
}


void BoInfo::debug() const
{
 QMap<int, QVariant>::Iterator it;
 for (it = d->mInfos.begin(); it != d->mInfos.end(); ++it) {
	QString value = valueToString(it.key());
	if (!value.isNull()) {
		QString name = keyToName(it.key());
		boDebug() << name  + ": " << value << endl;
	}
 }
}

QString BoInfo::findLibrary(const QString& lib) const
{
#warning FIXME!
 // TODO: we should check LD_LIBRARY_PATH, ld.so.cache and after that /usr/lib
 // and /lib.
 // I don't want to code all this... is there somewhere a convenience function
 // in glibc or so?
 QString file = QString("/usr/lib/%1").arg(lib);
 if (!KStandardDirs::exists(file)) {
	return QString();
 }
 return file;
}

bool BoInfo::libraryDependsOn(const QString& lib, const QString& dependsOn) const
{
 if (!KStandardDirs::exists(lib)) {
	boError() << k_funcinfo << lib << " does not exists" << endl;
	return false;
 }
 QString ldd = QString::fromLatin1("/usr/bin/ldd");
 if (!KStandardDirs::exists(ldd)) {
	boWarning() << k_funcinfo << "you don't have " << ldd << endl;
	return false;
 }
 K3ProcIO proc;
 proc << "/usr/bin/ldd" << QFile::encodeName(lib);
 if (!proc.start(K3ProcIO::Block)) {
	boWarning() << k_funcinfo << "error starting ldd" << endl;
	return false;
 }
 QRegExp exp(QString("^\\s*%1").arg(dependsOn));
 int lines = 0; // fallback
 QString line;
 while (proc.readln(line, true) > 0 && lines < 200) {
	if (line.indexOf(exp) >= 0) {
		return true;
	}
	lines++;
 }
 if (lines >= 200) {
	boWarning() << "problem with process - fallback reached" << endl;
 }
 return false;
}

QStringList BoInfo::checkProprietaryNVidiaDriver() const
{
 QStringList list;
 // code is partially from nvcheck.sh (By Will Weisser (waw0573@rit.edu))
 // (AB: the e-mail address is broken. don't know a recent one. we haven't been
 // able to find it out)
 // TODO: ensure the glx X-server module is loaded
 // TODO: ensure the nvidia driver is actually loaded (*not* nv)
 if (!getString(BoInfo::HaveXExtLibGLX_a).isEmpty()) {
	list.append(i18n("%1 exists (it should not)", QString::fromLatin1(XEXTLIBGLX_A)));
 }
 if (!getString(BoInfo::HaveXExtLibGLCore_a).isEmpty()) {
	list.append(i18n("%1 exists (it should not)", QString::fromLatin1(XEXTLIBGLCORE_A)));
 }
 if (getString(BoInfo::HaveProprietaryNVidiaXDriver).isEmpty()) {
	list.append(i18n("%1 does not exists (but it should)", QString::fromLatin1(NVIDIAXDRIVER)));
 }
 if (getString(BoInfo::HaveXExtLibGLX_so).isEmpty()) {
	list.append(i18n("%1 does not exists (but it should)", QString::fromLatin1(XEXTLIBGLX_SO)));
 }
 if (getString(BoInfo::HaveLibGL_so).isEmpty()) {
	list.append(i18n("libGL.so not found. Probably a boson bug - please tell us where your libGL.so resides."));
 }
 if (!getBool(BoInfo::LibGL_so_DependsOnLibGLCore)) {
	list.append(i18n("Your libGL.so seems not to depend on libGLcore - this is most probably not the proprietary NVidia libGL.so - please install the correct NVidia RPM."));
 }
 if (getString(BoInfo::HaveLibGLCore_so_1).isEmpty()) {
	list.append(i18n("libGLcore.so.1 is missing. It should be in the NVidia RPM for your graphics card."));
 }
 if (getString(BoInfo::KernelModuleNVidiaString).isNull()) {
	list.append(i18n("Boson has no kernel module list for you OS. This is not an error - you just have to check that the NVidia kernel module is loaded on your system (if existing for your OS)."));
 } else if (getString(BoInfo::KernelModuleNVidiaString).isEmpty()) {
	list.append(i18n("The nvidia kernel module is not loaded"));
 }

 // TODO: move the check to update() !
 int ret = getInt(BoInfo::DevNVidiaCTL);
 if (ret != CharSuccess) {
	QString error = makeCharacterDeviceErrorString(ret);
	list.append(i18n("Problem with %1: %2", QString::fromLatin1("/dev/nvidiactl"), error));
 }
 ret = getInt(BoInfo::DevNVidia0);
 if (ret != CharSuccess) {
	QString error = makeCharacterDeviceErrorString(ret);
	list.append(i18n("Problem with %1: %2", QString::fromLatin1("/dev/nvidia0"), error));
 }
 ret = getInt(BoInfo::DevNVidia1);
 if (ret != CharSuccess) {
	QString error = makeCharacterDeviceErrorString(ret);
	list.append(i18n("Problem with %1: %2", QString::fromLatin1("/dev/nvidia1"), error));
 }
 ret = getInt(BoInfo::DevNVidia2);
 if (ret != CharSuccess) {
	QString error = makeCharacterDeviceErrorString(ret);
	list.append(i18n("Problem with %1: %2", QString::fromLatin1("/dev/nvidia2"), error));
 }
 ret = getInt(BoInfo::DevNVidia3);
 if (ret != CharSuccess) {
	QString error = makeCharacterDeviceErrorString(ret);
	list.append(i18n("Problem with %1: %2", QString::fromLatin1("/dev/nvidia3"), error));
 }
 // TODO: finally the nvcheck.sh script checks for duplicated libGL
 // installations. we should do so as well.

 return list;
}


int BoInfo::checkCharacterDevice(const QString& file) const
{
 struct stat buf;
 int ret = CharSuccess;
 if (access(QFile::encodeName(file), F_OK) != 0) {
	ret |= CharDoesNotExist;
	// no need to check any further...
	return ret;
 }
 if (access(QFile::encodeName(file), W_OK) != 0) {
	ret |= CharCantWrite;
 }
 if (stat(QFile::encodeName(file), &buf) != 0) {
	ret |= CharCantStat;
 } else {
	if (!S_ISCHR(buf.st_mode)) {
		ret |= CharNoCharDevice;
	}
 }
 return ret;
}

QString BoInfo::makeCharacterDeviceErrorString(int ret) const
{
 QString error;
 if (ret & CharDoesNotExist) {
	error += i18n(" - device does not exist");
 }
 if (ret & CharCantRead) {
	error += i18n(" - can't read from device");
 }
 if (ret & CharCantWrite) {
	error += i18n(" - can't write to device");
 }
 if (ret & CharCantStat) {
	error += i18n(" - can't stat device");
 }
 if (ret & CharNoCharDevice) {
	error += i18n(" - is not a character device");
 }
 return error;
}

QMap<int, QVariant> BoInfo::completeData() const
{
 return d->mInfos;
}

QStringList BoInfo::openGLValues() const
{
 return getString(OpenGLValuesString).split('\n');
}

QString BoInfo::osType() const
{
 if (!d->mInfos.contains(BoInfo::OSTypeString)) {
	return QString();
 }
 QString s = getString(BoInfo::OSTypeString);
 s.replace(QRegExp("\n"), "");
 return s;
}

QString BoInfo::osVersion() const
{
 if (!d->mInfos.contains(BoInfo::OSVersionString)) {
	return QString();
 }
 QString s = getString(BoInfo::OSVersionString);
 s.replace(QRegExp("\n"), "");
 return s;
}



