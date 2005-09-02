/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
		mWidget->paintWidget();
	}
	virtual void paintBorder(ufo::UGraphics* g)
	{
		Q_UNUSED(g);
		mWidget->paintBorder();
	}
#if 0
	virtual ufo::UDimension getPreferredSize()
	{
		QSize s = mWidget->preferredSize();
		return ufo::UDimension(s.width(), s.height());
	}
	virtual ufo::UDimension getPreferredSize(const ufo::UDimension& maxSize)
	{
		QSize s = mRenderer->preferredSize(QSize(maxSize.w, maxSize.h));
		return ufo::UDimension(s.width(), s.height());
	}
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
	ufo::UDimension getUfoPreferredSize()
	{
		return ufo::UWidget::getPreferredSize();
	}
	ufo::UDimension getUfoPreferredSize(const ufo::UDimension& maxSize)
	{
		return ufo::UWidget::getPreferredSize(maxSize);
	}
	ufo::UDimension getUfoMinimumSize()
	{
		return ufo::UWidget::getMinimumSize();
	}
	ufo::UDimension getUfoMaximumSize()
	{
		return ufo::UWidget::getMaximumSize();
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
 BO_CHECK_NULL_RET(widget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)widget())->ufoPaint(g);
}

void BoUfoCustomWidget::paintWidget()
{
 BO_CHECK_NULL_RET(widget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)widget())->ufoPaintWidget(g);
}

void BoUfoCustomWidget::paintBorder()
{
 BO_CHECK_NULL_RET(widget());
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 ((UCustomWidgetRenderer*)widget())->ufoPaintBorder(g);
}


