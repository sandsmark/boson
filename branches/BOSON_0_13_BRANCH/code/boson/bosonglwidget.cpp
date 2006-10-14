/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include "bosonglwidget.h"
#include "bosonglwidget.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"

#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <bogl.h>
#include <boglx.h>
#include <qgl.h> // convertToGLFormat()
#include <qimage.h> // convertToGLFormat()
#include <qpaintdevicemetrics.h>
#include <qintdict.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>

class BoPixmap : public QPixmap
{
public:
	QPaintDeviceX11Data* getX11DataFoo(bool def) const
	{
		return getX11Data(def);
	}
	void setX11DataFoo(const QPaintDeviceX11Data* data)
	{
		setX11Data(data);
	}
};

static void *gl_pixmap_visual = 0;

#define USE_COLORMAP 1

#if USE_COLORMAP
struct _CMapEntry {
    _CMapEntry();
   ~_CMapEntry();
    Colormap		cmap;
    bool		alloc;
    XStandardColormap	scmap;
};

_CMapEntry::_CMapEntry()
{
    cmap = 0;
    alloc = FALSE;
    scmap.colormap = 0;
}

_CMapEntry::~_CMapEntry()
{
    if ( alloc )
	XFreeColormap( QPaintDevice::x11AppDisplay(), cmap );
}

static QIntDict<_CMapEntry> *cmap_dict = 0;
static bool		    mesa_gl   = FALSE;

static void cleanup_cmaps()
{
    if ( !cmap_dict )
	return;
    cmap_dict->setAutoDelete( TRUE );
    delete cmap_dict;
    cmap_dict = 0;
}

static Colormap choose_cmap( Display *dpy, XVisualInfo *vi )
{
 if ( !cmap_dict ) {
	cmap_dict = new QIntDict<_CMapEntry>;
	const char *v = glXQueryServerString(dpy, vi->screen, GLX_VERSION);
	if ( v ) {
		mesa_gl = strstr(v,"Mesa") != 0;
	}
	qAddPostRoutine( cleanup_cmaps );
 }

 _CMapEntry *x = cmap_dict->find( (long) vi->visualid + ( vi->screen * 256 ) );
 if ( x ) {					// found colormap for visual
	return x->cmap;
 }

    x = new _CMapEntry();

    XStandardColormap *c;
    int n, i;

    // qDebug( "Choosing cmap for vID %0x", vi->visualid );

    if ( vi->visualid ==
	 XVisualIDFromVisual( (Visual*)QPaintDevice::x11AppVisual( ) ) ) {
	// qDebug( "Using x11AppColormap" );
	return QPaintDevice::x11AppColormap( );
    }

    if ( mesa_gl ) {				// we're using MesaGL
	Atom hp_cmaps = XInternAtom( dpy, "_HP_RGB_SMOOTH_MAP_LIST", TRUE );
	if ( hp_cmaps && vi->visual->c_class == TrueColor && vi->depth == 8 ) {
	    if ( XGetRGBColormaps(dpy,RootWindow(dpy,vi->screen),&c,&n,
				  hp_cmaps) ) {
		i = 0;
		while ( i < n && x->cmap == 0 ) {
		    if ( c[i].visualid == vi->visual->visualid ) {
			x->cmap = c[i].colormap;
			x->scmap = c[i];
			//qDebug( "Using HP_RGB scmap" );

		    }
		    i++;
		}
		XFree( (char *)c );
	    }
	}
    }
#if !defined(Q_OS_SOLARIS)
    if ( !x->cmap ) {
	if ( XmuLookupStandardColormap(dpy,vi->screen,vi->visualid,vi->depth,
				       XA_RGB_DEFAULT_MAP,FALSE,TRUE) ) {
	    if ( XGetRGBColormaps(dpy,RootWindow(dpy,vi->screen),&c,&n,
				  XA_RGB_DEFAULT_MAP) ) {
		i = 0;
		while ( i < n && x->cmap == 0 ) {
		    if ( c[i].visualid == vi->visualid ) {
			x->cmap = c[i].colormap;
			x->scmap = c[i];
			//qDebug( "Using RGB_DEFAULT scmap" );
		    }
		    i++;
		}
		XFree( (char *)c );
	    }
	}
    }
#endif
    if ( !x->cmap ) {				// no shared cmap found
	x->cmap = XCreateColormap( dpy, RootWindow(dpy,vi->screen), vi->visual,
				   AllocNone );
	x->alloc = TRUE;
	// qDebug( "Allocating cmap" );
    }

    // associate cmap with visualid
    cmap_dict->insert((long)vi->visualid + (vi->screen * 256), x);
    return x->cmap;
}

#endif // USE_COLORMAP


BoContext* BoContext::mCurrentContext = 0;

class BoContext::BoContextPrivate
{
public:
	BoContextPrivate()
	{
		mContext = 0;
		mGLXPixmap = 0;
	}
	GLXContext mContext;
	Q_UINT32 mGLXPixmap;
};

BoContext::BoContext(QPaintDevice* device)
{
 mIsInitialized = false;
 mPaintDevice = device;
 mVi = 0;
 mValid = false;
 mPlane = 0;
 mDepth = 0;
 mAlphaSize = 0;
 mAccumRedSize = 0;
 mStencilSize = 0;
 mStereo = false;
 mDirect = false;
 d = new BoContextPrivate;
 if (!mPaintDevice) {
	boError() << k_funcinfo << "NULL paint device" << endl;
	return;
 }
 if (mPaintDevice->devType() != QInternal::Widget && mPaintDevice->devType() != QInternal::Pixmap) {
	boError() << k_funcinfo << "context device must be a widget or a pixmap" << endl;
	return;
 }
}

BoContext::~BoContext()
{
 boDebug() << k_funcinfo << endl;
 doneCurrent();
 boDebug() << k_funcinfo << "destroy glx context" << endl;
 glXDestroyContext(mPaintDevice->x11Display(), d->mContext);
 boDebug() << k_funcinfo << "glx context destroyed" << endl;
 if (mVi) {
	XFree(mVi);
 }
 d->mContext = 0;
 mVi = 0;
 delete d;
 boDebug() << k_funcinfo << "done" << endl;
}

bool BoContext::create(bool wantDirect, bool wantDoubleBuffer)
{
 mValid = chooseContext(wantDirect, wantDoubleBuffer);
 return mValid;
}

void BoContext::swapBuffers()
{
 if (!isValid()) {
	boError() << k_funcinfo << "invalid context" << endl;
	return;
 }
 glXSwapBuffers(mPaintDevice->x11Display(), ((QWidget*)mPaintDevice)->winId());
}

void BoContext::makeCurrent()
{
 if (!isValid()) {
	boError() << k_funcinfo << "invalid context" << endl;
	return;
 }
 bool ok = true;
 if (deviceIsPixmap()) {
	ok = glXMakeCurrent(mPaintDevice->x11Display(),
			(GLXPixmap)d->mGLXPixmap,
			(GLXContext)d->mContext);
 } else {
	ok = glXMakeCurrent(mPaintDevice->x11Display(),
			((QWidget*)mPaintDevice)->winId(),
			(GLXContext)d->mContext);
 }
 if (ok) {
	mCurrentContext = this;
 } else {
	boWarning() << k_funcinfo << "failed" << endl;
 }
}

void BoContext::doneCurrent()
{
 glXMakeCurrent(mPaintDevice->x11Display(), 0, 0);
 mCurrentContext = 0;
}

// AB: shamelessy stolen from QGLContext
void* BoContext::tryVisual(const QGLFormat& f)
{
 int spec[40];
 int i = 0;
 spec[i++] = GLX_LEVEL;
 spec[i++] = f.plane();

#if defined(GLX_VERSION_1_1) && defined(GLX_EXT_visual_info)
 static bool useTranspExt = FALSE;
 static bool useTranspExtChecked = FALSE;
 if (f.plane() && !useTranspExtChecked && mPaintDevice) {
	QCString estr( glXQueryExtensionsString( mPaintDevice->x11Display(),
						 mPaintDevice->x11Screen()));
	useTranspExt = estr.contains( "GLX_EXT_visual_info" );
	//# (A bit simplistic; that could theoretically be a substring)
	if ( useTranspExt ) {
		QCString cstr(glXGetClientString(mPaintDevice->x11Display(),
				GLX_VENDOR));
		useTranspExt = !cstr.contains( "Xi Graphics" ); // bug workaround
		if ( useTranspExt ) {
			// bug workaround - some systems (eg. FireGL) refuses to return an overlay
			// visual if the GLX_TRANSPARENT_TYPE_EXT attribute is specfied, even if 
			// the implementation supports transparent overlays
			int tmpSpec[] = { GLX_LEVEL, f.plane(), GLX_TRANSPARENT_TYPE_EXT,
					GLX_TRANSPARENT_INDEX_EXT, None };
			XVisualInfo * vinf = glXChooseVisual(mPaintDevice->x11Display(),
					mPaintDevice->x11Screen(), tmpSpec );
		if ( !vinf ) {
			useTranspExt = false;
		}
	    }
	}
	useTranspExtChecked = true;
 }
 if ( f.plane() && useTranspExt ) {
	// Required to avoid non-transparent overlay visual(!) on some systems
	spec[i++] = GLX_TRANSPARENT_TYPE_EXT;
	spec[i++] = GLX_TRANSPARENT_INDEX_EXT; //# Depending on format, really
 }
#endif

 if (f.doubleBuffer()) {
	spec[i++] = GLX_DOUBLEBUFFER;
 }
 spec[i++] = GLX_DEPTH_SIZE;
 spec[i++] = 1;

 if ( f.stereo() ) {
	spec[i++] = GLX_STEREO;
 }
 if ( f.stencil() ) {
	spec[i++] = GLX_STENCIL_SIZE;
	spec[i++] = 1;
 }

 {
	// RGBA stuff
	spec[i++] = GLX_RGBA;
	spec[i++] = GLX_RED_SIZE;
	spec[i++] = 1;
	spec[i++] = GLX_GREEN_SIZE;
	spec[i++] = 1;
	spec[i++] = GLX_BLUE_SIZE;
	spec[i++] = 1;
	if ( f.alpha() ) {
		spec[i++] = GLX_ALPHA_SIZE;
		spec[i++] = 1;
	}
	if ( f.accum() ) {
		spec[i++] = GLX_ACCUM_RED_SIZE;
		spec[i++] = 1;
		spec[i++] = GLX_ACCUM_GREEN_SIZE;
		spec[i++] = 1;
		spec[i++] = GLX_ACCUM_BLUE_SIZE;
		spec[i++] = 1;
		if ( f.alpha() ) {
			spec[i++] = GLX_ACCUM_ALPHA_SIZE;
			spec[i++] = 1;
		}
	}
 }

 spec[i] = None;
 return glXChooseVisual(mPaintDevice->x11Display(),
		mPaintDevice->x11Screen(), spec);
}

// AB: shamelessy stolen from QGLContext
void* BoContext::chooseVisual(bool wantDoubleBuffer)
{
 QGLFormat fmt;
 fmt.setDoubleBuffer(wantDoubleBuffer);
 void* vis = 0;
 bool fail = FALSE;
 bool tryDouble = !fmt.doubleBuffer();  // Some GL impl's only have double
 bool triedDouble = FALSE;
 while( !fail && !( vis = tryVisual( fmt ) ) ) {
	if ( tryDouble ) {
		fmt.setDoubleBuffer( TRUE );
		tryDouble = FALSE;
		triedDouble = TRUE;
		continue;
	} else if ( triedDouble ) {
		fmt.setDoubleBuffer( FALSE );
		triedDouble = FALSE;
	}
	if ( fmt.stereo() ) {
		fmt.setStereo( FALSE );
		continue;
	}
	if ( fmt.accum() ) {
		fmt.setAccum( FALSE );
		continue;
	}
	if ( fmt.stencil() ) {
		fmt.setStencil( FALSE );
		continue;
	}
	fail = true;
 }
 return vis;
}

bool BoContext::chooseContext(bool wantDirect, bool wantDoubleBuffer)
{
 if (!mPaintDevice) {
	boError() << k_funcinfo << "NULL paint device" << endl;
	return false;
 }
 Display* disp = mPaintDevice->x11Display();
 mVi = chooseVisual(wantDoubleBuffer);
 if ( !mVi ) {
	boError() << k_funcinfo << "NULL visual returned by chooseVisual()" << endl;
	return false;
 }
 if (deviceIsPixmap() &&
		(((XVisualInfo*)mVi)->depth != mPaintDevice->x11Depth() ||
		((XVisualInfo*)mVi)->screen != mPaintDevice->x11Screen())) {
	// AB: copied from Qt 3.2.0
	boWarning() << "ab1 code" << endl;
	XFree(mVi);
	XVisualInfo appVisInfo;
	memset(&appVisInfo, 0, sizeof(XVisualInfo));
	appVisInfo.visualid = XVisualIDFromVisual( (Visual*)mPaintDevice->x11Visual() );
	appVisInfo.screen = mPaintDevice->x11Screen();
	int nvis;
	mVi = XGetVisualInfo(disp, VisualIDMask | VisualScreenMask, &appVisInfo, &nvis);
	if (!mVi) {
		boError() << k_funcinfo << "NULL visual for painting to pixmap" << endl;
		return false;
	}
	int useGL;
	glXGetConfig( disp, (XVisualInfo*)mVi, GLX_USE_GL, &useGL );
	if (!useGL) {
		boError() << k_funcinfo << "cannot use GL for pixmap ?!" << endl;
		return false;
	}
 }


 int res;
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_LEVEL, &res );
 mPlane = res;
// glFormat.setPlane( res );
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_DOUBLEBUFFER, &res );
 if (!res && wantDoubleBuffer) {
	KMessageBox::error(0, i18n("Double buffering is not supported by your system. Exit now"));
	kapp->exit(1);
	return false;
 } else if (!wantDoubleBuffer && res) {
	boWarning() << k_funcinfo << "requested non-double buffer, but double buffering is enabled!" << endl;
 }
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_DEPTH_SIZE, &res );
 mDepth = res;
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_RGBA, &res );
 if (!res) {
	KMessageBox::error(0, i18n("RGBA is not supported by your system. Exit now"));
	kapp->exit(1);
	return false;
 }
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_ALPHA_SIZE, &res );
 mAlphaSize = res;
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_ACCUM_RED_SIZE, &res );
 mAccumRedSize = res;
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_STENCIL_SIZE, &res );
 mStencilSize = res;
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_STEREO, &res );
 mStereo = res;

 d->mContext = glXCreateContext( disp, (XVisualInfo*)mVi, None, wantDirect);
 if (!d->mContext)  {
	boError() << k_funcinfo << "NULL context created" << endl;
	return false;
 }
 mDirect = glXIsDirect(disp, d->mContext);

 if (deviceIsPixmap()) {
	d->mGLXPixmap = (Q_UINT32)glXCreateGLXPixmap(disp, (XVisualInfo*)mVi,
			mPaintDevice->handle());
	if (!d->mGLXPixmap) {
		boError() << k_funcinfo << "glXCreateGLXPixmap() returned NULL pixmap" << endl;
		return false;
	}
 }
 return true;
}

class BosonGLWidget::BosonGLWidgetPrivate
{
public:
	BosonGLWidgetPrivate()
	{
		mContext = 0;
	}
	BoContext* mContext;

	bool mWantDirect;
};

// WNoAutoErase: Qt 3.2
BosonGLWidget::BosonGLWidget(QWidget* parent, const char* name, bool direct)
#if QT_VERSION >= 0x030200
	: QWidget(parent, name, Qt::WNoAutoErase)
#else
	: QWidget(parent, name, Qt::WRepaintNoErase)
#endif
{
 d = new BosonGLWidgetPrivate;
 d->mWantDirect = direct;
 init();
}

BosonGLWidget::~BosonGLWidget()
{
 delete d->mContext;
 delete d;
}

void BosonGLWidget::init()
{
 setBackgroundMode(NoBackground);
 mAutoSwap = true;
 setContext(new BoContext(this));
}

void BosonGLWidget::makeCurrent()
{
 d->mContext->makeCurrent();
}

void BosonGLWidget::reparent(QWidget* parent, WFlags f, const QPoint& p, bool showIt)
{
 // AB: shamelessy stolen from QGLWidget::reparent()
 // ### Another work-around for the Utah-GLX driver -
 // ### if the old context is not destroyed before the window is
 // ### reparented, it crashes badly (driver v0.10 November 2000)
 static bool utahGLX = QString( glXGetClientString( x11Display(),
			GLX_EXTENSIONS ) ).contains( "GLX_utah" );
 if ( utahGLX ) {
	delete d->mContext;
	QWidget::reparent( parent, f, p, FALSE );
	setContext( new BoContext(this));
	QWidget::reparent( parent, f, p, FALSE );
 } else {
	QWidget::reparent( parent, f, p, FALSE );
 }

 if ( showIt ) {
	show();
 }
}

void BosonGLWidget::resizeEvent(QResizeEvent*)
{
 if (!isValid()) {
	boError() << k_funcinfo << "Invalid GL widget!" << endl;
	return;
 }
 makeCurrent();
 if (!isInitialized()) {
	initGL();
 }
 glXWaitX(); // AB: do we need this? what is this?
 resizeGL(width(), height());
}

void BosonGLWidget::slotUpdateGL()
{
 if (!isValid()) {
	boError() << k_funcinfo << "Invalid GL widget! Cannot draw scene" << endl;
	return;
 }
 makeCurrent();
 if (context()->deviceIsPixmap()) {
	glDrawBuffer(GL_FRONT_LEFT);
 }
 if (!isInitialized()) {
	initGL();
	//AB: see QGLWidget::glDraw()
	QPaintDeviceMetrics dm(this);
	resizeGL(dm.width(), dm.height());
 }
 paintGL();
 if (!context()->deviceIsPixmap()) {
	// non-pixmap devices are _always_ double buffered for us!
	if (autoBufferSwap()) {
		swapBuffers();
	}
 } else {
	// no doublebuffering.
	glFlush();
 }
}

bool BosonGLWidget::isValid() const
{
 if (!context()) {
	boDebug() << k_funcinfo << "NULL context" << endl;
	return false;
 }
 return d->mContext->isValid();
}

void BosonGLWidget::initGL()
{
 if (!isValid()) {
	boError() << k_funcinfo << "Invalid GL widget! Cannot initialize" << endl;
	return;
 }
 makeCurrent();
 initializeGL();
 context()->setIsInitialized(true);

 resizeGL(width(), height());

 QString brokenDriver = glDriverBroken();
 if (!brokenDriver.isEmpty()) {
	boWarning() << "Possibly broken GL driver detected. Error message: " << brokenDriver << endl;
 }
}

bool BosonGLWidget::directRendering() const
{
 if (!d->mContext) {
	return false;
 }
 return d->mContext->isDirect();
}

QString BosonGLWidget::glDriverBroken()
{
 if (!isValid()) {
	boError() << k_funcinfo << "Invalid GL widget!" << endl;
	return false;
 }
 makeCurrent();
 if (!isInitialized()) {
	initGL();
 }
 QString GLvendor = BoGL::bogl()->OpenGLVendorString();
 QString GLXvendor = (const char*)glXGetClientString(x11Display(), GLX_VENDOR);
 bool GLIsNVidia = GLvendor.lower().contains("nvidia");
 bool GLXIsNVidia = GLXvendor.lower().contains("nvidia");

 if (GLIsNVidia != GLXIsNVidia) {
	if (GLIsNVidia) {
		return i18n("Vendor of GL driver is NVidia, but vendor of GLX driver is not. This may be caused by a wrong (e.g. MESA based) libGL.so library");
	} else if (GLXIsNVidia) {
		return i18n("Vendor of GL driver is not NVidia, but vendor of GLX driver is. This may be caused by a wrong libGL.so library");
	}
 }

 return QString::null;
}

void BosonGLWidget::swapBuffers()
{
 d->mContext->swapBuffers();
}

QImage BosonGLWidget::convertToGLFormat(const QImage& img)
{
 return QGLWidget::convertToGLFormat(img);
}

BoContext* BosonGLWidget::context() const
{
 return d->mContext;
}

bool BosonGLWidget::switchContext(BoContext* newContext)
{
 if (!newContext) {
	BO_NULL_ERROR(newContext);
	return false;
 }
 if (!newContext->isValid()) {
	boError() << k_funcinfo << "new context is not a valid context" << endl;
	return false;
 }

 if (context() && BoContext::currentContext() == context()) {
	context()->doneCurrent();
 }

 // warning: the old context is not deleted! we leave this to the user!
 // we will probably switch back to the old context later.
 d->mContext = newContext;

 if (context()) {
	context()->makeCurrent();
 }
 return true;
}

void BosonGLWidget::setContext(BoContext* context)
{
 if (!context) {
	boError() << k_funcinfo << "NULL context" << endl;
	return;
 }
 if (this->context()) {
	boWarning() << k_funcinfo << "already a context present! deleting it now" << endl;
	delete d->mContext;
	d->mContext = 0;
 }
 d->mContext = context;

 bool createFailed = !d->mContext->create(d->mWantDirect);
 if (createFailed) {
	boError() << k_funcinfo << "Could not create OpenGL context" << endl;
	return;
 }
 bool visible = isVisible();
 if (visible) {
	hide();
 }

#if USE_COLORMAP
 // AB: QGLWidget uses this code. I am not sure whether we need this - it
 // works for me without as well. ( i am referring to the colormap stuff )
 XVisualInfo *vi = (XVisualInfo*)d->mContext->mVi;
 XSetWindowAttributes a;

 a.colormap = choose_cmap(x11Display(), vi);	// find best colormap
 a.background_pixel = backgroundColor().pixel();
 a.border_pixel = black.pixel();
 Window p = RootWindow( x11Display(), vi->screen );
 if (parentWidget()) {
	p = parentWidget()->winId();
 }

 Window w = XCreateWindow(x11Display(), p,  x(), y(), width(), height(),
		0, vi->depth, InputOutput, vi->visual,
		CWBackPixel|CWBorderPixel|CWColormap, &a);

 Window *cmw;
 Window *cmwret;
 int count;
 if (XGetWMColormapWindows( x11Display(), topLevelWidget()->winId(),
		&cmwret, &count)) {
	cmw = new Window[count+1];
	memcpy( (char *)cmw, (char *)cmwret, sizeof(Window)*count );
	XFree( (char *)cmwret );
	int i;
	for (i=0; i<count; i++) {
		if ( cmw[i] == winId() ) {		// replace old window
			cmw[i] = w;
			break;
		}
	}
	if (i >= count) {			// append new window
		cmw[count++] = w;
	}
 } else {
	count = 1;
	cmw = new Window[count];
	cmw[0] = w;
 }

#if defined(GLX_MESA_release_buffers) && defined(QGL_USE_MESA_EXT)
 if (oldcx && oldcx->windowCreated()) {
	glXReleaseBuffersMESA(x11Display(), winId());
 }
#endif

 create(w);

 XSetWMColormapWindows(x11Display(), topLevelWidget()->winId(), cmw,
		count);
 delete [] cmw;
#endif // USE_COLORMAP

 if (visible) {
	show();
 }
 XFlush(x11Display());
}

void BosonGLWidget::paintEvent(QPaintEvent*)
{
 static bool resized = false;
 if (!resized) {
	// some drivers have initialization problems. they require a resize as
	// soon as the widget is visible, in order to work correctly.
	// otherwise e.g. glDrawPixels() won't work correctly (e.g. for my
	// tdfx driver) - dunno why.
	int w = width();
	int h = height();

	makeCurrent();
	resizeGL(w - 1, h - 1);
	resizeGL(w, h);
	resized = true;
 }
 slotUpdateGL();
}

// TODO: current QGLWidget has a method grabFrameBuffer() that also handles
// endianness!
// -> we should use that one instead of our own here!
QImage BosonGLWidget::screenShot()
{
 glFinish();
 int w = width();
 int h = height();
 unsigned char* buffer = new unsigned char[w * h * 4];
 glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
 QImage image(w, h, 32);
 for (int y = 0; y < h; y++) {
	QRgb* line = (QRgb*)image.scanLine(y); // AB: use setPixel() instead of scanLine() ! -> endianness must be handled
	int opengl_y = h - y;
	for (int x = 0; x < w; x++) {
		unsigned char* pixel = &buffer[(opengl_y * w + x) * 4];
		line[x] = qRgb(pixel[0], pixel[1], pixel[2]);
	}
 }
 delete[] buffer;
 return image;
}


QPixmap BosonGLWidget::renderPixmap(int w, int h, bool useContext)
{
 QSize sz = size();
 if ( (w > 0) && (h > 0) ) {
	sz = QSize( w, h );
 }
 BoPixmap pm;
 pm.resize( sz );

#if defined(Q_WS_X11)
 // If we are using OpenGL widgets we HAVE to make sure that
 // the default visual is GL enabled, otherwise it will wreck
 // havock when e.g trying to render to GLXPixmaps via
 // QPixmap. This is because QPixmap is always created with a
 // QPaintDevice that uses x_appvisual per default. Preferably,
 // use a visual that has depth and stencil buffers.

 if (!gl_pixmap_visual) {
	int nvis;
	Visual *vis = (Visual *) QPaintDevice::x11AppVisual();
	int screen = QPaintDevice::x11AppScreen();
	Display *appDpy = QPaintDevice::x11AppDisplay();
	XVisualInfo * vi;
	XVisualInfo visInfo;
	memset( &visInfo, 0, sizeof(XVisualInfo) );
	visInfo.visualid = XVisualIDFromVisual( vis );
	visInfo.screen = screen;
	vi = XGetVisualInfo( appDpy, VisualIDMask | VisualScreenMask, &visInfo, &nvis );
	if ( vi ) {
		int useGL;
		int ret = glXGetConfig( appDpy, vi, GLX_USE_GL, &useGL );
		if ( ret != 0 || !useGL ) {
			// We have to find another visual that is GL capable
			int i;
			XVisualInfo * visuals;
			memset( &visInfo, 0, sizeof(XVisualInfo) );
			visInfo.screen = screen;
			visInfo.c_class = vi->c_class;
			visInfo.depth = vi->depth;
			visuals = XGetVisualInfo( appDpy, VisualClassMask |
					  VisualDepthMask |
					  VisualScreenMask, &visInfo,
					  &nvis );
			if ( visuals ) {
				for ( i = 0; i < nvis; i++ ) {
					int ret = glXGetConfig( appDpy, &visuals[i], GLX_USE_GL, &useGL );
					if ( ret == 0 && useGL ) {
						vis = visuals[i].visual;
						break;
					}
				}
				XFree( visuals );
			}
		}
		XFree( vi );
	}
	gl_pixmap_visual = vis;
 }

 if (gl_pixmap_visual != QPaintDevice::x11AppVisual()) {
	int nvis = 0;
	XVisualInfo visInfo;
	memset( &visInfo, 0, sizeof(XVisualInfo) );
	visInfo.visualid = XVisualIDFromVisual( (Visual *) gl_pixmap_visual );
	visInfo.screen = QPaintDevice::x11AppScreen();
	XVisualInfo *vi = XGetVisualInfo( QPaintDevice::x11AppDisplay(), VisualIDMask | VisualScreenMask,
					  &visInfo, &nvis );
	if (vi) {
		QPaintDeviceX11Data* xd = pm.getX11DataFoo( TRUE );
		xd->x_depth = vi->depth;
		xd->x_visual = (Visual *) gl_pixmap_visual;
		pm.setX11DataFoo( xd );
		XFree(vi);
	}
 }
#else
#error foo
#endif

 d->mContext->doneCurrent();

 bool success = TRUE;

 if ( useContext && isValid() && renderCxPm( &pm ) ) {
	return pm;
 }

 BoContext* ocx = d->mContext;
 bool wasCurrent = (BoContext::currentContext() == ocx );
 ocx->doneCurrent();
#if 0
 // AB: QGLWidget:
 QGLFormat fmt = d->mContext->requestedFormat();
 fmt.setDirectRendering( FALSE );		// Direct is unlikely to work
 fmt.setDoubleBuffer( FALSE );		// We don't need dbl buf
 d->mContext = new BoContext( fmt, &pm );
#else
 d->mContext = new BoContext( &pm );
#endif
 d->mContext->create(false, false);

 if ( d->mContext->isValid() ) {
	slotUpdateGL();
 } else {
	success = FALSE;
 }

 delete d->mContext;
 d->mContext = ocx;

 if ( wasCurrent ) {
	ocx->makeCurrent();
 }

 if ( success ) {
#if defined(Q_WS_X11)
	if (gl_pixmap_visual != QPaintDevice::x11AppVisual()) {
		QImage image = pm.convertToImage();
		QPixmap p;
		p = image;
		return p;
	}
#endif
	return pm;
 } else {
	return QPixmap();
 }
}

bool BosonGLWidget::renderCxPm( QPixmap* pm )
{
  if ( ((XVisualInfo*)d->mContext->mVi)->depth != pm->depth() ) {
	return FALSE;
  }

 GLXPixmap glPm;
#if defined(GLX_MESA_pixmap_colormap) && defined(QGL_USE_MESA_EXT)
 glPm = glXCreateGLXPixmapMESA( x11Display(),
				(XVisualInfo*)d->mContext->mVi,
				(Pixmap)pm->handle(),
				choose_cmap( pm->x11Display(),
				(XVisualInfo*)d->mContext->mVi ) );
#else
 glPm = (Q_UINT32)glXCreateGLXPixmap( x11Display(),
					(XVisualInfo*)d->mContext->mVi,
					(Pixmap)pm->handle() );
#endif

 if ( !glXMakeCurrent( x11Display(), glPm, (GLXContext)d->mContext->d->mContext ) ) {
	glXDestroyGLXPixmap( x11Display(), glPm );
	return FALSE;
 }

 glDrawBuffer( GL_FRONT );
 if ( !d->mContext->isInitialized() ) {
	initGL();
 }
 resizeGL( pm->width(), pm->height() );
 paintGL();
 glFlush();
 makeCurrent();
 glXDestroyGLXPixmap( x11Display(), glPm );
 resizeGL( width(), height() );
 return TRUE;
}


