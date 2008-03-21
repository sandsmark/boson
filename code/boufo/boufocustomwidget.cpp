/*
    This file is part of the Boson game
    Copyright (C) 2004-2006 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufocustomwidget.h"
#include "boufocustomwidget.moc"

#include <bodebug.h>

class UCustomWidgetRenderer : public ufo::UWidget
{
	UFO_DECLARE_DYNAMIC_CLASS(UCustomWidgetRenderer)
public:
	UCustomWidgetRenderer(BoUfoWidget* widget)
		: ufo::UWidget()
	{
		mWidget = widget;
	}

	virtual void paint(ufo::UGraphics* g)
	{
		Q_UNUSED(g);
		mWidget->paint();
	}
	virtual void paintWidget(ufo::UGraphics* g)
	{
		Q_UNUSED(g);

		// AB: libufo uses pixel exact rendering in the way proposed in
		// the redbook - the appendix "programming tips".
		//
		// however by some reason it does NOT do the translation, but
		// rather adds the same translation in every GL call that is
		// done using a UGraphics object. I consider that nonsense, but
		// that's the way it is - so we have to fix it here.
		glTranslatef(0.375f, 0.375f, 0.0);
		mWidget->paintWidget();
		glTranslatef(-0.375f, -0.375f, 0.0);
	}
	virtual void paintBorder(ufo::UGraphics* g)
	{
		Q_UNUSED(g);
		mWidget->paintBorder();
	}

#if 0 // AB: we probably don't need to overwrite the "normal" (without parameters) getPreferredSize()
	virtual ufo::UDimension getPreferredSize() const
	{
		QSize s = mWidget->preferredSize();
		return sizeToDimension(s);
	}
#endif
	virtual ufo::UDimension getPreferredSize(const ufo::UDimension& maxSize) const
	{
		// check for an explicitly set size.
		if (getStyleHints()->preferredSize.isValid()) {
			return getStyleHints()->preferredSize;
		}

		QSize maxSize_ = dimensionToSize(maxSize);
		QSize s = mWidget->preferredSize(maxSize_);
		return sizeToDimension(s);
	}
#if 0
	virtual ufo::UDimension getMinimumSize()
	{
		QSize s = mRenderer->minimumSize();
		return ufo::UDimension(s.width(), s.height());
	}
	virtual ufo::UDimension getMaximumSize()
	{
		QSize s = mRenderer->maximumSize();
		return ufo::UDimension(s.width(), s.height());
	}
#endif


	void ufoPaint(ufo::UGraphics* g)
	{
		ufo::UWidget::paint(g);
	}
	void ufoPaintWidget(ufo::UGraphics* g)
	{
		ufo::UWidget::paintWidget(g);
	}
	void ufoPaintBorder(ufo::UGraphics* g)
	{
		ufo::UWidget::paintBorder(g);
	}

	// AB: here we use Qt parameters/returns QSize instead of libufos
	//     ufo::UDimension
	//     -> so this method cares about conversion, the caller does not
	//     need to
#if 0
	QSize getUfoPreferredSize() const
	{
		ufo::UDimension dim = ufo::UWidget::getPreferredSize();
		return dimensionToSize(dim);
	}
#endif

	QSize getUfoPreferredSize(const QSize& maxSize)
	{
		ufo::UDimension maxSize_ = sizeToDimension(maxSize);
		ufo::UDimension dim = ufo::UWidget::getPreferredSize(maxSize_);
		return dimensionToSize(dim);
	}

#if 0
	ufo::UDimension getUfoMaximumSize()
	{
		return ufo::UWidget::getMaximumSize();
	}
#endif

protected:
	QSize dimensionToSize(const ufo::UDimension& dim) const
	{
		QSize s(dim.w, dim.h);
		if (dim.w == ufo::UDimension::maxDimension.w) {
			s.setWidth(QCOORD_MAX);
		}
		if (dim.h == ufo::UDimension::maxDimension.h) {
			s.setHeight(QCOORD_MAX);
		}
		return s;
	}
	ufo::UDimension sizeToDimension(const QSize& s) const
	{
		ufo::UDimension dim(s.width(), s.height());
		if (s.width() == QCOORD_MAX) {
			dim.w = ufo::UDimension::maxDimension.w;
		}
		if (s.height() == QCOORD_MAX) {
			dim.h = ufo::UDimension::maxDimension.h;
		}
		return dim;
	}

private:
	BoUfoWidget* mWidget;
};

UFO_IMPLEMENT_DYNAMIC_CLASS(UCustomWidgetRenderer, UWidget)

BoUfoCustomWidget::BoUfoCustomWidget()
	: BoUfoWidget(new UCustomWidgetRenderer(this))
{
 setLayoutClass(UVBoxLayout);
}

BoUfoCustomWidget::~BoUfoCustomWidget()
{
}

void BoUfoCustomWidget::paint()
{
 BO_CHECK_NULL_RET(ufoWidget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)ufoWidget())->ufoPaint(g);
}

void BoUfoCustomWidget::paintWidget()
{
 BO_CHECK_NULL_RET(ufoWidget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)ufoWidget())->ufoPaintWidget(g);
}

void BoUfoCustomWidget::paintBorder()
{
 BO_CHECK_NULL_RET(ufoWidget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)ufoWidget())->ufoPaintBorder(g);
}

#if 0
QSize BoUfoCustomWidget::preferredSize() const
{
 return ((UCustomWidgetRenderer*)ufoWidget())->getUfoPreferredSize();
}
#endif

QSize BoUfoCustomWidget::preferredSize(const QSize& maxSize) const
{
 return ((UCustomWidgetRenderer*)ufoWidget())->getUfoPreferredSize(maxSize);
}


