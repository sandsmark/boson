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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosongameviewplugindefault.h"
#include "bosongameviewplugindefault.moc"

#include "../../../bomemory/bodummymemory.h"
#include "boufo.h"
#include "../../bosonconfig.h"
#include "editorrandommapwidget.h"
#include "boselectiondebugwidget.h"
#include "bodebugconfigswitches.h"
#include "bonetworktrafficwidget.h"

#include <bodebug.h>

class BosonGameViewPluginDefaultPrivate
{
public:
	BosonGameViewPluginDefaultPrivate()
	{
		mSelectionDebugWidget = 0;
		mDebugRenderingConfig = 0;
		mEditorRandomMapWidget = 0;
		mNetworkTrafficWidget = 0;
	}
	bool mInitialized;
	bool mGameMode;
	BoSelectionDebugWidget* mSelectionDebugWidget;
	BoDebugConfigSwitches* mDebugRenderingConfig;
	EditorRandomMapWidget* mEditorRandomMapWidget;
	BoNetworkTrafficWidget* mNetworkTrafficWidget;
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
 boDebug() << k_funcinfo << endl;
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

 d->mSelectionDebugWidget = new BoSelectionDebugWidget();
 ufoWidget()->addWidget(d->mSelectionDebugWidget);

 BoUfoWidget* w = new BoUfoWidget();
 w->setLayoutClass(BoUfoWidget::UHBoxLayout);
 d->mDebugRenderingConfig = new BoDebugConfigSwitches();
 d->mDebugRenderingConfig ->setTemplate(BoDebugConfigSwitches::Rendering);
 w->addWidget(d->mDebugRenderingConfig);
 BoUfoWidget* stretch1 = new BoUfoWidget();
 stretch1->setStretch(1);
 w->addWidget(stretch1);
 ufoWidget()->addWidget(w);

 d->mEditorRandomMapWidget = new EditorRandomMapWidget();
 ufoWidget()->addWidget(d->mEditorRandomMapWidget);

 w = new BoUfoWidget();
 w->setLayoutClass(BoUfoWidget::UHBoxLayout);
 d->mNetworkTrafficWidget = new BoNetworkTrafficWidget();
 w->addWidget(d->mNetworkTrafficWidget);
 BoUfoWidget* stretch2 = new BoUfoWidget();
 stretch2->setStretch(1);
 w->addWidget(stretch2);
 ufoWidget()->addWidget(w);

 BoUfoWidget* stretch = new BoUfoWidget();
 stretch->setStretch(1);
 ufoWidget()->addWidget(stretch);

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
// d->mSelectionDebugWidget->setVisible(mode);
 d->mEditorRandomMapWidget->setVisible(!mode);
}

void BosonGameViewPluginDefault::setCanvas(const BosonCanvas* c)
{
 BosonGameViewPluginBase::setCanvas(c);
// d->mSelectionDebugWidget->setCanvas(c);
 d->mEditorRandomMapWidget->setCanvas(c);
}

void BosonGameViewPluginDefault::setLocalPlayerIO(PlayerIO* io)
{
 BosonGameViewPluginBase::setLocalPlayerIO(io);
 d->mSelectionDebugWidget->setLocalPlayerIO(io);
 d->mEditorRandomMapWidget->setLocalPlayerIO(io);
}

void BosonGameViewPluginDefault::updateBeforePaint()
{
 bool showUnitDebug = boConfig->boolValue("ShowUnitDebugWidget");
 bool showRenderingConfig = boConfig->boolValue("debug_rendering_config");
 bool showNetworkTraffic = boConfig->boolValue("debug_network_traffic");
 if (showUnitDebug != d->mSelectionDebugWidget->isVisible()) {
	d->mSelectionDebugWidget->setVisible(showUnitDebug);
 }
 if (showUnitDebug) {
	d->mSelectionDebugWidget->update();
 }
 if (showRenderingConfig != d->mDebugRenderingConfig->isVisible()) {
	d->mDebugRenderingConfig->setVisible(showRenderingConfig);
 }
 if (showNetworkTraffic != d->mNetworkTrafficWidget->isVisible()) {
	d->mNetworkTrafficWidget->setVisible(showNetworkTraffic);
 }
 if (showRenderingConfig) {
	d->mDebugRenderingConfig->slotUpdate();
 }
 if (showNetworkTraffic) {
	d->mNetworkTrafficWidget->slotUpdate();
 }

 if (d->mGameMode) {
 } else {
	bool showRandomMap = boConfig->boolValue("EditorShowRandomMapGenerationWidget");
	if (showRandomMap != d->mEditorRandomMapWidget->isVisible()) {
		d->mEditorRandomMapWidget->setVisible(showRandomMap);
	}
 }
}

void BosonGameViewPluginDefault::slotSelectionChanged(BoSelection* selection)
{
 BosonGameViewPluginBase::slotSelectionChanged(selection);
 d->mSelectionDebugWidget->setSelection(selection);
}

