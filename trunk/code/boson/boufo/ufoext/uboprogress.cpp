/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2004 by Andreas Beckermann
    email             : b_mann at gmx.de
                             -------------------

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "uboprogress.h"

#include <bogl.h>

#include <ufo/ugraphics.hpp>
#include <ufo/util/udimension.hpp>

using namespace ufo;

// AB: these are the default values. pretty random values though
// -> a user can and probably will use widget->setPreferredSize() anyway
#define PREFERRED_WIDTH 20
#define PREFERRED_HEIGHT 10


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UBoProgress, UWidget)

// AB: by default min=0, max=100 which is perfect for percentage values.
// therefore if a user creates a progress widget he can use percentage values in
// setValue() without further modification
UBoProgress::UBoProgress(Orientation o)
	: UWidget(),
	m_min(0.0),
	m_max(100.0),
	m_value (50.0),
	m_orientation(o),
	m_startcolor(255, 0, 0),
	m_endcolor(0, 255, 0) {
}

void
UBoProgress::setMinimumValue(double v) {
	m_min = std::min(v, getMaximumValue());
	m_value = std::max(m_value, m_min);
	repaint();
}

void
UBoProgress::setMaximumValue(double v) {
	m_max = std::max(v, getMinimumValue());
	m_value = std::min(m_value, m_max);
	repaint();
}

void
UBoProgress::setValue(double v) {
	v = std::max(v, getMinimumValue());
	v = std::min(v, getMaximumValue());
	m_value = v;
	repaint();
}

void UBoProgress::setOrientation(Orientation o) {
	m_orientation = o;
	invalidate();
	repaint();
}

void UBoProgress::setStartColor(const UColor& color) {
	m_startcolor = color;
	repaint();
}

void UBoProgress::setEndColor(const UColor& color) {
	m_endcolor = color;
	repaint();
}

void UBoProgress::setColor(const UColor& color) {
	m_startcolor = color;
	m_endcolor = color;
	repaint();
}

void UBoProgress::paintWidget(UGraphics* g) {
	// TODO: support an icon

	paintGradient(g, startColor(), endColor());
}

#define CHECKERROR if (glGetError() != GL_NO_ERROR) \
	{ \
		std::cout << "ERROR" << std::endl; \
	}

void
UBoProgress::paintGradient(UGraphics * g, const UColor& from, const UColor& to) {
	const URectangle& rect = getInnerBounds();
	double l = getMaximumValue() - getMinimumValue();
	double factor;
	if (l != 0.0) {
		factor = (getValue() - getMinimumValue()) / l;
	} else {
		factor = 0.0;
	}

	// AB: unfortunatley we don't have access to ugl_driver, as it's in
	// UGL_Style only.
	// so 2 options:
	// 1. modify UGL_Style to contain a paintProgressBar() method
	// 2. use gl*() functions directly here
	// --> well, I want to get my things done, so I go of course for 2.
	//     furthermore I believe it is the cleaner solution, as these
	//     calls _belong_ to here in my opinion.
	g->setColor(from);
	glPushAttrib(GL_LIGHTING_BIT);
	glShadeModel(GL_SMOOTH);

	float realToR = (float)(to.getRed()   * factor + from.getRed()   * (1.0 - factor));
	float realToG = (float)(to.getGreen() * factor + from.getGreen() * (1.0 - factor));
	float realToB = (float)(to.getBlue()  * factor + from.getBlue()  * (1.0 - factor));
	float realToA = 1.0f;

	if (getOrientation() == Horizontal) {
		glBegin(GL_QUADS);
			glVertex2i(rect.x, rect.y);
			glVertex2i(rect.x, rect.y + rect.h);
			glColor4f(realToR, realToG, realToB, realToA);
			glVertex2i(rect.x + (int)(rect.w * factor), rect.y + rect.h);
			glVertex2i(rect.x + (int)(rect.w * factor), rect.y);
		glEnd();
	} else {
		glBegin(GL_QUADS);
			glVertex2i(rect.x, rect.y + rect.h);
			glVertex2i(rect.x + rect.w, rect.y + rect.h);
			glColor4f(realToR, realToG, realToB, realToA);
			glVertex2i(rect.x + rect.w, rect.y + rect.h - (int)(rect.h * factor));
			glVertex2i(rect.x, rect.y + rect.h - (int)(rect.h * factor));
		glEnd();
	}
	glPopAttrib();
}

UDimension
UBoProgress::getPreferredSize(const UDimension & maxSize) {
	UDimension size;
	if (getOrientation() == Horizontal) {
		size = UDimension(PREFERRED_WIDTH, PREFERRED_HEIGHT);
	} else {
		size = UDimension(PREFERRED_HEIGHT, PREFERRED_WIDTH);
	}
	size.clamp(maxSize);
	return size;
}


