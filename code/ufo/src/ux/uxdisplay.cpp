/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ux/uxdisplay.cpp
    begin             : Mon Jul 26 2004
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/ux/uxdisplay.hpp"

#include "ufo/ux/uxcontext.hpp"
#include "ufo/ux/uxtoolkit.hpp"
#include "ufo/ux/uxframe.hpp"

#include "ufo/uvideodriver.hpp"
#include "ufo/uvideodevice.hpp"

#include "ufo/events/umouseevent.hpp"
#include "ufo/events/umousewheelevent.hpp"
#include "ufo/events/ukeyevent.hpp"
#include "ufo/events/uquitevent.hpp"

#include "ufo/widgets/urootpane.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UXDisplay, UAbstractDisplay)

class UXDummyDriver : public UVideoDriver {
public: // Implements UVideoDriver
	virtual bool init() { return true; }
	virtual bool isInitialized() { return true; }
	virtual void quit() {}

	virtual void pumpEvents() {}

	virtual UVideoDevice * createVideoDevice() { return NULL; }
};


UXDisplay::UXDisplay(const std::string & videoDriver)
	: m_videoDriver(NULL)
	, m_frames()
	, m_contexts()
	, m_lastMousePressPos()
	, m_lastMousePressTime(0)
	, m_clickDelay(200)
	, m_clickCount(0)
	, m_mousePos()
{
	setDefault(this);
	UToolkit * tk = UToolkit::getToolkit();
	if (videoDriver != "") {
		tk->putProperty("video_driver", videoDriver);
	}
	if (videoDriver == "dummy") {
		m_videoDriver = new UXDummyDriver();
	} else {
		m_videoDriver = UToolkit::getToolkit()->createVideoDriver();
	}

	if (!m_videoDriver->isInitialized()) {
		if (!m_videoDriver->init()) {
			delete (m_videoDriver);
			m_videoDriver = NULL;
			uError() << "UXDisplay: couldn't init video driver\n";
		}
	}
}

UXDisplay::~UXDisplay() {
	if (m_videoDriver) {
		int size = m_frames.size();
		for (int i = 0; i < size; ++i) {
			if (m_frames[0]) {
				m_frames[0]->setVisible(false);
				m_frames[0]->unreference();
				m_frames.erase(m_frames.begin());
			}
		}
		m_videoDriver->quit();
		delete (m_videoDriver);
	}
}

bool
UXDisplay::isValid() const {
	return (m_videoDriver != NULL);
}

UVideoDriver *
UXDisplay::getVideoDriver() const {
	return m_videoDriver;
}

UXFrame *
UXDisplay::createFrame() {
	UVideoDevice * device = m_videoDriver->createVideoDevice();
	if (device) {
		device->setFrameStyle(FrameTitleBar | FrameMinMaxBox | FrameCloseBox);
		device->setInitialFrameState(FrameResizable);

		UXFrame * frame = new UXFrame(device);
		m_frames.push_back(frame);
		frame->reference();
		device->setFrame(frame);
		return frame;
	} else {
		//uError() << "UXDisplay: Couldn't create video device\n";
		return NULL;
	}
}

std::vector<UXFrame*>
UXDisplay::getFrames() const {
	return m_frames;
}

void
UXDisplay::pumpEvents() {
	m_videoDriver->pumpEvents();
}

std::vector<ufo::UContext*>
UXDisplay::getContexts() const {
	return m_contexts;
}

void
UXDisplay::setModState(UMod_t modifiers) {
	UAbstractDisplay::setModState(modifiers);
}

void
UXDisplay::pushMouseButtonDown(UContext * context,
		int x, int y, UMod_t button)
{
	int modifiers = getModState();
	// FIXME: should button down events have that button as modifier?
	// add the newly pressed button to the modifiers
	modifiers |= button;
	UEvent::Type type = UEvent::MousePressed;
	UPoint pos(x, y);

	m_lastMousePressPos = pos;

	UMouseEvent * e = new UMouseEvent(
		context->getRootPane(), // we do not need a source, but just in case..
		type,
		UMod_t(modifiers),  // modifieres
		pos,  // mouse pos
		button, // changed button
		0 // click count
	);

	pushEvent(e);
}

void
UXDisplay::pushMouseButtonUp(UContext * context,
		int x, int y, UMod_t button)
{
	int modifiers = getModState();
	// FIXME: should button up events have that button as modifier?
	// remove the newly released button to the modifiers
	modifiers &= ~button;
	UEvent::Type type = UEvent::MouseReleased;
	UPoint pos(x, y);

	UMouseEvent * e = new UMouseEvent(
		context->getRootPane(), // we do not need a source, but just in case..
		type,
		UMod_t(modifiers),  // modifieres
		pos,  // mouse pos
		button, // changed button
		0 // click count
	);

	pushEvent(e);

	uint32_t time = UXToolkit::getToolkit()->getTicks();
	if ((time - m_lastMousePressTime) <= m_clickDelay) {
		m_clickCount++;
	} else {
		m_clickCount = 0;
	}

	// a click event
	if (type == UEvent::MouseReleased &&
		m_lastMousePressPos.x < (pos.x + 2) &&
		m_lastMousePressPos.x > (pos.x - 2) &&
		m_lastMousePressPos.y < (pos.y + 2) &&
		m_lastMousePressPos.y > (pos.y - 2)
		) {

		e = new UMouseEvent(
			context->getRootPane(), // we do not need a source, but just in case..
			UEvent::MouseClicked,
			UMod_t(modifiers),  // modifieres
			pos,  // mouse pos
			button, // changed button
			m_clickCount // click count
		);
		pushEvent(e);

		m_lastMousePressTime = time;
	}
}

void
UXDisplay::pushMouseMove(UContext * context,
		int x, int y)
{
	UPoint pos(x, y);

	UMouseEvent * e = new UMouseEvent(
		context->getRootPane(), // we do not need a source, but just in case..
		UEvent::MouseMoved,
		getModState(),  // modifieres
		pos,  // mouse pos
		pos - m_mousePos,  // relative movement
		UMod::NoButton,
		0 // click count
	);
	m_mousePos = pos;
	pushEvent(e);
}

void
UXDisplay::pushMouseWheelDown(UContext * context,
		int x, int y, int delta, int mouseWheelNum)
{
	UMouseWheelEvent * e = new UMouseWheelEvent(
		context->getRootPane(), // we do not need a source, but just in case..
		UEvent::MouseWheel, // type
		getModState(),  // modifieres
		UPoint(x, y),  // mouse pos
		delta, // delta
		mouseWheelNum // wheel number (only one mouse wheel is supported by SDL)
	);

	pushEvent(e);
}

void
UXDisplay::pushMouseWheelUp(UContext * context,
		int x, int y, int delta, int mouseWheelNum)
{
	UMouseWheelEvent * e = new UMouseWheelEvent(
		context->getRootPane(), // we do not need a source, but just in case..
		UEvent::MouseWheel, // type
		getModState(),  // modifieres
		UPoint(x, y),  // mouse pos
		delta, // delta
		mouseWheelNum // wheel number (only one mouse wheel is supported by SDL)
	);

	pushEvent(e);
}

void
UXDisplay::pushKeyDown(UContext * context,
		UKeyCode_t keyCode, wchar_t keyChar)
{
	UMod_t modifiers = getModState();
	pushEvent(
		new UKeyEvent(
			context->getRootPane(),  // source
			UEvent::KeyPressed,
			modifiers,
			keyCode,
			keyChar
		)
	);

	// FIXME keytyped: is this correct?
	if (! /*std::*/iscntrl(keyChar)) {
		pushEvent(
			new UKeyEvent(
				context->getRootPane(),  // source
				UEvent::KeyTyped,
				modifiers,  // modifier
				UKey::UK_UNDEFINED,  // SDLkey
				keyChar
			)
		);
	}
}

void
UXDisplay::pushKeyUp(UContext * context,
		UKeyCode_t keyCode, wchar_t keyChar)
{
	pushEvent(
		new UKeyEvent(
			context->getRootPane(),  // source
			UEvent::KeyReleased,
			getModState(),
			keyCode,
			keyChar
		)
	);
}


#if 0
def UFO_USE_GLUT
void
UXDisplay::mouseFunc(UContext * context,
		int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		pushMouseButtonDown(context, UMod::NoModifier,
			x, y, mapGlutButton(button));
	} else {
		pushMouseButtonUp(context, UMod::NoModifier,
			x, y, mapGlutButton(button));
	}
}

void
UXDisplay::motionFunc(UContext * context, int x, int y) {
	pushMouseMove(context, UMod::NoModifier,
		x, y);
}

void
UXDisplay::passiveMotionFunc(UContext * context, int x, int y) {
	pushMouseMove(context, UMod::NoModifier,
		x, y);
}


void
UXDisplay::keyboardFunc(UContext * context,
		unsigned char key, int x, int y) {
	pushKeyDown(context, mapGlutModifier(glutGetModifiers()),
		mapGlutKey(key), key);
}

void
UXDisplay::specialFunc(UContext * context,
		int key, int x, int y) {
	pushKeyDown(context, mapGlutModifier(glutGetModifiers()),
		mapGlutSpecial(key), key);
}


void
UXDisplay::keyboardUpFunc(UContext * context,
		unsigned char key, int x, int y) {
	pushKeyUp(context, mapGlutModifier(glutGetModifiers()),
		mapGlutKey(key), key);
}

void
UXDisplay::specialUpFunc(UContext * context,
		int key, int x, int y) {
	pushKeyDown(context, mapGlutModifier(glutGetModifiers()),
		mapGlutSpecial(key), key);
}


UMod_t
UXDisplay::mapGlutModifier(int mod) {
	switch (modifier) {
		case GLUT_ACTIVE_SHIFT: return UMod::Shift;
		case GLUT_ACTIVE_CTRL: return UMod::Ctrl;
		case GLUT_ACTIVE_ALT: return UMod::Alt;
		default: return UMod::NoModifier;
	}
}


UMod_t
UXDisplay::mapGlutButton(int button) {
	// FIXME
	// can those values be OR'd together?
	if (button == GLUT_LEFT_BUTTON) {
		return UMod::LeftButton;
	} else if (button == GLUT_MIDDLE_BUTTON) {
		return UMod::MiddleButton;
	} else if (button == GLUT_RIGHT_BUTTON) {
		return UMod::RightButton;
	}
	return UMod::NoButton;
}

UKeyCode_t
UXDisplay::mapGlutKey(int key) {
	return UKeyCode_t(key);
}

UKeyCode_t
UXDisplay::mapGlutSpecial(int key) {
	switch (key) {
		// function keys
		case GLUT_KEY_F1: return UKey::UK_F1;
		case GLUT_KEY_F2: return UKey::UK_F2;
		case GLUT_KEY_F3: return UKey::UK_F3;
		case GLUT_KEY_F4: return UKey::UK_F4;
		case GLUT_KEY_F5: return UKey::UK_F5;
		case GLUT_KEY_F6: return UKey::UK_F6;
		case GLUT_KEY_F7: return UKey::UK_F7;
		case GLUT_KEY_F8: return UKey::UK_F8;
		case GLUT_KEY_F9: return UKey::UK_F9;
		case GLUT_KEY_F10: return UKey::UK_F10;
		case GLUT_KEY_F11: return UKey::UK_F11;
		case GLUT_KEY_F12: return UKey::UK_F12;
		// directional keys
		case GLUT_KEY_LEFT: return UKey::UK_LEFT;
		case GLUT_KEY_UP: return UKey::UK_UP;
		case GLUT_KEY_RIGHT: return UKey::UK_RIGHT;
		case GLUT_KEY_DOWN: return UKey::UK_DOWN;
		case GLUT_KEY_PAGE_UP: return UKey::UK_PAGEUP;
		case GLUT_KEY_PAGE_DOWN: return UKey::UK_PAGEDOWN;
		case GLUT_KEY_HOME: return UKey::UK_HOME;
		case GLUT_KEY_END: return UKey::UK_END;
		case GLUT_KEY_INSERT: return UKey::UK_INSERT;
		default: return UKey::UK_UNDEFINED;
	}
	return UKey::UK_UNDEFINED;
}

#endif

//
// Protected Methids
//

void
UXDisplay::registerContext(UXContext * context) {
	m_contexts.push_back(context);
}

void
UXDisplay::unregisterContext(UXContext * context) {
	m_contexts.erase(std::find(m_contexts.begin(), m_contexts.end(), context));
}
