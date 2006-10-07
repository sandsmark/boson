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

#ifndef BOSONGLWIDGET_H
#define BOSONGLWIDGET_H

#include <qwidget.h>

class QGLFormat;

// AB: most code was shamlessy stolen from QGLContext
class BoContext
{
public:
	BoContext(QPaintDevice*);
	~BoContext();

	bool create(bool wantDirect, bool wantDoubleBuffer = true);
	void makeCurrent();

	bool isValid() const
	{
		return mValid;
	}
	bool isDirect() const
	{
		return mDirect;
	}

	static BoContext* currentContext()
	{
		return mCurrentContext;
	}
	void swapBuffers();

	void doneCurrent();

	bool deviceIsPixmap() const
	{
		return (mPaintDevice->devType() == QInternal::Pixmap);
	}

	bool isInitialized() const
	{
		return mIsInitialized;
	}
	void setIsInitialized(bool i)
	{
		mIsInitialized = i;
	}

	/**
	 * Warning: you should not access this unless you know what you are
	 * doing!
	 *
	 * This is used for plib's puGetWindow() only.
	 **/
	QPaintDevice* paintDevice() const
	{
		return mPaintDevice;
	}

protected:
	bool chooseContext(bool wantDirect, bool wantDoubleBuffer);
	void* chooseVisual(bool wantDoubleBuffer);
	void* tryVisual(const QGLFormat& fmt);
	
private:
	class BoContextPrivate;
	BoContextPrivate* d;
	friend class BosonGLWidget;
	bool mIsInitialized;
	QPaintDevice* mPaintDevice;
	bool mValid;
	bool mDirect;
	void* mVi;

	static BoContext* mCurrentContext;

	// see man glXGetConfig:
	int mPlane;
	int mDepth; // number of bits in depth buffer
	int mAlphaSize;
	int mAccumRedSize;
	int mStencilSize;
	bool mStereo;
};

class BosonGLWidget : public QWidget
{
	Q_OBJECT
public:
	/**
	 * @param direct Use TRUE to enable direct rendering. Use FALSE for
	 * debugging/optimizing software rendering only. @ref directRendering
	 * will tell you whether your request for a direct/non-direct context
	 * succeeded.
	 **/
	BosonGLWidget(QWidget* parent, const char* name = 0, bool direct = true);
	~BosonGLWidget();

	virtual void paintGL() {}
	bool isValid() const;

	BoContext* context() const;
	bool directRendering() const;

	virtual void makeCurrent();
	void swapBuffers();

	/**
	 * Switch (usually temporarily) to a new context. This can be used e.g.
	 * to render to a pixamap - you create a new context for pixmap
	 * rendering and switch to it until you are done. Then you revert to
	 * "normal" rendering.
	 *
	 * The @p newContext must be a fully @ref BoContext::create'ed context
	 * and it must be @ref BoContext::isValid.
	 *
	 * You should store the old @ref context somewhere in order to switch
	 * back to it when you are done. If you won't do that you must delete
	 * it. The old @ref context is not touched here.
	 *
	 * @return Whether the switch was actually done.
	 **/
	bool switchContext(BoContext* newContext);

	/**
	 * Reimplemented (workaround for utah GLX driver). See
	 * @ref QGLWidget::reparent implementation (shamelessy stolen from
	 * there)
	 **/
	virtual void reparent(QWidget* parent, WFlags f, const QPoint& p, bool showIt);

	/**
	 * @return QGLWidget::convertToGLFormat
	 **/
	static QImage convertToGLFormat( const QImage& img );

	QImage screenShot();

	virtual QPixmap renderPixmap(int w = 0, int h = 0, bool useContext = FALSE);

public slots:
	virtual void slotUpdateGL();

protected:
	void initGL();
	virtual void initializeGL() {}
	virtual void resizeGL(int width, int height) { Q_UNUSED(width); Q_UNUSED(height); }
	bool isInitialized() const { return context()->isInitialized(); }
	void setContext(BoContext*);

	void setAutoBufferSwap(bool on) { mAutoSwap = on; }
	bool autoBufferSwap() const { return mAutoSwap; }

	/**
	 * Just calls @ref slotUpdateGL
	 **/
	virtual void paintEvent(QPaintEvent*);

	virtual void resizeEvent(QResizeEvent* e);

private:
	void init();
	bool renderCxPm( QPixmap* pm );

private:
	class BosonGLWidgetPrivate;
	BosonGLWidgetPrivate* d;
	bool mAutoSwap;
};


#endif

