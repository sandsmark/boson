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

	bool create(bool wantDirect);
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

protected:
	bool chooseContext(bool wantDirect);
	void* chooseVisual();
	void* tryVisual(const QGLFormat& fmt);
	
private:
	class BoContextPrivate;
	BoContextPrivate* d;
	friend class BosonGLWidget;
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
	BosonGLWidget(QWidget* parent, bool direct = true);
	~BosonGLWidget();

	virtual void paintGL() {}
	bool isValid() const;

	BoContext* context() const;
	bool directRendering() const;

	void makeCurrent();
	void swapBuffers();

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
	bool isInitialized() const { return mInitialized; }
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
	bool mInitialized;
};

#endif

