/*
    This file is part of the Boson game
    Copyright (C) 2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOSONEFFECTPROPERTIES_H
#define BOSONEFFECTPROPERTIES_H


#include "bosoneffect.h"

#include <qptrlist.h>
#include <qvaluelist.h>


class BosonEffectProperties;
class KSimpleConfig;
class QString;
class BoVector3;
class SpeciesTheme;
class SpeciesData;


/**
 * @short Factory class for loading effect properties
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesFactory
{
  public:
    /**
     * Loads effect properties from given KSimpleConfig object, using given
     *  group.
     **/
    static BosonEffectProperties* loadEffectProperties(KSimpleConfig* cfg, const QString& group);
    /**
     * Creates new effect properties with given type.
     **/
    static BosonEffectProperties* newEffectProperties(const QString& type);

  protected:
    static BosonEffectProperties* newParticleEffectProperties(const QString& type);
};


/**
 * @short Base class for effect properties
 *
 * This provides general API for loading properties and for creating new effects
 *  using loaded properties.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectProperties
{
  public:
    /**
     * Creates new BosonEffectProperties object.
     * Note that you shouldn't use this object until you also load the
     *  properties using @ref load.
     **/
    BosonEffectProperties();
    virtual ~BosonEffectProperties();

    /**
     * Loads the properties from the given KSimpleConfig object, from given
     *  group.
     * This method handles inheritance, so unless you don't want to use it, you
     *  need to call this method at the beginning of your reimplementation.
     * @param inherited If true, inherited properties are being loaded, not the
     *  actual ones
     **/
    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);

    /**
     * Use this, if your effect properties need 2-level loading.
     * When this is called, all effect properties will be loaded.
     **/
    virtual bool finishLoading(const SpeciesData* theme)  { return true; }


    /**
     * Creates new effect at given position.
     * If rotation is specified and non-zero, effect will be rotated.
     * Note that if effect doesn't support position and/or rotation, they'll be
     *  ignored
     **/
    virtual BosonEffect* newEffect(const BoVector3& pos, const BoVector3& rot = BoVector3()) const = 0;


    virtual BosonEffect::Type type() const = 0;

    /**
     * @return Unique id of this effect properties object.
     **/
    unsigned long int id() const  { return mId; }


    /**
     * @return Delay of the effect.
     * If this is >0, then effect will be delayed, which means that nothing
     *  happens for delay() seconds, and then effect will be started.
     **/
    float delay() const  { return mDelay; }


    /**
     * Static helper method for getting list of effect properties.
     * @return List of BosonEffectProperties with ids loaded from given kconfig
     *  object.
     * E.g. if you have "MyKey=1,2,4", then effect properties with ids 1, 2 and
     *  4 are returned.
     **/
    static QPtrList<BosonEffectProperties> loadEffectProperties(KSimpleConfig* cfg, QString key, SpeciesTheme* theme);
    /**
     * Same as above, but uses already specified list of ids instead of loading
     *  them.
     **/
    static QPtrList<BosonEffectProperties> loadEffectProperties(QValueList<unsigned long int> ids, SpeciesTheme* theme);

    /**
     * Static helper method to create new effects.
     * For each properties object in given list, creates new effect(s) with
     *  given position and rotation and finally returns list of created effects.
     **/
    static QPtrList<BosonEffect> newEffects(const QPtrList<BosonEffectProperties>* properties,
        const BoVector3& pos = BoVector3(), const BoVector3& rot = BoVector3());
    /**
     * Same as above, but takes single BosonEffectProperties object as an
     *  argument instead of a list.
     * Use this to correctly have _list_ of effects created for collection
     *  effects.
     **/
    static QPtrList<BosonEffect> newEffects(const BosonEffectProperties* properties,
        const BoVector3& pos = BoVector3(), const BoVector3& rot = BoVector3());


  protected:
    unsigned long int mId;
    float mDelay;
};



/**
 * @short Properties for fog effect
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesFog : public BosonEffectProperties
{
  public:
    BosonEffectPropertiesFog();


    virtual BosonEffect::Type type() const  { return BosonEffect::Fog; };


    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3& pos, const BoVector3& rot = BoVector3()) const;


    const BoVector4& color() const  { return mColor; }
    float start() const  { return mStart; }
    float end() const  { return mEnd; }
    float radius() const  { return mRadius; }


  protected:
    void reset();

    BoVector4 mColor;
    float mStart;
    float mEnd;
    float mRadius;
};



/**
 * @short Properties for fade effect
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesFade : public BosonEffectProperties
{
  public:
    BosonEffectPropertiesFade();


    virtual BosonEffect::Type type() const  { return BosonEffect::Fade; };


    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3& pos, const BoVector3& rot = BoVector3()) const;


    const BoVector4& startColor() const  { return mStartColor; }
    const BoVector4& endColor() const  { return mEndColor; }
    const BoVector4& geometry() const  { return mGeometry; }
    float time() const  { return mTime; }
    const int* blendFunc() const  { return mBlendFunc; }


  protected:
    void reset();


    BoVector4 mStartColor;
    BoVector4 mEndColor;
    BoVector4 mGeometry;  // geometry: x, y, w, h
    float mTime;
    int mBlendFunc[2];
};



/**
 * @short Properties for light effect
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesLight : public BosonEffectProperties
{
  public:
    BosonEffectPropertiesLight();


    virtual BosonEffect::Type type() const  { return BosonEffect::Light; };


    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3& pos, const BoVector3& rot = BoVector3()) const;


    const BoVector4& startAmbient() const  { return mStartAmbientColor; }
    const BoVector4& startDiffuse() const  { return mStartDiffuseColor; }
    const BoVector4& startSpecular() const  { return mStartSpecularColor; }
    const BoVector4& endAmbient() const  { return mEndAmbientColor; }
    const BoVector4& endDiffuse() const  { return mEndDiffuseColor; }
    const BoVector4& endSpecular() const  { return mEndSpecularColor; }
    const BoVector3& startAttenuation() const  { return mStartAttenuation; }
    const BoVector3& endAttenuation() const  { return mEndAttenuation; }
    const BoVector3& position() const  { return mPosition; }
    float life() const  { return mLife; }


  protected:
    void reset();


    BoVector4 mStartAmbientColor;
    BoVector4 mStartDiffuseColor;
    BoVector4 mStartSpecularColor;
    BoVector4 mEndAmbientColor;
    BoVector4 mEndDiffuseColor;
    BoVector4 mEndSpecularColor;
    BoVector3 mStartAttenuation;
    BoVector3 mEndAttenuation;
    BoVector3 mPosition;
    float mLife;
};



/**
 * @short Properties for bullet trail effect
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesBulletTrail : public BosonEffectProperties
{
  public:
    BosonEffectPropertiesBulletTrail();


    virtual BosonEffect::Type type() const  { return BosonEffect::BulletTrail; };


    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3& pos, const BoVector3& rot = BoVector3()) const;


    const BoVector4& color() const  { return mColor; }
    float minLength() const  { return mMinLength; }
    float maxLength() const  { return mMaxLength; }
    float width() const  { return mWidth; }


  protected:
    void reset();

    BoVector4 mColor;
    float mMinLength;
    float mMaxLength;
    float mWidth;
    float mProbability;
};



/**
 * @short Properties for collection effect
 *
 * Collection is special in that there is actually no BosonEffectCollection
 *  class. Instead, you should use @ref newEffectsList method which will return
 *  a list of created effects.
 * Collection effect is mostly meant to group list of effects together. This
 *  way, you can e.g. make a complex explosion consisting of several effects,
 *  and then use this collection effect as destroyed effect for many units. When
 *  you want to remove or add one effect from this explosion, you'll only have
 *  to edit collection effect and all units will automagically get the changes.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesCollection : public BosonEffectProperties
{
  public:
    BosonEffectPropertiesCollection();


    virtual BosonEffect::Type type() const  { return BosonEffect::Collection; };


    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);
    virtual bool finishLoading(const SpeciesData* theme);


    /**
     * Do _not_ use this!
     * Use @ref newEffectsList instead.
     * @return 0
     **/
    virtual BosonEffect* newEffect(const BoVector3& pos, const BoVector3& rot = BoVector3()) const;
    QPtrList<BosonEffect> newEffectsList(const BoVector3& pos, const BoVector3& rot = BoVector3()) const;


  protected:
    void reset();


    QPtrList<BosonEffectProperties> mEffects;
    QValueList<unsigned long int> mEffectIds;
};

#endif //BOSONEFFECTPROPERTIES_H

