/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonglwidget.h"
#include "bosonglwidget.moc"

#include "bodebug.h"

#include <kapplication.h>

#include <qgl.h> // convertToGLFormat()
#include <qimage.h> // convertToGLFormat()
#include <qpaintdevicemetrics.h>
#include <qintdict.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>

#define USE_COLORMAP 1

#if USE_COLORMAP
struct CMapEntry {
    CMapEntry();
   ~CMapEntry();
    Colormap		cmap;
    bool		alloc;
    XStandardColormap	scmap;
};

CMapEntry::CMapEntry()
{
    cmap = 0;
    alloc = FALSE;
    scmap.colormap = 0;
}

CMapEntry::~CMapEntry()
{
    if ( alloc )
	XFreeColormap( QPaintDevice::x11AppDisplay(), cmap );
}

static QIntDict<CMapEntry> *cmap_dict = 0;
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
	cmap_dict = new QIntDict<CMapEntry>;
	const char *v = glXQueryServerString(dpy, vi->screen, GLX_VERSION);
	if ( v ) {
		mesa_gl = strstr(v,"Mesa") != 0;
	}
	qAddPostRoutine( cleanup_cmaps );
 }

 CMapEntry *x = cmap_dict->find( (long) vi->visualid + ( vi->screen * 256 ) );
 if ( x ) {					// found colormap for visual
	return x->cmap;
 }

    x = new CMapEntry();

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
	}
	GLXContext mContext;
};

BoContext::BoContext(QPaintDevice* device)
{
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
 if (mPaintDevice->devType() != QInternal::Widget) {
	boError() << k_funcinfo << "context device must be a widget" << endl;
	return;
 }
}

BoContext::~BoContext()
{
 boDebug() << k_funcinfo << endl;
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

bool BoContext::create()
{
 mValid = chooseContext();
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
 glXMakeCurrent(mPaintDevice->x11Display(), ((QWidget*)mPaintDevice)->winId(), d->mContext);
 mCurrentContext = this;
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

 spec[i++] = GLX_DOUBLEBUFFER;
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
void* BoContext::chooseVisual()
{
 QGLFormat fmt;
 fmt.setDoubleBuffer(true);
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
	}
	else if ( triedDouble ) {
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
bool BoContext::chooseContext()
{
 if (!mPaintDevice) {
	boError() << k_funcinfo << "NULL paint device" << endl;
	return false;
 }
 Display* disp = mPaintDevice->x11Display();
 mVi = chooseVisual();
 if ( !mVi ) {
	return false;
 }

 int res;
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_LEVEL, &res );
 mPlane = res;
// glFormat.setPlane( res );
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_DOUBLEBUFFER, &res );
 if (!res) {
	// TODO: msg box!
	boError() << k_funcinfo << "double buffer is not supported! exit now.." << endl;
	kapp->exit(1);
	return false;
 }
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_DEPTH_SIZE, &res );
 mDepth = res;
 glXGetConfig( disp, (XVisualInfo*)mVi, GLX_RGBA, &res );
 if (!res) {
	// TODO: msg box!
	boError() << k_funcinfo << "RGBA not available - color index is NOT supported by boson!" << endl;
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

 d->mContext = 0;
 bool wantDirect = true; // for debugging false might make sense. also for optimizing software rendering?
 if (!d->mContext) {
	d->mContext = glXCreateContext( disp, (XVisualInfo *)mVi, None, wantDirect);
 }
 if (!d->mContext)  {
	boError() << k_funcinfo << "NULL context created" << endl;
	return false;
 }
 mDirect = glXIsDirect(disp, d->mContext);
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
};

BosonGLWidget::BosonGLWidget(QWidget* parent) : QWidget(parent)
{
 d = new BosonGLWidgetPrivate;
 boDebug() << k_funcinfo << endl;
 init();
}

BosonGLWidget::~BosonGLWidget()
{
 boDebug() << k_funcinfo << "delete context" << endl;
 delete d->mContext;
 boDebug() << k_funcinfo << "context deleted" << endl;
 delete d;
}

void BosonGLWidget::init()
{
 mInitialized = false; // see initGL()
 setBackgroundMode(NoBackground);
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
 if (!isInitialized()) {
	initGL();
	//AB: see QGLWidget::glDraw()
	QPaintDeviceMetrics dm(this);
	resizeGL(dm.width(), dm.height());
 }
 paintGL();
 swapBuffers();
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
 boDebug() << k_funcinfo << endl;
 makeCurrent();
 initializeGL();
 boDebug() << k_funcinfo << "done" << endl;
 mInitialized = true;

 // AB: this might even fix our resize-problem (see slotHack1() in BosonWidget)
 resizeGL(width(), height());
}

bool BosonGLWidget::directRendering() const
{
 if (!d->mContext) {
	return false;
 }
 return d->mContext->isDirect();
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

void BosonGLWidget::setContext(BoContext* context)
{
 boDebug() << k_funcinfo << endl;
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

 bool createFailed = !d->mContext->create();
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
 slotUpdateGL();
}

