/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
#include "opengloptions.h"
#include "opengloptions.moc"

#include "../bomemory/bodummymemory.h"
#include "configoptionwidgets.h"
#include "bosonconfig.h"
#include "bosoncursor.h"
#include "modelrendering/bosonmodeltextures.h"
#include "defines.h"
#include "bodebug.h"
#include "bogltooltip.h"
#include "bogroundrenderermanager.h"
#include "bo3dtools.h"
#include "bofullscreen.h"
#include "modelrendering/bomeshrenderermanager.h"
#include "bowaterrenderer.h"
#include "info/boinfo.h"
#include "botexture.h"
#include "bosongroundthemedata.h"
#include "boshader.h"

#include <kapplication.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtabwidget.h>


OpenGLOptions::OpenGLOptions(QWidget* parent)
	: QVBox(parent), OptionsWidget()
{
 QHBox* hbox = new QHBox(this);

 if (!boConfig->hasKey("RenderingSpeed")) {
	boConfig->addDynamicEntry(new BoConfigIntEntry(boConfig, "RenderingSpeed", (int)Defaults));
 }
 (void)new QLabel(i18n("Rendering speed/quality"), hbox);
 mRenderingSpeed = new QComboBox(hbox);
 mRenderingSpeed->insertItem(i18n("Defaults"));
 mRenderingSpeed->insertItem(i18n("Highest quality"));
 mRenderingSpeed->insertItem(i18n("Lowest quality (e.g. for software rendering)"));
 connect(mRenderingSpeed, SIGNAL(activated(int)), this, SLOT(slotRenderingSpeedChanged(int)));

 mAlignSelectBoxes = new ConfigOptionWidgetBool("AlignSelectionBoxes", this);
 mAlignSelectBoxes->setLabel(i18n("Align unit selection boxes to camera"));
 addConfigOptionWidget(mAlignSelectBoxes);

 QHBox* resolutionBox = new QHBox(this);
 (void)new QLabel(i18n("Resolution:"), resolutionBox);
 mResolution = new QComboBox(resolutionBox);
 mResolution->insertItem(i18n("Keep unchanged"));
 if (BoFullScreen::availableModes().count() != 0) {
	mResolution->insertStringList(BoFullScreen::availableModes());
	mResolution->setEnabled(true);
} else {
	mResolution->setEnabled(false);
 }
 mResolution->setCurrentItem(0);

 QPushButton* showDetails = new QPushButton(i18n("Show &Details"), this);
 showDetails->setToggleButton(true);
 connect(showDetails, SIGNAL(toggled(bool)), this, SLOT(slotShowDetails(bool)));

 mAdvanced = new AdvancedGLOptions(this);
 showDetails->setOn(false);
 slotShowDetails(false);

 mUpdateInterval = new ConfigOptionWidgetUInt("GLUpdateInterval", this);
 mUpdateInterval->setLabel(i18n("Update interval (low values hurt performance)"));
 mUpdateInterval->setRange(1, 100);
 QToolTip::add(mUpdateInterval, i18n("The update interval specifies after how many milli seconds the scene gets re-rendered and therefore directly influence the frames per seconds. But low values prevent boson from doing other important tasks and therefore you might end up in a game that takes several seconds until your units react to your commands! 20ms are usually a good value."));
 addConfigOptionWidget(mUpdateInterval);

}

OpenGLOptions::~OpenGLOptions()
{
}

void OpenGLOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 OptionsWidget::apply();
 boConfig->setIntValue("RenderingSpeed", renderingSpeed());

 if (mResolution->isEnabled() && mResolution->currentItem() > 0) {
	// first entry is "do not change", then a list, as provided by
	// BoFullScreen
	int index = mResolution->currentItem() - 1;
	if (!BoFullScreen::enterMode(index)) {
		boError() << k_funcinfo << "could not enter mode" << index << endl;
	 }
 }
 mResolution->setCurrentItem(0);

 mAdvanced->apply();

 boDebug(210) << k_funcinfo << "done" << endl;
}

void OpenGLOptions::setDefaults()
{
 OptionsWidget::setDefaults();
 mRenderingSpeed->setCurrentItem(0);
 slotRenderingSpeedChanged(0);

 mAdvanced->setDefaults();
}

void OpenGLOptions::loadFromConfigScript(const BosonConfigScript* script)
{
 BO_CHECK_NULL_RET(script);
 OptionsWidget::loadFromConfigScript(script);
 mAdvanced->loadFromConfigScript(script);
}

void OpenGLOptions::load()
{
 mAdvanced->setRenderingSpeed(boConfig->intValue("RenderingSpeed", Defaults));
 mResolution->setCurrentItem(0);

 mAdvanced->load();

 OptionsWidget::load();
}

void OpenGLOptions::slotRenderingSpeedChanged(int index)
{
 int speed = indexToRenderingSpeed(index);
 mAdvanced->setRenderingSpeed(speed);
 mRenderingSpeed->setCurrentItem(renderingSpeedToIndex(speed));
}

int OpenGLOptions::renderingSpeed() const
{
 return indexToRenderingSpeed(mRenderingSpeed->currentItem());
}

int OpenGLOptions::indexToRenderingSpeed(int index)
{
 switch (index) {
	default:
		boWarning(210) << k_funcinfo << "invalid index: " << index << endl;
	case 0:
		return Defaults;
	case 1:
		return BestQuality;
	case 2:
		return Fastest;
 }
 return BestQuality;
}

int OpenGLOptions::renderingSpeedToIndex(int speed)
{
 switch (speed) {
	default:
		boWarning(210) << k_funcinfo << "invalid value: " << speed << endl;
	case Defaults:
		return 0;
	case BestQuality:
		return 1;
	case Fastest:
		return 2;
 }
 return 0;
}

void OpenGLOptions::slotShowDetails(bool show)
{
 if (show) {
	mAdvanced->show();
 } else {
	mAdvanced->hide();
 }
}


AdvancedGLOptions::AdvancedGLOptions(OpenGLOptions* parent)
	: QVBox(parent)
{
 mOpenGLOptions = parent;
 QTabWidget* tabWidget = new QTabWidget(this);

 QHBox* hbox;

 QVBox* texturePage = new QVBox(tabWidget);
 tabWidget->addTab(texturePage, i18n("Textures"));
 hbox = new QHBox(texturePage);
 (void)new QLabel(i18n("Texture Filter"), hbox);
 mTextureFilter = new QComboBox(hbox);
 //mTextureFilter->insertItem(i18n("Use GL_NEAREST (fastest)"));
 mTextureFilter->insertItem(i18n("Use GL_LINEAR"));
 //mTextureFilter->insertItem(i18n("Use GL_NEAREST_MIPMAP_NEAREST"));
 //mTextureFilter->insertItem(i18n("Use GL_NEAREST_MIPMAP_LINEAR"));
 mTextureFilter->insertItem(i18n("Use GL_LINEAR_MIPMAP_NEAREST (bilinear)"));
 mTextureFilter->insertItem(i18n("Use GL_LINEAR_MIPMAP_LINEAR (trilinear)"));
 if (boTextureManager->maxTextureAnisotropy() >= 2) {
	mTextureFilter->insertItem(i18n("Low anisotropic"));
 }
 if (boTextureManager->maxTextureAnisotropy() >= 4) {
	mTextureFilter->insertItem(i18n("Medium anisotropic"));
 }
 if (boTextureManager->maxTextureAnisotropy() >= 8) {
	mTextureFilter->insertItem(i18n("High anisotropic"));
 }
 if (boTextureManager->maxTextureAnisotropy() >= 16) {
	mTextureFilter->insertItem(i18n("Very high anisotropic"));
 }

 QToolTip::add(mTextureFilter, i18n("Selects texture filtering method\nGL_LINEAR_MIPMAP_LINEAR looks very good.\nAnisotropic modes look even better but are slower."));

 mUseCompressedTextures = new ConfigOptionWidgetBool("TextureCompression", texturePage);
 mUseCompressedTextures->setLabel(i18n("Use compressed textures"));
 QToolTip::add(mUseCompressedTextures, i18n("Compressing textures reduces amount of memory used\nat the expense of very slight quality loss."));
 addConfigOptionWidget(mUseCompressedTextures);

 mUseColoredMipmaps = new ConfigOptionWidgetBool("TextureColorMipmaps", texturePage);
 mUseColoredMipmaps->setLabel(i18n("Use colored mipmaps"));
 QToolTip::add(mUseColoredMipmaps, i18n("Colors different mipmap levels so you can make difference between them.\nDo not enable this unless you know what you're doing."));
 addConfigOptionWidget(mUseColoredMipmaps);



 QVBox* miscPage = new QVBox(tabWidget);
 tabWidget->addTab(miscPage, i18n("Misc"));
 mUseLight = new ConfigOptionWidgetBool("UseLight", miscPage);
 mUseLight->setLabel(i18n("Enable light"));
 addConfigOptionWidget(mUseLight);

 mUseMaterials = new ConfigOptionWidgetBool("UseMaterials", miscPage);
 mUseMaterials->setLabel(i18n("Use materials"));
 QToolTip::add(mUseMaterials, i18n("Materials influence the way in which models (like units) are lighted. You can disable them to gain some performance."));
 addConfigOptionWidget(mUseMaterials);

 hbox = new QHBox(miscPage);
 (void)new QLabel(i18n("Ground render:"), hbox);
 mGroundRenderer = new QComboBox(hbox);

 mUseLOD = new ConfigOptionWidgetBool("UseLOD", miscPage);
 mUseLOD->setLabel(i18n("Use level of detail"));
 addConfigOptionWidget(mUseLOD);

 hbox = new QHBox(miscPage);
 (void)new QLabel(i18n("Default level of detail:"), hbox);
 mDefaultLOD = new QComboBox(hbox);
 mDefaultLOD->insertItem(i18n("All details (default)"));
 mDefaultLOD->insertItem(i18n("Lowest details"));
 mDefaultLOD->setCurrentItem(0);
 connect(mUseLOD, SIGNAL(signalValueChanged(bool)), hbox, SLOT(setEnabled(bool)));
 mSmoothShading = new ConfigOptionWidgetBool("SmoothShading", miscPage);
 mSmoothShading->setLabel(i18n("Smooth shade model"));
 addConfigOptionWidget(mSmoothShading);

 hbox = new QHBox(miscPage);
 (void)new QLabel(i18n("Mesh renderer:"), hbox);
 mMeshRenderer = new QComboBox(hbox);

 QVBox* shadersPage = new QVBox(tabWidget);
 tabWidget->addTab(shadersPage, i18n("Shaders"));
 mUseGroundShaders = new ConfigOptionWidgetBool("UseGroundShaders", shadersPage);
 mUseGroundShaders->setLabel(i18n("Use ground shaders (and shadows)"));
 mUseGroundShaders->setEnabled(BosonGroundThemeData::shadersSupported());
 addConfigOptionWidget(mUseGroundShaders);
 mUseUnitShaders = new ConfigOptionWidgetBool("UseUnitShaders", shadersPage);
 mUseUnitShaders->setLabel(i18n("Use unit shaders (and shadows)"));
 mUseUnitShaders->setEnabled(BosonGroundThemeData::shadersSupported());
 addConfigOptionWidget(mUseUnitShaders);

 hbox = new QHBox(shadersPage);
 (void)new QLabel(i18n("Shader quality:"), hbox);
 mShaderQuality = new QComboBox(hbox);
 mShaderQuality->setEnabled(BosonGroundThemeData::shadersSupported());
 mShaderQuality->insertItem(i18n("Extreme"));
 mShaderQuality->insertItem(i18n("High"));
 mShaderQuality->insertItem(i18n("Medium"));
 mShaderQuality->insertItem(i18n("Low"));
 mShaderQuality->setCurrentItem(2);
 hbox = new QHBox(shadersPage);
 (void)new QLabel(i18n("Shadow quality:"), hbox);
 mShadowQuality = new QComboBox(hbox);
 mShadowQuality->insertItem(i18n("High"));
 mShadowQuality->insertItem(i18n("Medium"));
 mShadowQuality->insertItem(i18n("Low"));
 mShadowQuality->setCurrentItem(0);
}

AdvancedGLOptions::~AdvancedGLOptions()
{
}

void AdvancedGLOptions::addConfigOptionWidget(ConfigOptionWidget* w)
{
 mOpenGLOptions->addConfigOptionWidget(w);
}

void AdvancedGLOptions::setRenderingSpeed(int speed)
{
 const BosonConfigScript* script = 0;
 switch ((OpenGLOptions::RenderingSpeed)speed) {
	default:
		boWarning(210) << k_funcinfo << "invalid value: " << speed << endl;
		break;
	case OpenGLOptions::Defaults:
		script = boConfig->configScript("DefaultRendering");
		break;
	case OpenGLOptions::BestQuality:
		script = boConfig->configScript("BestQualityRendering");
		break;
	case OpenGLOptions::Fastest:
		script = boConfig->configScript("FastRendering");
		break;
 }
 if (!script) {
	boWarning(210) << k_funcinfo << "no script found for speed " << speed << endl;
	return;
 }

 mOpenGLOptions->loadFromConfigScript(script);

}

int AdvancedGLOptions::textureFilter() const
{
 GLenum e;
 switch (mTextureFilter->currentItem()) {
	case 0:
		e = GL_LINEAR;
		break;
	case 1:
		e = GL_LINEAR_MIPMAP_NEAREST;
		break;
	default:
	case 2:
		e = GL_LINEAR_MIPMAP_LINEAR;
		break;
 }
 return (int)e;
}

void AdvancedGLOptions::setTextureFilter(int f)
{
 switch(f) {
	case GL_LINEAR:
		mTextureFilter->setCurrentItem(0);
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		mTextureFilter->setCurrentItem(1);
		break;
	default:
	case GL_LINEAR_MIPMAP_LINEAR:
		mTextureFilter->setCurrentItem(2);
		break;
 }
}

int AdvancedGLOptions::textureAnisotropy() const
{
 switch (mTextureFilter->currentItem()) {
	case 3:
		return 2;
		break;
	case 4:
		return 4;
		break;
	case 5:
		return 8;
		break;
	case 6:
		return 16;
		break;
	default:
		return 1;
		break;
 }
}

void AdvancedGLOptions::setTextureAnisotropy(int a)
{
 switch(a) {
	case 2:
		mTextureFilter->setCurrentItem(3);
		break;
	case 4:
		mTextureFilter->setCurrentItem(4);
		break;
	case 8:
		mTextureFilter->setCurrentItem(5);
		break;
	case 16:
		mTextureFilter->setCurrentItem(6);
		break;
	default:
		if (mTextureFilter->currentItem() >= 3) {
			mTextureFilter->setCurrentItem(2);
		}
		break;
 }
}

unsigned int AdvancedGLOptions::defaultLOD() const
{
 switch (mDefaultLOD->currentItem()) {
	case 0:
		return 0;
	default:
	case 1:
		// just a random, very high, number.
		// this will result in lowest details possible.
		return 5000;
 }
 return 0;
}

void AdvancedGLOptions::setDefaultLOD(unsigned int l)
{
 switch (l) {
	case 0:
		mDefaultLOD->setCurrentItem(0);
		break;
	default:
		mDefaultLOD->setCurrentItem(1);
		break;
 }
}


QString AdvancedGLOptions::shaderSuffixes()
{
 QString suffixes;
 if (mShaderQuality->currentItem() <= 0) {
	suffixes += "-vhi,";
 }
 if (mShaderQuality->currentItem() <= 1) {
	suffixes += "-hi,";
 }
 if (mShaderQuality->currentItem() <= 2) {
	suffixes += "-med,";
 }
 if (mShaderQuality->currentItem() <= 3) {
	suffixes += "-low,";
 }

 return suffixes;
}

int AdvancedGLOptions::shaderSuffixesToIndex(const QString& suffixes)
{
 if (suffixes.contains("vhi")) {
	return 0;
 } else if (suffixes.contains("hi")) {
	return 1;
 } else if (suffixes.contains("med")) {
	return 2;
 } else if (suffixes.contains("low")) {
	return 3;
 } else {
	return 2;  // Fallback to medium
 }
}

int AdvancedGLOptions::shadowMapResolutionToIndex(int resolution)
{
 if (resolution == 2048) {
	return 0;
 } else if (resolution == 1024) {
	return 1;
 } else if (resolution == 512) {
	return 2;
 } else {
	return 0;  // Fallback to 2048
 }
}

int AdvancedGLOptions::indexToShadowMapResolution(int index)
{
 const int resolutions[] = { 2048, 1024, 512 };
 if (index > 2) {
	return 2048;  // default
 } else {
	return resolutions[index];
 }
}

void AdvancedGLOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;

 QString changesThatNeedRestart;
 bool resetTexParameter = false;
 bool reloadTextures = false;
 if (boConfig->intValue("TextureFilter") != textureFilter()) {
	boConfig->setIntValue("TextureFilter", textureFilter());
	resetTexParameter = true;
	// TODO: reload only if mipmapping was turned on/off
	reloadTextures = true;
 }
 if (boConfig->intValue("TextureAnisotropy") != textureAnisotropy()) {
	boConfig->setIntValue("TextureAnisotropy", textureAnisotropy());
	resetTexParameter = true;
 }

 if(boConfig->boolValue("TextureCompression") != mUseCompressedTextures->isChecked()) {
	boConfig->setBoolValue("TextureCompression", mUseCompressedTextures->isChecked());
	reloadTextures = true;
 }
 if(boConfig->boolValue("TextureColorMipmaps") != mUseColoredMipmaps->isChecked()) {
	boConfig->setBoolValue("TextureColorMipmaps", mUseColoredMipmaps->isChecked());
	reloadTextures = true;
 }

 if(!changesThatNeedRestart.isEmpty()) {
	KMessageBox::information(this, i18n("Following changes take effect after restart:\n%1").arg(changesThatNeedRestart));
 }
 if (resetTexParameter) {
	// maybe display a message box now, asking for permission to reset the
	// parameters. currently we just reset them - leave it at this as long
	// as no problems appear
	boDebug(210) << k_funcinfo << "resetting all textures parameters" << endl;
	boTextureManager->textureFilterChanged();
 }
 if (reloadTextures) {
	boDebug(210) << k_funcinfo << "reloading all textures" << endl;
	boTextureManager->reloadTextures();
 }

 boConfig->setUIntValue("DefaultLOD", defaultLOD());

 if (!BoMeshRendererManager::manager()->makeRendererCurrent(mMeshRenderer->currentText())) {
	KMessageBox::sorry(this, i18n("Failed at making mesh renderer %1 current. Trying to use a default renderer (expect a crash, if we fail)...").arg(mMeshRenderer->currentText()));
	if (!BoMeshRendererManager::manager()->makeRendererCurrent(QString::null)) {
		KMessageBox::sorry(this, i18n("Failed at setting a default mesh renderer. Quitting now"));
		kapp->exit(1);
	}
 }
 if (!BoGroundRendererManager::manager()->makeRendererCurrent(mGroundRenderer->currentText())) {
	KMessageBox::sorry(this, i18n("Failed at making ground renderer %1 current. Trying to use a default renderer (expect a crash, if we fail)...").arg(mGroundRenderer->currentText()));
	if (!BoGroundRendererManager::manager()->makeRendererCurrent(QString::null)) {
		KMessageBox::sorry(this, i18n("Failed at setting a default ground renderer. Quitting now"));
		kapp->exit(1);
	}
 }
 BosonGroundThemeData::setUseGroundShaders(mUseGroundShaders->isChecked());

 if (shaderSuffixes() != boConfig->stringValue("ShaderSuffixes")) {
	boConfig->setStringValue("ShaderSuffixes", shaderSuffixes());
	boShaderManager->reloadShaders();
 }
 if (indexToShadowMapResolution(mShadowQuality->currentItem()) != boConfig->intValue("ShadowMapResolution")) {
	boConfig->setIntValue("ShadowMapResolution", indexToShadowMapResolution(mShadowQuality->currentItem()));
 }


 boDebug(210) << k_funcinfo << "done" << endl;
}

void AdvancedGLOptions::setDefaults()
{
}

void AdvancedGLOptions::loadFromConfigScript(const BosonConfigScript* script)
{

 // these are not handled by ConfigOptionWidgets:
 const BoConfigIntEntry* textureFilter = script->intValue("TextureFilter");
 if (textureFilter) {
	setTextureFilter(textureFilter->value());
 }
 const BoConfigStringEntry* groundRenderer = script->stringValue("GroundRendererClass");
 if (groundRenderer) {
	setCurrentGroundRenderer(groundRenderer->value());
 }

 const BoConfigStringEntry* shaderSuffixes = script->stringValue("ShaderSuffixes");
 if (shaderSuffixes) {
	mShaderQuality->setCurrentItem(shaderSuffixesToIndex(shaderSuffixes->value()));
 }
 const BoConfigIntEntry* shadowMapResolution = script->intValue("ShadowMapResolution");
 if (shadowMapResolution) {
	mShadowQuality->setCurrentItem(shadowMapResolutionToIndex(shadowMapResolution->value()));
 }
 const BoConfigStringEntry* meshRenderer = script->stringValue("MeshRenderer");
 if (meshRenderer) {
	setCurrentMeshRenderer(meshRenderer->value());
 }
}

void AdvancedGLOptions::load()
{
 setTextureFilter(boConfig->intValue("TextureFilter"));
 setTextureAnisotropy(boConfig->intValue("TextureAnisotropy"));
 setDefaultLOD(boConfig->uintValue("DefaultLOD", 0));

 setCurrentMeshRenderer(BoMeshRendererManager::manager()->currentRendererName());
 setCurrentGroundRenderer(BoGroundRendererManager::manager()->currentRendererName());
 mShaderQuality->setCurrentItem(shaderSuffixesToIndex(boConfig->stringValue("ShaderSuffixes")));
 mShadowQuality->setCurrentItem(shadowMapResolutionToIndex(boConfig->intValue("ShadowMapResolution")));
}


void AdvancedGLOptions::setCurrentMeshRenderer(const QString& renderer)
{
 mMeshRenderer->clear();
 QStringList renderers = BoMeshRendererManager::manager()->availableRenderers();
 mMeshRenderer->insertStringList(renderers);

 for (int i = 0; i < mMeshRenderer->count(); i++) {
	if (mMeshRenderer->text(i) == renderer) {
		mMeshRenderer->setCurrentItem(i);
		return;
	}
 }
}

void AdvancedGLOptions::setCurrentGroundRenderer(const QString& renderer)
{
 mGroundRenderer->clear();
 QStringList renderers = BoGroundRendererManager::manager()->availableRenderers();
 mGroundRenderer->insertStringList(renderers);
 for (int i = 0; i < mGroundRenderer->count(); i++) {
	if (mGroundRenderer->text(i) == renderer) {
		mGroundRenderer->setCurrentItem(i);
		return;
	}
 }
}



WaterOptions::WaterOptions(QWidget* parent) : QVBox(parent), OptionsWidget()
{
 mShaders = new ConfigOptionWidgetBool("WaterShaders", this);
 mShaders->setLabel(i18n("Enable shaders"));
 mShaders->setEnabled(boWaterRenderer->supportsShaders());
 addConfigOptionWidget(mShaders);

 mReflections = new ConfigOptionWidgetBool("WaterReflections", this);
 mReflections->setLabel(i18n("Enable reflections"));
 mReflections->setEnabled(boWaterRenderer->supportsReflections());
 addConfigOptionWidget(mReflections);

 mTranslucency = new ConfigOptionWidgetBool("WaterTranslucency", this);
 mTranslucency->setLabel(i18n("Enable translucent water"));
 mTranslucency->setEnabled(boWaterRenderer->supportsTranslucency());
 addConfigOptionWidget(mTranslucency);

 mBumpmapping = new ConfigOptionWidgetBool("WaterBumpmapping", this);
 mBumpmapping->setLabel(i18n("Enable bumpmapped water"));
 mBumpmapping->setEnabled(boWaterRenderer->supportsBumpmapping());
 addConfigOptionWidget(mBumpmapping);

 mAnimatedBumpmaps = new ConfigOptionWidgetBool("WaterAnimatedBumpmaps", this);
 mAnimatedBumpmaps->setLabel(i18n("Enable animated bumpmaps"));
 mAnimatedBumpmaps->setEnabled(boWaterRenderer->supportsBumpmapping());
 addConfigOptionWidget(mAnimatedBumpmaps);

 connect(mShaders, SIGNAL(signalValueChanged(bool)), this, SLOT(slotEnableShaders(bool)));
}

WaterOptions::~WaterOptions()
{
 boDebug(210) << k_funcinfo << endl;
}

void WaterOptions::loadFromConfigScript(const BosonConfigScript* script)
{
 OptionsWidget::loadFromConfigScript(script);
}

void WaterOptions::apply()
{
 boDebug(210) << k_funcinfo << endl;
 OptionsWidget::apply();
 boWaterRenderer->reloadConfiguration();
 boDebug(210) << k_funcinfo << "done" << endl;
}

void WaterOptions::setDefaults()
{
 OptionsWidget::setDefaults();
 slotEnableShaders(mShaders->isChecked());
}

void WaterOptions::load()
{
 OptionsWidget::load();
 slotEnableShaders(mShaders->isChecked());
}

void WaterOptions::slotEnableShaders(bool)
{
 mReflections->setEnabled(!mShaders->isChecked());
 mTranslucency->setEnabled(!mShaders->isChecked());
 mBumpmapping->setEnabled(!mShaders->isChecked());
}


