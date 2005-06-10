/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BO3DTOOLS_H
#define BO3DTOOLS_H

// Most of boson's math is in code/math.
// This file has two purposes
// a) It serves as a convenience header that includes all headers in code/math.
//    Note that this adds only small compilation overhead, as code/math depends
//    on very few files only (e.g. not on Qt)
// b) It adds Qt/KDE/Boson dependend code. In code/math no Qt dependency is
//    allowed, so everything that Qt is required for is added here.

#include "math/bo3dtoolsbase.h"
#include "math/bofixed.h"
#include "math/bofrustum.h"
#include "math/boglmatrices.h"
#include "math/bomath.h"
#include "math/bomatrix.h"
#include "math/boplane.h"
#include "math/boquaternion.h"
#include "math/borect.h"
#include "math/bovector.h"

#include "bomath.h"

#include <bogl.h>

class QPoint;
class QDataStream;
class QString;
class QDomElement;

QDataStream& operator<<(QDataStream& s, const BoVector2Float& v);
QDataStream& operator>>(QDataStream& s, BoVector2Float& v);
QDataStream& operator<<(QDataStream& s, const BoVector2Fixed& v);
QDataStream& operator>>(QDataStream& s, BoVector2Fixed& v);

QDataStream& operator<<(QDataStream& s, const BoVector3Float& v);
QDataStream& operator<<(QDataStream& s, const BoVector3Fixed& v);
QDataStream& operator>>(QDataStream& s, BoVector3Float& v);
QDataStream& operator>>(QDataStream& s, BoVector3Fixed& v);

QDataStream& operator<<(QDataStream& s, const BoVector4Float& v);
QDataStream& operator<<(QDataStream& s, const BoVector4Fixed& v);
QDataStream& operator>>(QDataStream& s, BoVector4Float& v);
QDataStream& operator>>(QDataStream& s, BoVector4Fixed& v);


bool saveVector2AsXML(const BoVector2Float&, QDomElement& root, const QString& name);
bool saveVector2AsXML(const BoVector2Fixed&, QDomElement& root, const QString& name);
bool loadVector2FromXML(BoVector2Float*, const QDomElement& root, const QString& name);
bool loadVector2FromXML(BoVector2Fixed*, const QDomElement& root, const QString& name);

bool saveVector3AsXML(const BoVector3Float&, QDomElement& root, const QString& name);
bool saveVector3AsXML(const BoVector3Fixed&, QDomElement& root, const QString& name);
bool loadVector3FromXML(BoVector3Float*, const QDomElement& root, const QString& name);
bool loadVector3FromXML(BoVector3Fixed*, const QDomElement& root, const QString& name);

bool saveVector4AsXML(const BoVector4Float&, QDomElement& root, const QString& name);
bool saveVector4AsXML(const BoVector4Fixed&, QDomElement& root, const QString& name);
bool loadVector4FromXML(BoVector4Float*, const QDomElement& root, const QString& name);
bool loadVector4FromXML(BoVector4Fixed*, const QDomElement& root, const QString& name);

// convenience function to convert a BoVector into a string for debugging
QString debugStringVector(const BoVector3Float&, int prec);
QString debugStringVector(const BoVector3Fixed&, int prec);
QString debugStringVector(const BoVector4Float&, int prec);
QString debugStringVector(const BoVector4Fixed&, int prec);

QString debugStringQuaternion(const BoQuaternion&, int prec);

/**
 * Create a new BoMatrix object and load the specified OpenGL matrix.
 * @param GL_MODELVIEW_MATRIX, GL_PROJECTION_MATRIX or GL_TEXTURE_MATRIX.
 * Note that all other values (also e.g. GL_TEXTURE) will result in the
 * identity matrix and generate an error
**/
BoMatrix createMatrixFromOpenGL(GLenum matrix);


/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short collection class for misc 3d functions
 **/
class Bo3dTools : public Bo3dToolsBase
{
  public:
    Bo3dTools() : Bo3dToolsBase()
    {
    }

    /**
     * @return How many degrees you have to rotate around z-axis for y-axis to go
     * through (x, y). (i.e. angle between (0, 1) and (x, y) when (x, y) is a
     * normalized vector)
     * @author Rivo Laks <rivolaks@hot.ee>
     **/
    static bofixed rotationToPoint(bofixed x, bofixed y)
    {
      return Bo3dToolsBase::rotationToPoint(x, y);
    }

    /**
     * This is the inverse operation to @ref rotationToPoint.
     * It calculates point (x, y) which is at intersection of circle with @p radius
     * and line which is rotated by @p angle around z-axis.
     * @author Rivo Laks <rivolaks@hot.ee>
     **/
    static void pointByRotation(bofixed* x, bofixed* y, const bofixed& angle, const bofixed& radius)
    {
      return Bo3dToolsBase::pointByRotation(x, y, angle, radius);
    }
    static void pointByRotation(float* x, float* y, const float angle, const float radius)
    {
      return Bo3dToolsBase::pointByRotation(x, y, angle, radius);
    }

    /**
     * Convert @p deg, given in degree, into radians.
     * @return @p deg as radians.
     **/
    static bofixed deg2rad(bofixed deg)
    {
      return Bo3dToolsBase::deg2rad(deg);
    }
    /**
     * Convert @p rad, given in radians, into degree.
     * @return @p rad as degree.
     **/
    static bofixed rad2deg(bofixed rad)
    {
      return Bo3dToolsBase::rad2deg(rad);
    }

    /**
     * @param plane An array of size 4 representing a plane. The first 3 floats
     * are the normal vector, the 4th value the distance.
     * @return The distance of @pos from the @p plane. Negative if @p pos is
     * behind the @p plane.
     **/
    static float distanceFromPlane(const float* plane, const BoVector3Float& pos)
    {
      return Bo3dToolsBase::distanceFromPlane(plane, pos);
    }

    /**
     * See @ref BosonBigDisplayBase::extractFrustum for more information about this stuff.
     *
     * We use a bounding spere so that we can easily rotate it.
     * @return 0 if the object is not in the frustum (i.e. is not visible)
     * otherwise the distance from the near plane. We might use this for the
     * level of detail.
     * @param viewFrustum This is the viewFrustum, as it is used by @ref
     * BosonBigDisplayBase. The view frustum is a 6x4 matrix
     **/
    static float sphereInFrustum(const float* viewFrustum, const BoVector3Float& pos, float radius)
    {
      return Bo3dToolsBase::sphereInFrustum(viewFrustum, pos, radius);
    }
    static bofixed sphereInFrustum(const float* viewFrustum, const BoVector3Fixed& pos, bofixed radius)
    {
      return Bo3dToolsBase::sphereInFrustum(viewFrustum, pos, radius);
    }

    /**
     * See @ref BosonBigDisplayBase::extractFrustum for more information about this stuff.
     *
     * @param viewFrustum This is the viewFrustum, as it is used by @ref
     * BosonBigDisplayBase. The view frustum is a 6x4 matrix
     **/
    static bool boxInFrustum(const float* viewFrustum, const BoVector3Float& min, const BoVector3Float& max)
    {
      return Bo3dToolsBase::boxInFrustum(viewFrustum, min, max);
    }

    /**
     * This is similar to @ref sphereInFrustum, but will test whether the sphere
     * is completely in the frustum.
     *
     * @return 0 if the sphere is not in the frustum at all, 1 if it is
     * partially in the frustum and 2 if the complete sphere is in the frustum.
     **/
    static int sphereCompleteInFrustum(const float* viewFrustum, const BoVector3Float& pos, float radius)
    {
      return Bo3dToolsBase::sphereCompleteInFrustum(viewFrustum, pos, radius);
    }
    static int sphereCompleteInFrustum(const float* viewFrustum, const BoVector3Fixed& pos, bofixed radius)
    {
      return Bo3dToolsBase::sphereCompleteInFrustum(viewFrustum, pos, radius);
    }

    /**
     * @overload
     **/
    inline static float sphereInFrustum(const float* viewFrustum, float x, float y, float z, float radius)
    {
      return Bo3dToolsBase::sphereInFrustum(viewFrustum, x, y, z, radius);
    }

    /**
     * @param modelviewMatrix A reference to the modelview matrix. You can use
     * @ref BoMatrix::loadMatrix with GL_MODELVIEW_MATRIX for this.
     * @param projectionMatrix A reference to the projection matrix. You can use
     * @ref BoMatrix::loadMatrix with GL_PROJECTION_MATRIX for this.
     * @ref viewport The viewport. Use glGetIntegerv(GL_VIEWPORT, viewport) for
     * this.
     * @param x The x-coordinate (world-, aka OpenGL- coordinates) of the point
     * that is to be projected.
     * @param y The y-coordinate (world-, aka OpenGL- coordinates) of the point
     * that is to be projected.
     * @param z The z-coordinate (world-, aka OpenGL- coordinates) of the point
     * that is to be projected.
     * @param pos Here the result will get returned. It will be in
     * window-coordinates (relative to the OpenGL widget of course).
     **/
    static bool boProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, float x, float y, float z, BoVector2Float* pos);
    static bool boProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, float x, float y, float z, QPoint* pos);
    static bool boProject(const BoGLMatrices& matrices, float x, float y, float z, BoVector2Float* pos);
    static bool boProject(const BoGLMatrices& matrices, float x, float y, float z, QPoint* pos);

    static bool boUnProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const BoVector2Float& pos, BoVector3Float* v, float z = -1.0);
    static bool boUnProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const QPoint& pos, BoVector3Float* v, float z = -1.0);
    static bool boUnProject(const BoGLMatrices& matrices, const BoVector2Float& pos, BoVector3Float* v, float z = -1.0);
    static bool boUnProject(const BoGLMatrices& matrices, const QPoint& pos, BoVector3Float* v, float z = -1.0);

    /**
     * This is a frontend to @ref boUnProject. It calculates the world-(aka
     * OpenGL-) coordinates (@p posX, @p posY, @p posZ) of given window
     * coordinates @p pos.
     *
     * This function takes the correct z value at @p pos into account (usually
     * the z/depth at the mouse cursor).
     * @param useRealDepth If TRUE this function will calculate the real
     * coordinates at @p pos, if FALSE it will calculate the coordinate at
     * @p pos with z=0.0. This is useful for e.g. @ref mapDistance, where
     * different z values could deliver wrong values.
     **/
    static bool mapCoordinates(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const BoVector2Float& pos, float* posX, float* posY, float* posZ, bool useRealDepth = true);

    /**
     * @overload
     * This version provides @p pos as a @ref QPoint
     **/
    static bool mapCoordinates(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const QPoint& pos, float* posX, float* posY, float* posZ, bool useRealDepth = true);

    /**
     * @overload
     **/
    static bool mapCoordinates(const BoGLMatrices& matrices, const BoVector2Float& pos, float* posX, float* posY, float* posZ, bool useRealDepth = true);

    /**
     * @overload
     * This version provides @p pos as @ref QPoint
     **/
    static bool mapCoordinates(const BoGLMatrices& matrices, const QPoint& pos, float* posX, float* posY, float* posZ, bool useRealDepth = true);

    /**
     * Map distances from window to world coordinates.
     *
     * Sometimes you need to know how much a certain amount of pixels (from a
     * widget) is in world-coordinates. This is e.g. the case for mouse
     * scrolling - the player moved the mouse by a certain distance and you need
     * to scroll the scene by a certain distance.
     **/
    static bool mapDistance(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, int windx, int windy, float* dx, float* dy);

    /**
     * Workaround for a probable driver bug. ATI's properitary drivers do
     * not return 1.0 in the depth buffer, even if it is cleared with 1.0.
     * This function sets the value that is actually returned.
     *
     * Note that we still assume that 0.0 is returned properly, if it is
     * cleared with 0.0
     **/
    static void enableReadDepthBufferWorkaround(float _1_0_depthValue);
    static void disableReadDepthBufferWorkaround();

    /**
     * Check whether there is any OpenGL error and output an error to the
     * konsole if there is one. The error state is cleared then (since @ref
     * glGetError is used).
     *
     * @param error If non-NULL this is set to the error (if any) or to GL_NO_ERROR
     * @param errorString If non-NULL this is set to the error string describing
     * @p error, or to an empty string if there is no error. @ref gluErrorString
     * is used for this string.
     * @param errorName If non-NULL this is set to the name of the enum that describes
     * @p error, or an empty string if there is no error.
     **/
    static bool checkError(GLenum* error = 0, QString* errorString = 0, QString* errorName = 0);

    /**
     * @return OpenGL blending function given in QString.
     * str can be one of "GL_SRC_APLHA", "GL_ONE_MINUS_SRC_ALPHA", "GL_ONE",
     *  "GL_ZERO", "GL_DST_COLOR", "GL_ONE_MINUS_DST_COLOR", "GL_DST_APLHA",
     *  "GL_ONE_MINUS_DST_ALPHA", "GL_SRC_ALPHA_SATURATE", "GL_SRC_COLOR",
     *  "GL_ONE_MINUS_SRC_COLOR". If str is not one of these, GL_INVALID_ENUM
     *  is returned
     **/
    static GLenum string2GLBlendFunc(const QString& str);
};


#endif // BO3DTOOLS_H
/*
 * vim:et sw=2
 */
