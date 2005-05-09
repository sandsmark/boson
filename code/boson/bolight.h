/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOLIGHT_H
#define BOLIGHT_H

#include "bo3dtools.h"


template<class T> class QValueVector;
class BoLight;

/**
 *
 **/
class BoLightManager
{
  public:
    BoLightManager();
    ~BoLightManager();

    BoLight* createLight();
    void deleteLight(int id);
    BoLight* light(int id);

    int nextFreeId();
    // TODO: make this private?
    void setLight(int id, BoLight* light);
    const QValueVector<BoLight*>* lights();

    static void initStatic();
    static void deleteStatic();
    static BoLightManager* manager();

  private:
    void init();
    QValueVector<BoLight*>* mLights;

    static BoLightManager* mLightManager;
};

/**
 * @short Class for OpenGL lights
 *
 * In OpenGL there different kinds of light, the main classes are positional and
 * directional lights. You can chose one of them in @ref setPosition.
 *
 * Directional lights are infinitely far away and therefore the rays of light
 * are parallel when they arrive at the scene. With @ref setPosition you don't
 * actually specify the position (it is infinitely far away!), but rather the
 * direction of the light. An example of a directional light is the sun.
 *
 * Positional lights have an actual position. Examples are lamps or fireplaces.
 * The position vector specifies the actual position of the light, i.e. where
 * the rays of light are originating from.
 *
 * The rays of light of a positional light are radiating (of course) in all
 * directions, if you do not want that you can use a "spotlight" in OpenGL.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoLight
{
  public:
    BoLight();
    ~BoLight();

    bool isEnabled() const  { return mEnabled; }
    void setEnabled(bool e);

    const BoVector4Float& ambient() const  { return mAmbient; }
    const BoVector4Float& diffuse() const  { return mDiffuse; }
    const BoVector4Float& specular() const  { return mSpecular; }

    void setAmbient(const BoVector4Float& a);
    void setDiffuse(const BoVector4Float& d);
    void setSpecular(const BoVector4Float& s);

    const BoVector4Float& position() const  { return mPos; }

    /**
     * Set the position of the light. The w component of @p pos must bei either
     * 0 or 1.
     *
     * If w is 0, the light is a directional light, i.e. one that is infinitely
     * far away. the position vector is the direction of the light then.
     *
     * If w is 1, the light is a positional light. The position vector (x,y,z)
     * specifies the position of the light. The light of a positional light goes
     * in all directions, unless you make it a spotlight.
     **/
    void setPosition(const BoVector4Float& pos);

    /**
     * @return The (x,y,z) vector of the position
     **/
    BoVector3Float position3() const  { return BoVector3Float(mPos.x(), mPos.y(), mPos.z()); }

    /**
     * Change the position (or for a directional light the direction) of the
     * light. The w component (whether the light is directional or positional)
     * is not changed.
     **/
    void setPosition3(const BoVector3Float& pos)  { setPosition(BoVector4Float(pos.x(), pos.y(), pos.z(), mPos.w())); }

    /**
     * Make the light a directional light of @p directional is TRUE, otherwise
     * make it a positional light.
     **/
    void setDirectional(bool directional)  { setPosition(BoVector4Float(mPos.x(), mPos.y(), mPos.z(), directional ? 0.0f : 1.0f)); }

    /**
     * @return Whether the light is directional. A directional light is
     * infinitely away, the @ref position3 specifies actually the direction of
     * the light.
     **/
    bool isDirectional() const  { return mPos.w() == 0.0f; }

    /**
     * @return The opposite of @ref isDirectional. The light waves of a
     * positional light go in all directions (unless for spotlights) and @ref
     * position3 specifies the actual position of the light.
     **/
    bool isPositional() const  { return mPos.w() != 0.0f; }

    void setConstantAttenuation(float a);
    void setLinearAttenuation(float a);
    void setQuadraticAttenuation(float a);
    void setAttenuation(const BoVector3Float& a);

    float constantAttenuation() const  { return mAttenuation.x(); }
    float linearAttenuation() const  { return mAttenuation.y(); }
    float quadraticAttenuation() const  { return mAttenuation.z(); }
    BoVector3Float attenuation() const  { return mAttenuation; }

    int id() const  { return mId; }

    void refreshPosition();

    /**
     * Updates all OpenGL states that this light affects.
     * You usually won't need to call it because the states are automatically
     *  updated when you change light's parameters. It might be necessary after
     *  doing e.g. context switches though.
     **/
    void updateStates();

  private:
    BoVector4Float mAmbient;
    BoVector4Float mDiffuse;
    BoVector4Float mSpecular;
    BoVector4Float mPos;
    BoVector3Float mAttenuation;
    bool mEnabled;
    int mId;
};

#endif // BOLIGHT_H
