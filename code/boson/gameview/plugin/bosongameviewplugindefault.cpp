/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosongameviewplugindefault.h"
#include "bosongameviewplugindefault.moc"

#include "../../../bomemory/bodummymemory.h"
#include "../../boufo/boufo.h"
#include "../../bosonconfig.h"
#include "editorrandommapwidget.h"

#include <bodebug.h>

class BosonGameViewPluginDefaultPrivate
{
public:
	BosonGameViewPluginDefaultPrivate()
	{
		mEditorRandomMapWidget = 0;
	}
	bool mInitialized;
	bool mGameMode;
	EditorRandomMapWidget* mEditorRandomMapWidget;
};

BosonGameViewPluginDefault::BosonGameViewPluginDefault()
	: BosonGameViewPluginBase()
{
 d = new BosonGameViewPluginDefaultPrivate();
 d->mInitialized = false;
 d->mGameMode = true;
}

BosonGameViewPluginDefault::~BosonGameViewPluginDefault()
{
 delete d;
}

void BosonGameViewPluginDefault::init()
{
 if (d->mInitialized) {
	return;
 }
 d->mInitialized = true;
 BosonGameViewPluginBase::init();

 QColor defaultColor = BoUfoLabel::defaultForegroundColor();
 BoUfoLabel::setDefaultForegroundColor(Qt::white);

 d->mEditorRandomMapWidget = new EditorRandomMapWidget();
 ufoWidget()->addWidget(d->mEditorRandomMapWidget);

 BoUfoLabel::setDefaultForegroundColor(defaultColor);
}

BoUfoWidget* BosonGameViewPluginDefault::createUfoWidget() const
{
 BoUfoWidget* w = new BoUfoWidget();
 w->setLayoutClass(BoUfoWidget::UVBoxLayout);
 w->setName("GameViewPluginDefault"); // for debugging
 return w;
}

void BosonGameViewPluginDefault::quitGame()
{
}

void BosonGameViewPluginDefault::setGameMode(bool mode)
{
 d->mGameMode = mode;
 d->mEditorRandomMapWidget->setVisible(!mode);
}

void BosonGameViewPluginDefault::setCanvas(const BosonCanvas* c)
{
 BosonGameViewPluginBase::setCanvas(c);
 d->mEditorRandomMapWidget->setCanvas(c);
}

void BosonGameViewPluginDefault::setLocalPlayerIO(PlayerIO* io)
{
 BosonGameViewPluginBase::setLocalPlayerIO(io);
 d->mEditorRandomMapWidget->setLocalPlayerIO(io);
}

void BosonGameViewPluginDefault::updateBeforePaint()
{
 if (d->mGameMode) {

 } else {
	bool showRandomMap = boConfig->boolValue("EditorShowRandomMapGenerationWidget");
	if (showRandomMap != d->mEditorRandomMapWidget->isVisible()) {
		d->mEditorRandomMapWidget->setVisible(showRandomMap);
	}
 }
}

