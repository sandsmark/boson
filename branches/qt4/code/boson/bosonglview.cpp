/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonglview.h"
#include "bosonglview.moc"

#include "bodebug.h"
#include "bosonfpscounter.h"
#include <bogl.h>
#include <boglx.h>

#include <klocale.h>

#include <QGLWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>

class MyGLWidget : public QGLWidget
{
public:
	MyGLWidget(const QGLFormat& format, QWidget* parent = 0)
		: QGLWidget(format, parent)
	{
	}

	void initialize()
	{
		glInit();
	}

protected:
	virtual void initializeGL()
	{
		static bool isInitialized = false;
		if (isInitialized) {
			return;
		}
		isInitialized = true;
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glDisable(GL_DITHER); // we don't need this (and its enabled by default)

		// for anti-aliased lines (currently unused):
		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		// for anti-aliased points (currently unused):
		glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
		// for anti-aliased polygons (currently unused):
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

		BoGL::bogl()->initialize();

		// TODO: initialize texture manager here?
		//       -> probably too boson specific already!
	}
};

BosonGLView::BosonGLView(QWidget* parent, bool direct)
	: QGraphicsView(parent)
{
 mIsInitialized = false;

 // we handle scene updates on our own.
 setViewportUpdateMode(QGraphicsView::NoViewportUpdate);

 setFocusPolicy(Qt::WheelFocus);
 setMouseTracking(true);
 //qApp->setGlobalMouseTracking(true);

 mFPSCounter = new BosonFPSCounter(this);
 mScene = new QGraphicsScene();
 setScene(mScene);

 QGLFormat format(direct? QGL::DirectRendering : QGL::IndirectRendering);
 mGLWidget = new MyGLWidget(format, 0);
 mGLWidget->setUpdatesEnabled(false);
 setViewport(mGLWidget);
}

BosonGLView::~BosonGLView()
{
 boDebug();
 delete mGLWidget;
 delete mScene;
 delete mFPSCounter;
 boDebug() << "done";
}

bool BosonGLView::initializeGL()
{
 if (mIsInitialized) {
	return true;
 }
 if (!mGLWidget->isValid()) {
	boError() << "created GL widget is not valid. OpenGL not available/working on this system?";
	return false;
 }
 mGLWidget->initialize();
 mFPSCounter->reset();
 mIsInitialized = true;
 return true;
}

QGLFormat BosonGLView::format() const
{
 return mGLWidget->format();
}

void BosonGLView::addWidgetToScene(QWidget* widget, qreal zValue)
{
 QGraphicsProxyWidget* proxy = mScene->addWidget(widget);
 proxy->setZValue(zValue);
 proxy->setPos(0, 0);
// proxy->resize(200, 200);
 proxy->show();
}

void BosonGLView::slotRenderFrame()
{
// boDebug();
 if (!mIsInitialized) {
	boError() << "not yet initialized";
	return;
 }
 if (viewport()->updatesEnabled()) {
	boError() << "viewport()->updatesEnabled() is TRUE. this is unexpected - only BosonGLWidget should manage viewport updates!";
 }
 mFPSCounter->startFrame();
 viewport()->setUpdatesEnabled(true);
 viewport()->repaint();
 viewport()->setUpdatesEnabled(false);
 mFPSCounter->endFrame();
}

QString BosonGLView::checkIfGLDriverIsBroken()
{
 if (!mGLWidget->isValid()) {
	boError() << "Invalid GL widget!";
	return QString();
 }
 if (!initializeGL()) {
	boError() << "GL could not be initialized";
	return QString();
 }
 mGLWidget->makeCurrent();
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

 return QString();
}
