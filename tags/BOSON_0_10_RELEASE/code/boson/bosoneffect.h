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

#ifndef BOSONEFFECT_H
#define BOSONEFFECT_H


#include "bo3dtools.h"


class QDomElement;
class KRandomSequence;
class QString;
class BosonEffectPropertiesFade;


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
    enum Type { Fog, Fade, Collection,
        // All particle effect subtypes _must_ come after Particle!
        Particle, ParticleGeneric, ParticleTrail };


    BosonEffect();
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
    const BoVector3& position() const  { return mPosition; }

    /**
     * Set effect's position.
     * Reimplement in subclasses, if necessary.
     **/
    virtual void setPosition(const BoVector3& pos)  { mPosition = pos; }

    /**
     * Sets effect's rotation to given values
     * Note that this only changes some effects, such as particle systems, for
     *  others, it does nothing. If your effect supports rotating, you will need
     *  to reimplement this method. Default implementation does nothing.
     * Note that all rotations should be done in order z, x, y, which is same as in
     *  the rendering code
     **/
    virtual void setRotation(const BoVector3& rotation)  {}


    /**
     * Updates the effect.
     * This can be used for dynamic effects, such as particle systems, which
     * change over time. Default implementation does nothing.
     * @param elapsed number of seconds elapsed since the last time that update() was called
     **/
    virtual void update(float elapsed)  {}


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


    virtual bool saveAsXML(QDomElement& root) const = 0;
    virtual bool loadFromXML(const QDomElement& root) = 0;


    /**
     * @return Random float number between min and max (inclusive).
     * You can use this to easily get random parameters between given limits.
     **/
    static float getFloat(float min, float max);

    /**
     * Initializes static things.
     * Call this _once_ at startup.
     * @param particletexdir Texture directory for particle textures.
     **/
    static void initStatic(const QString& particletexdir);


  protected:
    BoVector3 mPosition;
    bool mActive;


    static KRandomSequence* mRandom;
};



/**
 * @short Fog effect
 *
 * This is usual fog effect, you caan specify color of the fog and distances
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
    //BosonEffectFog(BosonEffectPropertiesFog* prop);
    /**
     * Constructs new fog effect.
     **/
    BosonEffectFog(const BoVector4& color, float start, float end, float radius = 0);


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
    const BoVector4& color() const  { return mColor; }


    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    float mStart;
    float mEnd;
    float mRadius;
    BoVector4 mColor;
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
 *  specify you own custom OpenGL blending functions, defaults are GL_SRC_ALPHA
 *  and GL_ONE_MINUS_SRC_ALPHA.
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
    const BoVector4& color() const  { return mColor; }
    /**
     * @return Current geometry of the effect.
     * This is a BoVector4 containing (in this order) x-coordinate of the
     *  lower-left corner of the rectangle, y-coordinate of the same corner,
     *  width, height.
     **/
    const BoVector4& geometry() const  { return mGeometry; }
    /**
     * @return Integer array (with 2 elements) containing OpenGL blending
     *  funtions for this effect.
     **/
    const int* blendFunc() const  { return mBlendFunc; }


    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    const BosonEffectPropertiesFade* mProperties;
    float mTimeLeft;
    BoVector4 mColor;
    BoVector4 mGeometry;
    int mBlendFunc[2];
};

#endif  //BOSONEFFECT_H

