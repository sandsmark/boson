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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>
#include <ufo/gl/ugl_image.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufoimage.h"

#include <bodebug.h>

#include <qimage.h>
#include <qgl.h>
//Added by qt3to4:
#include <QPixmap>


BoUfoImageIO::BoUfoImageIO()
{
 init();
}

BoUfoImageIO::BoUfoImageIO(const QPixmap& p)
{
 init();
 setPixmap(p);
}

BoUfoImageIO::BoUfoImageIO(const QImage& img)
{
 init();
 setImage(img);
}

void BoUfoImageIO::init()
{
 mImageIO = 0;
}

BoUfoImageIO::~BoUfoImageIO()
{
 if (mImageIO) {
	mImageIO->unreference();
 }
}

void BoUfoImageIO::setPixmap(const QPixmap& p)
{
 if (mImageIO) {
	boError() << k_funcinfo << "data is already set" << endl;
	return;
 }
 QImage img = p.toImage();
 setImage(img);
}

void BoUfoImageIO::setImage(const QImage& _img)
{
 if (mImageIO) {
	boError() << k_funcinfo << "data is already set" << endl;
	return;
 }
 // AB: atm UImage uses a format with y-coordinates flipped
 QImage img = QGLWidget::convertToGLFormat(_img.mirrored(false, true));
 mImageIO = new ufo::UImageIO(img.bits(), img.width(), img.height(), 4);
 mImageIO->reference();
}


BoUfoImage::BoUfoImage()
{
 init();
}

BoUfoImage::BoUfoImage(const QPixmap& p)
{
 init();
 load(p);
}

BoUfoImage::BoUfoImage(const QImage& img)
{
 init();
 load(img);
}

BoUfoImage::BoUfoImage(const BoUfoImage& img)
{
 init();
 *this = img;
}

BoUfoImage::~BoUfoImage()
{
 if (mImage) {
	mImage->unreference();
 }
}

BoUfoImage& BoUfoImage::operator=(const BoUfoImage& img)
{
 if (mImage) {
	mImage->unreference();
	mImage = 0;
 }
 load(img);
 return *this;
}

void BoUfoImage::load(const QPixmap& p)
{
 if (p.isNull()) {
	return;
 }
 BoUfoImageIO io(p);
 set(&io);
}

void BoUfoImage::load(const QImage& img)
{
 if (img.isNull()) {
	return;
 }
 BoUfoImageIO io(img);
 set(&io);
}

void BoUfoImage::load(const BoUfoImage& img)
{
 if (!img.image()) {
	return;
 }
 set(img.image());
}

void BoUfoImage::init()
{
 mImage = 0;
}

void BoUfoImage::set(BoUfoImageIO* io)
{
 BO_CHECK_NULL_RET(io);
 BO_CHECK_NULL_RET(io->imageIO());
 if (mImage) {
	mImage->unreference();
	mImage = 0;
 }
 ufo::UImage* image = new ufo::UGL_Image(io->imageIO());
 set(image);
}

void BoUfoImage::set(ufo::UImage* img)
{
 BO_CHECK_NULL_RET(img);
 if (mImage) {
	boError() << k_funcinfo << "image not NULL" << endl;
	return;
 }
 mImage = img;
 mImage->reference();
}

unsigned int BoUfoImage::width() const
{
 if (!mImage) {
	return 0;
 }
 int w = mImage->getImageSize().w;
 if (w < 0) {
	return 0;
 }
 return (unsigned int)w;
}

unsigned int BoUfoImage::height() const
{
 if (!mImage) {
	return 0;
 }
 int h = mImage->getImageSize().h;
 if (h < 0) {
	return 0;
 }
 return (unsigned int)h;
}

void BoUfoImage::paint()
{
 paint(QPoint(0, 0));
}

void BoUfoImage::paint(const QPoint& pos)
{
 paint(QRect(pos, QSize(width(), height())));
}

void BoUfoImage::paint(const QPoint& pos, const QSize& size)
{
 paint(QRect(pos, size));
}

void BoUfoImage::paint(const QRect& rect)
{
 if (!mImage) {
	return;
 }
 ufo::UToolkit* tk = ufo::UToolkit::getToolkit();
 BO_CHECK_NULL_RET(tk);
 ufo::UContext* c = tk->getCurrentContext();
 BO_CHECK_NULL_RET(c);
 ufo::UGraphics* g = c->getGraphics();
 BO_CHECK_NULL_RET(g);
 mImage->paintDrawable(g, ufo::URectangle(rect.x(), rect.y(), rect.width(), rect.height()));
}

