/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Rivo Laks (rivolaks@hot.ee)

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

#ifndef BOSONEFFECTPROPERTIES_H
#define BOSONEFFECTPROPERTIES_H


#include "bosoneffect.h"
#include "bo3dtools.h"

#include <q3ptrlist.h>
#include <q3valuelist.h>
#include <q3intdict.h>
#include <qstring.h>


class BosonEffectProperties;
class KConfig;
class SpeciesTheme;
class SpeciesData;



#define boEffectPropertiesManager BosonEffectPropertiesManager::bosonEffectPropertiesManager()
/**
 * @short Manages effect properties
 *
 * This class holds list of all available effect properties, provides access to
 *  them and takes care of loading them.
 *
 * Reason for this class is that effects are now species-independant and thus
 *  cannot be managed by SpeciesTheme.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesManager
{
  public:
    BosonEffectPropertiesManager();
    ~BosonEffectPropertiesManager();

    static void initStatic();
    static void deleteStatic();
    static BosonEffectPropertiesManager* bosonEffectPropertiesManager();

    /**
    * Load the @ref BosonEffectProperties for all effect
    * speciefied in the effects.boson file of this theme.
    **/
    void loadEffectProperties();

    const BosonEffectProperties* effectProperties(quint32 id) const;

  protected:
    /**
     * Loads effect properties from given KConfig object, using given
     *  group.
     **/
    static BosonEffectProperties* loadEffectProperties(KConfig* cfg, const QString& group);
    /**
     * Creates new effect properties with given type.
     **/
    static BosonEffectProperties* newEffectProperties(const QString& type);

    static BosonEffectProperties* newParticleEffectProperties(const QString& type);

  private:
    Q3IntDict<BosonEffectProperties> mEffectProperties;

    static BosonEffectPropertiesManager* mManager;
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
     * Loads the properties from the given KConfig object, from given
     *  group.
     * This method handles inheritance, so unless you don't want to use it, you
     *  need to call this method at the beginning of your reimplementation.
     * @param inherited If true, inherited properties are being loaded, not the
     *  actual ones
     **/
    virtual bool load(KConfig* cfg, const QString& group, bool inherited = false);

    /**
     * Use this, if your effect properties need 2-level loading.
     * When this is called, all effect properties will have been loaded.
     **/
    virtual bool finishLoading(const BosonEffectPropertiesManager* manager)  { return true; }


    /**
     * Creates new effect at given position.
     * If rotation is specified and non-zero, effect will be rotated.
     * Note that if effect doesn't support position and/or rotation, they'll be
     *  ignored
     **/
    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const = 0;


    virtual BosonEffect::Type type() const = 0;

    /**
     * @return Unique id of this effect properties object.
     **/
    quint32 id() const  { return mId; }


    /**
     * @return Delay of the effect.
     * If this is >0, then effect will be delayed, which means that nothing
     *  happens for delay() seconds, and then effect will be started.
     **/
    float delay() const  { return mDelay; }


    // TODO: Maybe move those to BosonEffectPropertiesManager?
    /**
     * Static helper method for getting list of effect properties.
     * @return List of BosonEffectProperties with ids loaded from given kconfig
     *  object.
     * E.g. if you have "MyKey=1,2,4", then effect properties with ids 1, 2 and
     *  4 are returned.
     **/
    //static Q3PtrList<BosonEffectProperties> loadEffectProperties(KConfig* cfg, QString key);
    /**
     * Same as above, but uses already specified list of ids instead of loading
     *  them.
     **/
    static Q3PtrList<BosonEffectProperties> loadEffectProperties(const Q3ValueList<quint32>& ids);

    /**
     * Static helper method to create new effects.
     * For each properties object in given list, creates new effect(s) with
     *  given position and rotation and finally returns list of created effects.
     **/
    static Q3PtrList<BosonEffect> newEffects(const Q3PtrList<BosonEffectProperties>* properties,
        const BoVector3Fixed& pos = BoVector3Fixed(), const BoVector3Fixed& rot = BoVector3Fixed());
    /**
     * Same as above, but takes single BosonEffectProperties object as an
     *  argument instead of a list.
     * Use this to correctly have _list_ of effects created for collection
     *  effects.
     **/
    static Q3PtrList<BosonEffect> newEffects(const BosonEffectProperties* properties,
        const BoVector3Fixed& pos = BoVector3Fixed(), const BoVector3Fixed& rot = BoVector3Fixed());


  protected:
    quint32 mId;
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


    virtual bool load(KConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;


    const BoVector4Float& color() const  { return mColor; }
    float start() const  { return mStart; }
    float end() const  { return mEnd; }
    float radius() const  { return mRadius; }


  protected:
    void reset();

    BoVector4Float mColor;
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
    ~BosonEffectPropertiesFade();


    virtual BosonEffect::Type type() const  { return BosonEffect::Fade; };


    virtual bool load(KConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;


    const BoVector4Float& startColor() const  { return mStartColor; }
    const BoVector4Float& endColor() const  { return mEndColor; }
    const BoVector4Fixed& geometry() const  { return mGeometry; }
    float time() const  { return mTime; }
    const int* blendFunc() const  { return mBlendFunc; }
    BoShader* shader(int pass) const  { return mShader[pass]; }
    int downscale(int pass) const  { return mDownscaleFactor[pass]; }
    int passes() const  { return mPasses; }


  protected:
    void reset();


    BoVector4Float mStartColor;
    BoVector4Float mEndColor;
    BoVector4Fixed mGeometry;  // geometry: x, y, w, h
    float mTime;
    int mBlendFunc[2];

    int mPasses;
    BoShader** mShader;
    QString* mShaderFilename;
    int* mDownscaleFactor;
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


    virtual bool load(KConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;


    const BoVector4Float& startAmbient() const  { return mStartAmbientColor; }
    const BoVector4Float& startDiffuse() const  { return mStartDiffuseColor; }
    const BoVector4Float& startSpecular() const  { return mStartSpecularColor; }
    const BoVector4Float& endAmbient() const  { return mEndAmbientColor; }
    const BoVector4Float& endDiffuse() const  { return mEndDiffuseColor; }
    const BoVector4Float& endSpecular() const  { return mEndSpecularColor; }
    const BoVector3Float& startAttenuation() const  { return mStartAttenuation; }
    const BoVector3Float& endAttenuation() const  { return mEndAttenuation; }
    const BoVector3Fixed& position() const  { return mPosition; }
    float life() const  { return mLife; }


  protected:
    void reset();


    BoVector4Float mStartAmbientColor;
    BoVector4Float mStartDiffuseColor;
    BoVector4Float mStartSpecularColor;
    BoVector4Float mEndAmbientColor;
    BoVector4Float mEndDiffuseColor;
    BoVector4Float mEndSpecularColor;
    BoVector3Float mStartAttenuation;
    BoVector3Float mEndAttenuation;
    BoVector3Fixed mPosition;
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


    virtual bool load(KConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;


    const BoVector4Float& color() const  { return mColor; }
    float minLength() const  { return mMinLength; }
    float maxLength() const  { return mMaxLength; }
    float width() const  { return mWidth; }


  protected:
    void reset();

    BoVector4Float mColor;
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


    virtual bool load(KConfig* cfg, const QString& group, bool inherited = false);
    virtual bool finishLoading(const BosonEffectPropertiesManager* theme);


    /**
     * Do _not_ use this!
     * Use @ref newEffectsList instead.
     * @return 0
     **/
    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;
    Q3PtrList<BosonEffect> newEffectsList(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;


  protected:
    void reset();


    Q3PtrList<BosonEffectProperties> mEffects;
    Q3ValueList<quint32> mEffectIds;
};

#endif //BOSONEFFECTPROPERTIES_H

