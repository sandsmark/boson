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

#include "uboprogressui.h"

#include "uboprogress.h"
#include <bogl.h>

#include <ufo/ui/uuimanager.hpp>
#include <ufo/ui/ustyle.hpp>
#include <ufo/ugraphics.hpp>
#include <ufo/gl/ugl_driver.hpp>
#include <ufo/font/ufont.hpp>
#include <ufo/util/udimension.hpp>


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBoProgressUI, UWidgetUI)

UBoProgressUI * UBoProgressUI::m_progressUI = new UBoProgressUI();

std::string UBoProgressUI::m_lafId("UBoProgress");

UBoProgressUI * UBoProgressUI::createUI(UWidget * w) {
	return m_progressUI;
}

void
UBoProgressUI::paint(UGraphics * g, UWidget * w) {
	UWidgetUI::paint(g, w);

	UBoProgress * progress = dynamic_cast<UBoProgress *>(w);

	// TODO: support custom colors
	// TODO: support an icon

	UColor from(255, 0, 0);
	UColor to(0, 255, 0);
	paintGradient(g, progress, from, to);
}

void
UBoProgressUI::paintGradient(UGraphics * g, const UBoProgress * progress, const UColor& from, const UColor& to) {
	const URectangle& rect = progress->getInnerBounds();
	double l = progress->getMaximumValue() - progress->getMinimumValue();
	double factor;
	if (l != 0.0) {
		factor = (progress->getValue() - progress->getMinimumValue()) / l;
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

	UColor realTo(  (float)(to.getRed()   * factor + from.getRed()   * (1.0 - factor)),
			(float)(to.getGreen() * factor + from.getGreen() * (1.0 - factor)),
			(float)(to.getBlue()  * factor + from.getBlue()  * (1.0 - factor)),
			1.0f);
	if (progress->getOrientation() == Horizontal) {
		glBegin(GL_QUADS);
			glVertex2i(rect.x, rect.y);
			glVertex2i(rect.x, rect.y + rect.h);
			g->setColor(realTo);
			glVertex2i(rect.x + (int)(rect.w * factor), rect.y + rect.h);
			glVertex2i(rect.x + (int)(rect.w * factor), rect.y);
		glEnd();
	} else {
		glBegin(GL_QUADS);
			glVertex2i(rect.x, rect.y + rect.h);
			glVertex2i(rect.x + rect.w, rect.y + rect.h);
			g->setColor(realTo);
			glVertex2i(rect.x + rect.w, rect.y + rect.h - (int)(rect.h * factor));
			glVertex2i(rect.x, rect.y + rect.h - (int)(rect.h * factor));
		glEnd();
	}
	glPopAttrib();
}

const std::string &
UBoProgressUI::getLafId() {
	return m_lafId;
}


// AB: these are the default values. pretty random values though
// -> a user can and probably will use widget->setPreferredSize() anyway
#define PREFERRED_WIDTH 20
#define PREFERRED_HEIGHT 10

UDimension
UBoProgressUI::getPreferredSize(const UWidget * w) {
	const UBoProgress * progress = dynamic_cast<const UBoProgress *>(w);
	if (!progress) {
		return UDimension();
	}
	if (progress->getOrientation() == Horizontal) {
		return UDimension(PREFERRED_WIDTH, PREFERRED_HEIGHT);
	} else {
		return UDimension(PREFERRED_HEIGHT, PREFERRED_WIDTH);
	}
	return UDimension();
}


