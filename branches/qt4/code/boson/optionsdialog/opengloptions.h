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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef OPENGLOPTIONS_H
#define OPENGLOPTIONS_H

#include <q3vbox.h>
#include <q3valuelist.h>

#include "optionswidgets.h"
#include "global.h"

class AdvancedGLOptions;

class OpenGLOptions : public Q3VBox, public OptionsWidget
{
	Q_OBJECT
public:
	enum RenderingSpeed {
		Defaults = 0,
		Fastest = 10,
		BestQuality = 100
	};

public:
	OpenGLOptions(QWidget* parent);
	~OpenGLOptions();

	virtual void loadFromConfigScript(const BosonConfigScript* script);
	virtual void apply();
	virtual void setDefaults();
	virtual void load();

protected:
	/**
	 * @return See @ref RenderingSpeed
	 **/
	int renderingSpeed() const;

	/**
	 * Create a @ref RenderingSpeed value from an index of the combo box
	 * (see @ref slotRenderingSpeedChanged)
	 **/
	static int indexToRenderingSpeed(int index);
	static int renderingSpeedToIndex(int speed);

protected slots:
	void slotRenderingSpeedChanged(int);
	void slotShowDetails(bool);

signals:
	void signalFontChanged(const BoFontInfo& font);
	void signalOpenGLSettingsUpdated();

private:
	ConfigOptionWidgetBool* mAlignSelectBoxes;
	QComboBox* mRenderingSpeed;
	QComboBox* mResolution;
	AdvancedGLOptions* mAdvanced;

	ConfigOptionWidgetUInt* mUpdateInterval;
};

class AdvancedGLOptions : public Q3VBox
{
	Q_OBJECT
public:
	AdvancedGLOptions(OpenGLOptions* parent);
	~AdvancedGLOptions();

	/**
	 * Set the rendering speed. This is preset collection of rendering
	 * settings, so that users don't have to play around with GL_LINEAR,
	 * GL_NEAREST and all that.
	 * @param speed See @ref RenderingSpeed
	 **/
	void setRenderingSpeed(int speed);

	virtual void loadFromConfigScript(const BosonConfigScript* script);
	virtual void apply();
	virtual void setDefaults();
	virtual void load();

protected:
	void addConfigOptionWidget(ConfigOptionWidget*);

protected:
	int textureFilter() const;
	void setTextureFilter(int f);

	int textureAnisotropy() const;
	void setTextureAnisotropy(int f);

	void setDefaultLOD(unsigned int l);
	unsigned int defaultLOD() const;

	void setCurrentMeshRenderer(const QString&);
	void setCurrentGroundRenderer(const QString&);

	QString shaderSuffixes();
	int shaderSuffixesToIndex(const QString& suffixes);

	int shadowMapResolutionToIndex(int resolution);
	int indexToShadowMapResolution(int index);



private:
	OpenGLOptions* mOpenGLOptions;

	QComboBox* mTextureFilter;
	ConfigOptionWidgetBool* mUseCompressedTextures;
	ConfigOptionWidgetBool* mUseColoredMipmaps;
	ConfigOptionWidgetBool* mUseLight;
	ConfigOptionWidgetBool* mUseMaterials;
	QComboBox* mGroundRenderer;
	ConfigOptionWidgetBool* mUseGroundShaders;
	ConfigOptionWidgetBool* mUseUnitShaders;
	QComboBox* mShaderQuality;
	QComboBox* mShadowQuality;
	ConfigOptionWidgetBool* mUseLOD;
	QComboBox* mDefaultLOD;
	ConfigOptionWidgetBool* mSmoothShading;
	QComboBox* mMeshRenderer;
};

class WaterOptions : public Q3VBox, public OptionsWidget
{
	Q_OBJECT
public:
	WaterOptions(QWidget* parent);
	~WaterOptions();

	virtual void loadFromConfigScript(const BosonConfigScript* script);
	virtual void apply();
	virtual void setDefaults();
	virtual void load();

protected slots:
	void slotEnableShaders(bool);

private:
	ConfigOptionWidgetBool* mShaders;
	ConfigOptionWidgetBool* mReflections;
	ConfigOptionWidgetBool* mTranslucency;
	ConfigOptionWidgetBool* mBumpmapping;
	ConfigOptionWidgetBool* mAnimatedBumpmaps;
};


#endif
