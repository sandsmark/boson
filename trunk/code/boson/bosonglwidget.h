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

public slots:
	void slotUpdateGL();

protected:
	void initGL();
	virtual void initializeGL() {}
	virtual void resizeGL(int width, int height) { Q_UNUSED(width); Q_UNUSED(height); }
	bool isInitialized() const { return context()->isInitialized(); }
	void setContext(BoContext*);

	/**
	 * Just calls @ref slotUpdateGL
	 **/
	virtual void paintEvent(QPaintEvent*);

	virtual void resizeEvent(QResizeEvent* e);

private:
	void init();

private:
	class BosonGLWidgetPrivate;
	BosonGLWidgetPrivate* d;
};

/**
 * @short Class that handles mouse movement diffs.
 * In boson you are often in a situation where the user presses a button and
 * you must do someting with all mousemove events that occur from then on
 * until the button is released again. Examples are
 * @li RMB scrolling
 * @li selection rect
 * @li model rotation using the mouse in borender
 * This class can be used for this.
 *
 * You basically call @ref moveEvent in every @ref QWidget::mouseMoveEvent of
 * the relevant widget. When the button is pressed initially (i.e. in the @ref
 * QWidget::mousePressEvent) you call it once again to reset old values.
 * Whenever you need to know how far the mouse was moved since the last move
 * event, you can use @ref dx and @ref dy.
 *
 * For advanced use, you can use @ref start, @ref stop and @ref isStopped. The
 * latter will tell you whether the button, that was @ref start'ed is still
 * pressed. Note that in @ref moveEvent it is also checked whether the button is
 * still pressed and @ref stop gets called if this is not the case.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMouseMoveDiff
{
public:
	BoMouseMoveDiff();

	/**
	 * @param buttonState Which buttons were pressed when the move even
	 * occurred. -1 means @ref QMouseEvent::NoButton
	 **/
	void moveEvent(int x, int y, int buttonState = -1);

	/**
	 * @overload
	 **/
	void moveEvent(const QPoint& pos, int buttonState = -1);

	/**
	 * @overload
	 **/
	void moveEvent(QMouseEvent* event);

	/**
	 * Make @ref isStopped return TRUE again. The mouse moves are still
	 * recognized (see @ref dx and @ref dy), but they are meant to be
	 * unused, until @ref start is called again.
	 **/
	void stop();

	/**
	 * @return The current mouse x-position (i.e. the position in the last
	 * @ref moveEvent)
	 **/
	int x() const
	{
		return mX;
	}

	/**
	 * @return The current mouse y-position (i.e. the position in the last
	 * @ref moveEvent)
	 **/
	int y() const
	{
		return mY;
	}

	/**
	 * @return The mouse x-position before the last @ref moveEvent
	 **/
	int oldX() const
	{
		return mOldX;
	}

	/**
	 * @return The mouse y-position before the last @ref moveEvent
	 **/
	int oldY() const
	{
		return mOldY;
	}

	/**
	 * @return The distance the mouse was moved in x direction between the
	 * last two @ref moveEvent calls.
	 **/
	int dx() const
	{
		return (mX - mOldX);
	}

	/**
	 * @return The distance the mouse was moved in y direction between the
	 * last two @ref moveEvent calls.
	 **/
	int dy() const
	{
		return (mY - mOldY);
	}

	/**
	 * Make @ref isStopped return FALSE, until @p button is not pressed
	 * anymore (or @ref stop is called). This tells this class, that we want
	 * to do something with the mouse move event while the button is
	 * pressed.
	 *
	 * An example use of this is the selection rect - you will call
	 * start(QMouseEvent::LeftButton) to start the selection rect and should
	 * continue to paint it until @ref isStopped is TRUE again.
	 *
	 * Note that the behaviour of @ref dx and @ref dy is not influenced by
	 * this.
	 **/
	void start(int button);

	/**
	 * @return TRUE unless @ref start was called and the button is still
	 * pressed.
	 **/
	bool isStopped() const;

	/**
	 * @return The button that is waited for to be released, i.e. the button
	 * specified by @ref start until it is released. Once it is released
	 * this will return @ref QMouseEvent::NoButton
	 **/
	int button() const
	{
		return mButton;
	}

private:
	int mButton;
	int mX;
	int mY;
	int mOldX;
	int mOldY;
};

#endif

