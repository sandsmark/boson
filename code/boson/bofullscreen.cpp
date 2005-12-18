/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "../bomemory/bodummymemory.h"
#include <bodebug.h>

#include <kstaticdeleter.h>

#include <qstringlist.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qdesktopwidget.h>

#include <stdlib.h>
#include <config.h>

/**
 * This function makes sure that we enter the original screen mode/resolution
 * again when boson is quit. It is a function, not a method, so that it can
 * easily be called from certain points (such as atexit()).
 *
 * It forwards the request to enter the original mode to the particular
 * implementation (probably xrandr). Note that there may be some requirements on
 * when this may be called (e.g. existence of a main window).
 **/
static void bo_enter_orig_mode();

#ifdef HAVE_XRANDR
// TODO: use QLibrary instead of linking to Xrandr!
// -> no need for the #ifdef, less trouble for packagers, ...
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

static bool gUseXrandr = true;
static bool gXRROriginalModeValid = false;
static bool gXRRUseOriginalMode = false;
static SizeID gXRROriginalMode;
static Rotation gXRROriginalRotation;
XRRScreenConfiguration* gXRRScreenConfig = 0;

static bool bo_init_xrandr(Display* dpy, Window root);
static void bo_deinit_xrandr();
static void bo_xrr_enter_orig_mode();
static bool bo_xrr_enter_mode(int mode, XRRScreenConfiguration* sc);
#endif // HAVE_XRANDR

static KStaticDeleter<BoFullScreen> sd;
BoFullScreen* BoFullScreen::mBoFullScreen = 0;

class BoFullScreenPrivate
{
public:
	BoFullScreenPrivate()
	{
	}

	bool mInitialized;
};

BoFullScreen::BoFullScreen()
{
 d = new BoFullScreenPrivate;
 d->mInitialized = false;

 atexit(bo_enter_orig_mode);
}

BoFullScreen::~BoFullScreen()
{
 // reset to the original resolution asap.
 // will probably called twice, due to atexit()
 enterOriginalMode();

#ifdef HAVE_XRANDR
 bo_deinit_xrandr();
#endif // HAVE_XRANDR
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
#ifdef HAVE_XRANDR
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

 gUseXrandr = bo_init_xrandr(dpy, root);
 if (!gUseXrandr) {
	return;
 }
#endif // HAVE_XRANDR
}

QStringList BoFullScreen::availableModes()
{
 initStatic();
 if (mBoFullScreen) {
	return mBoFullScreen->availableModeList();
 }
 return QStringList();
}

void BoFullScreen::resizeToFullScreen(QWidget* w, int width, int height)
{
 BO_CHECK_NULL_RET(w);
 if (!w->isTopLevel()) {
	boError() << k_funcinfo << "w must be a toplevel widget" << endl;
	return;
 }
 w->reparent(0, QWidget::WType_TopLevel |
		QWidget::WStyle_Customize |
		QWidget::WStyle_NoBorder |
		QWidget::WDestructiveClose
		/* | w->getWFlags() & 0xffff0000*/,
		w->mapToGlobal(QPoint(0, 0)));
 w->resize(width, height);
 w->raise();
 w->show();
 w->setActiveWindow();
}

void BoFullScreen::leaveFullScreen()
{
 BO_CHECK_NULL_RET(qApp);
 QWidget* w = qApp->mainWidget();
 if (!w) {
	return;
 }
 if (!w->isTopLevel()) {
	boError() << k_funcinfo << "w must be a toplevel widget" << endl;
	return;
 }
 BoFullScreen::enterOriginalMode();
 w->reparent(0, QWidget::WType_TopLevel | QWidget::WDestructiveClose,
		QPoint(0, 0));
 w->show();
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
#ifdef HAVE_XRANDR
 if (!gUseXrandr) {
	return list;
 }
 if (!gXRRScreenConfig) {
	boError() << k_funcinfo << "NULL screen config" << endl;
	return list;
 }
 int count = 0;
 XRRScreenSize* sizes = XRRConfigSizes(gXRRScreenConfig, &count);
 if (count <= 0 || !sizes) {
	boWarning() << k_funcinfo << "oops - no sizes available" << endl;
	return list;
 }
 for (int i  = 0; i < count; i++) {
	QString s = QString("%1x%2").arg(sizes[i].width).arg(sizes[i].height);
	list.append(s);
 }
#endif // HAVE_XRANDR
 return list;
}

bool BoFullScreen::enterModeInList(int index)
{
 initModes();
 if (index < 0) {
	// go into "usual" QWidget::showFullScreen() mode
	// -> even if we cannot change resolution
	BoFullScreen::enterOriginalMode();
	if (!qApp) {
		BO_NULL_ERROR(qApp);
		return false;
	}
	if (!qApp->desktop()) {
		BO_NULL_ERROR(qApp->desktop());
		return false;
	}
	QWidget* w = qApp->mainWidget();
	if (!w) {
		BO_NULL_ERROR(w);
		return false;
	}
	QRect r = qApp->desktop()->screenGeometry(qApp->desktop()->screenNumber(w));

	resizeToFullScreen(w, r.width(), r.height());
	return true;
 }
#ifdef HAVE_XRANDR
 if (!gUseXrandr) {
	return false;
 }
 if (!gXRRScreenConfig) {
	BO_NULL_ERROR(gXRRScreenConfig);
	return false;
 }
 int count = 0;
 XRRScreenSize* sizes = XRRConfigSizes(gXRRScreenConfig, &count);
 if (count <= 0 || !sizes || index >= count) {
	boError() << k_funcinfo << "unable to switch to mode " << index << " count=" << count << endl;
	return false;
 }
 return bo_xrr_enter_mode(index, gXRRScreenConfig);
#endif // HAVE_XRANDR
 return false;
}

bool BoFullScreen::enterOriginalMode()
{
 bo_enter_orig_mode();
 return true;
}

static void bo_enter_orig_mode()
{
#ifdef HAVE_XRANDR
 bo_xrr_enter_orig_mode();
 return;
#endif // HAVE_XRANDR

}

#ifdef HAVE_XRANDR
static bool bo_init_xrandr(Display* dpy, Window root)
{
 boDebug() << k_funcinfo << endl;
 int event_base, error_base;
 if (!XRRQueryExtension(dpy, &event_base, &error_base)) {
	boWarning() << k_funcinfo << "Xrandr extension not available. you cannot change the resolution!" << endl;
	return false;
 }
 if (gXRRScreenConfig) {
	boDebug() << k_funcinfo << "xrandr already initialized!" << endl;
	return true;
 }
 gXRRScreenConfig = XRRGetScreenInfo(dpy, root);
 if (!gXRRScreenConfig) {
	boWarning() << k_funcinfo << "NULL screen config. Xrandr disabled" << endl;
	return false;
 }
 gXRROriginalMode = XRRConfigCurrentConfiguration(gXRRScreenConfig, &gXRROriginalRotation);
 gXRROriginalModeValid = true;
 return true;
}

static void bo_deinit_xrandr()
{
 boDebug() << k_funcinfo << endl;
 if (gXRRScreenConfig) {
	XRRFreeScreenConfigInfo(gXRRScreenConfig);
 }
 gXRRScreenConfig = 0;
 gXRROriginalModeValid = false;
}

static void bo_xrr_enter_orig_mode()
{
 if (!gXRROriginalModeValid) {
	return;
 }
 if (!gXRRUseOriginalMode) {
	// we never changed the original mode, or already changed back!
	return;
 }
 boDebug() << k_funcinfo << "entering mode " << gXRROriginalMode << endl;
 bo_xrr_enter_mode(gXRROriginalMode, gXRRScreenConfig);
}

static bool bo_xrr_enter_mode(int mode, XRRScreenConfiguration* sc)
{
 boDebug() << k_funcinfo << "entering mode " << mode << endl;
 if (!gUseXrandr) {
	return false;
 }
 if (mode < 0) {
	return false;
 }
 if (!qApp) {
	BO_NULL_ERROR(qApp);
	return false;
 }
 Display* dpy = 0;
 Window root;
 QWidget* w = qApp->mainWidget();
 if (!w) {
	// AB: I don't know whether the x11Display() of the main widget can
	// differ from the x11AppDisplay(), so we try the x11Display() if the
	// widget if possible, but also use the appdisplay if necessary
	// (fallback)
	dpy = QPaintDevice::x11AppDisplay();
	root = QPaintDevice::x11AppRootWindow();
 } else {
	dpy = w->x11Display();
	int screen = w->x11Screen();
	root = RootWindow(dpy, screen);
 }
 if (!dpy) {
	boError() << k_funcinfo << "cannot get a usable display" << endl;
	return false;
 }

 XRRScreenConfiguration* delete_sc = 0;
 if (!sc) { // 0 is allowed for failsafe switch on destruction
	delete_sc = XRRGetScreenInfo(dpy, root);
	sc = delete_sc;
 }
 if (!sc) {
	boError() << k_funcinfo << "NULL screen config" << endl;
	return false;
 }
 Status stat = XRRSetScreenConfig(dpy, sc, root, mode,
		gXRROriginalRotation, CurrentTime);
 if (delete_sc) {
	XRRFreeScreenConfigInfo(delete_sc);
 }
 if (stat == BadValue) {
	boError() << k_funcinfo << "unable to switch mode" << endl;
	gXRRUseOriginalMode = true;
	return false;
 }
 if (gXRROriginalMode == mode) {
	gXRRUseOriginalMode = false;
 } else {
	gXRRUseOriginalMode = true;
 }
 if (w) {
	// make it "fullscreen" as in w->showFullScreen(), i.e. make it
	// fill the entire screen (but no more!)
	// AB: we cannot use showFullScreen() directly here, as maybe
	// Qt/KDE doesn't support Xrandr

	int count;
	XRRScreenSize* sizes = XRRConfigSizes(sc, &count);
	if (mode >= count) {
		boError() << k_funcinfo << "oops - something weird happened!" << endl;
		bo_xrr_enter_orig_mode();
		return false;
	}
	int width = sizes[mode].width;
	int height = sizes[mode].height;

	BoFullScreen::resizeToFullScreen(w, width, height);
 }
 return true;
}
#endif // HAVE_XRANDR

