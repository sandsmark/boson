/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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

#include "bo3dtools.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bosonprofiling.h"
#include <bogl.h>

#include <qstring.h>
#include <qdatastream.h>
#include <qdom.h>

#include <math.h>

static float workaround_depth_value_1_0 = 1.0f;
static bool workaround_depth_value_enabled = false;

QString debugStringVector(const BoVector2Float& v, int prec)
{
  return QString("%1,%2").arg(v.x(), 0, 'f', prec).arg(v.y(), 0, 'f', prec);
}

QString debugStringVector(const BoVector2Fixed& v, int prec)
{
  BoVector2Float v2;
  v2.set(v[0], v[1]);
  return debugStringVector(v2, prec);
}

bool saveVector2AsXML(const BoVector2Fixed& v, QDomElement& root, const QString& name)
{
  root.setAttribute(name + ".x", v[0]);
  root.setAttribute(name + ".y", v[1]);
  return true;
}

bool saveVector2AsXML(const BoVector2Float& v, QDomElement& root, const QString& name)
{
  root.setAttribute(name + ".x", v[0]);
  root.setAttribute(name + ".y", v[1]);
  return true;
}

bool loadVector2FromXML(BoVector2Float* v, const QDomElement& root, const QString& name)
{
  BoVector2Float backup = *v;
  bool ok;
  bool ret = true;

  v->setX(root.attribute(name + ".x").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".x' attribute ('" <<
        root.attribute(name + ".x") << "')" << endl;
    ret = false;
    v->setX(backup.x());
  }
  v->setY(root.attribute(name + ".y").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".y' attribute ('" <<
        root.attribute(name + ".y") << "')" << endl;
    ret = false;
    v->setY(backup.y());
  }
  return ret;
}

bool loadVector2FromXML(BoVector2Fixed* v, const QDomElement& root, const QString& name)
{
  BoVector2Fixed backup = *v;
  bool ok;
  bool ret = true;

  v->setX(root.attribute(name + ".x").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".x' attribute ('" <<
        root.attribute(name + ".x") << "')" << endl;
    ret = false;
    v->setX(backup.x());
  }
  v->setY(root.attribute(name + ".y").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".y' attribute ('" <<
        root.attribute(name + ".y") << "')" << endl;
    ret = false;
    v->setY(backup.y());
  }
  return ret;
}

bool saveMatrixAsXML(const BoMatrix& m, QDomElement& root)
{
  QDomDocument doc = root.ownerDocument();
  for(int row = 1; row <= 4; row++)
  {
    QDomElement r = doc.createElement("Row");
    for(int column = 1; column <= 4; column++)
    {
      QDomElement c = doc.createElement("Column");
      c.setAttribute("Value", m.element(row, column));
      r.appendChild(c);
    }
    root.appendChild(r);
  }
  return true;
}

bool loadMatrixFromXML(BoMatrix* m, const QDomElement& root)
{
  int row = 1;
  for(QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling())
  {
    QDomElement r = n.toElement();
    if(r.isNull() || r.tagName() != "Row")
    {
      continue;
    }
    int column = 1;
    for(QDomNode n2 = r.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
    {
      QDomElement c = n2.toElement();
      if(c.isNull() || c.tagName() != "Column")
      {
        continue;
      }
      QString value = c.attribute("Value");
      if(value.isEmpty())
      {
        boError() << k_funcinfo << "no Value for column " << column << " of row " << row << endl;
        return false;
      }
      bool ok;
      float v = value.toDouble(&ok);
      if(!ok)
      {
        boError() << k_funcinfo << "invalid Value for column " << column << " of row " << row << endl;
        return false;
      }
      m->setElement(row, column, v);
      column++;
    }
    if (column != 5)
    {
      boError() << k_funcinfo << "need exactly 4 columns. have: "<< column - 1 << endl;
      return false;
    }
    row++;
  }
  if (row != 5)
  {
    boError() << k_funcinfo << "need exactly 4 rows. have: "<< row - 1 << endl;
    return false;
  }
  return true;
}

QDataStream& operator<<(QDataStream& s, const BoVector2Fixed& v)
{
  return s << v[0] << v[1];
}

QDataStream& operator>>(QDataStream& s, BoVector2Fixed& v)
{
  bofixed x, y;
  s >> x >> y;
  v.set(x, y);
  return s;
}

QDataStream& operator<<(QDataStream& s, const BoVector2Float& v)
{
  return s << v[0] << v[1];
}

QDataStream& operator>>(QDataStream& s, BoVector2Float& v)
{
  float x, y;
  s >> x >> y;
  v.set(x, y);
  return s;
}


QString debugStringVector(const BoVector3Float& v, int prec)
{
  return QString("%1,%2,%3").arg(v.x(), 0, 'f', prec).arg(v.y(), 0, 'f', prec).arg(v.z(), 0, 'f', prec);
}

QString debugStringVector(const BoVector3Fixed& v, int prec)
{
  BoVector3Float v2;
  v2.set(v[0], v[1], v[2]);
  return debugStringVector(v2, prec);
}

bool saveVector3AsXML(const BoVector3Fixed& v, QDomElement& root, const QString& name)
{
  root.setAttribute(name + ".x", v[0]);
  root.setAttribute(name + ".y", v[1]);
  root.setAttribute(name + ".z", v[2]);
  return true;
}

bool saveVector3AsXML(const BoVector3Float& v, QDomElement& root, const QString& name)
{
  root.setAttribute(name + ".x", v[0]);
  root.setAttribute(name + ".y", v[1]);
  root.setAttribute(name + ".z", v[2]);
  return true;
}

bool loadVector3FromXML(BoVector3Float* v, const QDomElement& root, const QString& name)
{
  BoVector3Float backup = *v;
  bool ok;
  bool ret = true;

  v->setX(root.attribute(name + ".x").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".x' attribute ('" <<
        root.attribute(name + ".x") << "')" << endl;
    ret = false;
    v->setX(backup.x());
  }
  v->setY(root.attribute(name + ".y").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".y' attribute ('" <<
        root.attribute(name + ".y") << "')" << endl;
    ret = false;
    v->setY(backup.y());
  }
  v->setZ(root.attribute(name + ".z").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".z' attribute ('" <<
        root.attribute(name + ".z") << "')" << endl;
    ret = false;
    v->setZ(backup.z());
  }
  return ret;
}

bool loadVector3FromXML(BoVector3Fixed* v, const QDomElement& root, const QString& name)
{
  BoVector3Fixed backup = *v;
  bool ok;
  bool ret = true;

  v->setX(root.attribute(name + ".x").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".x' attribute ('" <<
        root.attribute(name + ".x") << "')" << endl;
    ret = false;
    v->setX(backup.x());
  }
  v->setY(root.attribute(name + ".y").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".y' attribute ('" <<
        root.attribute(name + ".y") << "')" << endl;
    ret = false;
    v->setY(backup.y());
  }
  v->setZ(root.attribute(name + ".z").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".z' attribute ('" <<
        root.attribute(name + ".z") << "')" << endl;
    ret = false;
    v->setZ(backup.z());
  }
  return ret;
}

QDataStream& operator<<(QDataStream& s, const BoVector3Float& v)
{
  return s << (float)v[0] << (float)v[1] << (float)v[2];
}

QDataStream& operator<<(QDataStream& s, const BoVector3Fixed& v)
{
  return s << v[0] << v[1] << v[2];
}

QDataStream& operator>>(QDataStream& s, BoVector3Float& v)
{
  float x, y, z;
  s >> x >> y >> z;
  v.set(x, y, z);
  return s;
}

QDataStream& operator>>(QDataStream& s, BoVector3Fixed& v)
{
  bofixed x, y, z;
  s >> x >> y >> z;
  v.set(x, y, z);
  return s;
}

QString debugStringVector(const BoVector4Float& v, int prec)
{
  return QString("%1,%2,%3,%4").arg(v.x(), 0, 'f', prec).arg(v.y(), 0, 'f', prec).arg(v.z(), 0, 'f', prec).arg(v.w(), 0, 'f', prec);
}

QString debugStringVector(const BoVector4Fixed& v, int prec)
{
  BoVector4Float v2;
  v2.set(v[0], v[1], v[2], v[3]);
  return debugStringVector(v2, prec);
}

bool saveVector4AsXML(const BoVector4Float& v, QDomElement& root, const QString& name)
{
  root.setAttribute(name + ".x", v[0]);
  root.setAttribute(name + ".y", v[1]);
  root.setAttribute(name + ".z", v[2]);
  root.setAttribute(name + ".w", v[3]);
  return true;
}

bool saveVector4AsXML(const BoVector4Fixed& v, QDomElement& root, const QString& name)
{
  root.setAttribute(name + ".x", v[0]);
  root.setAttribute(name + ".y", v[1]);
  root.setAttribute(name + ".z", v[2]);
  root.setAttribute(name + ".w", v[3]);
  return true;
}

bool loadVector4FromXML(BoVector4Float* v, const QDomElement& root, const QString& name)
{
  BoVector4Float backup = *v;
  bool ok;
  bool ret = true;

  v->setX(root.attribute(name + ".x").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".x' attribute ('" <<
        root.attribute(name + ".x") << "')" << endl;
    ret = false;
    v->setX(backup.x());
  }
  v->setY(root.attribute(name + ".y").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".y' attribute ('" <<
        root.attribute(name + ".y") << "')" << endl;
    ret = false;
    v->setY(backup.y());
  }
  v->setZ(root.attribute(name + ".z").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".z' attribute ('" <<
        root.attribute(name + ".z") << "')" << endl;
    ret = false;
    v->setZ(backup.z());
  }
  v->setW(root.attribute(name + ".w").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".w' attribute ('" <<
        root.attribute(name + ".w") << "')" << endl;
    ret = false;
    v->setW(backup.w());
  }
  return ret;
}

bool loadVector4FromXML(BoVector4Fixed* v, const QDomElement& root, const QString& name)
{
  BoVector4Fixed backup = *v;
  bool ok;
  bool ret = true;

  v->setX(root.attribute(name + ".x").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".x' attribute ('" <<
        root.attribute(name + ".x") << "')" << endl;
    ret = false;
    v->setX(backup.x());
  }
  v->setY(root.attribute(name + ".y").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".y' attribute ('" <<
        root.attribute(name + ".y") << "')" << endl;
    ret = false;
    v->setY(backup.y());
  }
  v->setZ(root.attribute(name + ".z").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".z' attribute ('" <<
        root.attribute(name + ".z") << "')" << endl;
    ret = false;
    v->setZ(backup.z());
  }
  v->setW(root.attribute(name + ".w").toFloat(&ok));
  if(!ok)
  {
    boError() << k_funcinfo << "Error loading '" << name << ".w' attribute ('" <<
        root.attribute(name + ".w") << "')" << endl;
    ret = false;
    v->setW(backup.w());
  }
  return ret;
}

QDataStream& operator<<(QDataStream& s, const BoVector4Float& v)
{
  return s << (float)v[0] << (float)v[1] << (float)v[2] << (float)v[3];
}

QDataStream& operator<<(QDataStream& s, const BoVector4Fixed& v)
{
  return s << v[0] << v[1] << v[2] << v[3];
}

QDataStream& operator>>(QDataStream& s, BoVector4Float& v)
{
  float x, y, z, w;
  s >> x >> y >> z >> w;
  v.set(x, y, z, w);
  return s;
}

QDataStream& operator>>(QDataStream& s, BoVector4Fixed& v)
{
  bofixed x, y, z, w;
  s >> x >> y >> z >> w;
  v.set(x, y, z, w);
  return s;
}

QString debugStringQuaternion(const BoQuaternion& q, int prec)
{
  return QString("(%1,(%2))").arg(q.w(), 0, 'f', prec).arg(debugStringVector(q.v(), prec));
}

BoMatrix createMatrixFromOpenGL(GLenum matrix)
{
  GLfloat data[16];
  switch (matrix)
  {
    case GL_MODELVIEW_MATRIX:
    case GL_PROJECTION_MATRIX:
    case GL_TEXTURE_MATRIX:
      break;
    default:
      boError() << k_funcinfo << "Invalid matrix enum " << (int)matrix << endl;
      return BoMatrix();
  }
  glGetFloatv(matrix, data);
  return BoMatrix(data);
}


bool Bo3dTools::boProject(const BoGLMatrices& m, GLfloat x, GLfloat y, GLfloat z, BoVector2Float* pos)
{
  return boProject(m.modelviewMatrix(), m.projectionMatrix(), m.viewport(),
      x, y, z, pos);
}

bool Bo3dTools::boProject(const BoGLMatrices& m, GLfloat x, GLfloat y, GLfloat z, QPoint* pos)
{
  BoVector2Float p;
  bool ret = boProject(m, x, y, z, &p);
  *pos = QPoint((int)p.x(), (int)p.y());
  return ret;
}

bool Bo3dTools::boProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, GLfloat x, GLfloat y, GLfloat z, QPoint* pos)
{
  BoVector2Float p;
  bool ret = boProject(modelviewMatrix, projectionMatrix, viewport, x, y, z, &p);
  *pos = QPoint((int)p.x(), (int)p.y());
  return ret;
}

bool Bo3dTools::boProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, GLfloat x, GLfloat y, GLfloat z, BoVector2Float* pos)
{
  // AB: once again - most credits go to mesa :)
  BoVector4Float v;
  v.setX(x);
  v.setY(y);
  v.setZ(z);
  v.setW(1.0f);

  BoVector4Float v2;
  modelviewMatrix.transform(&v2, &v);
  projectionMatrix.transform(&v, &v2);

  if(v[3] == 0.0f)
  {
    printf("ERROR: boProject(): Can't divide by zero\n");
    return false;
  }
  v2.setX(v[0] / v[3]);
  v2.setY(v[1] / v[3]);
  v2.setZ(v[2] / v[3]);

  pos->setX((int)(viewport[0] + (1 + v2[0]) * viewport[2] / 2));
  pos->setY((int)(viewport[1] + (1 + v2[1]) * viewport[3] / 2));

  // return the actual window y
  pos->setY(viewport[3] - pos->y());
  return true;
}

bool Bo3dTools::boUnProject(const BoGLMatrices& m, const BoVector2Float& pos, BoVector3Float* ret, float z)
{
  return boUnProject(m.modelviewMatrix(), m.projectionMatrix(), m.viewport(),
      pos, ret, z);
}

bool Bo3dTools::boUnProject(const BoGLMatrices& m, const QPoint& pos, BoVector3Float* ret, float z)
{
  return boUnProject(m, BoVector2Float((float)pos.x(), (float)pos.y()), ret, z);
}

bool Bo3dTools::boUnProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const QPoint& pos, BoVector3Float* ret, float z)
{
  return boUnProject(modelviewMatrix, projectionMatrix, viewport,
                     BoVector2Float((float)pos.x(), (float)pos.y()), ret, z);
}

bool Bo3dTools::boUnProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const BoVector2Float& pos, BoVector3Float* ret, float depth)
{
  PROFILE_METHOD
  // AB: most code is from mesa's gluUnProject().
  BoMatrix A(projectionMatrix);
  BoMatrix B;

  // A = A x Modelview (== Projection x Modelview)
  A.multiply(&modelviewMatrix);

  // B = A^(-1)
  if(!A.invert(&B))
  {
    printf("ERROR: boUnProject(): Could not invert (Projection x Modelview)\n");
    return false;
  }

  // AB: we could calculate the inverse whenever camera changes!
  // --> less inverses to be calculated.

  GLint realy = viewport[3] - (GLint)pos.y() - 1;

  BoVector4Float v;
  BoVector4Float result;
  v.setX( ((GLfloat)((pos.x() - viewport[0]) * 2)) / viewport[2] - 1.0f);
  v.setY( ((GLfloat)((realy - viewport[1]) * 2)) / viewport[3] - 1.0f);
#if 0
  // mesa uses this
  v.setX( (pos.x() - viewport[0]) * 2 / viewport[2] - 1.0f);
  v.setY( (realy - viewport[1]) * 2 / viewport[3] - 1.0f);
#endif
  v.setZ(2 * depth - 1.0f);
  v.setW(1.0f);
  B.transform(&result, &v);

  if(result[3] == 0.0f)
  {
    printf("ERROR: boUnProject(): Can't divide by zero\n");
    return false;
  }

  ret->set(result[0] / result[3], result[1] / result[3], result[2] / result[3]);

  return true;
}

bool Bo3dTools::boUnProjectUseDepthBuffer(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const BoVector2Float& pos, BoVector3Float* ret)
{
  const GLint realy = viewport[3] - (GLint)pos.y() - 1;
  float depth;
  glReadPixels(((GLint)pos.x()), realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
  if (workaround_depth_value_enabled)
  {
    depth /= workaround_depth_value_1_0;
  }
  return boUnProject(modelviewMatrix, projectionMatrix, viewport, pos, ret, depth);
}

bool Bo3dTools::mapCoordinates(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const BoVector2Float& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ)
{
 return mapCoordinates(modelviewMatrix, projectionMatrix, viewport, pos, posX, posY, posZ, false);
}

bool Bo3dTools::mapCoordinatesUseDepthBuffer(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const BoVector2Float& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ)
{
 return mapCoordinates(modelviewMatrix, projectionMatrix, viewport, pos, posX, posY, posZ, true);
}

bool Bo3dTools::mapCoordinates(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const BoVector2Float& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth)
{
  PROFILE_METHOD
  GLint realy = viewport[3] - (GLint)pos.y() - 1;
  // we basically calculate a line here .. nearX/Y/Z is the starting point,
  // farX/Y/Z is the end point. From these points we can calculate a direction.
  // using this direction and the points nearX(Y)/farX(Y) you can build triangles
  // and then find the point that is on z=0.0
  GLdouble nearX, nearY, nearZ;
  GLdouble farX, farY, farZ;
  BoVector3Float near, far;
  if(!boUnProject(modelviewMatrix, projectionMatrix, viewport, pos, &near, 0.0f))
  {
    return false;
  }
  if(!boUnProject(modelviewMatrix, projectionMatrix, viewport, pos, &far, 1.0f))
  {
    return false;
  }
  nearX = near[0];
  nearY = near[1];
  nearZ = near[2];
  farX = far[0];
  farY = far[1];
  farZ = far[2];

  GLdouble zAtPoint = 0.0f;

  // AB: 0.0f is reached when we have a point that is outside the actual window!
  if (useRealDepth)
  {
    BoVector3Float v;
    if(!boUnProjectUseDepthBuffer(modelviewMatrix, projectionMatrix, viewport, pos, &v))
    {
      return false;
    }
    zAtPoint = v[2];
  }
  else
  {
    // assume we're using z = 0.0f
    zAtPoint = 0.0f;
  }

  // simple maths .. however it took me pretty much time to do this.. I haven't
  // done this for way too long time!
  GLdouble dist = (nearZ - zAtPoint); // distance from nearZ to our actual z. for z=0.0 this is equal to nearZ.
  GLdouble tanAlphaX = (nearX - farX) / (nearZ - farZ);
  *posX = (GLfloat)(nearX - tanAlphaX * dist);

  GLdouble tanAlphaY = (nearY - farY) / (nearZ - farZ);
  *posY = (GLfloat)(nearY - tanAlphaY * dist);

  *posZ = zAtPoint;
  return true;
}

bool Bo3dTools::mapDistance(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, int windx, int windy, GLfloat* dx, GLfloat* dy)
{
  PROFILE_METHOD
  GLfloat moveZ; // unused
  GLfloat moveX1, moveY1;
  GLfloat moveX2, moveY2;
  if(windx >= viewport[2]) // viewport[2] == width
  {
    printf("ERROR: mapDistance(): windx (%d) must be < %d\n", windx, viewport[2]);
    return false;
  }
  if(windy >= viewport[3]) // viewport[3] == height
  {
    printf("ERROR: mapDistance(): windy (%d) must be < %d\n", windy, viewport[3]);
    return false;
  }
  if(!mapCoordinates(modelviewMatrix, projectionMatrix, viewport,
              BoVector2Float(viewport[2] / 2 - windx / 2, viewport[3] / 2 - windy / 2),
              &moveX1, &moveY1, &moveZ))
  {
    printf("ERROR: mapDistance(): Cannot map coordinates\n");
    return false;
  }
  if(!mapCoordinates(modelviewMatrix, projectionMatrix, viewport,
              BoVector2Float(viewport[2] / 2 + windx / 2, viewport[3] / 2 + windy / 2),
              &moveX2, &moveY2, &moveZ))
  {
    printf("ERROR: mapDistance(): Cannot map coordinates\n");
    return false;
  }
  *dx = moveX2 - moveX1;
  *dy = moveY2 - moveY1;
  return true;
}


bool Bo3dTools::checkError(GLenum* error, QString* errorString, QString* errorName)
{
  bool ret = true;
  GLenum e = glGetError();
  QString s;
  switch(e)
  {
    case GL_INVALID_ENUM:
      s =  "GL_INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      s = "GL_INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      s = "GL_INVALID_OPERATION";
      break;
    case GL_STACK_OVERFLOW:
      s = "GL_STACK_OVERFLOW";
      break;
    case GL_STACK_UNDERFLOW:
      s = "GL_STACK_UNDERFLOW";
      break;
    case GL_OUT_OF_MEMORY:
      s = "GL_OUT_OF_MEMORY";
      break;
    case GL_NO_ERROR:
      ret = false;
      break;
    default:
      s = QString("Unknown OpenGL Error: %1").arg((int)e);
      break;
  }
  if(!s.isEmpty())
  {
    boError() << s << endl;
  }
  QString string;
  if(e != GL_NO_ERROR)
  {
    string = QString((char*)gluErrorString(e));
    boError() << "Error string: " << string << endl;
  }
  if(error)
  {
    *error = e;
  }
  if(errorString)
  {
    *errorString = string;
  }
  if(errorName)
  {
    *errorName = s;
  }
  return ret;
}

GLenum Bo3dTools::string2GLBlendFunc(const QString& str)
{
  if(str == "GL_SRC_ALPHA")
  {
    return GL_SRC_ALPHA;
  }
  else if(str == "GL_ONE_MINUS_SRC_ALPHA")
  {
    return GL_ONE_MINUS_SRC_ALPHA;
  }
  else if(str == "GL_ONE")
  {
    return GL_ONE;
  }
  else if(str == "GL_ZERO")
  {
    return GL_ZERO;
  }
  else if(str == "GL_DST_COLOR")
  {
    return GL_DST_COLOR;
  }
  else if(str == "GL_ONE_MINUS_DST_COLOR")
  {
    return GL_ONE_MINUS_DST_COLOR;
  }
  else if(str == "GL_DST_ALPHA")
  {
    return GL_DST_ALPHA;
  }
  else if(str == "GL_ONE_MINUS_DST_ALPHA")
  {
    return GL_ONE_MINUS_DST_ALPHA;
  }
  else if(str == "GL_SRC_ALPHA_SATURATE")
  {
    return GL_SRC_ALPHA_SATURATE;
  }
  else if(str == "GL_SRC_COLOR")
  {
    return GL_SRC_COLOR;
  }
  else if(str == "GL_ONE_MINUS_SRC_COLOR")
  {
    return GL_ONE_MINUS_SRC_COLOR;
  }
  else
  {
    // Invalid string was given
    return GL_INVALID_ENUM;
  }
}

void Bo3dTools::enableReadDepthBufferWorkaround(float _1_0_depthValue)
{
  workaround_depth_value_1_0 = _1_0_depthValue;
  workaround_depth_value_enabled = true;
}

void Bo3dTools::disableReadDepthBufferWorkaround()
{
  workaround_depth_value_enabled = false;
}


/*
 * vim:et sw=2
 */
