/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ux/uxframe.cpp
    begin             : Wed Jul 28 2004
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/ux/uxframe.hpp"

#include "ufo/uvideodevice.hpp"
#include "ufo/utoolkit.hpp"

#include "ufo/ux/uxcontext.hpp"
#include "ufo/ux/uxdisplay.hpp"

#include "ufo/uvideodevice.hpp"
#include "ufo/uvideodriver.hpp"
#include "ufo/widgets/uwidget.hpp"
#include "ufo/widgets/urootpane.hpp"

#include "ufo/gl/ugl_driver.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/urepaintmanager.hpp"

#include "ufo/image/uimageio.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UXFrame, UObject)

UXFrame::UXFrame(UVideoDevice * device)
	: m_videoDevice(device)
	, m_context(NULL)
	, m_isVisible(false)
{
	m_videoDevice->sigMoved().connect(slot(*this, &UXFrame::deviceMoved));
	m_videoDevice->sigResized().connect(slot(*this, &UXFrame::deviceResized));
	openContext();
	UToolkit::getToolkit()->makeContextCurrent(NULL);
}

UXFrame::~UXFrame() {
	setVisible(false);
	delete (m_videoDevice);
	closeContext();
	//UXApp::getInstance()->unregisterFrame(this);
}

void
UXFrame::setVisible(bool vis) {
	if (vis == isVisible()) {
		return ;
	}
	if (vis) {
		m_videoDevice->show();
		m_isVisible = true;
		UToolkit::getToolkit()->makeContextCurrent(m_context);
	} else {
		m_isVisible = false;
		m_videoDevice->hide();
	}
}

bool
UXFrame::isVisible() {
	return m_isVisible;
}

UXContext *
UXFrame::getContext() const {
	return m_context;
}

URootPane *
UXFrame::getRootPane() const {
	return m_context->getRootPane();
}

UWidget *
UXFrame::getContentPane() const {
	return m_context->getRootPane()->getContentPane();
}

void
UXFrame::pack() {
	setSize(m_context->getRootPane()->getPreferredSize());
}

void
UXFrame::repaint(bool force) {
	if (m_isVisible) {
		//m_videoDevice->makeContextCurrent();
		if (force || m_context->needsRepaint()) {
			UToolkit::getToolkit()->makeContextCurrent(m_context);
			//m_context->getRootPane()->repaint();
			//m_context->pushAttributes();
			m_context->repaint();
			//m_context->popAttributes();
			m_videoDevice->swapBuffers();
		}
	}
}


void
UXFrame::setTitle(std::string titleA) {
	m_videoDevice->setTitle(titleA);
}
std::string
UXFrame::getTitle() {
	return m_videoDevice->getTitle();
}


void
UXFrame::setDepth(int depth) {
	m_videoDevice->setDepth(depth);
}

int
UXFrame::getDepth() const {
	return m_videoDevice->getDepth();
}


void
UXFrame::setFullScreened(bool b) {
	if (b == bool(m_videoDevice->getFrameState() & FrameFullScreen)) {
		return;
	}
	if (isVisible()) {
		//m_videoDevice->toggleFrameState(FrameFullScreen);
	} else {
		m_videoDevice->setInitialFrameState(m_videoDevice->getFrameState() | FrameFullScreen);
	}
	// FIXME: Does the video device fire a refresh signal if needed?
}

bool
UXFrame::isFullScreened() const {
	return (m_videoDevice->getFrameState() & FrameFullScreen);
}

void
UXFrame::setFrameStyle(uint32_t frameStyle) {
	m_videoDevice->setFrameStyle(frameStyle);
}

FrameStyle
UXFrame::getFrameStyle() const {
	return FrameStyle(m_videoDevice->getFrameStyle());
}


void
UXFrame::setInitialFrameState(uint32_t initialState) {
	m_videoDevice->setInitialFrameState(initialState);
}

FrameState
UXFrame::getFrameState() const {
	return FrameState(m_videoDevice->getFrameState());
}

void
UXFrame::setResizable(bool b) {
	if (b == bool(m_videoDevice->getFrameStyle() & FrameResizable)) {
		return;
	}
	if (isVisible()) {
		//m_videoDevice->toggleFrameState(FrameFullScreen);
	} else {
		m_videoDevice->setFrameStyle(m_videoDevice->getFrameStyle() | FrameFullScreen);
	}
}

bool
UXFrame::isResizable() const {
	return (m_videoDevice->getFrameStyle() & FrameResizable);
}

void
UXFrame::setSize(int w, int h) {
	m_videoDevice->setSize(w, h);
	sigResized().emit(this);
}
void
UXFrame::setSize(const UDimension & d) {
	setSize(d.w, d.h);
}
UDimension
UXFrame::getSize() const {
	return m_videoDevice->getSize();
}

void
UXFrame::setLocation(int x, int y) {
	m_videoDevice->setLocation(x, y);
	sigMoved().emit(this);
}

void
UXFrame::setLocation(const UPoint & p) {
	setLocation(p.x, p.y);
}

UPoint
UXFrame::getLocation() const {
	return m_videoDevice->getLocation();
}

void
UXFrame::setBounds(int x, int y, int w, int h) {
	m_videoDevice->setLocation(x, y);
	m_videoDevice->setSize(w, h);

	m_context->setDeviceBounds(URectangle(x, y, w, h));
	m_context->setContextBounds(URectangle(0, 0, w, h));

	sigMoved().emit(this);
	sigResized().emit(this);
}

void
UXFrame::setBounds(const URectangle & rect) {
	setBounds(rect.x, rect.y, rect.w, rect.h);
}

URectangle
UXFrame::getBounds() const {
	return URectangle(m_videoDevice->getLocation(), m_videoDevice->getSize());
}

void
UXFrame::makeScreenShot(std::string fileNameA) {
	// get an offscreen array of pixel data
	UImageIO * dump = getContext()->getGraphics()->dump();
	dump->reference();

	// save via SDL
	/*
	SDL_Surface * surface = SDL_CreateRGBSurfaceFrom(dump->getPixels(),
		dump->getWidth(), dump->getHeight(), m_depth,
		dump->getWidth() * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	SDL_SaveBMP(surface, fileNameA.c_str());

	SDL_FreeSurface(surface);
	*/
	dump->save(fileNameA);
	dump->unreference();
}

void
UXFrame::swapBuffers() {
	m_videoDevice->swapBuffers();
}

void
UXFrame::makeContextCurrent() {
	m_videoDevice->makeContextCurrent();
}

UVideoDevice *
UXFrame::getVideoDevice() const {
	return m_videoDevice;
}

void
UXFrame::openContext() {
	URectangle device(m_videoDevice->getLocation(), m_videoDevice->getSize());
	URectangle context(UPoint(0, 0), m_videoDevice->getSize());

	m_context = new UXContext(device, context);//, m_flags);
	m_context->m_frame = this;
	/*UToolkit::getToolkit()->makeContextCurrent(m_context);

std::cerr << "set device bounds\n";
	m_context->setDeviceBounds(m_bounds);
	m_context->setContextBounds(URectangle(0, 0, m_bounds.w, m_bounds.h));

std::cerr << "set device init\n";
	m_context->init();
std::cerr << "done\n";*/
}

void
UXFrame::closeContext() {
	m_context->dispose();
	m_context = NULL;
}

void
UXFrame::deviceMoved(UVideoDevice * videoDevice) {
	// schedule a repaint
	// FIXME: always needed?
	getContext()->getRootPane()->repaint();
	sigMoved().emit(this);
}

void
UXFrame::deviceResized(UVideoDevice * videoDevice) {
	UXContext * context = getContext();
	UGraphics * graphics = context->getGraphics();

	UPoint pos = m_videoDevice->getLocation();
	UDimension size = m_videoDevice->getSize();

	context->setDeviceBounds(URectangle(pos, size));
	context->setContextBounds(URectangle(0, 0, size.w, size.h));

	graphics->resetDeviceViewMatrix();

	context->getRootPane()->repaint();
	sigResized().emit(this);
}
