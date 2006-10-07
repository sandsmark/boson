/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

// note the copyright above: this is LGPL!

#ifndef BOUFOFONTRENDERER_H
#define BOUFOFONTRENDERER_H

#include "ufo/font/ufontrenderer.hpp"

namespace ufo {
	class UFontMetrics;
	class UPluginBase;
	class UGraphics;
};

class BoUfoFontMetrics;

class BosonGLFont;

/**
 * @short This is NOT a public boson class. It is used internally by BoUfo only
 *
 * @author Johannes Schmidt <schmidtjf@users.sourceforge.net>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BoUfoFontRenderer : public ufo::UFontRenderer
{
	UFO_DECLARE_ABSTRACT_CLASS(BoUfoFontRenderer)
public:
	BoUfoFontRenderer(BosonGLFont* bofont);
	~BoUfoFontRenderer();

	virtual int drawString(ufo::UGraphics* g, const char* text, unsigned int nChar, int x = 0, int y = 0);

	virtual void beginDrawing(ufo::UGraphics* g);
	virtual void endDrawing(ufo::UGraphics* g);

	virtual void refresh();

	virtual const ufo::UFontMetrics* getFontMetrics() const;

	virtual ufo::UFontInfo getFontInfo() const;

	virtual std::string getSystemName() const;


public:
	static ufo::UPluginBase* createPlugin();
	static void destroyPlugin(ufo::UPluginBase* plugin);

private:
	friend class BoUfoFontMetrics;

private:
	BosonGLFont* mFont;
	BoUfoFontMetrics* mFontMetrics;
};

#endif

