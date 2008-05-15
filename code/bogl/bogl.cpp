/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks <rivolaks@hot.ee>

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

#define GL_GLEXT_LEGACY
#define GLX_GLXEXT_PROTOTYPES

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include "bogl.h"
#include "boglx.h"

#include "bogl_private.h"
#include "bodebug.h"

#include <qstringlist.h>
#include <qlibrary.h>

BoGL* BoGL::mBoGL = 0;


#ifndef GLX_ARB_get_proc_address
  // AB: if this is a linux system, it MUST support GLX_ARB_get_proc_address, as
  //     it is part of the linux OpenGL ARB.
#error cannot find GLX_ARB_get_proc_address. OpenGL installation broken?
#endif //GLX_ARB_get_proc_address


QStringList boglGetOpenGLExtensions()
{
 return BoGL::bogl()->OpenGLExtensions();
}

QStringList boglGetGLUExtensions()
{
 return BoGL::bogl()->GLUExtensions();
}

unsigned int boglGetOpenGLVersion()
{
 return BoGL::bogl()->OpenGLVersion();
}

QString boglGetOpenGLVersionString()
{
 return BoGL::bogl()->OpenGLVersionString();
}

QString boglGetOpenGLVendorString()
{
 return BoGL::bogl()->OpenGLVendorString();
}

QString boglGetOpenGLRendererString()
{
 return BoGL::bogl()->OpenGLRendererString();
}

QString boglGetGLUVersionString()
{
 return BoGL::bogl()->GLUVersionString();
}



BoGL::BoGL()
{
 d = new BoGLPrivate();
 d->mIsInitialized = false;;
 d->mIsResolved = false;
}

BoGL::~BoGL()
{
 delete d;
}

// TODO: delete on program exit (to make valgrind happy)
BoGL* BoGL::bogl()
{
 if (!mBoGL) {
	mBoGL = new BoGL();
 }
 return mBoGL;
}

bool BoGL::isResolved() const
{
 return d->mIsResolved;
}

bool BoGL::initialize()
{
 if (isInitialized()) {
	return isInitialized();
 }
 if (!isResolved()) {
	boError() << k_funcinfo << "GL methods have not yet been resolved." << endl;
	return isInitialized();
 }
 if (glXGetCurrentContext() == 0) {
	boError() << k_funcinfo << "cannot initialize BoGL without a current GLX context" << endl;
	// AB: most (if not all) GL functions need a current context.
	//     this includes glGetString(), even for e.g. GL_VERSION !!
	//     -> glGetString(GL_VERSION) without a current context may work for
	//        some drivers, but does not work for all drivers.
	//        so don't allow this.
	return isInitialized();
 }
 d->mIsInitialized = true;

 // Find out OpenGL version
 QString oglversionstring = QString((const char*)glGetString(GL_VERSION));
 unsigned int oglversionmajor = 0, oglversionminor = 0, oglversionrelease = 0;
 int oglversionlength = oglversionstring.indexOf(' ');
 if (oglversionlength == -1) {
	oglversionlength = oglversionstring.length();
 }
 QString versionstr = oglversionstring.left(oglversionlength);
 QStringList versioninfo = oglversionstring.left(oglversionlength).split(QChar('.'));
 if (versioninfo.count() < 2 || versioninfo.count() > 3) {
	boError() << k_funcinfo << "versioninfo has "
			<< versioninfo.count()
			<< " entries (version string: '"
			<< oglversionstring
			<< "')"
			<< endl;
 } else {
	oglversionmajor = versioninfo[0].toUInt();
	oglversionminor = versioninfo[1].toUInt();
	if (versioninfo.count() == 3) {
		oglversionrelease = versioninfo[2].toUInt();
	}
 }
 d->mOpenGLVersion = MAKE_VERSION_BOGL(oglversionmajor, oglversionminor, oglversionrelease);
 d->mOpenGLVersionString = oglversionstring;
 d->mOpenGLVendorString = QString::fromLatin1((const char*)glGetString(GL_VENDOR));
 d->mOpenGLRendererString = QString::fromLatin1((const char*)glGetString(GL_RENDERER));

 QString glExtensions = (const char*)glGetString(GL_EXTENSIONS);
 d->mOpenGLExtensions = glExtensions.split(" ");

 d->mGLUVersionString = QString::fromLatin1((const char*)gluGetString(GLU_VERSION));

 QString gluExtensions = (const char*)gluGetString(GLU_EXTENSIONS);
 d->mGLUExtensions = gluExtensions.split(" ");

 return isInitialized();
}

bool BoGL::isInitialized() const
{
 return d->mIsInitialized;
}

const QString& BoGL::OpenGLFile() const
{
 return d->mOpenGLLibraryFile;
}

const QString& BoGL::GLUFile() const
{
 return d->mGLULibraryFile;
}

unsigned int BoGL::OpenGLVersion() const
{
 const_cast<BoGL*>(this)->initialize();
 return d->mOpenGLVersion;
}

QString BoGL::OpenGLVersionString() const
{
 const_cast<BoGL*>(this)->initialize();
 return d->mOpenGLVersionString;
}

QString BoGL::OpenGLVendorString() const
{
 const_cast<BoGL*>(this)->initialize();
 return d->mOpenGLVendorString;
}

QString BoGL::OpenGLRendererString() const
{
 const_cast<BoGL*>(this)->initialize();
 return d->mOpenGLRendererString;
}

QStringList BoGL::OpenGLExtensions() const
{
 const_cast<BoGL*>(this)->initialize();
 return d->mOpenGLExtensions;
}

QString BoGL::GLUVersionString() const
{
 const_cast<BoGL*>(this)->initialize();
 return d->mGLUVersionString;
}

QStringList BoGL::GLUExtensions() const
{
 const_cast<BoGL*>(this)->initialize();
 return d->mGLUExtensions;
}

