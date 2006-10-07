/*
    This file is part of the Boson game
    Copyright (C) 2005 Rivo Laks (rivolaks@hot.ee)

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


#include "transformer.h"

#include "debug.h"
#include "lod.h"
#include "mesh.h"
#include "frame.h"
#include "bo3dtools.h"

#include <qvaluelist.h>


Transformer::Transformer() : Processor()
{
  mModelSize = 1.0f;
  mCenterModel = false;
  setName("Transformer");
}

Transformer::~Transformer()
{
}

bool Transformer::process()
{
  if(lod() == 0)
  {
    boError() << k_funcinfo << "NULL LOD!" << endl;
    return false;
  }

  // Pass 1: transform matrices in nodes to get wanted model size
  if(!resizeModel())
  {
    return false;
  }

  // Pass 2: apply transformations (from frames) to all meshes
  if(!applyTransformations())
  {
    return false;
  }

  return true;
}

bool Transformer::applyTransformations()
{
  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    if(!applyTransformations(lod(), lod()->mesh(i)))
    {
      return false;
    }
  }

  return true;
}

bool Transformer::applyTransformations(LOD* lod, Mesh* mesh)
{
  if(!lod)
  {
    BO_NULL_ERROR(lod);
    return false;
  }
  if(!mesh)
  {
    BO_NULL_ERROR(mesh);
    return false;
  }
  // Find all frame nodes that have this mesh

  // All matrices (in all frames) corresponding to that mesh
  QValueList<BoMatrix*> matrices;
  // All matrices _in base frame_ corresponding to that mesh. These are used
  //  to calculate size for the mesh and to transform it.
  QValueList<BoMatrix*> baseframematrices;
  QValueList<unsigned int> baseframenodes;

  for(unsigned int i = 0; i < lod->frameCount(); i++)
  {
    Frame* f = lod->frame(i);
    for(unsigned int j = 0; j < f->nodeCount(); j++)
    {
      if(f->mesh(j) == mesh)
      {
        BoMatrix* m = f->matrix(j);
        if(!m)
        {
          boError() << k_funcinfo << "NULL matrix at node " << j << " in frame " << i << " for mesh " << mesh->name() << endl;
          return false;
        }
        if(m->hasNaN())
        {
          boError() << k_funcinfo << "matrix at node " << j << " in frame " << i << " for mesh " << mesh->name() << " has NaN entry!" << endl;
          return false;
        }
        matrices.append(m);
        if(i == baseFrame())
        {
          baseframematrices.append(m);
          baseframenodes.append(j);
        }
      }
    }
  }

  // There has to be at least one matrix (and node) per mesh
  if(matrices.count() == 0)
  {
    // FIXME: maybe even support this? Such meshes would just be removed later...
    boError() << k_funcinfo << "No matrices for mesh " << mesh->name() << endl;
    return false;
  }
  // If base frame doesn't contain any nodes using that mesh, then use first
  //  found node from some other frame.
  if(baseframematrices.count() == 0)
  {
    boError() << k_funcinfo << "No matrices for mesh " << mesh->name()
        << " found in base frame" << endl;
    return false;
  }

  // Find the base matrix that we'll use to transform the mesh
  BoMatrix* matrix = 0;
  if(baseframematrices.count() > 1)
  {
    // Find the matrix which transforms the mesh into maximum size.
    // We use simple approach here: we take the bounding box of the mesh,
    //  transform it with all matrices and then choose the matrix where
    //  resulting box had largest volume
    BoVector3Float min = mesh->vertex(0)->pos;
    BoVector3Float max = mesh->vertex(0)->pos;
    for(unsigned int i = 1; i < mesh->vertexCount(); i++)
    {
      const BoVector3Float& pos = mesh->vertex(i)->pos;
      min.setX(QMIN(min.x(), pos.x()));
      min.setY(QMIN(min.y(), pos.y()));
      min.setZ(QMIN(min.z(), pos.z()));
      max.setX(QMAX(max.x(), pos.x()));
      max.setY(QMAX(max.y(), pos.y()));
      max.setZ(QMAX(max.z(), pos.z()));
    }

    float maxvolume = -1.0;
    unsigned int maxi = 0;
    QValueList<BoMatrix*>::Iterator it = baseframematrices.begin();
    for(unsigned int i = 0; it != baseframematrices.end(); ++it, i++)
    {
      float vol = transformedBBoxVolume(min, max, *it);
      if(vol > maxvolume)
      {
        matrix = *it;
        maxvolume = vol;
        maxi = i;
      }
    }
    mesh->setBaseNode(*baseframenodes.at(maxi));
  }
  else
  {
    matrix = baseframematrices.first();
    mesh->setBaseNode(baseframenodes.first());
  }

  if(!matrix)
  {
    boError() << k_funcinfo << "NULL matrix" << endl;
    return false;
  }

  if(matrix->isIdentity())
  {
    // Nothing to do
    return true;
  }

  // Transform the mesh
  boDebug() << k_funcinfo << "Transforming mesh..." << endl;
  for(unsigned int i = 0; i < mesh->vertexCount(); i++)
  {
    Vertex* v = mesh->vertex(i);
    // Transform only the position
    BoVector3Float newpos;
    matrix->transform(&newpos, &v->pos);
    v->pos = newpos;
  }

  if(matrices.count() == 1)
  {
    matrix->loadIdentity();
    return true;
  }

  // We now need to multiply all nodes using the same mesh with matrix's
  //  inverse
  BoMatrix imatrix;
  if(!matrix->invert(&imatrix))
  {
    boError() << k_funcinfo << "Couldn't invert matrix!" << endl;
    return false;
  }

  for(QValueList<BoMatrix*>::Iterator it = matrices.begin(); it != matrices.end(); ++it)
  {
    (*it)->multiply(&imatrix);
  }

  // Set the used matrix to identity matrix. This is more accurate and lets us
  //  do simple matrix->isIdentity() checks later if necessary.
  matrix->loadIdentity();

  return true;
}

float Transformer::transformedBBoxVolume(const BoVector3Float& origmin, const BoVector3Float& origmax, BoMatrix* matrix)
{
  // Create 8 vertices of the bounding box
  QValueList<BoVector3Float> vertices;
  vertices.append(BoVector3Float(origmin.x(), origmin.y(), origmin.z()));
  vertices.append(BoVector3Float(origmin.x(), origmin.y(), origmax.z()));
  vertices.append(BoVector3Float(origmin.x(), origmax.y(), origmin.z()));
  vertices.append(BoVector3Float(origmin.x(), origmax.y(), origmax.z()));
  vertices.append(BoVector3Float(origmax.x(), origmin.y(), origmin.z()));
  vertices.append(BoVector3Float(origmax.x(), origmin.y(), origmax.z()));
  vertices.append(BoVector3Float(origmax.x(), origmax.y(), origmin.z()));
  vertices.append(BoVector3Float(origmax.x(), origmax.y(), origmax.z()));

  // Min/max coordinates of a transformed bounding box
  BoVector3Float min, max;
  matrix->transform(&min, &vertices.first());
  max = min;

  // Tranform the vertices and find new min/max coords
  QValueList<BoVector3Float>::Iterator it = vertices.begin();
  for(; it != vertices.end(); ++it)
  {
    BoVector3Float v;
    matrix->transform(&v, &(*it));
    min.setX(QMIN(min.x(), v.x()));
    min.setY(QMIN(min.y(), v.y()));
    min.setZ(QMIN(min.z(), v.z()));
    max.setX(QMAX(max.x(), v.x()));
    max.setY(QMAX(max.y(), v.y()));
    max.setZ(QMAX(max.z(), v.z()));
  }

  float ret = fabsf((max.x() - min.x()) * (max.y() - min.y()) * (max.z() - min.z()));
  if(isnan(ret))
  {
    boError() << k_funcinfo << "calculated volume is not a number" << endl;
    ret = 0.0f;
  }
  return ret;
}

#if 0
bool Transformer::resizeModel()
{
  boDebug() << k_funcinfo << "Resizing model..." << endl;
  // Find min/max coords of the model
  Frame* f = lod()->frame(baseFrame());
  BoVector3Float min = f->mesh(0)->vertex(0)->pos;
  BoVector3Float max = f->mesh(0)->vertex(0)->pos;

  for(unsigned int i = 0; i < f->nodeCount(); i++)
  {
    Mesh* mesh = f->mesh(i);
    BoMatrix* matrix = f->matrix(i);
    for(unsigned int j = 0; j < mesh->vertexCount(); j++)
    {
      BoVector3Float pos;
      matrix->transform(&pos, &mesh->vertex(j)->pos);

      min.setX(QMIN(min.x(), pos.x()));
      min.setY(QMIN(min.y(), pos.y()));
      min.setZ(QMIN(min.z(), pos.z()));
      max.setX(QMAX(max.x(), pos.x()));
      max.setY(QMAX(max.y(), pos.y()));
      max.setZ(QMAX(max.z(), pos.z()));
    }
  }


  // How much we need to scale the model
  float scale;
  if((max.x() - min.x()) > (max.y() - min.y()))
  {
    // Model's width is bigger than height
    scale = modelSize() / (max.x() - min.x());
  }
  else
  {
    // Model's height is bigger than width
    scale = modelSize() / (max.y() - min.y());
  }
  // We want model's center point to be at (0; 0)
  BoVector3Float translate;
  translate.setX(-(modelSize() / 2) - (min.x() * scale));
  translate.setY(-(modelSize() / 2) - (min.y() * scale));
  // We also want model to be exactly _on ground_
  translate.setZ(-(min.z() * scale));


  // Scale and translate all vertices in all meshes
  for(unsigned int i = 0; i < lod()->meshCount(); i++)
  {
    Mesh* mesh = lod()->mesh(i);
    for(unsigned int j = 0; j < mesh->vertexCount(); j++)
    {
      // Scale ...
      mesh->vertex(j)->pos.scale(scale);
      // ... and translate
      mesh->vertex(j)->pos += translate;
    }
  }

  return true;
}
#endif

bool Transformer::resizeModel()
{
  boDebug() << k_funcinfo << "Resizing model..." << endl;
  // Find min/max coords of the model
  Frame* f = lod()->frame(baseFrame());
  // Initial coord
  BoVector3Float min;
  f->matrix(0)->transform(&min, &f->mesh(0)->vertex(0)->pos);
  BoVector3Float max = min;

  for(unsigned int i = 0; i < f->nodeCount(); i++)
  {
    Mesh* mesh = f->mesh(i);
    BoMatrix* matrix = f->matrix(i);
    for(unsigned int j = 0; j < mesh->vertexCount(); j++)
    {
      BoVector3Float pos;
      matrix->transform(&pos, &mesh->vertex(j)->pos);

      min.setX(QMIN(min.x(), pos.x()));
      min.setY(QMIN(min.y(), pos.y()));
      min.setZ(QMIN(min.z(), pos.z()));
      max.setX(QMAX(max.x(), pos.x()));
      max.setY(QMAX(max.y(), pos.y()));
      max.setZ(QMAX(max.z(), pos.z()));
    }
  }


  // How much we need to scale the model
  float scale;
  if((max.x() - min.x()) > (max.y() - min.y()))
  {
    // Model's width is bigger than height
    scale = modelSize() / (max.x() - min.x());
  }
  else
  {
    // Model's height is bigger than width
    scale = modelSize() / (max.y() - min.y());
  }
  if(isnan(scale))
  {
    boError() << k_funcinfo << "value of scale is not a number" << endl;
    return false;
  }
  if(!isfinite(scale))
  {
    boError() << k_funcinfo << "infinite value for scale" << endl;
    return false;
  }
  BoVector3Float translate;
  if(centerModel())
  {
    // We want model's center point to be at (0; 0)
    translate.setX(-(modelSize() / 2) - (min.x() * scale));
    translate.setY(-(modelSize() / 2) - (min.y() * scale));
    // We also want model to be exactly _on ground_
    translate.setZ(-(min.z() * scale));
  }


  BoMatrix transformmatrix;
  transformmatrix.translate(translate);
  transformmatrix.scale(scale, scale, scale);
  // Modify matrices in all nodes and in all frames
  for(unsigned int i = 0; i < lod()->frameCount(); i++)
  {
    Frame* f2 = lod()->frame(i);
    for(unsigned int i = 0; i < f2->nodeCount(); i++)
    {
      BoMatrix* matrix = f2->matrix(i);
      BoMatrix tmp(transformmatrix);
      tmp.multiply(matrix);
      if(tmp.hasNaN())
      {
        boError() << k_funcinfo << "calculated matrix has NaN entries" << endl;
        return false;
      }
      matrix->loadMatrix(tmp);
    }
  }

  return true;
}

/*
 * vim: et sw=2
 */
