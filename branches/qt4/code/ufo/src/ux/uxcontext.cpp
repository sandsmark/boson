/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ux/uxcontext.cpp
    begin             : Tue Jul 27 2004
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

#include "ufo/ux/uxcontext.hpp"

#include "ufo/ux/uxtoolkit.hpp"
#include "ufo/ux/uxdisplay.hpp"

#include "ufo/urepaintmanager.hpp"
#include "ufo/ugraphics.hpp"

#include "ufo/gl/ugl_driver.hpp"

#include "ufo/widgets/urootpane.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UXContext, UAbstractContext)


UXContext::UXContext(const URectangle & deviceBounds,
		const URectangle & contextBounds)
	: UAbstractContext(NULL)
	, m_frame(NULL)
{
	UXDisplay * display = dynamic_cast<UXDisplay*>(UDisplay::getDefault());
	if (display) {
		display->registerContext(this);
	} else {
		uError() << "UXContext: no valid display object found.\n";
	}
	UToolkit::getToolkit()->makeContextCurrent(this);

	this->setDeviceBounds(deviceBounds);
	this->setContextBounds(contextBounds);

	this->init();
}

UXContext::UXContext(const URectangle & bounds)
	: UAbstractContext(NULL)
	, m_frame(NULL)
{
	UXDisplay * display = dynamic_cast<UXDisplay*>(UDisplay::getDefault());
	if (display) {
		display->registerContext(this);
	} else {
		uError() << "UXContext: no valid display object found.\n";
	}
	UToolkit::getToolkit()->makeContextCurrent(this);

	this->setDeviceBounds(bounds);
	this->setContextBounds(bounds);

	this->init();
}

UXContext::~UXContext() {
	dispose();
	UXDisplay * display = dynamic_cast<UXDisplay*>(UDisplay::getDefault());
	if (display) {
		display->unregisterContext(this);
	}
}


bool
UXContext::needsRepaint() {
	return getRepaintManager()->isDirty();
}

void
UXContext::pushAttributes() {
	//glPushAttrib(GL_ALL_ATTRIB_BITS);
	ugl_driver->glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT |
		GL_LIGHTING_BIT | GL_LINE_BIT | GL_LINE_BIT | GL_SCISSOR_BIT |
		GL_TEXTURE_BIT | GL_VIEWPORT_BIT);

	ugl_driver->glMatrixMode(GL_PROJECTION);
	ugl_driver->glPushMatrix();
	ugl_driver->glMatrixMode(GL_MODELVIEW);
	ugl_driver->glPushMatrix();
}

void
UXContext::popAttributes() {
	ugl_driver->glMatrixMode(GL_PROJECTION);
	ugl_driver->glPopMatrix();
	ugl_driver->glMatrixMode(GL_MODELVIEW);
	ugl_driver->glPopMatrix();

	ugl_driver->glPopAttrib();
}

void
UXContext::repaint() {
	// clear repaint manager before repainting as some widgets
	// immediately want a repaint
	getRepaintManager()->clearDirtyRegions();

	UGraphics * g = getGraphics();
	g->begin();
	getRootPane()->paint(g);
	g->end();
}

UXFrame *
UXContext::getFrame() const {
	return m_frame;
}


void
UXContext::lock()
{}

void
UXContext::unlock()
{}
