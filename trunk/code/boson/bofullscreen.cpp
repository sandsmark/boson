/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bofullscreen.h"

#define QT_CLEAN_NAMESPACE 1

#include <bodebug.h>

#include <kstaticdeleter.h>

#include <qstringlist.h>

#include <stdlib.h>
#include <config.h>

#ifdef HAVE_XFREE86_XRANDR
// always prefer xrandr to vidmode, if possible
// better is to decide this on runtime (if boson was compiled with xrandr, but
// its not available when run), but since vidmode cant be used atm, this doesnt
// matter.
#undef HAVE_XFREE86_VIDMODE
#endif // HAVE_XFREE86_XRANDR

// for the VidMode extension we must grab the mouse, which is all but easy for a
// normal program. we need to use a confine_to window (see XGrabPointer man
// page), but Qt needs to grab/ungrab the mouse sometimes which makes our grab
// useless.
// without grabbing the mouse, the window is kinda unusable in a different
// resolution
#undef HAVE_XFREE86_VIDMODE

#ifdef HAVE_XFREE86_VIDMODE
#include <qapplication.h>
#include <qwidget.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>

static bool bo_enter_mode(XF86VidModeModeInfo* mode);
static void bo_enter_orig_mode();

static bool gUseVidMode = true;
static XF86VidModeModeInfo gOriginalMode;
static bool gOriginalModeValid = false;
#endif

#ifdef HAVE_XFREE86_XRANDR
// TODO: use QLibrary instead of linking to Xrandr!
// -> no need for the #ifdef, less trouble for packagers, ...
#include <qapplication.h>
#include <qwidget.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

static bool gUseXrandr = true;
static bool gOriginalModeValid = false;

static bool bo_init_xrandr(Display* dpy);
static bool bo_xrr_enter_mode(int mode, XRRScreenConfiguration* sc);
#endif // HAVE_XFREE86_XRANDR

static KStaticDeleter<BoFullScreen> sd;
BoFullScreen* BoFullScreen::mBoFullScreen = 0;

class BoFullScreenPrivate
{
public:
	BoFullScreenPrivate()
	{
#ifdef HAVE_XFREE86_XRANDR
		mScreenConfig = 0;
#endif // HAVE_XFREE86_XRANDR
#ifdef HAVE_XFREE86_VIDMODE
		mModes = 0;
		mModeCount = 0;
#endif // HAVE_XFREE86_VIDMODE
	}
#ifdef HAVE_XFREE86_VIDMODE
	XF86VidModeModeInfo** mModes;
	int mModeCount;
#endif // HAVE_XFREE86_VIDMODE
#ifdef HAVE_XFREE86_XRANDR
	XRRScreenConfiguration* mScreenConfig;
#endif // HAVE_XFREE86_XRANDR

	bool mInitialized;
};

BoFullScreen::BoFullScreen()
{
 d = new BoFullScreenPrivate;
 d->mInitialized = false;

#ifdef HAVE_XFREE86_VIDMODE
 // AB: won't work. at this point everything is closed already!
 // -> no qApp pointer, no display, ...
 atexit(bo_enter_orig_mode);
#endif
}

BoFullScreen::~BoFullScreen()
{
#ifdef HAVE_XFREE86_XRANDR
 if (d->mScreenConfig) {
	XRRFreeScreenConfigInfo(d->mScreenConfig);
 }
#endif // HAVE_XFREE86_XRANDR
#ifdef HAVE_XFREE86_VIDMODE
	if (d->mModes) {
		// in case the resolution got ever changed, we need to change
		// back again
		// will probably called twice, due to atexit()
		// AB: won't work. at this point everything is closed already!
		// -> no qApp pointer, no display, ...
		bo_enter_orig_mode();
	}
	XFree(d->mModes);
	d->mModes = 0;
	d->mModeCount  = 0;
#endif // HAVE_XFREE86_VIDMODE
 delete d;
}

void BoFullScreen::initStatic()
{
 if (mBoFullScreen) {
	return;
 }
 mBoFullScreen = new BoFullScreen();
 sd.setObject(mBoFullScreen);
}

void BoFullScreen::initModes()
{
#ifdef HAVE_XFREE86_XRANDR
 if (!gUseXrandr) {
	return;
 }
 if (!qApp->mainWidget()) {
	BO_NULL_ERROR(qApp->mainWidget());
	gUseXrandr = false;
	return;
 }
 QWidget* w = qApp->mainWidget();
 Display* dpy = w->x11Display();
 int screen = w->x11Screen();
 Window root = RootWindow(dpy, screen);

 gUseXrandr = bo_init_xrandr(dpy);
 if (!gUseXrandr) {
	return;
 }
 if (!d->mScreenConfig) {
	d->mScreenConfig = XRRGetScreenInfo(dpy, root);
 }
 if (!d->mScreenConfig) {
	boWarning() << k_funcinfo << "NULL screen config. Xrandr disabled" << endl;
	gUseXrandr = false;
	return;
 }
 // TODO: store current mode
#endif // HAVE_XFREE86_XRANDR
#ifdef HAVE_XFREE86_VIDMODE
 if (d->mModes) {
	return;
 }
 if (!gUseVidMode) {
	// we have initialized before and found out that we cannot use the
	// VidMode extension!
	return;
 }
 if (!qApp->mainWidget()) {
	BO_NULL_ERROR(qApp->mainWidget());
	gUseVidMode = false;
	return;
 }
 QWidget* w = qApp->mainWidget();
 Display* dpy = w->x11Display();
 int scr = w->x11Screen();
// Window win = w->winId();
 int event_base, error_base;
 if (!XF86VidModeQueryExtension(dpy, &event_base, &error_base)) {
	boWarning() << k_funcinfo << "VidMode extension not available. you cannot change the resolution!" << endl;
	gUseVidMode = false;
	return;
 } else {
	gUseVidMode = true;
 }
 if (XF86VidModeGetAllModeLines(dpy, scr, &d->mModeCount, &d->mModes)) {
	boDebug() << k_funcinfo << "mode list successfully retrieved from VidMode extension" << endl;
 } else {
	boWarning() << k_funcinfo << "XFree86 VidMode extension not available." << endl;
	d->mModeCount = 0;
	d->mModes = 0;
 }

 // AB: this nice hack was shamelessy stolen from SDL, so that we don't have to
 // copy the modeline manually to the modeinfo
 int dotclock;
 XF86VidModeModeLine* l = (XF86VidModeModeLine*)((char*)(&gOriginalMode) + sizeof gOriginalMode.dotclock);
 bool ret = XF86VidModeGetModeLine(dpy, scr, &dotclock, l);
 gOriginalMode.dotclock = dotclock;

 if (!ret) {
	boError() << k_funcinfo << "Could not retrieve current modeline" << endl;
	gOriginalModeValid = false;
 } else {
	gOriginalModeValid = true;
 }
#endif // HAVE_XFREE86_VIDMODE
}

QStringList BoFullScreen::availableModes()
{
 initStatic();
 if (mBoFullScreen) {
	return mBoFullScreen->availableModeList();
 }
 return QStringList();
}

bool BoFullScreen::enterMode(int index)
{
 boDebug() << k_funcinfo << index << endl;
 initStatic();
 if (!mBoFullScreen) {
	return false;
 }
 return mBoFullScreen->enterModeInList(index);
}

QStringList BoFullScreen::availableModeList()
{
 QStringList list;
 initModes();
#ifdef HAVE_XFREE86_XRANDR
 if (!gUseXrandr) {
	return list;
 }
 if (!d->mScreenConfig) {
	boDebug() << k_funcinfo << "NULL screen config" << endl;
	return list;
 }
 int count = 0;
 XRRScreenSize* sizes = XRRConfigSizes(d->mScreenConfig, &count);
 if (count <= 0 || !sizes) {
	boWarning() << k_funcinfo << "oops - no sizes available" << endl;
	return list;
 }
 for (int i  = 0; i < count; i++) {
	QString s = QString("%1x%2").arg(sizes[i].width).arg(sizes[i].height);
	list.append(s);
 }
#endif // HAVE_XFREE86_XRANDR
#ifdef HAVE_XFREE86_VIDMODE
 if (!gUseVidMode) {
	return list;
 }
 if (d->mModes && d->mModeCount > 1) {
	for (int i  = 0; i < d->mModeCount; i++) {
		QString s = QString("%1x%2").arg(d->mModes[i]->hdisplay).arg(d->mModes[i]->vdisplay);
		list.append(s);
	}
 }
#endif // HAVE_XFREE86_VIDMODE
 return list;
}

bool BoFullScreen::enterModeInList(int index)
{
 if (index < 0) {
	return false;
 }
 initModes();
#ifdef HAVE_XFREE86_XRANDR
 if (!gUseXrandr) {
	return false;
 }
 if (!d->mScreenConfig) {
	return false;
 }
 int count = 0;
 XRRScreenSize* sizes = XRRConfigSizes(d->mScreenConfig, &count);
 if (count <= 0 || !sizes || index >= count) {
	boError() << k_funcinfo << "unable to switch to mode " << index << " count=" << count << endl;
	return false;
 }
 return bo_xrr_enter_mode(index, d->mScreenConfig);
#endif // HAVE_XFREE86_VIDMODE
#ifdef HAVE_XFREE86_VIDMODE
 if (!gUseVidMode) {
	return false;
 }
 if (!d->mModes) {
	boWarning() << k_funcinfo << "no modes available" << endl;
	return false;
 }
 if (index >= d->mModeCount) {
	return false;
 }
 XF86VidModeModeInfo* mode = d->mModes[index];
 if (!mode) {
	BO_NULL_ERROR(mode);
	return false;
 }
 return bo_enter_mode(mode);
#endif // HAVE_XFREE86_VIDMODE
 return false;
}

bool BoFullScreen::enterOriginalMode()
{
#ifdef HAVE_XFREE86_VIDMODE
 bo_enter_orig_mode();
#endif
 return true;
}

#ifdef HAVE_XFREE86_VIDMODE
static bool bo_enter_mode(XF86VidModeModeInfo* mode)
{
 if (!gUseVidMode) {
	return false;
 }
 if (!mode) {
	BO_NULL_ERROR(mode);
	return false;
 }
 if (!qApp->mainWidget()) {
	BO_NULL_ERROR(qApp->mainWidget());
	return false;
 }
 QWidget* w = qApp->mainWidget();
 Display* dpy = w->x11Display();
 int scr = w->x11Screen();
 boDebug() << k_funcinfo << "switching to " << mode->hdisplay << "x" << mode->vdisplay << endl;
 return XF86VidModeSwitchToMode(dpy, scr, mode);
}

static void bo_enter_orig_mode()
{
 if (gOriginalModeValid) {
	bo_enter_mode(&gOriginalMode);
 }
}
#endif // HAVE_XFREE86_VIDMODE

#ifdef HAVE_XFREE86_XRANDR
static bool bo_init_xrandr(Display* dpy)
{
 int event_base, error_base;
 if (!XRRQueryExtension(dpy, &event_base, &error_base)) {
	boWarning() << k_funcinfo << "Xrandr extension not available. you cannot change the resolution!" << endl;
	return false;
 } else {
	return true;
 }
 return false;
}

static bool bo_xrr_enter_mode(int mode, XRRScreenConfiguration* sc)
{
 if (!gUseXrandr) {
	return false;
 }
 if (mode < 0) {
	return false;
 }
 QWidget* w = qApp->mainWidget();
 Display* dpy = w->x11Display();
 int screen = w->x11Screen();
 Window root = RootWindow(dpy, screen);

 XRRScreenConfiguration* delete_sc = 0;
 if (!sc) { // 0 is allowed for failsafe switch on destruction
	delete_sc = XRRGetScreenInfo(dpy, root);
	sc = delete_sc;
 }
 if (!sc) {
	boError() << k_funcinfo << "NULL screen config" << endl;
	return false;
 }
 Rotation currentRotation;
 XRRConfigRotations(sc, &currentRotation);
 Status stat = XRRSetScreenConfig(dpy, sc, root, mode,
		currentRotation, CurrentTime);
 if (delete_sc) {
	XRRFreeScreenConfigInfo(delete_sc);
 }
 if (stat == BadValue) {
	boError() << k_funcinfo << "unable to switch mode" << endl;
	return false;
 }
 return true;
}
#endif // HAVE_XFREE86_XRANDR

