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
#include <ufo/ux/ux.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufofontinfo.h"

#include "boufomanager.h"
#include "bodebug.h"

BoUfoFontInfo::BoUfoFontInfo()
{
 ufo::UFontInfo info;
 init("", info);
}

BoUfoFontInfo::BoUfoFontInfo(const BoUfoFontInfo& font)
{
 *this = font;
}

BoUfoFontInfo::BoUfoFontInfo(const QString& fontPlugin, const ufo::UFontInfo& font)
{
 init(fontPlugin, font);
}

void BoUfoFontInfo::init(const QString& fontPlugin, const ufo::UFontInfo& font)
{
 mFontPlugin = fontPlugin;
 if (mFontPlugin.isEmpty()) {
	mFontPlugin = "builtin_font";
 }
 mPointSize = font.pointSize;
 mFixedSize = false;
 mStyle = 0;

 if (fontPlugin == "builtin_font") {
	// AB: the builtin fonts don't use font.face
	switch (font.family) {
		default:
		case ufo::UFontInfo::DefaultFamily:
		case ufo::UFontInfo::SansSerif:
		case ufo::UFontInfo::Decorative:
			mFamily = "SansSerif";
			break;
		case ufo::UFontInfo::Serif:
			mFamily = "Serif";
			break;
		case ufo::UFontInfo::MonoSpaced:
			mFamily = "MonoSpaced";
			mFixedSize = true;
			break;
	}
 } else {
	mFamily = QString::fromLatin1(font.face.c_str());
	if (mFamily.isEmpty()) {
		mFamily = QString("Unknown");
	}
 }

 // non-builtin plugins currently don't use the family property, but for
 // completelyness we check anyway
 if (font.family == ufo::UFontInfo::MonoSpaced) {
	mFixedSize = true;
 }

 switch (font.weight) {
	default:
		break;
	case ufo::UFontInfo::Bold:
	case ufo::UFontInfo::Black:
		mStyle |= StyleBold;
		break;
 }
 if (font.style & ufo::UFontInfo::Italic) {
	mStyle |= StyleItalic;
 }
 if (font.style & ufo::UFontInfo::Underline) {
	mStyle |= StyleUnderline;
 }
 if (font.style & ufo::UFontInfo::StrikeOut) {
	mStyle |= StyleStrikeOut;
 }
}

BoUfoFontInfo& BoUfoFontInfo::operator=(const BoUfoFontInfo& font)
{
 mFontPlugin = font.fontPlugin();
 mFamily = font.family();
 mStyle = font.style();
 mPointSize = font.pointSize();
 mFixedSize = font.fixedSize();
 return *this;
}

bool BoUfoFontInfo::operator==(const BoUfoFontInfo& info) const
{
 if (fontPlugin() != info.fontPlugin()) {
	return false;
 }
 if (family() != info.family()) {
	return false;
 }
 if (fixedSize() != info.fixedSize()) {
	return false;
 }
 if (pointSize() != info.pointSize()) {
	return false;
 }
 if (style() != info.style()) {
	return false;
 }
 return true;
}

ufo::UFont BoUfoFontInfo::ufoFont(BoUfoManager* manager) const
{
 QString originalFontPlugin = manager->ufoToolkitProperty("font");
 if (!fontPlugin().isEmpty()) {
	manager->setUfoToolkitProperty("font", fontPlugin());
 }

 ufo::UFont font = ufo::UFont(ufoFontInfo());
 font.getRenderer(); // force to create the renderer with the current plugin

 manager->setUfoToolkitProperty("font", originalFontPlugin);
 return font;
}

ufo::UFontInfo BoUfoFontInfo::ufoFontInfo() const
{
 ufo::UFontInfo::Family family = ufo::UFontInfo::DefaultFamily;
 std::string face = "";
 bool fixed = fixedSize();

 if (mFontPlugin == "builtin_font") {
	if (mFamily == "Decorative") {
		family = ufo::UFontInfo::SansSerif;
	} else if (mFamily == "SansSerif") {
		family = ufo::UFontInfo::SansSerif;
	} else if (mFamily == "Serif") {
		family = ufo::UFontInfo::Serif;
	} else if (mFamily == "MonoSpaced") {
		fixed = true;
		family = ufo::UFontInfo::MonoSpaced;
	}
	face = ""; // unused by this plugin
 } else {
	family = ufo::UFontInfo::DefaultFamily; // unused by all other plugins
	face = std::string(mFamily.latin1());
 }

 int weight = ufo::UFontInfo::Normal;
 int style = ufo::UFontInfo::AnyStyle;
 if (italic()) {
	style |= ufo::UFontInfo::Italic;
 }
 if (underline()) {
	style |= ufo::UFontInfo::Underline;
 }
 if (strikeOut()) {
	style |= ufo::UFontInfo::StrikeOut;
 }
 if (style == ufo::UFontInfo::AnyStyle) {
	style = ufo::UFontInfo::Plain;
 }

 if (fixed) {
	family = ufo::UFontInfo::MonoSpaced;
	face = "";
 }

 ufo::UFontInfo fontInfo;
 fontInfo.family = family;
 fontInfo.face = face;
 fontInfo.pointSize = pointSize();
 fontInfo.weight = weight;
 fontInfo.style = style;
 fontInfo.encoding = ufo::UFontInfo::Encoding_Default;
 return fontInfo;
}

QString BoUfoFontInfo::debugString() const
{
 QString ret;
 ret += QString::fromLatin1("font plugin=%1").arg(fontPlugin());
 ret += QString::fromLatin1(" family=%1").arg(family());
 ret += QString::fromLatin1(" pointSize=%1").arg(pointSize());
 ret += QString::fromLatin1(" style mask=%1").arg(style());
 if (italic()) {
	ret += QString::fromLatin1(" italic");
 }
 if (bold()) {
	ret += QString::fromLatin1(" bold");
 }
 if (underline()) {
	ret += QString::fromLatin1(" underline");
 }
 if (strikeOut()) {
	ret += QString::fromLatin1(" strikeOut");
 }
 if (fixedSize()) {
	ret += QString::fromLatin1(" fixedSize");
 }
 return ret;
}

