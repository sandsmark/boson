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

#define QT_CLEAN_NAMESPACE

#include "boinfo.h"
#include "bodebug.h"
#include "boglquerystates.h"

#include <qwidget.h>
#include <qregexp.h>
#include <qstringlist.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

void BoInfo::updateOpenGLInfo(QWidget* widget)
{
 // OpenGL (warning: we don't have compile-time versions here!)
 QString extensions;
 QString oglversionstring = QString((const char*)glGetString(GL_VERSION));
 insert(BoInfo::OpenGLVersionString, oglversionstring.latin1());
 insert(BoInfo::OpenGLVendorString, (const char*)glGetString(GL_VENDOR));
 insert(BoInfo::OpenGLRendererString, (const char*)glGetString(GL_RENDERER));
 extensions = (const char*)glGetString(GL_EXTENSIONS);
 extensions.replace(QRegExp(" "), "\n");
 insert(BoInfo::OpenGLExtensionsString, extensions);

 // Find out OpenGL version
 unsigned int oglversionmajor = 0, oglversionminor = 0, oglversionrelease = 0;
 int oglversionlength = oglversionstring.find(' ');
 if (oglversionlength == -1) {
	oglversionlength = oglversionstring.length();
 }
 QString versionstr = oglversionstring.left(oglversionlength);
 QStringList versioninfo = QStringList::split(QChar('.'), oglversionstring.left(oglversionlength));
 if (versioninfo.count() < 2 || versioninfo.count() > 3) {
	boError() << k_funcinfo << "versioninfo has " << versioninfo.count() <<
			" entries (version string: '" << oglversionstring << "')" << endl;
 } else {
	oglversionmajor = versioninfo[0].toUInt();
	oglversionminor = versioninfo[1].toUInt();
	if (versioninfo.count() == 3) {
		oglversionrelease = versioninfo[2].toUInt();
	}
 }
 insert(BoInfo::OpenGLVersion, MAKE_VERSION(oglversionmajor, oglversionminor, oglversionrelease));

 insert(BoInfo::GLUVersionString, (const char*)gluGetString(GLU_VERSION));
 extensions = (const char*)gluGetString(GLU_EXTENSIONS);
 extensions.replace(QRegExp(" "), "\n");
 insert(BoInfo::GLUExtensionsString, extensions);

 GLXContext context = glXGetCurrentContext();
 if (!context) {
	boWarning() << k_funcinfo << "NULL context" << endl;
 }
 if (!widget || !context) {
	// it *may* be possible that GL/GLU infos are valid (don't depend on
	// this! OpenGL must have been initialized by
	// BosonGLWidget::initializeGL() or glGetString() returns NULL). But GLX
	// infos are can't be retrieved.
	insert(BoInfo::HaveOpenGLData, (bool)false);
	return;
 }

 int glxmajor, glxminor;
 glXQueryVersion(widget->x11Display(), &glxmajor, &glxminor);
 insert(BoInfo::GLXVersionMajor, (int)glxmajor);
 insert(BoInfo::GLXVersionMinor, (int)glxminor);
 insert(BoInfo::GLXClientVersionString, (const char*)glXGetClientString(widget->x11Display(), GLX_VERSION));
 insert(BoInfo::GLXClientVendorString, (const char*)glXGetClientString(widget->x11Display(), GLX_VENDOR));
 extensions = (const char*)glXGetClientString(widget->x11Display(), GLX_EXTENSIONS);
 extensions.replace(QRegExp(" "), "\n");
 insert(BoInfo::GLXClientExtensionsString, extensions);
 insert(BoInfo::GLXServerVersionString, (const char*)glXQueryServerString(widget->x11Display(), widget->x11Screen(), GLX_VERSION));
 insert(BoInfo::GLXServerVendorString, (const char*)glXQueryServerString(widget->x11Display(), widget->x11Screen(), GLX_VENDOR));
 extensions.replace(QRegExp(" "), "\n");
 extensions = (const char*)glXQueryServerString(widget->x11Display(), widget->x11Screen(), GLX_EXTENSIONS);
 insert(GLXServerExtensionsString, extensions);
 insert(BoInfo::IsDirect, (bool)glXIsDirect(widget->x11Display(), context));

 BoGLQueryStates glStates;
 glStates.init();
 QStringList implementationValueList = glStates.implementationValueList();
 QString implementationValues;
 for (unsigned int i = 0; i < implementationValueList.count(); i++) {
	implementationValues += implementationValueList[i] + "\n";
 }
 insert(BoInfo::OpenGLValuesString, implementationValues);

 insert(BoInfo::HaveOpenGLData, (bool)true);
}

// AB: "xwininfo" (program) might be interesting for us
void BoInfo::updateXInfo(QWidget* widget)
{
 // AB: see xdpyinfo.c for more information on these here
 if (!widget) {
	insert(HaveXData, (bool)false); // FIXME: bool seems not to work (int gets used)
	return;
 }
 Display* dpy = widget->x11Display();
 int scr = widget->x11Screen();
 insert(BoInfo::XDisplayName, (const char*)DisplayString(dpy));
 insert(BoInfo::XProtocolVersion, (int)ProtocolVersion(dpy));
 insert(BoInfo::XProtocolRevision, (int)ProtocolRevision(dpy));
 insert(BoInfo::XVendorString, (const char*)ServerVendor(dpy));
 insert(BoInfo::XVendorReleaseNumber, (int)VendorRelease(dpy));
 insert(BoInfo::XDefaultScreen, (int)DefaultScreen(dpy));
 insert(BoInfo::XScreenCount, (int)ScreenCount(dpy));
 insert(BoInfo::XScreen, (int)widget->x11Screen());
 // AB: according to XLib docs the Display*() things have been misnamed. they
 // should be Screen*() instead. so we use Screen* in this class
 insert(BoInfo::XScreenWidth, (int)DisplayWidth(dpy, scr));
 insert(BoInfo::XScreenHeight, (int)DisplayHeight(dpy, scr));
 insert(BoInfo::XScreenWidthMM, (int)DisplayWidthMM(dpy, scr));
 insert(BoInfo::XScreenHeightMM, (int)DisplayHeightMM(dpy, scr));

 // AB: depth:
 // XLib docs say about DefaultDepthOfScreen(): return the depth of the root
 // window
 // --> it seems that this is *not* the DefaultDepth entry in the XFree86Config
 // file
 //
 // XGetWindowAttributes returns a XWindowAttributes struct which contains depth
 //
 // XLib docs: see chapter 16.7
 //
 // XLib docs: Appendix C about extensions! and whether theyre loaded...

 int n = 0;
 char** extensionsList = XListExtensions(dpy, &n);
 if (extensionsList) {
	// this list may contain interesting infos, such as whether the GLX
	// module is available and so.
	// I believe this is the list of *loaded* extensions , so we can actually
	// use it to check whether glx/dri/whatever is loaded. (at least for me
	// XINERAMA does not appear in the list, and it actually doesn't get
	// loaded)
	//
	// Please mail me if that is wrong!
	QString string;
	for (int i = 0; i < n; i++) {
		string += extensionsList[i];
		if (i != n - 1) {
			string += '\n';
		}
	}
	// AB: xdpyinfo.c does not free this, since "XLib can depend on contents
	// being unaltered". Is this ok for us??
	XFreeExtensionList(extensionsList);
	insert(BoInfo::XExtensionsString, string);
 } else {
	boWarning() << k_funcinfo << "received NULL X extensions list" << endl;
 }

 insert(HaveXData, (bool)true);
}

