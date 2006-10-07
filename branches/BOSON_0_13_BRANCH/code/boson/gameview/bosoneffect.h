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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef BOSONEFFECT_H
#define BOSONEFFECT_H


#include "bo3dtools.h"


class QDomElement;
class KRandomSequence;
class QString;
class BosonEffectProperties;
class BosonEffectPropertiesFade;
class BosonEffectPropertiesFog;
class BosonEffectPropertiesLight;
class BosonEffectPropertiesBulletTrail;
class BoLight;
class BoShader;


/**
 * @short Base class for all effects
 *
 * This class provides few things which are common to all effects. It also
 * provides more or less general API to ease managing of collection of effects.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 */
class BosonEffect
{
  public:
    /**
     * Describes type of the effect
     */
    enum Type { Fog, Fade, Collection, Light, BulletTrail,
        // All particle effect subtypes _must_ come after Particle!
        Particle, ParticleGeneric, ParticleTrail, ParticleEnvironmental };


    BosonEffect(const BosonEffectProperties* prop = 0);
    virtual ~BosonEffect();

    /**
     * @return Type of the effect
     * Must be reimplemented in subclasses
     **/
    virtual Type type() const = 0;


    /**
     * @return Position of this particle effect (absolute position, in opengl
     *  coordinates).
     * Note that for some effects, position isn't used.
     **/
    const BoVector3Fixed& position() const  { return mPosition; }

    /**
     * Set effect's position.
     * Reimplement in subclasses, if necessary.
     **/
    virtual void setPosition(const BoVector3Fixed& pos)  { mPosition = pos; }

    /**
     * @return Rotation of this particle effect.
     * Note that for some effects, rotation isn't used.
     * Also note that all rotations should be done in order z, x, y, which is
     *  same as in the rendering code
     **/
    const BoVector3Fixed& rotation() const  { return mRotation; }

    /**
     * Sets effect's rotation to given values
     * Note that this only changes some effects, such as particle systems, for
     *  others, it does nothing. If your effect supports rotating, you will need
     *  to reimplement this method.
     * Note that all rotations should be done in order z, x, y, which is same
     *  as in the rendering code
     **/
    virtual void setRotation(const BoVector3Fixed& rotation)  { mRotation = rotation; }


    /**
     * Updates the effect.
     * This can be used for dynamic effects, such as particle systems, which
     *  change over time.
     * Default implementation decreases delay and calls @ref start once it's 0.
     * @param elapsed number of seconds elapsed since the last time that update() was called
     **/
    virtual void update(float elapsed);

    void doDelayedUpdates();


    /**
     * @return Whether the effect is active or not
     * Unactive effects will be deleted
     **/
    virtual bool isActive() const  { return mActive; }
    /**
     * Obsoletes the effect, making it non-active, so that it will be deleted.
     * Effects may delay becoming non-active, e.g. particle system may just stop
     * emitting particles when made obsolete, but actually become non-active
     *  when it's last particle has died.
     **/
    virtual void makeObsolete()  { mActive = false; }
    /**
     * @return Whether the effect has been started.
     * This is used for delayed effect. If you set effect's delay to e.g. 1
     *  second, then after the effect is created, nothing will be done for 1
     *  second. After that, effect is "started".
     * Non-started effects will not be rendered.
     **/
    bool hasStarted() const  { return mStarted; }
    /**
     * Called when effect is started (see @ref hasStarted for description about
     *  what "started" means). Reimplement if necessary (e.g. to create intial
     *  particles in particle effect).
     * Default implementation just sets started status to true.
     **/
    virtual void start();

    /**
     * @return effect's owner id.
     * Owner id is id of the BosonItem which owns this effect. This means that
     *  effect is "tied" to this item and whenever the item moves or rotates,
     *  the effect is also moved/rotated.
     *  If effect is not owned by any item, it's ownerId is 0.
     **/
    unsigned int ownerId() const  { return mOwnerId; }
    /**
     * Sets effect's owner id to given value.
     * See @ref ownerId for explanation about owner ids.
     **/
    void setOwnerId(unsigned int id)  { mOwnerId = id; }


    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


    /**
     * @return Random float number between min and max (inclusive).
     * You can use this to easily get random parameters between given limits.
     **/
    static float getRandomFloat(float min, float max);
    /**
     * @return Random bofixed number between min and max (inclusive).
     * You can use this to easily get random parameters between given limits.
     **/
    static bofixed getRandomBofixed(bofixed min, bofixed max);

    /**
     * Initializes static things.
     * Call this _once_ at startup.
     * @param particletexdir Texture directory for particle textures.
     **/
    static void initStatic(const QString& particletexdir);

    /**
     * @return Whether effect supports delayed updates.
     * With delayed updates, effect is updates only when it's visible. But if
     *  your effect always needs to be updates, no matter if it's visible or
     *  not, reimplement this method to return false.
     **/
    virtual bool supportsDelayedUpdates() const  { return true; }
    /**
     * @return Maximum number of delayed updates that will be done when effect
     *  becomes visible.
     **/
    virtual int maxDelayedUpdates() const  { return 40; }

    /**
     * Marks effect for delayed update.
     * If effect does not support delayed updates, it is updated immediately.
     * Note that elapsed should always be 0.05
     **/
    void markUpdate(float elapsed);

  protected:
    BoVector3Fixed mPosition;
    BoVector3Fixed mRotation;
    bool mActive;
    bool mStarted;
    float mDelay;
    unsigned int mOwnerId;
    // TODO: maybe remove mProperties pointers from subclasses and use only
    //  this one?
    const BosonEffectProperties* mGeneralProperties;

    int mUpdateCounter;


    static KRandomSequence* mRandom;
};



/**
 * @short Fog effect
 *
 * This is usual fog effect, you can specify color of the fog and distances
 *  where the fog starts and ends. Everything in the world will be then fogged
 *  according to their distance from the camera (linear fog model is used).
 *
 * ((You can also set radius for the fog, which specifies how far from fog's
 *  @ref position it takes effect. E.g. if radius is 20, then if camera is 20 or
 *  more units away from the position of the fog, it will not take effect. If
 *  it's 10 units away, it will be made 2 times "lighter".)) <- this is not yet true
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectFog : public BosonEffect
{
  public:
    /**
     * Constructs new fog effect.
     **/
    BosonEffectFog(const BosonEffectPropertiesFog* prop);
    /**
     * Constructs new fog effect.
     **/
    BosonEffectFog(const BoVector4Float& color, float start, float end, float radius = 0);


    virtual Type type() const  { return Fog; }


    /**
     * @return Fog start distance.
     * This specifies how close to the camera fogging will start, e.g. if you
     *  set it to 10, then objects 10 or less units away from camera will not be
     *  fogged at all (assuming endDist > startDist).
     * Note that you can also set start distance to be bigger than end distance.
     *  In this case, the fog will surround the camera: the closer the object is
     *  to the camera, the more fogged it will be.
     **/
    float startDistance() const  { return mStart; }
    /**
     * @return Fog end distance.
     * It specified how far from the camera the fog will end. Beyond this
     *  distance, everything will be totally fogged, so you won't be able to see
     *  anything there.
     */
    float endDistance() const  { return mEnd; }
    // NOT YET SUPPORTED!!!
    float radius() const  { return mRadius; }
    /**
     * @return Color of the fog
     **/
    const BoVector4Float& color() const  { return mColor; }


    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    const BosonEffectPropertiesFog* mProperties;
    float mStart;
    float mEnd;
    float mRadius;
    BoVector4Float mColor;
};



/**
 * @short Effect for drawing blended 2d rectangles on the screen.
 *
 * With this effect, you can draw arbitrary 2d rectangles on the screen. This
 *  can be used for e.g. fade-out or fade-in effects in cinematics. You can set
 *  the geometry of the rectangle with BoVector4, format is (x, y, w, h) and
 *  coordinates are relative to window: (0; 0) is lower-left corner of the
 *  window and (1; 1) is upper-right corner.
 * You can specify start and end colors for the rectangle and a life-time.
 *  Rectangle will smoothly change from start color to end color during it's
 *  life (given in seconds). If life is set to -1, fade effect will last until
 *  you call @ref makeObsolete and no blending will be done. You can also
 *  specify your own custom OpenGL blending functions, defaults are
 *  GL_SRC_ALPHA and GL_ONE_MINUS_SRC_ALPHA.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectFade : public BosonEffect
{
  public:
    /**
     * Construct new fade effect using specified properties
     **/
    BosonEffectFade(const BosonEffectPropertiesFade* prop);


    virtual Type type() const  { return Fade; }


    virtual void update(float elapsed);
    virtual bool supportsDelayedUpdates() const  { return false; }
    /**
     * Make fade effect obsolete (meaning that it will be deleted in the next
     *  advance call).
     * Note that this only has effect for effects with infinite lifetime (-1),
     *  for others, it does nothing.
     **/
    virtual void makeObsolete();


    /**
     * @return Current color of the effect.
     * This will change over time for dynamic effects.
     **/
    const BoVector4Float& color() const  { return mColor; }
    /**
     * @return Current geometry of the effect.
     * This is a BoVector4 containing (in this order) x-coordinate of the
     *  lower-left corner of the rectangle, y-coordinate of the same corner,
     *  width, height.
     **/
    const BoVector4Fixed& geometry() const  { return mGeometry; }
    /**
     * @return Integer array (with 2 elements) containing OpenGL blending
     *  funtions for this effect.
     **/
    const int* blendFunc() const  { return mBlendFunc; }
    /**
     * @return Shader which should be used when rendering this effect.
     * When it's NULL, no shader should be used.
     **/
    BoShader* shader(int pass) const;
    int downscale(int pass) const;
    int passes() const;


    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    const BosonEffectPropertiesFade* mProperties;
    float mTimeLeft;
    BoVector4Float mColor;
    BoVector4Fixed mGeometry;
    int mBlendFunc[2];
};



/**
 * @short Effect for using real OpenGL lights.
 *
 * With this effect, you can add real dynamic OpenGL lights to the scene.
 * Note though, that there is a limit for how many lights can be in scene
 *  at once and that too many lights can result in noticeable slowdowns. So
 *  it's best to use them only for few things, when you really need them, and
 *  make their lifetime short.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectLight : public BosonEffect
{
  public:
    /**
     * Construct new light effect using specified properties
     **/
    BosonEffectLight(const BosonEffectPropertiesLight* prop);
    virtual ~BosonEffectLight();


    virtual Type type() const  { return Light; }


    virtual void update(float elapsed);
    virtual bool supportsDelayedUpdates() const  { return false; }

    virtual void start();

    virtual void setPosition(const BoVector3Fixed& pos);

    /**
     * Make light obsolete (meaning that it will be deleted in the next advance
     *  call).
     * Note that this only has effect for effects with infinite lifetime (-1),
     *  for others, it does nothing.
     **/
    virtual void makeObsolete();


    const BoLight* light() const  { return mLight; }

    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    const BosonEffectPropertiesLight* mProperties;
    float mTimeLeft;
    BoLight* mLight;
};



/**
 * @short Effect for rendering bullet trails.
 *
 * Bullet trails are rendered as short lines, which have random length
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectBulletTrail : public BosonEffect
{
  public:
    /**
     * Construct new line effect using specified properties
     **/
    BosonEffectBulletTrail(const BosonEffectPropertiesBulletTrail* prop, const BoVector3Fixed& pos);
    virtual ~BosonEffectBulletTrail();


    virtual Type type() const  { return BulletTrail; }


    virtual void update(float elapsed);


    const BoVector3Fixed& startPoint() const  { return mStart; }
    const BoVector3Fixed& endPoint() const  { return mEnd; }
    const BoVector4Float& color() const;
    float width() const;


    virtual int maxDelayedUpdates() const  { return 5; }

    /**
     * Make line obsolete (meaning that it will be deleted in the next advance
     *  call).
     * Note that this only has effect for effects with infinite lifetime (-1),
     *  for others, it does nothing.
     **/
    virtual void makeObsolete();

    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    const BosonEffectPropertiesBulletTrail* mProperties;
    BoVector3Fixed mLastPos;
    short int mAdvanced;
    bool mShouldMakeObsolete;
    BoVector3Fixed mStart;
    BoVector3Fixed mEnd;
};

/*
 * vim: et sw=2
 */
#endif  //BOSONEFFECT_H

