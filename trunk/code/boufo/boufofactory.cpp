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

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufofactory.h"

#include "boufo.h"
#include <bodebug.h>

#include <qstringlist.h>

BoUfoWidget* BoUfoFactory::createWidget(const QString& className)
{
 if (!widgets().contains(className)) {
	boError() << k_funcinfo << "don't know class " << className << endl;
	return 0;
 }
#define CLASSNAME(name) if (className == #name) { return new name(); }
 CLASSNAME(BoUfoWidget)
 CLASSNAME(BoUfoHBox)
 CLASSNAME(BoUfoVBox)
 CLASSNAME(BoUfoGroupBox)
 CLASSNAME(BoUfoPushButton)
 CLASSNAME(BoUfoCheckBox)
 CLASSNAME(BoUfoRadioButton)
 CLASSNAME(BoUfoButtonGroupWidget)
 CLASSNAME(BoUfoSlider)
 CLASSNAME(BoUfoProgress)
 CLASSNAME(BoUfoExtendedProgress)
 CLASSNAME(BoUfoNumInput)
 CLASSNAME(BoUfoLabel)
 CLASSNAME(BoUfoLineEdit)
 CLASSNAME(BoUfoTextEdit)
 CLASSNAME(BoUfoComboBox)
 CLASSNAME(BoUfoListBox)
 CLASSNAME(BoUfoMatrix)
 CLASSNAME(BoUfoTabWidget)
 CLASSNAME(BoUfoWidgetStack)
 CLASSNAME(BoUfoLayeredPane)
#undef CLASSNAME
 return 0;
}

QStringList BoUfoFactory::widgets()
{
 QStringList list;
 list.append("BoUfoWidget");
 list.append("BoUfoHBox");
 list.append("BoUfoVBox");
 list.append("BoUfoGroupBox");
 list.append("BoUfoPushButton");
 list.append("BoUfoCheckBox");
 list.append("BoUfoRadioButton");
 list.append("BoUfoButtonGroupWidget");
 list.append("BoUfoSlider");
 list.append("BoUfoProgress");
 list.append("BoUfoExtendedProgress");
 list.append("BoUfoNumInput");
 list.append("BoUfoLabel");
 list.append("BoUfoLineEdit");
 list.append("BoUfoTextEdit");
 list.append("BoUfoComboBox");
 list.append("BoUfoListBox");
 list.append("BoUfoMatrix");
 list.append("BoUfoTabWidget");
 list.append("BoUfoWidgetStack");
 list.append("BoUfoLayeredPane");
 return list;
}

